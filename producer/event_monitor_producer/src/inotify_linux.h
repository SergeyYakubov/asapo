#ifndef ASAPO_INOTIFY_H
#define ASAPO_INOTIFY_H

#include <stdint.h>
#include <unistd.h>

#include "asapo/preprocessor/definitions.h"


namespace asapo {

class Inotify {
  public:
    ASAPO_VIRTUAL int Init();
    ASAPO_VIRTUAL int AddWatch(int fd, const char* name, uint32_t mask);
    ASAPO_VIRTUAL int DeleteWatch(int fd, int wd);
    ASAPO_VIRTUAL ssize_t Read(int fd, void* buf, size_t nbytes);
};

}
#endif //ASAPO_INOTIFY_H
