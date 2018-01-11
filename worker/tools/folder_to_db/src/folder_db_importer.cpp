#include "folder_db_importer.h"

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
    default:
        err = FolderToDbImportError::kUnknownDbError;
        break;
    }
    return err;
}


FolderToDbImporter::FolderToDbImporter() :
    io__{new hidra2::SystemIO}, db__{new hidra2::MongoDBClient} {
}

FolderToDbImportError FolderToDbImporter::ConnectToDb(const std::string& uri, const std::string& folder) const {
    DBError err = db__->Connect(uri, kDBName, folder);
    return MapDBError(err);
}

FolderToDbImportError FolderToDbImporter::ImportSingleFile(const FileInfo& file, bool ignore_duplicates) const {
    auto err = db__->Insert(file, ignore_duplicates);
    return MapDBError(err);
}


FolderToDbImportError FolderToDbImporter::ImportFilelist(const FileInfos& file_list, bool ignore_duplicates) const {
    for (auto& file : file_list) {
        auto err = ImportSingleFile(file, ignore_duplicates);
        if (err != FolderToDbImportError::kOK) {
            return err;
        }
    }
    return FolderToDbImportError::kOK;
}


FileInfos FolderToDbImporter::GetFilesInFolder(const std::string& folder, FolderToDbImportError* err) const {
    IOError err_io;
    auto file_list = io__->FilesInFolder(folder, &err_io);
    *err = MapIOError(err_io);
    return file_list;
}


FolderToDbImportError FolderToDbImporter::Convert(const ConvertParameters& parameters,
                                                  FolderImportStatistics* statistics) const {
    auto err = ConnectToDb(parameters.uri, parameters.folder);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    auto file_list = GetFilesInFolder(parameters.folder, &err);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    err = ImportFilelist(file_list, parameters.ignore_duplicates);

    high_resolution_clock::time_point t3 = high_resolution_clock::now();

    if (err == FolderToDbImportError::kOK && statistics) {
        statistics->n_files_converted = file_list.size();
        statistics->time_read_folder = std::chrono::duration_cast<std::chrono::nanoseconds>( t2 - t1);
        statistics->time_import_files = std::chrono::duration_cast<std::chrono::nanoseconds>( t3 - t2);
    }

    return err;


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