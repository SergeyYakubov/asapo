#include "folder_db_importer.h"

#include <future>
#include <algorithm>

#include "io/io_factory.h"
#include "database/database.h"


namespace asapo {

using std::chrono::system_clock;

FolderToDbImporter::FolderToDbImporter() :
    io__{GenerateDefaultIO()}, db_factory__{new asapo::DatabaseFactory} {
}

Error FolderToDbImporter::ConnectToDb(const std::unique_ptr<asapo::Database>& db) const {
    return db->Connect(db_uri_, db_name_);
}

Error FolderToDbImporter::ImportSingleFile(const std::unique_ptr<asapo::Database>& db,
                                           const FileInfo& file) const {
    return db->Insert(std::string(kDBDataCollectionNamePrefix) + "_default", file, ignore_duplicates_);
}

Error FolderToDbImporter::ImportFilelistChunk(const std::unique_ptr<asapo::Database>& db,
                                              const FileInfos& file_list, uint64_t begin, uint64_t end) const {
    for (auto i = begin; i < end; i++) {
        auto err = ImportSingleFile(db, file_list[(size_t)i]);
        if (err != nullptr) {
            return err;
        }
    }
    return nullptr;
}

Error FolderToDbImporter::PerformParallelTask(const FileInfos& file_list, uint64_t begin,
                                              uint64_t end) const {
    Error err;
    auto db = CreateDbClient(&err);
    if (err) {
        return err;
    }

    err = ConnectToDb(db);
    if (err) {
        return err;
    }

    return ImportFilelistChunk(db, file_list, begin, end);
}
std::unique_ptr<asapo::Database> FolderToDbImporter::CreateDbClient(Error* err) const {
    return db_factory__->Create(err);
}

Error WaitParallelTasks(std::vector<std::future<Error>>* res) {
    Error err{nullptr};
    for (auto& fut : *res) {
        auto task_result = fut.get();
        if (task_result != nullptr) {
            err = std::move(task_result);
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
                                          std::vector<std::future<Error>>* res,
                                          TaskSplitParameters* p) const {
    p->next_chunk_size = p->chunk + (p->remainder ? 1 : 0);
    if (p->next_chunk_size == 0) return;

    auto method = async_ ? std::launch::async : std::launch::deferred;

    res->push_back(std::async(method, &FolderToDbImporter::PerformParallelTask, this,
                              file_list, p->begin, p->begin + p->next_chunk_size));

    p->begin = p->begin + p->next_chunk_size;
    if (p->remainder) p->remainder -= 1;
}

Error FolderToDbImporter::ImportFilelist(const FileInfos& file_list) const {
    auto split_parameters = ComputeSplitParameters(file_list, n_tasks_);

    std::vector<std::future<Error>>res;
    for (unsigned int i = 0; i < n_tasks_; i++) {
        ProcessNextChunk(file_list, &res, &split_parameters);
    }

    return WaitParallelTasks(&res);
}


FileInfos FolderToDbImporter::GetFilesInFolder(const std::string& folder, Error* err) const {
    auto file_list = io__->FilesInFolder(folder, err);
    return file_list;
}


Error FolderToDbImporter::Convert(const std::string& uri, const std::string& folder,
                                  const std::string& db_name,
                                  FolderImportStatistics* statistics) const {
    db_uri_ = uri;
    db_name_ = db_name;
    auto time_begin = system_clock::now();

    Error err;
    auto file_list = GetFilesInFolder(folder, &err);
    if (err) {
        return err;
    }

    auto time_end_read_folder = system_clock::now();

    err = ImportFilelist(file_list);

    auto time_end_import = system_clock::now();

    if (err == nullptr && statistics) {
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