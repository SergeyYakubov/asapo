#include "system_wrappers/system_io.h"

#include <cstring>

//#include <dirent.h>

#include <sys/stat.h>
#include <algorithm>

#include <cerrno>

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

IOErrors IOErrorFromErrno() {
    IOErrors err;
    switch (errno) {
    case 0:
        err = IOErrors::NO_ERROR;
        break;
    case ENOENT:
    case ENOTDIR:
        err = IOErrors::FILE_NOT_FOUND;
        break;
    case EACCES:
        err = IOErrors::PERMISSIONS_DENIED;
        break;
    default:
        err = IOErrors::UNKWOWN_ERROR;
        break;
    }
    return err;
}

std::vector<FileInfo> SystemIO::FilesInFolder(const string& folder, IOErrors* err) {
    std::vector<FileInfo> files{};
    return files;
}

}
