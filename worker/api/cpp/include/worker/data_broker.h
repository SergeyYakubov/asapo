#ifndef HIDRA2_DATASOURCE_H
#define HIDRA2_DATASOURCE_H

#include <memory>
#include <string>
#include <iostream>

namespace hidra2 {

enum class WorkerErrorCode {
    ERR__NO_ERROR,
    ERR__MEMORY_ERROR,
    ERR__EMPTY_DATASOURCE,
    SOURCE_NOT_FOUND,
    UNKNOWN_IO_ERROR
};

class DataBroker {
  public:
    virtual WorkerErrorCode Connect() = 0;
};

class DataBrokerFactory {
  public:
    static std::unique_ptr<DataBroker> Create(const std::string& source_name, WorkerErrorCode* return_code) noexcept;
};

}
#endif //HIDRA2_DATASOURCE_H
