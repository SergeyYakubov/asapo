#include "FolderDBConverter.h"

#include "system_wrappers/system_io.h"
#include "database/mongo_database.h"

namespace hidra2 {

FolderDBConverterError MapIOError(IOError io_err) {
    FolderDBConverterError err;
    switch (io_err) {
    case IOError::kNoError:
        err = FolderDBConverterError::kOK;
        break;
    default:
        err = FolderDBConverterError::kIOError;
        break;
    }
    return err;
}

FolderDBConverterError MapDBError(DBError db_err) {
    FolderDBConverterError err;
    switch (db_err) {
    case DBError::kNoError:
        err = FolderDBConverterError::kOK;
        break;
    case DBError::kConnectionError:
        err = FolderDBConverterError::kDBConnectionError;
        break;
    }
    return err;
}



FolderDBConverter::FolderDBConverter() :
    io__{new hidra2::SystemIO}, db__{new hidra2::MongoDB} {
}

FolderDBConverterError FolderDBConverter::Convert(const std::string& uri, const std::string& folder) {
    DBError err_db = db__->Connect(uri, kDBName, folder);
    if (err_db != DBError::kNoError) {
        return MapDBError(err_db);
    }

    IOError err_io;
    auto file_list = io__->FilesInFolder(folder,&err_io);
    if (err_io != IOError::kNoError){
        return MapIOError(err_io);
    }

    return FolderDBConverterError::kOK;
}

}