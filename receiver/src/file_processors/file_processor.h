#ifndef ASAPO_FILE_PROCESSOR_H
#define ASAPO_FILE_PROCESSOR_H

#include "asapo/io/io.h"
#include "asapo/logger/logger.h"

namespace asapo {

class Request;

Error GetRootFolder(const Request* request, std::string* root_folder);

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
