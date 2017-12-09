#ifndef HIDRA2_DATASOURCE_H
#define HIDRA2_DATASOURCE_H

#include <memory>
#include <string>
#include <iostream>
#include <common/file_info.h>
#include <vector>

namespace hidra2 {

enum class WorkerErrorCode {
    OK,
    MEMORY_ERROR,
    EMPTY_DATASOURCE,
    SOURCE_NOT_FOUND,
    SOURCE_NOT_CONNECTED,
    SOURCE_ALREADY_CONNECTED,
    ERROR_READING_FROM_SOURCE,
    PERMISSIONS_DENIED,
    NO_DATA,
    WRONG_INPUT,
    UNKNOWN_IO_ERROR
};

class DataBroker {
  public:
    virtual WorkerErrorCode Connect() = 0;
    virtual WorkerErrorCode GetNext(FileInfo* info, FileData* data) = 0;
    virtual ~DataBroker() = default; // needed for unique_ptr to delete itself
};

class DataBrokerFactory {
  public:
    static std::unique_ptr<DataBroker> Create(const std::string& source_name, WorkerErrorCode* return_code) noexcept;
};

}
#endif //HIDRA2_DATASOURCE_H
