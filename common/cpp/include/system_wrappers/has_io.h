#ifndef HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H

#include "io.h"

namespace hidra2 {

class HasIO {
  protected:
    static IO* const  kDefaultIO;

    IO* io;

    explicit HasIO();
  public:

    void SetIO__(IO* io);
    IO* GetIO__();
};

}


#endif //HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H
