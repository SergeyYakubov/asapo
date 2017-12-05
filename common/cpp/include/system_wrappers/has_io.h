#ifndef HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H
#define HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H

#include "io.h"

namespace hidra2 {

class HasIO {
 protected:
  static IO* const kDefaultIO;
  IO* io;

  HasIO();
 public:
  void __set_io(IO* io);
};

}


#endif //HIDRA2_SYSTEM_WRAPPERS__HAS_IO_H
