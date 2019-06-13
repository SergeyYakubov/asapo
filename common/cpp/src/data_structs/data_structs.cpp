#include "common/data_structs.h"

#include "json_parser/json_parser.h"

#include "preprocessor/definitions.h"

#include <iostream>

namespace asapo {


std::string FileInfo::Json() const {
    auto nanoseconds_from_epoch = std::chrono::time_point_cast<std::chrono::nanoseconds>(modify_date).
                                  time_since_epoch().count();

    std::string x = name;
//todo: change this - use / when sending file from windows
#ifdef WIN32
    std::string old {kPathSeparator};
    std::string rep = old + kPathSeparator;
    int pos = 0;
    while ((pos = x.find(old, pos)) != std::string::npos) {
        x.replace(pos, old.length(), rep);
        pos += rep.length();
    }
#endif


    int64_t buf_id_int = static_cast<int64_t>(buf_id);
    std::string s = "{\"_id\":" + std::to_string(id) + ","
                    "\"size\":" + std::to_string(size) + ","
                    "\"name\":\"" + x + "\","
                    "\"lastchange\":" + std::to_string(nanoseconds_from_epoch) + ","
                    "\"source\":\"" + source + "\","
                    "\"buf_id\":" + std::to_string(buf_id_int) + ","
                    "\"meta\":" + (metadata.size() == 0 ? std::string("{}") : metadata)
                    + "}";
    return s;
}

bool TimeFromJson(const JsonStringParser& parser, const std::string name, std::chrono::system_clock::time_point* val) {
    uint64_t nanoseconds_from_epoch;
    if (parser.GetUInt64(name, &nanoseconds_from_epoch)) {
        return false;
    }

    std::chrono::nanoseconds ns = std::chrono::nanoseconds {nanoseconds_from_epoch};
    *val = std::chrono::system_clock::time_point
    {std::chrono::duration_cast<std::chrono::system_clock::duration>(ns)};

    return true;
}



bool FileInfo::SetFromJson(const std::string& json_string) {
    auto old = *this;

    JsonStringParser parser(json_string);

    if (parser.GetUInt64("_id", &id) ||
            parser.GetUInt64("size", &size) ||
            parser.GetString("name", &name) ||
            parser.GetString("source", &source) ||
            parser.GetUInt64("buf_id", &buf_id) ||
            parser.Embedded("meta").GetRawString(&metadata) ||
            !TimeFromJson(parser, "lastchange", &modify_date)) {
        *this = old;
        return false;
    }

//ignore error if meta not found

    return true;
}

std::string FileInfo::FullName(const std::string& base_path) const  {
    std::string full_name;
    full_name = base_path.empty() ? "" : base_path + kPathSeparator;
    return full_name + name;
}

}
