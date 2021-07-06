#ifndef ASAPO_JSON_PARSER_H
#define ASAPO_JSON_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <string>

#include "asapo/common/error.h"
#include "asapo/io/io.h"

namespace asapo {

class RapidJson;

class JsonParser {
  public:
    Error GetUInt64(const std::string& name, uint64_t* val) const noexcept;
    Error GetBool(const std::string& name, bool* val) const noexcept;
    Error GetString(const std::string& name, std::string* val) const noexcept;
    Error GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept;
    Error GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept;
    Error GetArrayRawStrings(const std::string& name, std::vector<std::string>* val) const noexcept;
    Error GetRawString(std::string* val) const noexcept;
    Error GetFlattenedString(const std::string& prefix, const std::string& separator, std::string* val) const noexcept;

    JsonParser Embedded(const std::string& name) const noexcept;
    ~JsonParser();
  protected:
    JsonParser(const std::string& json, const std::unique_ptr<IO>* io); // nullptr as second parameter will use default IO
    JsonParser(const std::string& json);
    JsonParser(JsonParser&& other);
  private:
    std::unique_ptr<IO> default_io_;
    std::unique_ptr<RapidJson>  rapid_json_;
    JsonParser(RapidJson* rapid_json_);

};


class JsonStringParser : public JsonParser {
  public:
    JsonStringParser(const std::string& json): JsonParser(json) {};
};


class JsonFileParser : public JsonParser {
  public:
    JsonFileParser(const std::string& json, const std::unique_ptr<IO>* io = nullptr): JsonParser(json, io) {};
};


}





#endif //ASAPO_JSON_PARSER_H
