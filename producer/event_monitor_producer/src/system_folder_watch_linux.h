#ifndef ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
#define ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H

#include <vector>
#include <string>
#include <map>

#include "common/error.h"
#include "preprocessor/definitions.h"
#include "asapo_producer.h"
#include "common.h"
#include "io/io.h"
#include "io/io_factory.h"
#include <sys/inotify.h>
#include <unistd.h>
#include "inotify_event.h"

namespace asapo {


const uint64_t kBufLen  = 2000 * (sizeof(struct inotify_event) + FILENAME_MAX + 1);
const uint32_t kInotifyWatchFlags  = IN_CLOSE_WRITE |
                                     IN_MOVED_TO    |
                                     IN_MOVED_FROM  |
                                     IN_CREATE      |
                                     IN_DELETE_SELF |
                                     IN_EXCL_UNLINK |
                                     IN_DONT_FOLLOW |
                                     IN_ONLYDIR;



class SystemFolderWatch {
  public:
    VIRTUAL Error StartFolderMonitor(const std::string& root_folder, const std::vector<std::string>& monitored_folders);
    VIRTUAL FileEvents GetFileEventList(Error* err);
  private:
    Error AddFolderAndSubfoldersToWatch(std::string folder);
    Error AddFolderToWatch(std::string folder);
    std::unique_ptr<IO> io_{GenerateDefaultIO()};
    Error ProcessInotifyEvent(const InotifyEvent& event, FileEvents* file_events);
  private:
    char buffer[kBufLen]  __attribute__ ((aligned(8)));
    std::map<int, std::string> watched_folders_paths_;
    int watch_fd_ = -1;
    Error ReadInotifyEvents(int* bytes_read);
    Error ProcessInotifyEvents(int bytes_in_buffer, FileEvents* events);
    Error FindEventFolder(const InotifyEvent& event, std::string* folder);
    std::string root_folder_;
};

bool DirectoryEvent(const InotifyEvent& event);

}



#endif //ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
