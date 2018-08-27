#ifndef ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
#define ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H

#include <vector>
#include <string>

#include "common/error.h"
#include "preprocessor/definitions.h"
#include "asapo_producer.h"
#include "common.h"

namespace asapo {

class SystemFolderWatch {
  public:
    VIRTUAL Error StartFolderMonitor(const std::vector<std::string>& monitored_folders);
    VIRTUAL FileEvents GetFileEventList(Error* err);
  private:
};

}

#endif //ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
