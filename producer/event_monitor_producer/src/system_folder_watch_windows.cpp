#include "system_folder_watch_windows.h"

namespace asapo {

Error SystemFolderWatch::StartFolderMonitor(const std::string& root_folder,
                                            const std::vector<std::string>& monitored_folders) {
    return nullptr;
}

FilesToSend SystemFolderWatch::GetFileList(Error* err) {
    FilesToSend events;
    *err = nullptr;
    return events;
}


}

