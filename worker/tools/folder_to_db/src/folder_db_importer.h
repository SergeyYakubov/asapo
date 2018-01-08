#ifndef HIDRA2_FOLDERDBCONVERTER_H
#define HIDRA2_FOLDERDBCONVERTER_H

#include "system_wrappers/io.h"
#include "database/database.h"

namespace hidra2 {

enum class FolderToDbImportError {
    kOK,
    kDBConnectionError,
    kImportError,
    kIOError
};

class FolderToDbImporter {
  public:
    explicit FolderToDbImporter();

    FolderToDbImportError Convert(const std::string& folder, const std::string& uri) const;

    std::unique_ptr<hidra2::IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<hidra2::Database> db__; // modified in testings to mock system calls,otherwise do not touch
  private:
    FolderToDbImportError ConnectToDb(const std::string& uri, const std::string& folder) const;
    FileInfos GetFilesInFolder(const std::string& folder, FolderToDbImportError* err) const;
    FolderToDbImportError ImportFilelist(FileInfos file_list) const;

};

}

#endif //HIDRA2_FOLDERDBCONVERTER_H
