#include "system_folder_watch_linux.h"


#include "event_monitor_error.h"
#include "eventmon_logger.h"
#include "io/io_factory.h"

namespace asapo {

Error SystemFolderWatch::AddFolderToWatch(std::string folder) {
    int id = inotify__->AddWatch(watch_fd_, folder.c_str(), kInotifyWatchFlags);
    if (id == -1) {
        return EventMonitorErrorTemplates::kSystemError.Generate("cannot add watch for " + folder);
    }
    GetDefaultEventMonLogger()->Debug("added folder to monitor: " + folder);
    std::string relative_path = folder;
    relative_path.erase(0, root_folder_.size() + 1);
    watched_folders_paths_[id] = relative_path;
    return nullptr;
}

Error SystemFolderWatch::AddFolderAndSubfoldersToWatch(std::string folder) {
    auto err = AddFolderToWatch(folder);
    if (err) {
        return err;
    }
    auto subdirs = io__-> GetSubDirectories(folder, &err);
    if (err) {
        return err;
    }
    for (auto& subdir : subdirs) {
        err = AddFolderToWatch(subdir);
        if (err) {
            return err;
        }
    }

    return nullptr;
}


Error SystemFolderWatch::StartFolderMonitor(const std::string& root_folder,
                                            const std::vector<std::string>& monitored_folders) {
    watch_fd_ = inotify__->Init();
    if (watch_fd_ == -1) {
        return EventMonitorErrorTemplates::kSystemError.Generate("cannot initialize inotify");
    }
    root_folder_ = root_folder;
    for (auto& folder : monitored_folders) {
        auto err = AddFolderAndSubfoldersToWatch(root_folder_ + "/" + folder);
        if (err) {
            return EventMonitorErrorTemplates::kSystemError.Generate("cannot initialize inotify: " + err->Explain());
        }
    }
    return nullptr;
}

std::map<int, std::string>::iterator SystemFolderWatch::FindEventIterator(const InotifyEvent& event, Error* err) {
    auto it = watched_folders_paths_.find(event.Descriptor());
    *err = nullptr;
    if (it == watched_folders_paths_.end()) {
        *err = EventMonitorErrorTemplates::kSystemError.Generate("cannot find monitored folder for wd number " + std::to_string(
                    event.Descriptor()));
    }
    return it;
}



Error SystemFolderWatch::FindEventPath(const InotifyEvent& event, std::string* folder, bool add_root) {
    Error err;
    auto it = FindEventIterator(event, &err);
    if (err) {
        return err;
    }
    if (add_root) {
        *folder = root_folder_ + "/" + it->second + "/" + event.Name();
    } else {
        *folder = it->second + "/" + event.Name();
    }

    return nullptr;
}

Error SystemFolderWatch::ProcessFileEvent(const InotifyEvent& event, FilesToSend* files) {
    if (!event.IsNewFileInFolderEvent()) {
        return nullptr;
    }
    std::string fname;
    auto err = FindEventPath(event, &fname, false);
    if (err) {
        return err;
    }
    files->emplace_back(std::move(fname));
    GetDefaultEventMonLogger()->Debug((event.GetMask() & IN_CLOSE_WRITE) ? "file closed: " : "file moved: " + fname);
    return nullptr;
}

Error SystemFolderWatch::ProcessNewDirectoryInFolderEvent(const InotifyEvent& event) {
    std::string newpath;
    auto err = FindEventPath(event, &newpath, true);
    if (err) {
        return err;
    }
    return  AddFolderAndSubfoldersToWatch(newpath);
}

std::map<int, std::string>::iterator SystemFolderWatch::RemoveFolderFromWatch(const
        std::map<int, std::string>::iterator& it) {
    inotify__->DeleteWatch(it->first, watch_fd_);
    GetDefaultEventMonLogger()->Debug("removed folder from monitor: " + it->second);
    return watched_folders_paths_.erase(it);
}

void SystemFolderWatch::RemoveFolderWithSubfoldersFromWatch(const std::string& path) {
    for (auto val = watched_folders_paths_.begin(); val != watched_folders_paths_.end();) {
        if ((path.size() <= val->second.size()) && std::equal(path.begin(), path.end(), val->second.begin())) {
            val = RemoveFolderFromWatch(val);
        } else {
            ++val;
        }
    }
}

Error SystemFolderWatch::ProcessDeleteDirectoryInFolderEvent(const InotifyEvent& event) {
    Error
    err;
    auto it = FindEventIterator(event, &err);
    if (err) {
        return err;
    }

    if (event.IsDeleteDirectoryInFolderEventByMove()) {
        std::string path = it->second + std::string("/") + event.Name();
        RemoveFolderWithSubfoldersFromWatch(path);
    } else {
        RemoveFolderFromWatch(it);
    }

    return nullptr;
}


Error SystemFolderWatch::ProcessDirectoryEvent(const InotifyEvent& event) {
    if (event.IsNewDirectoryInFolderEvent()) {
        return ProcessNewDirectoryInFolderEvent(event);
    }

    if (event.IsDeleteDirectoryInFolderEvent()) {
        return ProcessDeleteDirectoryInFolderEvent(event);
    }

    return nullptr;
}

Error SystemFolderWatch::ProcessInotifyEvent(const InotifyEvent& event, FilesToSend* file_events) {
    if (event.IsDirectoryEvent()) {
        return ProcessDirectoryEvent(event);
    } else {
        return ProcessFileEvent(event, file_events);
    }

}

Error SystemFolderWatch::ReadInotifyEvents(int* bytes_read) {
    *bytes_read = inotify__->Read(watch_fd_, buffer_.get(), kBufLen);
    if (*bytes_read < 0) {
        return EventMonitorErrorTemplates::kSystemError.Generate("read from inotify fd");
    }
    return nullptr;
}

Error SystemFolderWatch::ProcessInotifyEvents(int bytes_in_buffer_, FilesToSend* events) {
    int nerrors = 0;
    int nevents = 0;
    for (char* p = buffer_.get(); p < buffer_.get() + bytes_in_buffer_; ) {
        InotifyEvent event{(struct inotify_event*) p, watched_folders_paths_};
        auto err = ProcessInotifyEvent(event, events);
        if (err) {
            GetDefaultEventMonLogger()->Error("error processing inotify event: " + err->Explain());
            nerrors++;
        }
        p += event.Length();
        nevents++;
    }

    if (nerrors < nevents) {
        return nullptr;
    } else {
        return EventMonitorErrorTemplates::kSystemError.Generate("error processing inotify events");
    }
}


FilesToSend SystemFolderWatch::GetFileList(Error* err) {
    int bytes_read;
    *err = ReadInotifyEvents(&bytes_read);
    if (*err) {
        return {};
    }

    FilesToSend events;
    *err = ProcessInotifyEvents(bytes_read, &events);
    if (*err) {
        return {};
    }
    return events;
}

SystemFolderWatch::SystemFolderWatch() : io__{GenerateDefaultIO()}, inotify__{new Inotify()}, buffer_{new char[kBufLen]} {
}

}