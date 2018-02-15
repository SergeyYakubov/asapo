#include "json_parser/json_parser.h"
#include "rapid_json.h"

namespace hidra2 {


JsonParser::~JsonParser() {

}

JsonParser::JsonParser(const std::string& json, bool read_from_file) : rapid_json_{new RapidJson(json, read_from_file)} {
}

Error JsonParser::GetUInt64(const std::string& name, uint64_t* val) const noexcept {
    return rapid_json_->GetUInt64(name, val);
}

Error JsonParser::GetString(const std::string& name, std::string* val) const noexcept {
    return rapid_json_->GetString(name, val);
}

}



