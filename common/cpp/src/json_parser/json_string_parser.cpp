
#include "json_parser/json_parser.h"
#include "rapid_json.h"

namespace hidra2 {


JsonStringParser::~JsonStringParser() {

}

JsonStringParser::JsonStringParser(const std::string& json) : rapid_json_{new RapidJson(json)} {
}

Error JsonStringParser::GetUInt64(const std::string& name, uint64_t* val) const noexcept {
    return rapid_json_->GetUInt64(name, val);
}

Error JsonStringParser::GetString(const std::string& name, std::string* val) const noexcept {
    return rapid_json_->GetString(name, val);
}

}



