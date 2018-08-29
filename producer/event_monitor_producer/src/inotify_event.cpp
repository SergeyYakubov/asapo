#include "inotify_event.h"

namespace asapo {

InotifyEvent::InotifyEvent(const struct inotify_event* inotify_event,
                           const std::map<int, std::string>& watched_folders_paths):
    inotify_event_{inotify_event}, watched_folders_paths_{watched_folders_paths} {

}

uint32_t InotifyEvent::Length() const {
    return sizeof(struct inotify_event) + inotify_event_->len;
};

void InotifyEvent::Print() const {
    printf("    wd =%2d; ", inotify_event_->wd);
    if (inotify_event_->cookie > 0)
        printf("cookie =%4d; ", inotify_event_->cookie);

    printf("mask = ");
    if (inotify_event_->mask & IN_ACCESS)        printf("IN_ACCESS ");
    if (inotify_event_->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
    if (inotify_event_->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
    if (inotify_event_->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
    if (inotify_event_->mask & IN_CREATE)        printf("IN_CREATE ");
    if (inotify_event_->mask & IN_DELETE)        printf("IN_DELETE ");
    if (inotify_event_->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
    if (inotify_event_->mask & IN_IGNORED)       printf("IN_IGNORED ");
    if (inotify_event_->mask & IN_ISDIR)         printf("IN_ISDIR ");
    if (inotify_event_->mask & IN_MODIFY)        printf("IN_MODIFY ");
    if (inotify_event_->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
    if (inotify_event_->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
    if (inotify_event_->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
    if (inotify_event_->mask & IN_OPEN)          printf("IN_OPEN ");
    if (inotify_event_->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
    if (inotify_event_->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
    printf("\n");

    if (inotify_event_->len > 0)
        printf("        name = %s\n", inotify_event_->name);
}

bool InotifyEvent::IsDirectoryEvent() const {
    return inotify_event_->mask & IN_ISDIR;
}
int InotifyEvent::Descriptor() const {
    return inotify_event_->wd;
}
const char* InotifyEvent::Name() const {
    return inotify_event_->name;
}
uint32_t InotifyEvent::GetMask() const  {
    return inotify_event_->mask;
}

}