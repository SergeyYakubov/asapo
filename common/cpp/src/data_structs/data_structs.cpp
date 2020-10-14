#include "common/data_structs.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>


#include "json_parser/json_parser.h"
#include "preprocessor/definitions.h"

using std::chrono::high_resolution_clock;

#ifdef _WIN32
#define timegm _mkgmtime
#endif


namespace asapo {

const std::string SourceCredentials::kDefaultStream = "detector";
const std::string SourceCredentials::kDefaultBeamline = "auto";
const std::string SourceCredentials::kDefaultBeamtimeId = "auto";

std::string GetStringFromSourceType(SourceType type) {
    switch (type) {
        case SourceType::kRaw:return "raw";
        case SourceType::kProcessed:return "processed";
    }
}

Error GetSourceTypeFromString(std::string stype,SourceType *type) {
    Error err;
    if (stype=="raw") {
        *type = SourceType::kRaw;
        return nullptr;
    } else if (stype=="processed") {
        *type = SourceType::kProcessed;
        return nullptr;
    } else {
        return TextError("cannot parse/wrong source type: "+stype);
    }
}

std::string FileInfo::Json() const {
    auto nanoseconds_from_epoch = NanosecsEpochFromTimePoint(timestamp);
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
                    "\"timestamp\":" + std::to_string(nanoseconds_from_epoch) + ","
                    "\"source\":\"" + source + "\","
                    "\"buf_id\":" + std::to_string(buf_id_int) + ","
                    "\"meta\":" + (metadata.size() == 0 ? std::string("{}") : metadata)
                    + "}";
    return s;
}

std::chrono::high_resolution_clock::time_point TimePointfromNanosec(uint64_t nanoseconds_from_epoch){
    std::chrono::nanoseconds ns = std::chrono::nanoseconds {nanoseconds_from_epoch};
    return std::chrono::high_resolution_clock::time_point
        {std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(ns)};
}

bool TimeFromJson(const JsonStringParser& parser, const std::string& name, std::chrono::high_resolution_clock::time_point* val) {
    uint64_t nanoseconds_from_epoch;
    if (parser.GetUInt64(name, &nanoseconds_from_epoch)) {
        return false;
    }
    *val = TimePointfromNanosec(nanoseconds_from_epoch);
    return true;
}

bool DataSet::SetFromJson(const std::string& json_string) {
    auto old = *this;

    auto parser = JsonStringParser(std::move(json_string));

    std::vector<std::string> vec_fi_endcoded;
    Error parse_err;
    (parse_err = parser.GetArrayRawStrings("images", &vec_fi_endcoded)) ||
    (parse_err = parser.GetUInt64("_id", &id));
    if (parse_err) {
        *this = old;
        return false;
    }
    for (auto fi_encoded : vec_fi_endcoded) {
        FileInfo fi;
        if (!fi.SetFromJson(fi_encoded)) {
            *this = old;
            return false;
        }
        content.emplace_back(fi);
    }
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
            !TimeFromJson(parser, "timestamp", &timestamp)) {
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

uint64_t EpochNanosecsFromNow() {
    return NanosecsEpochFromTimePoint(high_resolution_clock::now());
}

uint64_t NanosecsEpochFromTimePoint(std::chrono::high_resolution_clock::time_point time_point) {
    return (uint64_t) std::chrono::duration_cast<std::chrono::nanoseconds>(time_point.time_since_epoch()).count();
}

std::string StreamInfo::Json(bool add_last_id) const {
    auto nanoseconds_from_epoch = NanosecsEpochFromTimePoint(timestamp);
    return (add_last_id?"{\"lastId\":" + std::to_string(last_id) + ",":"{")+
                    "\"name\":\"" + name + "\","
                    "\"timestamp\":" + std::to_string(nanoseconds_from_epoch)
                    + "}";
}

bool StreamInfo::SetFromJson(const std::string& json_string,bool read_last_id) {
    auto old = *this;
    JsonStringParser parser(json_string);
    uint64_t id;
    if ((read_last_id?parser.GetUInt64("lastId", &last_id):nullptr) ||
        parser.GetString("name", &name) ||
        !TimeFromJson(parser, "timestamp", &timestamp)) {
        *this = old;
        return false;
    }
    return true;
}

}
