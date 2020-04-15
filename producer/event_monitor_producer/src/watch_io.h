#ifndef ASAPO_WATCH_IO_H
#define ASAPO_WATCH_IO_H

#include <windows.h>

#include "preprocessor/definitions.h"
#include "common/error.h"
#include "io/io.h"

namespace asapo {

class WatchIO {
  public:
    explicit WatchIO();
    VIRTUAL HANDLE Init(const char* folder, Error* err);
    VIRTUAL Error ReadDirectoryChanges(HANDLE handle, LPVOID buffer, DWORD buffer_length, LPDWORD bytes_returned);
    VIRTUAL bool IsDirectory(const std::string& path);
  private:
    std::unique_ptr<IO>io_;
};

}

#endif //ASAPO_WATCH_IO_H
