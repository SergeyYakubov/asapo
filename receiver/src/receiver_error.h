#ifndef HIDRA2_RECEIVER_ERROR_H
#define HIDRA2_RECEIVER_ERROR_H

#include "common/error.h"

namespace hidra2 {

enum class ReceiverErrorType {
    kInvalidOpCode,
};

//TODO Make a marco to create error class and error template class
class ReceiverError : public SimpleError {
  private:
    ReceiverErrorType receiver_error_type_;
  public:
    ReceiverError(const std::string& error, ReceiverErrorType receiver_error_type) : SimpleError(error,
                ErrorType::kReceiverError) {
        receiver_error_type_ = receiver_error_type;
    }

    ReceiverErrorType GetReceiverErrorType() const noexcept {
        return receiver_error_type_;
    }
};

class ReceiverErrorTemplate : public SimpleErrorTemplate {
  protected:
    ReceiverErrorType receiver_error_type_;
  public:
    ReceiverErrorTemplate(const std::string& error, ReceiverErrorType receiver_error_type) : SimpleErrorTemplate(error,
                ErrorType::kReceiverError) {
        receiver_error_type_ = receiver_error_type;
    }

    inline ReceiverErrorType GetReceiverErrorType() const noexcept {
        return receiver_error_type_;
    }

    inline Error Generate() const noexcept override {
        return Error(new ReceiverError(error_, receiver_error_type_));
    }

    inline bool operator==(const Error& rhs) const override {
        return SimpleErrorTemplate::operator==(rhs)
               && GetReceiverErrorType() == ((ReceiverError*) rhs.get())->GetReceiverErrorType();
    }
};

namespace ReceiverErrorTemplates {
auto const kInvalidOpCode = ReceiverErrorTemplate {
                                "Invalid Opcode", ReceiverErrorType::kInvalidOpCode
                            };
};
}

#endif //HIDRA2_RECEIVER_ERROR_H
