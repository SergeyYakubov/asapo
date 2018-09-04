#ifndef ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H
#define ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H

#include <vector>
#include <string>

#include "common/error.h"
#include "preprocessor/definitions.h"
#include "asapo_producer.h"
#include "common.h"
#include "io/io.h"


namespace asapo {

class SystemFolderWatch {
  public:
  SystemFolderWatch();
  VIRTUAL Error StartFolderMonitor(const std::string& root_folder,
                                     const std::vector<std::string>& monitored_folders);
    VIRTUAL FilesToSend GetFileList(Error* err);
    std::unique_ptr<IO> io__;
  private:

};

}

#endif //ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H
