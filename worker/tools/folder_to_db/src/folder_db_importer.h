#ifndef HIDRA2_FOLDERDBCONVERTER_H
#define HIDRA2_FOLDERDBCONVERTER_H

#include <iostream>
#include <chrono>
#include <future>


#include "system_wrappers/io.h"
#include "database/database.h"

namespace hidra2 {

enum class FolderToDbImportError {
    kOK,
    kDBConnectionError,
    kImportError,
    kIOError,
    kUnknownDbError,
    kMemoryError
};

struct FolderImportStatistics {
    uint64_t n_files_converted{0};
    std::chrono::nanoseconds time_read_folder{};
    std::chrono::nanoseconds time_import_files{};
    friend std::ostream& operator<<(std::ostream& os, const FolderImportStatistics& stat);
};

struct TaskSplitParameters {
    uint64_t chunk;
    uint64_t remainder;
    uint64_t begin{0};
    uint64_t next_chunk_size;
};

class FolderToDbImporter {
  public:
    FolderToDbImporter();
//! Read folder content and write file to the database. We do not optimize
//! the procedure via bulk write to see the performance of a
//! single operation (and it is already fast enough)
    FolderToDbImportError Convert(const std::string& uri, const std::string& folder,
                                  const std::string& db_name,
                                  FolderImportStatistics* statistics = nullptr) const;

    unsigned int SetNParallelTasks(unsigned int ntasks, bool async = true);
    void IgnoreDuplicates(bool ignore_duplicates = true);
    std::unique_ptr<hidra2::IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<hidra2::DatabaseFactory>
    db_factory__; // modified in testings to mock system calls,otherwise do not touch

  private:
    bool ignore_duplicates_{false};
    unsigned int n_tasks_{1};
    bool async_{true};
    mutable std::string db_uri_ ;
    mutable std::string db_name_;
    FolderToDbImportError ConnectToDb(const std::unique_ptr<hidra2::Database>& db) const;
    FileInfos GetFilesInFolder(const std::string& folder, FolderToDbImportError* err) const;
    FolderToDbImportError ImportFilelist(const FileInfos& file_list) const;
    FolderToDbImportError PerformParallelTask(const FileInfos& file_list, uint64_t begin,
                                              uint64_t end) const;
    FolderToDbImportError ImportSingleFile(const std::unique_ptr<hidra2::Database>& db,
                                           const FileInfo& file) const;
    FolderToDbImportError ImportFilelistChunk(const std::unique_ptr<hidra2::Database>& db,
                                              const FileInfos& file_list, uint64_t begin, uint64_t end) const;

    std::unique_ptr<Database> CreateDbClient(FolderToDbImportError* err) const;
    void ProcessNextChunk(const FileInfos& file_list, std::vector<std::future<FolderToDbImportError>>* res,
                          TaskSplitParameters* p) const;

};

}

#endif //HIDRA2_FOLDERDBCONVERTER_H
