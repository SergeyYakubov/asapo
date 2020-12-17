#ifndef ASAPO_SYSTEM_FOLDER_WATCH_MACOS_DUMMY_H
#define ASAPO_SYSTEM_FOLDER_WATCH_MACOS_DUMMY_H

#include "common.h"
#include "asapo/preprocessor/definitions.h"


// dummy file to make it compile on macos


namespace asapo {

class SystemFolderWatch {
  public:
    VIRTUAL Error StartFolderMonitor(const std::string& root_folder, const std::vector<std::string>& monitored_folders) {
        return nullptr;
    };
    VIRTUAL FilesToSend GetFileList(Error* err) {
        return {};
    };
};

}
#endif //ASAPO_SYSTEM_FOLDER_WATCH_MACOS_DUMMY_H
