#include "system_wrappers/system_io.h"
#include "system_wrappers/has_io.h"

hidra2::IO* const hidra2::HasIO::kDefaultIO = new hidra2::SystemIO();

hidra2::HasIO::HasIO() {
    io = kDefaultIO;
}

void hidra2::HasIO::SetIO__(hidra2::IO* io) {
    this->io = io;
}

hidra2::IO* hidra2::HasIO::GetIO__() {
    return io;
}
