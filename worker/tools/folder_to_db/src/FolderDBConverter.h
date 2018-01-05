#ifndef HIDRA2_FOLDERDBCONVERTER_H
#define HIDRA2_FOLDERDBCONVERTER_H

#include "system_wrappers/io.h"
#include "database/database.h"

namespace hidra2 {

enum class FolderDBConverterError {
    kOK,
    kDBConnectionError,
    kIOError
};

class FolderDBConverter {
  public:
    explicit FolderDBConverter();

    FolderDBConverterError Convert(const std::string& uri, const std::string& folder);

    std::unique_ptr<hidra2::IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<hidra2::Database> db__; // modified in testings to mock system calls,otherwise do not touch

};

}

#endif //HIDRA2_FOLDERDBCONVERTER_H
