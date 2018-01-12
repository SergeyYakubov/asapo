#include "folder_db_importer.h"

#include <future>
#include <algorithm>

#include "system_wrappers/system_io.h"
#include "database/mongodb_client.h"


namespace hidra2 {

using std::chrono::high_resolution_clock;


FolderToDbImportError MapIOError(IOError io_err) {
    FolderToDbImportError err;
    switch (io_err) {
    case IOError::kNoError:
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
    io__{new hidra2::SystemIO}, db_factory__{new hidra2::MongoDatabaseFactory} {
}

FolderToDbImportError FolderToDbImporter::ConnectToDb(const std::unique_ptr<hidra2::Database>& db) const {
    DBError err = db->Connect(db_uri_, kDBName, db_collection_name);
    return MapDBError(err);
}

FolderToDbImportError FolderToDbImporter::ImportSingleFile(const std::unique_ptr<hidra2::Database>& db,
        const FileInfo& file) const {

    auto err = db->Insert(file, ignore_duplicates_);
    return MapDBError(err);
}

FolderToDbImportError FolderToDbImporter::ImportPartOfFilelist(const FileInfos& file_list, uint64_t begin,
        uint64_t end) const {
    DBError db_err;
    std::unique_ptr<hidra2::Database> db = db_factory__->Create(&db_err);

    auto err = ConnectToDb(db);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    for (auto i = begin; i < end; i++) {
        auto err = ImportSingleFile(db, file_list[i]);
        if (err != FolderToDbImportError::kOK) {
            return err;
        }
    }
    return FolderToDbImportError::kOK;
}

FolderToDbImportError FolderToDbImporter::ImportFilelist(const FileInfos& file_list) const {
    auto grain = file_list.size() / n_tasks_;
    if (grain == 0) {
        grain = file_list.size();
    }

    std::vector<std::future<FolderToDbImportError>>res;
    for (auto i = 0; i < file_list.size(); i += grain) {
        auto end = std::min(i + grain, file_list.size());
        res.push_back(std::async(std::launch::async, &FolderToDbImporter::ImportPartOfFilelist, this,
                                 file_list, i, end));
    }
    FolderToDbImportError err{FolderToDbImportError::kOK};
    for (auto& fut : res) {
        auto task_result = fut.get();
        if (task_result != FolderToDbImportError::kOK) {
            err = task_result;
        }
    }
    return err;
}


FileInfos FolderToDbImporter::GetFilesInFolder(const std::string& folder, FolderToDbImportError* err) const {
    IOError err_io;
    auto file_list = io__->FilesInFolder(folder, &err_io);
    *err = MapIOError(err_io);
    return file_list;
}


FolderToDbImportError FolderToDbImporter::Convert(const std::string& uri, const std::string& folder,
                                                  FolderImportStatistics* statistics) const {
    db_uri_ = uri;
    db_collection_name = folder;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    FolderToDbImportError err;
    auto file_list = GetFilesInFolder(folder, &err);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    err = ImportFilelist(file_list);

    high_resolution_clock::time_point t3 = high_resolution_clock::now();

    if (err == FolderToDbImportError::kOK && statistics) {
        statistics->n_files_converted = file_list.size();
        statistics->time_read_folder = std::chrono::duration_cast<std::chrono::nanoseconds>( t2 - t1);
        statistics->time_import_files = std::chrono::duration_cast<std::chrono::nanoseconds>( t3 - t2);
    }

    return err;


}

void FolderToDbImporter::IgnoreDuplicates(bool ignore_duplicates) {
    ignore_duplicates_ = ignore_duplicates;
}

void FolderToDbImporter::RunInParallel(unsigned int ntasks) {
    unsigned int nthreads = std::thread::hardware_concurrency();
    n_tasks_ = std::max((unsigned int)1, std::min(ntasks, nthreads));
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