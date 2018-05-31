#ifndef ASAPO_REQUEST_H
#define ASAPO_REQUEST_H

#include "receiver_error.h"
#include "common/networking.h"
#include "io/io.h"
#include "request_handler.h"
#include "request_handler_file_write.h"
#include "request_handler_db_write.h"
#include "statistics.h"

namespace asapo {

using RequestHandlerList = std::vector<const RequestHandler*>;

class Request {
  public:
    virtual Error Handle(std::unique_ptr<Statistics>*);
    virtual ~Request() = default;
    Request(const GenericRequestHeader& request_header, SocketDescriptor socket_fd);
    void AddHandler(const RequestHandler*);
    const RequestHandlerList& GetListHandlers() const;
    virtual uint64_t GetDataSize() const;
    virtual uint64_t GetDataID() const;
    virtual std::string GetFileName() const;

    virtual const FileData& GetData() const;
    std::unique_ptr<IO> io__;
  private:
    Error AllocateDataBuffer();
    Error ReceiveData();
    const GenericRequestHeader request_header_;
    const SocketDescriptor socket_fd_;
    FileData data_buffer_;
    RequestHandlerList handlers_;
};

class RequestFactory {
  public:
    virtual std::unique_ptr<Request> GenerateRequest(const GenericRequestHeader& request_header,
                                                     SocketDescriptor socket_fd, Error* err) const noexcept;
  private:
    RequestHandlerFileWrite request_handler_filewrite_;
    RequestHandlerDbWrite request_handler_dbwrite_;
};

}

#endif //ASAPO_REQUEST_H

