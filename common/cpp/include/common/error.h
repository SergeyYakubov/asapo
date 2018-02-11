#ifndef HIDRA2_ERROR_H
#define HIDRA2_ERROR_H

#include <string>

namespace hidra2 {

class ErrorInterface {
  public:
    virtual std::string Explain() const noexcept = 0;
    virtual void Set(const std::string& error) noexcept = 0;
};

using Error = std::unique_ptr<ErrorInterface>;

class SimpleError: public ErrorInterface {
  private:
    std::string error_;
  public:
    explicit SimpleError(const std::string& error): error_{error} {

    }
    std::string Explain() const noexcept override  {
        return error_;
    }
    void Set(const std::string& error)noexcept override  {
        error_ = error;
    }

};

inline Error TextError(const std::string& error) {
    return Error{new SimpleError{error}};
}

}
#endif //HIDRA2_ERROR_H
