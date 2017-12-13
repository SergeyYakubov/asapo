#ifndef HIDRA2_FILE_REFERNCE_HANDLER_H
#define HIDRA2_FILE_REFERNCE_HANDLER_H

#include <cstdint>
#include <common/networking.h>
#include <map>
#include <system_wrappers/has_io.h>
#include <memory>

namespace hidra2 {

enum FileReferenceHandlerError {
    FILE_REFERENCE_HANDLER_ERR__OK,
    FILE_REFERENCE_HANDLER_ERR__OPEN_FAILED,
    FILE_REFERENCE_HANDLER_ERR__ALLOCATE_STORAGE_FAILED,
};

class FileReferenceHandler : HasIO {
  public:
    struct FileInformation {
        std::string filename;
        uint64_t    file_size;
        uint32_t    owner;

        int         fd;
    };
  private:
    static FileReferenceId kGlobalFileRefernceId;
    static std::map<FileReferenceId, std::shared_ptr<FileReferenceHandler::FileInformation>> kFileRefernceMap;
  public:
    FileReferenceId                     add_file    (std::string filename, uint64_t file_size, uint32_t owner_connection_id,
                                                     FileReferenceHandlerError& err);
    std::shared_ptr<FileInformation>    get_file    (FileReferenceId file_reference_id);
    void                                remove_file (FileReferenceId file_reference_id);
};

}

#endif //HIDRA2_FILE_REFERNCE_HANDLER_H
