#include "system/system_io.h"
#include "system/io_factory.h"

namespace hidra2 {

IO* GenerateDefaultIO() {
    return new SystemIO;
}


}

