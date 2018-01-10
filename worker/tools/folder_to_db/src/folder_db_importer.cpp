#include "folder_db_importer.h"

#include "system_wrappers/system_io.h"
#include "database/mongodb_client.h"

namespace hidra2 {

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
        err = FolderToDbImportError::kImportError;
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

FolderToDbImportError FolderToDbImporter::ImportSingleFile(const FileInfo& file) const {
    auto err = db__->Insert(file);
    return MapDBError(err);
}


FolderToDbImportError FolderToDbImporter::ImportFilelist(const FileInfos& file_list) const {
    for (auto& file : file_list) {
        auto err = ImportSingleFile(file);
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


FolderToDbImportError FolderToDbImporter::Convert(const std::string& folder, const std::string& uri) const {
    auto err = ConnectToDb(uri, folder);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    auto file_list = GetFilesInFolder(folder, &err);
    if (err != FolderToDbImportError::kOK) {
        return err;
    }

    return ImportFilelist(file_list);
}
}