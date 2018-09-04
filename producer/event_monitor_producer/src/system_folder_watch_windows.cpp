#include "system_folder_watch_windows.h"

#include <windows.h>
#include <memory>

#include "io/io_factory.h"
#include "single_folder_monitor.h"

namespace asapo {

Error SystemFolderWatch::StartFolderMonitor(const std::string& root_folder,
                                            const std::vector<std::string>& monitored_folders) {
    for (auto& folder:monitored_folders ) {
    auto thread = io__->NewThread([root_folder, folder] {
      auto folder_monitor = std::unique_ptr<SingleFolderMonitor>(new SingleFolderMonitor(root_folder, folder));
      folder_monitor->Monitor();
    });

    if (thread) {
        thread->detach();
    }
    }
/*
    HANDLE hDir = CreateFile(
        root_folder.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
*/
    return nullptr;
}

FilesToSend SystemFolderWatch::GetFileList(Error* err) {
    FilesToSend events;
    *err = nullptr;
    return events;
}
SystemFolderWatch::SystemFolderWatch() :io__{GenerateDefaultIO()}{

}

}

