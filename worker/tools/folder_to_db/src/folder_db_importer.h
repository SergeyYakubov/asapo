#ifndef HIDRA2_FOLDERDBCONVERTER_H
#define HIDRA2_FOLDERDBCONVERTER_H

#include <iostream>
#include <chrono>


#include "system_wrappers/io.h"
#include "database/database.h"

namespace hidra2 {

enum class FolderToDbImportError {
    kOK,
    kDBConnectionError,
    kImportError,
    kIOError,
    kUnknownDbError
};

struct ConvertParameters {
    bool ignore_duplicates{false};
    std::string folder;
    std::string uri;
};


struct FolderImportStatistics {
    uint64_t n_files_converted{0};
    std::chrono::nanoseconds time_read_folder{};
    std::chrono::nanoseconds time_import_files{};
    friend std::ostream& operator<<(std::ostream& os, const FolderImportStatistics& stat);
};

class FolderToDbImporter {
  public:
    explicit FolderToDbImporter();
//! Read folder content and write file to the database. We do not optimize
//! the procedure (bulk read, multithreading) to be able to see performance of a single thread
//! for single operation (and it is already fast enough)
    FolderToDbImportError Convert(const ConvertParameters& parameters,
                                  FolderImportStatistics* statistics = nullptr) const;

    std::unique_ptr<hidra2::IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<hidra2::Database> db__; // modified in testings to mock system calls,otherwise do not touch
  private:
    FolderToDbImportError ConnectToDb(const std::string& uri, const std::string& folder) const;
    FileInfos GetFilesInFolder(const std::string& folder, FolderToDbImportError* err) const;
    FolderToDbImportError ImportSingleFile(const FileInfo& file, bool ignore_duplicates) const;
    FolderToDbImportError ImportFilelist(const FileInfos& file_list, bool ignore_duplicates) const;

};

}

#endif //HIDRA2_FOLDERDBCONVERTER_H
