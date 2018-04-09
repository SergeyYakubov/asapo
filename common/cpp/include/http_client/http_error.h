#ifndef HIDRA2_HTTP_ERROR_H
#define HIDRA2_HTTP_ERROR_H

#include "common/error.h"
#include "http_client.h"

namespace hidra2 {

class HttpError: public SimpleError {
  public:
    HttpError(const std::string& error, HttpCode http_code): SimpleError{error, ErrorType::kHttpError}, http_code_{http_code} {
    }
    HttpCode GetCode() const {
        return http_code_;
    }
  private:
    HttpCode http_code_;
};

}

#endif //HIDRA2_HTTP_ERROR_H
