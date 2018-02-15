#ifndef HIDRA2_RAPID_JSON_H
#define HIDRA2_RAPID_JSON_H

#include "rapidjson/document.h"
#include "common/error.h"
#include "system_wrappers/io.h"


namespace hidra2 {

enum class ValueType {
    kUint64,
    kString
};

class RapidJson {
  public:
    RapidJson(const std::string& json, bool read_from_file);
    Error GetUInt64(const std::string& name, uint64_t* val) const noexcept;
    Error GetString(const std::string& name, std::string* val) const noexcept;
    std::unique_ptr<IO> io__;
  private:
    mutable rapidjson::Document doc_;
    std::string json_;
    bool read_from_file_;
    mutable bool initialized_ = false;
    Error LazyInitialize() const noexcept;
    hidra2::Error GetValue(const std::string& name, ValueType type, rapidjson::Value* val)const noexcept;
};

}
#endif //HIDRA2_RAPID_JSON_H
