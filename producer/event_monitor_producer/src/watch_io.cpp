#include "watch_io.h"
#include "io/io_factory.h"
namespace asapo {

HANDLE WatchIO::Init(const char* folder, Error* err) {
    HANDLE hDir = CreateFile(
        folder,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
    if (hDir == INVALID_HANDLE_VALUE ) {
        *err = io_->GetLastError();
    }
    return hDir;
}

WatchIO::WatchIO() :io_{GenerateDefaultIO()}{

}

}