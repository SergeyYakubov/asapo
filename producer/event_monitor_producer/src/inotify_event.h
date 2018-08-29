#ifndef ASAPO_INOTIFY_EVENT_H
#define ASAPO_INOTIFY_EVENT_H

#include <sys/inotify.h>
#include <map>
#include <string>

#include "common/error.h"
#include "common.h"


namespace asapo {

class InotifyEvent {
  public:
    InotifyEvent(const struct inotify_event* inotify_event, const std::map<int, std::string>& watched_folders_paths);
    uint32_t Length() const ;
    void Print() const;
    bool IsDirectoryEvent() const ;
    int Descriptor() const ;
    const char* Name() const ;
    uint32_t GetMask() const;
  private:
    const struct inotify_event* inotify_event_;
    const std::map<int, std::string> watched_folders_paths_;
};

}

#endif //ASAPO_INOTIFY_EVENT_H
