#ifndef ASAPO_SYSTEM_FOLDER_WATCH_MACOS_DUMMY_H
#define ASAPO_SYSTEM_FOLDER_WATCH_MACOS_DUMMY_H

#include "common.h"
#include "asapo/preprocessor/definitions.h"


// dummy file to make it compile on macos


namespace asapo {

class SystemFolderWatch {
  public:
    VIRTUAL ~SystemFolderWatch() = default;
    VIRTUAL Error StartFolderMonitor(const std::string&, const std::vector<std::string>&) {
        return nullptr;
    };
    VIRTUAL FilesToSend GetFileList(Error*) {
        return {};
    };
};

}
#endif //ASAPO_SYSTEM_FOLDER_WATCH_MACOS_DUMMY_H
