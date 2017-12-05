#include "system_wrappers/system_io.h"

namespace hidra2 {

std::vector<std::string> SystemIO::FilesInFolder(std::string folder, IOErrors* err) {
    *err=IOErrors::FOLDER_NOT_FOUND;
    return {};
}

}