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
Error WatchIO::ReadDirectoryChanges(HANDLE handle, LPVOID buffer, DWORD buffer_length, LPDWORD bytes_returned) {
    DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE;
    auto res = ReadDirectoryChangesW(handle,buffer,buffer_length,true,filter,bytes_returned,nullptr,nullptr );
    printf("after read changes\n");
    if (res) {
        printf("after read changes ok\n");
        return nullptr;
    } else {
        printf("after read problem\n");
        return io_->GetLastError();
    }
}

}