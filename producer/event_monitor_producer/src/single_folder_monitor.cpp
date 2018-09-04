#include "single_folder_monitor.h"

namespace asapo {

SingleFolderMonitor::SingleFolderMonitor(std::string root_folder, std::string folder) : root_folder_{std::move(root_folder)},
                                                                                        folder_{std::move(folder)} {

}

void SingleFolderMonitor::Monitor() {
}

}
