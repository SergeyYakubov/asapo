#ifndef HIDRA2_JSON_PARSER_H
#define HIDRA2_JSON_PARSER_H

#include <string>
#include <memory>

#include "common/error.h"

namespace hidra2 {

class RapidJson;

class JsonParser {
  public:
    JsonParser(const std::string& json, bool read_from_file);
    ~JsonParser();
    Error GetUInt64(const std::string& name, uint64_t* val) const noexcept;
    Error GetString(const std::string& name, std::string* val) const noexcept;
  private:
    std::unique_ptr<RapidJson>  rapid_json_;
};

}





#endif //HIDRA2_JSON_PARSER_H
