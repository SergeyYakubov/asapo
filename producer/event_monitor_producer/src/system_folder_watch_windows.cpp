#include "system_folder_watch_windows.h"

#include <windows.h>
#include <memory>

#include "asapo/io/io_factory.h"
#include "single_folder_watch_windows.h"

namespace asapo {

Error SystemFolderWatch::StartFolderMonitor(const std::string& root_folder,
                                            const std::vector<std::string>& monitored_folders) {
    for (auto& folder : monitored_folders ) {
        auto thread = io__->NewThread("FolderMonitor", [root_folder, folder, this] {
            auto folder_watch = std::unique_ptr<SingleFolderWatch>(new SingleFolderWatch(root_folder, folder, &event_list_));
            while (true) {
                auto err = folder_watch->Watch();
                if (err) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
            }
        });

        threads_.emplace_back(std::move(thread));
    }

    return nullptr;
}

FilesToSend SystemFolderWatch::GetFileList(Error* err) {
    *err = nullptr;
    return event_list_.GetAndClearEvents();
}

SystemFolderWatch::SystemFolderWatch() : io__{GenerateDefaultIO()} {

}

}

