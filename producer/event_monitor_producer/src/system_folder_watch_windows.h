#ifndef ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H
#define ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H

#include <vector>
#include <string>

#include "asapo/common/error.h"
#include "asapo/preprocessor/definitions.h"
#include "asapo/asapo_producer.h"
#include "common.h"
#include "asapo/io/io.h"
#include "shared_event_list.h"

namespace asapo {



class SystemFolderWatch {
  public:
    SystemFolderWatch();
    ASAPO_VIRTUAL Error StartFolderMonitor(const std::string& root_folder,
                                     const std::vector<std::string>& monitored_folders);
    ASAPO_VIRTUAL FilesToSend GetFileList(Error* err);
    std::unique_ptr<IO> io__;
  private:
    SharedEventList event_list_;
    std::vector<std::unique_ptr<std::thread>> threads_;
};

}

#endif //ASAPO_SYSTEM_FOLDER_WATCH_WINDOWS_H
