#include "asapo/json_parser/json_parser.h"
#include "rapid_json.h"
#include "asapo/io/io_factory.h"

namespace asapo {

JsonParser::~JsonParser() {

}

JsonParser::JsonParser(const std::string& json, const std::unique_ptr<IO>* io ) :
    default_io_{GenerateDefaultIO()}, rapid_json_{new RapidJson(json, io != nullptr ? io : & default_io_)} {
}

JsonParser::JsonParser(const std::string& json) : rapid_json_{new RapidJson(json, nullptr)} {
}

Error JsonParser::GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept {
    return rapid_json_->GetArrayUInt64(name, val);

}

Error JsonParser::GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept {
    return rapid_json_->GetArrayString(name, val);
}


Error JsonParser::GetBool(const std::string& name, bool* val) const noexcept {
    return rapid_json_->GetBool(name, val);
}


Error JsonParser::GetUInt64(const std::string& name, uint64_t* val) const noexcept {
    return rapid_json_->GetUInt64(name, val);
}

Error JsonParser::GetString(const std::string& name, std::string* val) const noexcept {
    return rapid_json_->GetString(name, val);
}

JsonParser JsonParser::Embedded(const std::string& name) const noexcept {
    RapidJson* rapid_json = new RapidJson(*rapid_json_.get(), name) ;
    return JsonParser(rapid_json);
}

JsonParser::JsonParser(RapidJson* json) : rapid_json_{json} {
}

JsonParser::JsonParser(JsonParser&& other) {
    rapid_json_ = std::move(other.rapid_json_);
}

Error JsonParser::GetRawString(std::string* val) const noexcept {
    return rapid_json_->GetRawString(val);
}

Error JsonParser::GetArrayRawStrings(const std::string& name, std::vector<std::string>* val) const noexcept {
    return rapid_json_->GetArrayRawStrings(name, val);
}
Error JsonParser::GetFlattenedString(const std::string& prefix, const std::string& separator,
                                     std::string* val) const noexcept {
    return rapid_json_->GetFlattenedString(prefix, separator, val);
}

}



