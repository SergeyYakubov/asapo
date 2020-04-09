#ifndef ASAPO_FILE_PROCESSOR_H
#define ASAPO_FILE_PROCESSOR_H

#include "io/io.h"
#include "logger/logger.h"

namespace asapo {

class Request;

class FileProcessor {
  public:
    FileProcessor();
    virtual ~FileProcessor() = default;
    virtual Error ProcessFile(const Request* request, bool overwrite) const = 0;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
};

}

#endif //ASAPO_FILE_PROCESSOR_H
