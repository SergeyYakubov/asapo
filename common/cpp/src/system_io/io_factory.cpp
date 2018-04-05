#include "io/io_factory.h"

#include "system_io.h"

namespace hidra2 {

IO* GenerateDefaultIO() {
    return new SystemIO;
}


}

