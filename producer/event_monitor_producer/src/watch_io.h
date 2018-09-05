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
 private:
  std::unique_ptr<IO>io_;
};

}

#endif //ASAPO_WATCH_IO_H
