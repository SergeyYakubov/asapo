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

namespace asapo {

class SystemFolderWatch {
  public:
    VIRTUAL Error StartFolderMonitor(const std::string& root_folder, const std::vector<std::string>& monitored_folders);
    VIRTUAL FileEvents GetFileEventList(Error* err);
  private:
    Error AddFolderToWatch(std::string folder, bool recursive);
    std::unique_ptr<IO> io_{GenerateDefaultIO()};
    Error ProcessInotifyEvent(struct inotify_event* i, FileEvents* events);
  private:
    static const uint64_t kBufLen  = 2000 * (sizeof(struct inotify_event) + FILENAME_MAX + 1);
    std::map<int, std::string> watched_folders_paths_;
    int watch_fd_ = -1;
    std::string root_folder_;
};

}



#endif //ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
