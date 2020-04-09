#ifndef ASAPO_REQUEST_HANDLER_FILE_PROCESS_H
#define ASAPO_REQUEST_HANDLER_FILE_PROCESS_H

#include "request_handler.h"
#include "logger/logger.h"
#include "../file_processors/file_processor.h"
#include "io/io.h"

namespace asapo {

class RequestHandlerFileProcess final : public ReceiverRequestHandler {
  public:
    RequestHandlerFileProcess() = delete;
    RequestHandlerFileProcess(const FileProcessor* file_processor);
    StatisticEntity GetStatisticEntity() const override;
    Error ProcessRequest(Request* request) const override;
    std::unique_ptr<IO> io__;
    const AbstractLogger* log__;
  private:
    Error ProcessFileExistSituation(Request* request) const;
    const FileProcessor* file_processor_;

};
}
#endif //ASAPO_REQUEST_HANDLER_FILE_PROCESS_H
