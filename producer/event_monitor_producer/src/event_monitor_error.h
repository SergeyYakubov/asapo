#ifndef ASAPO_EventMonitor_ERROR_H
#define ASAPO_EventMonitor_ERROR_H

#include "common/error.h"

namespace asapo {

enum class EventMonitorErrorType {
    kNoNewEvent,
    kSystemError
};

class EventMonitorError : public SimpleError {
  private:
    EventMonitorErrorType error_type_;
  public:
    EventMonitorError(const std::string& error, EventMonitorErrorType error_type) : SimpleError(error,
                ErrorType::kHidraError) {
        error_type_ = error_type;
    }

    EventMonitorErrorType GetEventMonitorErrorType() const noexcept {
        return error_type_;
    }
};

class EventMonitorErrorTemplate : public SimpleErrorTemplate {
  protected:
    EventMonitorErrorType error_type_;
  public:
    EventMonitorErrorTemplate(const std::string& error, EventMonitorErrorType error_type) : SimpleErrorTemplate(error,
                ErrorType::kHidraError) {
        error_type_ = error_type;
    }

    inline EventMonitorErrorType GetEventMonitorErrorType() const noexcept {
        return error_type_;
    }

    inline Error Generate() const noexcept override {
        return Error(new EventMonitorError(error_, error_type_));
    }

    inline bool operator==(const Error& rhs) const override {
        return SimpleErrorTemplate::operator==(rhs)
               && GetEventMonitorErrorType() == ((EventMonitorError*) rhs.get())->GetEventMonitorErrorType();
    }
};

static inline std::ostream& operator<<(std::ostream& os, const EventMonitorErrorTemplate& err) {
    return os << err.Text();
}


namespace EventMonitorErrorTemplates {
auto const kNoNewEvent = EventMonitorErrorTemplate {
    "no new event", EventMonitorErrorType::kNoNewEvent
};

auto const kSystemError = EventMonitorErrorTemplate {
    "system error", EventMonitorErrorType::kSystemError
};



};
}

#endif //ASAPO_EventMonitor_ERROR_H

