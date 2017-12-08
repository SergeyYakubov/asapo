#ifndef HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H

#include "io.h"
#include "io_utils.h"

namespace hidra2 {

class HasIO {
  protected:
    static IO* const kDefaultIO;

    IO* io;
    IOUtils* io_utils;

    HasIO();
  public:

    void __set_io(IO* io);
    IO* __get_io();

    void __set_io_utils(IOUtils* io_utils);
    IOUtils* __get_io_utils();
};

}


#endif //HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H
