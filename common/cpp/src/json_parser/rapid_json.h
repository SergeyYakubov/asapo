#ifndef HIDRA2_RAPID_JSON_H
#define HIDRA2_RAPID_JSON_H

#include "rapidjson/document.h"
#include "common/error.h"
#include "system/io.h"


namespace hidra2 {

enum class ValueType {
    kUint64,
    kString,
    kObject,
    kArray
};

class RapidJson {
  public:
    RapidJson(const std::string& json, bool read_from_file);
    RapidJson(const RapidJson& parent, const std::string& subname);
    Error GetUInt64(const std::string& name, uint64_t* val) const noexcept;
    Error GetString(const std::string& name, std::string* val) const noexcept;
    Error GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept;
    Error GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept;
    std::unique_ptr<IO> io__;
  private:
    mutable rapidjson::Document doc_;
    mutable rapidjson::Value object_;
    std::string json_;
    bool read_from_file_;
    mutable bool initialized_ = false;
    Error LazyInitialize() const noexcept;
    Error embedded_error_ = nullptr;

    hidra2::Error GetValue(const std::string& name, ValueType type, rapidjson::Value* val)const noexcept;
};

}
#endif //HIDRA2_RAPID_JSON_H
