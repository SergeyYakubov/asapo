#ifndef HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H
#define HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H

#include "io.h"

namespace hidra2 {

class IOUtils {
  private:
    IO** io_;
  public:
    explicit IOUtils(IO** io);

    bool is_valid_fd(int fd);
};

}

#endif //HIDRA2_SYSTEM_WRAPPERS__SYSTEM_UTIL_H
