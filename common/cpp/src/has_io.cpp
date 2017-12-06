#include "system_wrappers/system_io.h"
#include "system_wrappers/has_io.h"

hidra2::IO* const hidra2::HasIO::kDefaultIO = new hidra2::SystemIO();

hidra2::HasIO::HasIO() {
    io = kDefaultIO;
    //utils = new IOUtils();
}

void hidra2::HasIO::__set_io(hidra2::IO* io) {
    this->io = io;
}

hidra2::IO* hidra2::HasIO::__get_io() {
    return io;
}

void hidra2::HasIO::__set_io_utils(hidra2::IOUtils* io_utils) {
    this->io_utils = io_utils;
}

hidra2::IOUtils* hidra2::HasIO::__get_io_utils() {
    return this->io_utils;
}
