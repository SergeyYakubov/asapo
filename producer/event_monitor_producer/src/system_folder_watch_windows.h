#ifndef ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H
#define ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H

#include <vector>
#include <string>

#include "common/error.h"
#include "preprocessor/definitions.h"
#include "asapo_producer.h"
#include "common.h"


namespace asapo {

class SystemFolderWatch {
  Error StartFolderMonitor(const std::vector<std::string>& monitored_folders);
  FileEvents GetFileEventList(Error* err);

};

}

#endif //ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H
