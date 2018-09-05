#ifndef ASAPO_SINGLE_FOLDER_MONITOR_H
#define ASAPO_SINGLE_FOLDER_MONITOR_H

#include <string>

#include "watch_io.h"
#include "logger/logger.h"

namespace asapo {

class SingleFolderWatch {
  public:
    explicit SingleFolderWatch(std::string root_folder,std::string folder);
    void Watch();
    std::unique_ptr<WatchIO> watch_io__;
    const AbstractLogger* log__;
 private:
  std::string root_folder_;
  std::string folder_;
  Error Init();
  HANDLE handle_;
};

}

#endif //ASAPO_SINGLE_FOLDER_MONITOR_H
