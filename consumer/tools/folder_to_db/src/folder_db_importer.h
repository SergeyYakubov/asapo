#ifndef ASAPO_FOLDERDBCONVERTER_H
#define ASAPO_FOLDERDBCONVERTER_H

#include <iostream>
#include <chrono>
#include <future>


#include "asapo/io/io.h"
#include "asapo/database/database.h"
#include "asapo/common/error.h"
namespace asapo {

namespace FolderToDbImportError {

auto const kDBConnectionError = "DB Connection Error";
auto const kImportError = "Import Error";
auto const kUnknownDbError = "Unknown DB Error";
auto const kMemoryError = "Memory error";

}

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
    Error Convert(const std::string& uri, const std::string& folder,
                  const std::string& db_name,
                  FolderImportStatistics* statistics = nullptr) const;

    unsigned int SetNParallelTasks(unsigned int ntasks, bool async = true);
    void IgnoreDuplicates(bool ignore_duplicates = true);
    std::unique_ptr<asapo::IO> io__; // modified in testings to mock system calls,otherwise do not touch
    std::unique_ptr<asapo::DatabaseFactory>
    db_factory__; // modified in testings to mock system calls,otherwise do not touch

  private:
    bool ignore_duplicates_{false};
    unsigned int n_tasks_{1};
    bool async_{true};
    mutable std::string db_uri_ ;
    mutable std::string db_name_;
    Error ConnectToDb(const std::unique_ptr<asapo::Database>& db) const;
    MessageMetas GetFilesInFolder(const std::string& folder, Error* err) const;
    Error ImportFilelist(const MessageMetas& file_list) const;
    Error PerformParallelTask(const MessageMetas& file_list, uint64_t begin,
                              uint64_t end) const;
    Error ImportSingleFile(const std::unique_ptr<asapo::Database>& db,
                           const MessageMeta& file) const;
    Error ImportFilelistChunk(const std::unique_ptr<asapo::Database>& db,
                              const MessageMetas& file_list, uint64_t begin, uint64_t end) const;

    std::unique_ptr<Database> CreateDbClient(Error* err) const;
    void ProcessNextChunk(const MessageMetas& file_list, std::vector<std::future<Error>>* res,
                          TaskSplitParameters* p) const;

};

}

#endif //ASAPO_FOLDERDBCONVERTER_H
