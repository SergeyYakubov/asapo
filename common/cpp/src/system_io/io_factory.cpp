#include "system_wrappers/system_io.h"
#include "system_wrappers/io_factory.h"

namespace hidra2 {

IO* GenerateDefaultIO() {
    return new SystemIO;
}


}

