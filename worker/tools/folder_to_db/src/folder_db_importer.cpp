#include "folder_db_importer.h"

#include <future>
#include <algorithm>

#include "system_wrappers/system_io.h"
#include "database/database.h"


namespace hidra2 {

using std::chrono::high_resolution_clock;


FolderToDbImportError MapIOError(IOErrors io_err) {
    FolderToDbImportError err;
    switch (io_err) {
    case IOErrors::kNoError:
        err = FolderToDbImportError::kOK;
        break;
    default:
        err = FolderToDbImportError::kIOError;
        break;
        break;
    }
    return err;
}

FolderToDbImportError MapDBError(DBError db_err) {
    FolderToDbImportError err;
    switch (db_err) {
    case DBError::kNoError:
        err = FolderToDbImportError::kOK;
        break;
    case DBError::kConnectionError:
        err = FolderToDbImportError::kDBConnectionError;
        break;
    case DBError::kInsertError:
    case DBError::kDuplicateID:
        err = FolderToDbImportError::kImportError;
        break;
    case DBError::kMemoryError:
        err = FolderToDbImportError::kMemoryError;
        break;

    default:
        err = FolderToDbImportError::kUnknownDbError;
        break;
    }
    return err;
}


FolderToDbImporter::FolderToDbImporter() :
    io__{new hidra2::SystemIO}, db_factory__{new hidra2::DatabaseFactory} {
}

FolderToDbImportError FolderToDbImporter::ConnectToDb(const std::unique_ptr<hidra2::Database>& db) const {
    DBError err = db->Connect(db_uri_, db_name_, kDBCollectionName);
    return MapDBError(err);
}

FolderToDbImportError FolderToDbImporter::ImportSingleFile(const std::unique_ptr<hidra2::Database>& db,
        const FileInfo& file) const {

    auto err = db->Insert(file, ignore_duplicates_);
    return MapDBError(err);
}

FolderToDbImportError FolderToDbImporter::ImportFilelistChunk(const std::unique_ptr<hidra2::Database>& db,
        const FileInfos& file_list, uint64_t begin, uint64_t end) const {
    for (auto i = begin; i < end; i++) {
        auto err = ImportSingleFile(db, file_list[i]);
        if (err != FolderToDbImportError::kOK) {
            return err;
        }
    }
    return FolderToDbImportError::kOK;
}

FolderToDbImportError FolderToDbImporter::PerformParallelTask(const FileInfos& file_list, uint64_t begin,
        uint64_t end) const {
    FolderToDbImportError err;
    auto db = CreateDbClient(&err);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    err = ConnectToDb(db);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    return ImportFilelistChunk(db, file_list, begin, end);
}
std::unique_ptr<hidra2::Database> FolderToDbImporter::CreateDbClient(FolderToDbImportError* err) const {
    DBError db_err;
    auto db = db_factory__->Create(&db_err);
    *err = MapDBError(db_err);
    return db;
}

FolderToDbImportError WaitParallelTasks(std::vector<std::future<FolderToDbImportError>>* res) {
    FolderToDbImportError err{FolderToDbImportError::kOK};
    for (auto& fut : *res) {
        auto task_result = fut.get();
        if (task_result != FolderToDbImportError::kOK) {
            err = task_result;
        }
    }
    return err;
}


TaskSplitParameters ComputeSplitParameters(const FileInfos& file_list, int ntasks) {
    TaskSplitParameters parameters;
    parameters.chunk = file_list.size() / ntasks;
    parameters.remainder = file_list.size() % ntasks;
    return parameters;
}

void FolderToDbImporter::ProcessNextChunk(const FileInfos& file_list,
                                          std::vector<std::future<FolderToDbImportError>>* res,
                                          TaskSplitParameters* p) const {
    p->next_chunk_size = p->chunk + (p->remainder ? 1 : 0);
    if (p->next_chunk_size == 0) return;

    auto method = async_ ? std::launch::async : std::launch::deferred;

    res->push_back(std::async(method, &FolderToDbImporter::PerformParallelTask, this,
                              file_list, p->begin, p->begin + p->next_chunk_size));

    p->begin = p->begin + p->next_chunk_size;
    if (p->remainder) p->remainder -= 1;
}

FolderToDbImportError FolderToDbImporter::ImportFilelist(const FileInfos& file_list) const {
    auto split_parameters = ComputeSplitParameters(file_list, n_tasks_);

    std::vector<std::future<FolderToDbImportError>>res;
    for (unsigned int i = 0; i < n_tasks_; i++) {
        ProcessNextChunk(file_list, &res, &split_parameters);
    }

    return WaitParallelTasks(&res);
}


FileInfos FolderToDbImporter::GetFilesInFolder(const std::string& folder, FolderToDbImportError* err) const {
    IOErrors err_io;
    auto file_list = io__->FilesInFolder(folder, &err_io);
    *err = MapIOError(err_io);
    return file_list;
}


FolderToDbImportError FolderToDbImporter::Convert(const std::string& uri, const std::string& folder,
                                                  const std::string& db_name,
                                                  FolderImportStatistics* statistics) const {
    db_uri_ = uri;
    db_name_ = db_name;
    auto time_begin = high_resolution_clock::now();

    FolderToDbImportError err;
    auto file_list = GetFilesInFolder(folder, &err);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    auto time_end_read_folder = high_resolution_clock::now();

    err = ImportFilelist(file_list);

    auto time_end_import = high_resolution_clock::now();

    if (err == FolderToDbImportError::kOK && statistics) {
        statistics->n_files_converted = file_list.size();
        statistics->time_read_folder = std::chrono::duration_cast<std::chrono::nanoseconds>( time_end_read_folder - time_begin);
        statistics->time_import_files = std::chrono::duration_cast<std::chrono::nanoseconds>
                                        ( time_end_import - time_end_read_folder);
    }

    return err;
}

void FolderToDbImporter::IgnoreDuplicates(bool ignore_duplicates) {
    ignore_duplicates_ = ignore_duplicates;
}

unsigned int FolderToDbImporter::SetNParallelTasks(unsigned int ntasks, bool async) {
    unsigned int nthreads = std::thread::hardware_concurrency();
    n_tasks_ = std::max((unsigned int)1, std::min(ntasks, nthreads));
    async_ = async;
    return n_tasks_;
}

std::ostream& operator<<(std::ostream& os, const FolderImportStatistics& stat) {
    os << "Processed files: " << stat.n_files_converted << "\n"
       << "Folder read in : "
       << std::chrono::duration_cast<std::chrono::milliseconds>(stat.time_read_folder).count()
       << "ms" << "\n"
       << "Files imported in : "
       << std::chrono::duration_cast<std::chrono::milliseconds>(stat.time_import_files).count()
       << "ms";
    return os;
}


}