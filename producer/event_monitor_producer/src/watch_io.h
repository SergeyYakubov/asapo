#ifndef ASAPO_WATCH_IO_H
#define ASAPO_WATCH_IO_H

#include <windows.h>

#include "asapo/preprocessor/definitions.h"
#include "asapo/common/error.h"
#include "asapo/io/io.h"

namespace asapo {

class WatchIO {
  public:
    explicit WatchIO();
    ASAPO_VIRTUAL HANDLE Init(const char* folder, Error* err);
    ASAPO_VIRTUAL Error ReadDirectoryChanges(HANDLE handle, LPVOID buffer, DWORD buffer_length, LPDWORD bytes_returned);
    ASAPO_VIRTUAL bool IsDirectory(const std::string& path);
  private:
    std::unique_ptr<IO>io_;
};

}

#endif //ASAPO_WATCH_IO_H
