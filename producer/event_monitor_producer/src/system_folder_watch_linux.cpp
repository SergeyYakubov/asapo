#include "system_folder_watch_linux.h"


#include "event_monitor_error.h"
#include "eventmon_logger.h"

namespace asapo {

Error SystemFolderWatch::AddFolderToWatch(std::string folder, bool recursive) {
    int id = inotify_add_watch(watch_fd_, folder.c_str(),
                               IN_CLOSE_WRITE
                               | IN_MOVED_TO
                               | IN_MOVED_FROM
                               | IN_CREATE
                               | IN_DELETE_SELF
//                               | IN_MOVE_SELF
                               | IN_EXCL_UNLINK
                               | IN_DONT_FOLLOW
                               | IN_ONLYDIR);
    if (id == -1) {
        return EventMonitorErrorTemplates::kSystemError.Generate("cannot add watch for " + folder);
    } else {
        GetDefaultEventMonLogger()->Debug("added folder to monitor: " + folder);
    }
    std::string relative_path = folder;
    relative_path.erase(0, root_folder_.size() + 1);
    watched_folders_paths_[id] = relative_path;
    if (recursive) {
        Error err;
        auto subdirs = io_-> GetSubDirectories(folder, &err);
        if (err) {
            return err;
        }
        for (auto& subdir : subdirs) {
            err = AddFolderToWatch(subdir, false);
            if (err) {
                return err;
            }
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
        auto err = AddFolderToWatch(root_folder_ + "/" + folder, true);
        if (err) {
            return EventMonitorErrorTemplates::kSystemError.Generate("cannot initialize inotify: " + err->Explain());
        }
    }
    return nullptr;
}

Error SystemFolderWatch::ProcessInotifyEvent(struct inotify_event* i, FileEvents* events) {

    printf("    wd =%2d; ", i->wd);
    if (i->cookie > 0)
        printf("cookie =%4d; ", i->cookie);

    printf("mask = ");
    if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
    if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
    if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
    if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
    if (i->mask & IN_CREATE)        printf("IN_CREATE ");
    if (i->mask & IN_DELETE)        printf("IN_DELETE ");
    if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
    if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
    if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
    if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
    if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
    if (i->mask & IN_OPEN)          printf("IN_OPEN ");
    if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
    if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
    printf("\n");

    if (i->len > 0)
        printf("        name = %s\n", i->name);


    if ((i->mask & IN_ISDIR) && ((i->mask & IN_CREATE) || (i->mask & IN_MOVED_TO))) {
        auto it = watched_folders_paths_.find(i->wd);
        if (it == watched_folders_paths_.end()) {
            return EventMonitorErrorTemplates::kSystemError.Generate("cannot find monitored folder to create in " + std::to_string(
                        i->wd));
        }

        std::string newpath = root_folder_ + "/" + it->second + "/" + i->name;
        auto err = AddFolderToWatch(newpath, true);
        if (err) {
            return err;
        }
    }

    if ((i->mask & IN_DELETE_SELF) || ((i->mask & IN_ISDIR) && ((i->mask & IN_MOVED_FROM)))) {
        auto it = watched_folders_paths_.find(i->wd);
        if (it == watched_folders_paths_.end()) {
            return EventMonitorErrorTemplates::kSystemError.Generate("cannot find monitored folder to delete " + std::to_string(
                        i->wd));
        }
        std::string oldpath = it->second;
        if (i->mask & IN_MOVED_FROM) {
            oldpath += std::string("/") + i->name;
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
    if (!(i->mask & IN_ISDIR)) {
        if ((i->mask & IN_CLOSE_WRITE) || (i->mask & IN_MOVED_TO)) {
            auto it = watched_folders_paths_.find(i->wd);
            if (it == watched_folders_paths_.end()) {
                return EventMonitorErrorTemplates::kSystemError.Generate("cannot find monitored folder for file " + std::to_string(
                            i->wd));
            }
            std::string fname = it->second + "/" + i->name;
            FileEvent event;
            event.type = (i->mask & IN_CLOSE_WRITE) ? EventType::closed : EventType::renamed_to;
            event.name = fname;
            events->emplace_back(std::move(event));
            GetDefaultEventMonLogger()->Debug((i->mask & IN_CLOSE_WRITE) ? "file closed: " : "file moved: " + fname);
        }
    }



    return nullptr;
}


FileEvents SystemFolderWatch::GetFileEventList(Error* err) {
    FileEvents events;

    char buffer[kBufLen]  __attribute__ ((aligned(8)));

    int numRead = read(watch_fd_, buffer, sizeof(buffer));
    if (numRead == 0) {
        *err = TextError("readfrom inotify fd returned 0!");
        printf("mask = ");

        return events;
    }

    if (numRead == -1) {
        *err = TextError("read from inotify fd returned -1!");
        return events;
    }

    int nerrors = 0;
    for (char* p = buffer; p < buffer + numRead; ) {
        struct inotify_event* event = (struct inotify_event*) p;
        *err = ProcessInotifyEvent(event, &events);
        if (*err) {
            GetDefaultEventMonLogger()->Error("error processing inotify event: " + (*err)->Explain());
            nerrors++;
        }
        p += sizeof(struct inotify_event) + event->len;
    }

    if (nerrors == 0) {
        *err = nullptr;
    } else {
        *err = TextError("There were " + std::to_string(nerrors) + " error(s) while processing event");
    }

    return events;
}

}