#include "json_parser/json_parser.h"
#include "rapid_json.h"

namespace hidra2 {


JsonParser::~JsonParser() {

}

JsonParser::JsonParser(const std::string& json, bool read_from_file) : rapid_json_{new RapidJson(json, read_from_file)} {
}

Error JsonParser::GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept {
    return rapid_json_->GetArrayUInt64(name, val);

}

Error JsonParser::GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept {
    return rapid_json_->GetArrayString(name, val);
}


Error JsonParser::GetUInt64(const std::string& name, uint64_t* val) const noexcept {
    return rapid_json_->GetUInt64(name, val);
}

Error JsonParser::GetString(const std::string& name, std::string* val) const noexcept {
    return rapid_json_->GetString(name, val);
}

JsonParser JsonParser::Embedded(const std::string& name) const noexcept {
    RapidJson* rapid_json = new RapidJson(*rapid_json_.get(), name);
    return JsonParser(rapid_json);
}

JsonParser::JsonParser(RapidJson* json) : rapid_json_{json} {
}

JsonParser::JsonParser(JsonParser&& other) {
    rapid_json_ = std::move(other.rapid_json_);
}

}


