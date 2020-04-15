#ifndef ASAPO_WRITE_FILE_PROCESSOR_H
#define ASAPO_WRITE_FILE_PROCESSOR_H

#include "file_processor.h"

namespace asapo {

class WriteFileProcessor final : public FileProcessor {
  public:
    WriteFileProcessor();
    Error ProcessFile(const Request* request, bool overwrite) const override;
};

}

#endif //ASAPO_WRITE_FILE_PROCESSOR_H
