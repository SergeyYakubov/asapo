#ifndef ASAPO_RAPID_JSON_H
#define ASAPO_RAPID_JSON_H

#include "rapidjson/document.h"
#include "asapo/common/error.h"
#include "asapo/io/io.h"


namespace asapo {

enum class ValueType {
    kUint64,
    kInt64,
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
    Error GetArrayRawStrings(const std::string& name, std::vector<std::string>* val) const noexcept;
    Error GetRawString(std::string* val) const noexcept;
    Error GetFlattenedString(const std::string& prefix, const std::string& separator, std::string* val)const noexcept;
  private:
    Error GetInt64(const std::string& name, int64_t* val) const noexcept;
    const std::unique_ptr<IO>* io__;
    mutable rapidjson::Document doc_;
    mutable rapidjson::Value object_;
    mutable rapidjson::Value* object_p_;
    std::string json_;
    mutable bool initialized_ = false;
    Error LazyInitialize() const noexcept;
    Error CheckValueType(const std::string& name, ValueType type, const rapidjson::Value* val) const;
    Error embedded_error_ = nullptr;

    asapo::Error GetValuePointer(const std::string& name, ValueType type, rapidjson::Value** val)const noexcept;
};

}
#endif //ASAPO_RAPID_JSON_H
