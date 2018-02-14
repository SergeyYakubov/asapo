#ifndef HIDRA2_RAPID_JSON_H
#define HIDRA2_RAPID_JSON_H

#include "rapidjson/document.h"
#include "common/error.h"

namespace hidra2 {

enum class ValueType {
    kUint64,
    kString
};


class RapidJson {
  public:
    RapidJson(const std::string& json);
    Error GetUInt64(const std::string& name, uint64_t* val) const noexcept;
    Error GetString(const std::string& name, std::string* val) const noexcept;
  private:
    mutable rapidjson::Document doc_;
    std::string json_;
    mutable bool initialized_ = false;
    bool LazyInitialize() const noexcept;
    hidra2::Error GetValue(const std::string& name, ValueType type, rapidjson::Value* val)const noexcept;

};

}
#endif //HIDRA2_RAPID_JSON_H
