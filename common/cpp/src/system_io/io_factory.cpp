#include "asapo/io/io_factory.h"

#include "system_io.h"

namespace asapo {

IO* GenerateDefaultIO() {
    return new SystemIO;
}


}

