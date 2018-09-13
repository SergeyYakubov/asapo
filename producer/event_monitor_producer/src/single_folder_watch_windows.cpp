#include "single_folder_watch_windows.h"

#include "eventmon_logger.h"

#include <iostream>
#include <string>

namespace asapo {

SingleFolderWatch::SingleFolderWatch(std::string root_folder, std::string folder,SharedEventList* event_list) :
                                                                                    watch_io__{new WatchIO()},
                                                                                    log__{GetDefaultEventMonLogger()},
                                                                                    root_folder_{std::move(root_folder)},
                                                                                        folder_{std::move(folder)},
                                                                                         buffer_{new char[kBufLen]},
                                                                                           event_list_{event_list}

                                                                                    {
}

Error SingleFolderWatch::Init()  {
    if (handle_) {
        return nullptr;
    }
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
    if (Init()!=nullptr) {
        return;
    }
    DWORD bytes_read = 0;
    auto err =watch_io__->ReadDirectoryChanges(handle_,buffer_.get(),kBufLen,&bytes_read);
    if (err == nullptr) {
        ProcessEvents(bytes_read);
    }

}

Error SingleFolderWatch::ProcessEvent(const WinEvent &event) {
    event.Print();
    std::string fname = event.FileName();
    event_list_->AddEvent(fname);
    return nullptr;
}
void SingleFolderWatch::ProcessEvents(DWORD bytes_to_read) {
    for (char* p = buffer_.get(); p < buffer_.get() + bytes_to_read; ) {
        WinEvent event{(FILE_NOTIFY_INFORMATION*) p};
        ProcessEvent(event);
        p += event.Offset();
        if (event.Offset() == 0) {
            break;
        }
    }
}

}


