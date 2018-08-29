#include "system_folder_watch_linux.h"


#include "event_monitor_error.h"
#include "eventmon_logger.h"

namespace asapo {

Error SystemFolderWatch::AddFolderToWatch(std::string folder) {
    int id = inotify_add_watch(watch_fd_, folder.c_str(), kInotifyWatchFlags);
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
    auto subdirs = io_-> GetSubDirectories(folder, &err);
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
    watch_fd_ = inotify_init();
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


Error SystemFolderWatch::FindEventFolder(const InotifyEvent& event, std::string* folder) {
    auto it = watched_folders_paths_.find(event.Descriptor());
    if (it == watched_folders_paths_.end()) {
        return EventMonitorErrorTemplates::kSystemError.Generate("cannot find monitored folder for wd number " + std::to_string(
                    event.Descriptor()));
    }
    *folder = root_folder_ + "/" + it->second + "/" + event.Name();
    return nullptr;
}



Error SystemFolderWatch::ProcessInotifyEvent(const InotifyEvent& event, FileEvents* file_events) {
    event.Print();

    if (event.IsDirectoryEvent() && ((event.GetMask() & IN_CREATE) || (event.GetMask() & IN_MOVED_TO))) {
        std::string newpath;
        auto err = FindEventFolder(event, &newpath);
        if (err) {
            return err;
        }
        err = AddFolderAndSubfoldersToWatch(newpath);
        if (err) {
            return err;
        }
    }

    if ((event.GetMask() & IN_DELETE_SELF) || ((event.GetMask() & IN_ISDIR) && ((event.GetMask() & IN_MOVED_FROM)))) {
        auto it = watched_folders_paths_.find(event.Descriptor());
        if (it == watched_folders_paths_.end()) {
            return EventMonitorErrorTemplates::kSystemError.Generate("cannot find monitored folder to delete " + std::to_string(
                        event.Descriptor()));
        }
        std::string oldpath = it->second;
        if (event.GetMask() & IN_MOVED_FROM) {
            oldpath += std::string("/") + event.Name();
            for (auto val = watched_folders_paths_.begin(); val != watched_folders_paths_.end();) {
                if ((oldpath.size() <= val->second.size()) && std::equal(oldpath.begin(), oldpath.end(), val->second.begin())) {
                    inotify_rm_watch(val->first, watch_fd_);
                    GetDefaultEventMonLogger()->Debug("removed folder from monitor: " + val->second);
                    val = watched_folders_paths_.erase(val);
                } else {
                    ++val;
                }

            }

        } else {
            inotify_rm_watch(it->first, watch_fd_);
            watched_folders_paths_.erase(it);
            GetDefaultEventMonLogger()->Debug("removed folder from monitor: " + oldpath);
        }
    }
    if (!(event.GetMask() & IN_ISDIR)) {
        if ((event.GetMask() & IN_CLOSE_WRITE) || (event.GetMask() & IN_MOVED_TO)) {
            auto it = watched_folders_paths_.find(event.Descriptor());
            if (it == watched_folders_paths_.end()) {
                return EventMonitorErrorTemplates::kSystemError.Generate("cannot find monitored folder for file " + std::to_string(
                            event.Descriptor()));
            }
            std::string fname = it->second + "/" + event.Name();
            FileEvent file_event;
            file_event.type = (event.GetMask() & IN_CLOSE_WRITE) ? EventType::closed : EventType::renamed_to;
            file_event.name = fname;
            file_events->emplace_back(std::move(file_event));
            GetDefaultEventMonLogger()->Debug((event.GetMask() & IN_CLOSE_WRITE) ? "file closed: " : "file moved: " + fname);
        }
    }



    return nullptr;
}

Error SystemFolderWatch::ReadInotifyEvents(int* bytes_read) {
    *bytes_read = read(watch_fd_, buffer, sizeof(buffer));
    if (*bytes_read < 0) {
        return EventMonitorErrorTemplates::kSystemError.Generate("read from inotify fd");
    }
    return nullptr;
}

Error SystemFolderWatch::ProcessInotifyEvents(int bytes_in_buffer, FileEvents* events) {
    int nerrors = 0;
    int nevents = 0;
    for (char* p = buffer; p < buffer + bytes_in_buffer; ) {
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


FileEvents SystemFolderWatch::GetFileEventList(Error* err) {
    int bytes_read;
    *err = ReadInotifyEvents(&bytes_read);
    if (*err) {
        return {};
    }

    FileEvents events;
    *err = ProcessInotifyEvents(bytes_read, &events);
    if (*err) {
        return {};
    }
    return events;
}

}