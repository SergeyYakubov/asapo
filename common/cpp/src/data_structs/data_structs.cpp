#include "common/data_structs.h"

#include "rapidjson/document.h"

using namespace rapidjson;

namespace hidra2 {


std::string FileInfo::Json() const {
    auto nanoseconds_from_epoch = std::chrono::time_point_cast<std::chrono::nanoseconds>(modify_date).
                                  time_since_epoch().count();
    std::string s = "{\"_id\":" + std::to_string(id) + ","
                    "\"size\":" + std::to_string(size) + ","
                    "\"base_name\":\"" + base_name + "\","
                    "\"lastchange\":" + std::to_string(nanoseconds_from_epoch) + ","
                    "\"relative_path\":\"" + relative_path + "\"}";
    return s;
}

bool IntFromJson(const Document& d, const std::string name, uint64_t* val) {
    auto iterator = d.FindMember(name.c_str());
    if (iterator == d.MemberEnd()) {
        return false;
    }
    if (iterator->value.IsInt64()) {
        *val = iterator->value.GetInt64();
        return true;
    }
    return false;
}

bool TimeFromJson(const Document& d, const std::string name, std::chrono::system_clock::time_point* val) {
    uint64_t nanoseconds_from_epoch;
    if (!IntFromJson(d, name, &nanoseconds_from_epoch)) {
        return false;
    }

    std::chrono::nanoseconds ns = std::chrono::nanoseconds {nanoseconds_from_epoch};
    *val = std::chrono::system_clock::time_point
    {std::chrono::duration_cast<std::chrono::system_clock::duration>(ns)};

    return true;
}

bool StringFromJson(const Document& d, const std::string name, std::string* val) {
    auto iterator = d.FindMember(name.c_str());
    if (iterator == d.MemberEnd()) {
        return false;
    }
    if (iterator->value.IsString()) {
        *val = iterator->value.GetString();
        return true;
    }
    return false;
}

bool FileInfo::SetFromJson(const std::string& json_string) {
    auto old = *this;

    Document d;
    if ( d.Parse(json_string.c_str()).HasParseError()) {
        return false;
    }

    if (!IntFromJson(d, "_id", &id) ||
            !IntFromJson(d, "size", &size) ||
            !StringFromJson(d, "base_name", &base_name) ||
            !StringFromJson(d, "relative_path", &relative_path) ||
            !TimeFromJson(d, "lastchange", &modify_date)) {
        *this = old;
        return false;
    }

    return true;
}

std::string FileInfo::FullName(const std::string& base_path) {
    std::string full_name;
    full_name = base_path.empty() ? "" : base_path + "/";
    full_name += relative_path.empty() ? "" : relative_path + "/";
    return full_name + base_name;
}

}
