#ifndef ASAPO_RECEIVE_FILE_PROCESSOR_H
#define ASAPO_RECEIVE_FILE_PROCESSOR_H

#include "file_processor.h"

namespace asapo {

class ReceiveFileProcessor final : public FileProcessor {
  public:
    ReceiveFileProcessor();
    Error ProcessFile(const Request* request, bool overwrite) const override;
};


}

#endif //ASAPO_RECEIVE_FILE_PROCESSOR_H
