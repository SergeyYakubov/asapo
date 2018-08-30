#include "inotify_event.h"

namespace asapo {

InotifyEvent::InotifyEvent(const struct inotify_event* inotify_event,
                           const std::map<int, std::string>& watched_folders_paths):
    inotify_event_{inotify_event}, watched_folders_paths_{watched_folders_paths} {

}

uint32_t InotifyEvent::Length() const {
    return sizeof(struct inotify_event) + inotify_event_->len;
};

bool InotifyEvent::IsDirectoryEvent() const {
    return inotify_event_->mask & IN_ISDIR || inotify_event_->mask & IN_DELETE_SELF;
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
bool InotifyEvent::IsNewFileInFolderEvent() const {
    return !IsDirectoryEvent() && ((GetMask() & IN_CLOSE_WRITE) || (GetMask() & IN_MOVED_TO));
}
bool InotifyEvent::IsNewDirectoryInFolderEvent() const {
    return IsDirectoryEvent() && ((GetMask() & IN_CREATE) || (GetMask() & IN_MOVED_TO));
}
bool InotifyEvent::IsDeleteDirectoryInFolderEvent() const {
    return IsDirectoryEvent() && ((GetMask() & IN_DELETE_SELF) || (GetMask() & IN_MOVED_FROM));
}
uint32_t InotifyEvent::NameLength() const {
    return inotify_event_->len;
}
bool InotifyEvent::IsDeleteDirectoryInFolderEventByMove() const {
    return IsDeleteDirectoryInFolderEvent() && (GetMask() & IN_MOVED_FROM);
}

}