#include "system_folder_watch_linux.h"

namespace asapo {

Error SystemFolderWatch::StartFolderMonitor(const std::vector<std::string>& monitored_folders) {
    return nullptr;
}

FileEvents SystemFolderWatch::GetFileEventList(Error* err) {
    FileEvents events;
    *err = nullptr;
    return events;
}


}