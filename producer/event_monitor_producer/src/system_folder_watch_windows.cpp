#include "system_folder_watch_windows.h"

#include <windows.h>
#include <memory>

#include "io/io_factory.h"
#include "single_folder_watch_windows.h"

namespace asapo {

Error SystemFolderWatch::StartFolderMonitor(const std::string& root_folder,
                                            const std::vector<std::string>& monitored_folders) {
    for (auto& folder:monitored_folders ) {
    auto thread = io__->NewThread([root_folder, folder,this] {
      auto folder_watch = std::unique_ptr<SingleFolderWatch>(new SingleFolderWatch(root_folder, folder,&event_list_));
      folder_watch->Watch();
    });

    threads_.emplace_back(std::move(thread));
//    if (thread) {
//        thread->detach();
//    }
    }

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

