#include "single_folder_watch_windows.h"

#include "eventmon_logger.h"
namespace asapo {

SingleFolderWatch::SingleFolderWatch(std::string root_folder, std::string folder) : watch_io__{new WatchIO()},
                                                                                    log__{GetDefaultEventMonLogger()},
                                                                                    root_folder_{std::move(root_folder)},
                                                                                        folder_{std::move(folder)}
                                                                                    {
}

Error SingleFolderWatch::Init()  {
    std::string full_path = this->root_folder_ + kPathSeparator + this->folder_;
    Error err;
    handle_ = this->watch_io__->Init(full_path.c_str(), &err);
    if (err) {
        this->log__->Error("cannot add folder watch for "+full_path+": "+err->Explain());
        return err;
    }
    return nullptr;
}

void SingleFolderWatch::Watch() {
    if (!Init()) {
        return;
    }
}

}
