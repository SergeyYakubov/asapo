#ifndef ASAPO_INOTIFY_EVENT_H
#define ASAPO_INOTIFY_EVENT_H

#include <sys/inotify.h>
#include <map>
#include <string>

#include "asapo/common/error.h"
#include "common.h"


namespace asapo {

class InotifyEvent {
  public:
    InotifyEvent(const struct inotify_event* inotify_event, const std::map<int, std::string>& watched_folders_paths);
    uint32_t Length() const ;
    bool IsDirectoryEvent() const ;
    bool IsNewFileInFolderEvent() const;
    bool IsNewDirectoryInFolderEvent() const;
    bool IsDeleteDirectoryInFolderEvent() const;
    bool IsDeleteDirectoryInFolderEventByMove() const;
    int Descriptor() const ;
    const char* Name() const ;
    uint32_t GetMask() const;
    void Print() const;
  private:
    const struct inotify_event* inotify_event_;
    const std::map<int, std::string>& watched_folders_paths_;
};

}

#endif //ASAPO_INOTIFY_EVENT_H
