#ifndef HIDRA2_JSON_PARSER_H
#define HIDRA2_JSON_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <string>

#include "common/error.h"

namespace hidra2 {

class RapidJson;

class JsonParser {
  public:
    JsonParser(const std::string& json, bool read_from_file);
    ~JsonParser();
    JsonParser(JsonParser&& other);
    Error GetUInt64(const std::string& name, uint64_t* val) const noexcept;
    Error GetString(const std::string& name, std::string* val) const noexcept;
    Error GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept;
    Error GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept;
    JsonParser Embedded(const std::string& name) const noexcept;
  private:
    std::unique_ptr<RapidJson>  rapid_json_;
    JsonParser(RapidJson* rapid_json_);

};

}





#endif //HIDRA2_JSON_PARSER_H
