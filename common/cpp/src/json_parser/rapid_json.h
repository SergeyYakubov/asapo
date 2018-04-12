#ifndef HIDRA2_RAPID_JSON_H
#define HIDRA2_RAPID_JSON_H

#include "rapidjson/document.h"
#include "common/error.h"
#include "io/io.h"


namespace hidra2 {

enum class ValueType {
    kUint64,
    kString,
    kObject,
    kArray,
    kBool
};

class RapidJson {
  public:
    RapidJson(const std::string& json, const std::unique_ptr<IO>* io);
    RapidJson(const RapidJson& parent, const std::string& subname);
    Error GetUInt64(const std::string& name, uint64_t* val) const noexcept;
    Error GetBool(const std::string& name, bool* val) const noexcept;
    Error GetString(const std::string& name, std::string* val) const noexcept;
    Error GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept;
    Error GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept;
  private:
    const std::unique_ptr<IO>* io__;
    mutable rapidjson::Document doc_;
    mutable rapidjson::Value object_;
    mutable rapidjson::Value* object_p_;
    std::string json_;
    mutable bool initialized_ = false;
    Error LazyInitialize() const noexcept;
    Error embedded_error_ = nullptr;

    hidra2::Error GetValuePointer(const std::string& name, ValueType type, rapidjson::Value** val)const noexcept;
};

}
#endif //HIDRA2_RAPID_JSON_H
