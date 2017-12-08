#include <iostream>
#include <fcntl.h>
#include "file_refernce_handler.h"

namespace hidra2 {

std::map<FileReferenceId, std::shared_ptr<FileReferenceHandler::FileInformation>> FileReferenceHandler::kFileRefernceMap;
FileReferenceId FileReferenceHandler::kGlobalFileRefernceId = 0;

hidra2::FileReferenceId FileReferenceHandler::add_file(std::string filename,
                                                       uint64_t file_size,
                                                       uint32_t owner_connection_id,
                                                       FileReferenceHandlerError& err) {

    FileReferenceId file_ref_id = ++kGlobalFileRefernceId;

    std::string full_path = filename;//TODO
    int fd = open(full_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd == -1) {
        err = FILE_REFERENCE_HANDLER_ERR__OPEN_FAILED;
        return 0;
    }

    if(fallocate(fd, 0, 0, file_size) == -1) {
        err = FILE_REFERENCE_HANDLER_ERR__ALLOCATE_STORAGE_FAILED;
        return 0;
    }

    auto file_info = new FileInformation();
    file_info->filename = filename;
    file_info->file_size = file_size;
    file_info->owner = owner_connection_id;
    file_info->fd = fd;


    kFileRefernceMap[file_ref_id] = std::shared_ptr<FileReferenceHandler::FileInformation>(file_info);

    err = FILE_REFERENCE_HANDLER_ERR__OK;
    return file_ref_id;
}

std::shared_ptr<FileReferenceHandler::FileInformation> FileReferenceHandler::get_file(FileReferenceId file_reference_id) {
    return kFileRefernceMap[file_reference_id];
}

void FileReferenceHandler::remove_file(FileReferenceId file_reference_id) {
    if(kFileRefernceMap.count(file_reference_id) != 1) {
        return; //Not found
    }

    auto file_info = kFileRefernceMap[file_reference_id];
    close(file_info->fd);

    kFileRefernceMap.erase(file_reference_id);
}

}
