#include "common/data_structs.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>


#include "json_parser/json_parser.h"
#include "preprocessor/definitions.h"

using std::chrono::system_clock;

#ifdef _WIN32
#define timegm _mkgmtime
#endif


namespace asapo {

const std::string SourceCredentials::kDefaultStream = "detector";
const std::string SourceCredentials::kDefaultBeamline = "auto";
const std::string SourceCredentials::kDefaultBeamtimeId = "auto";


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

std::string IsoDateFromEpochNanosecs(uint64_t time_from_epoch_nanosec) {
    std::chrono::nanoseconds ns = std::chrono::nanoseconds {time_from_epoch_nanosec};
    auto tp = std::chrono::system_clock::time_point
    {std::chrono::duration_cast<std::chrono::system_clock::duration>(ns)};
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm timetm = *std::gmtime(&time);
    std::stringstream ssTp;
    auto zz = time_from_epoch_nanosec % 1000000000;

    std::string s;
    char buff[100];

    sprintf(buff, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2d", timetm.tm_year + 1900, timetm.tm_mon + 1, timetm.tm_mday,
            timetm.tm_hour, timetm.tm_min, timetm.tm_sec);
    if (zz > 0) {
        sprintf(buff + 19, ".%.9ld", zz);
    }

    return buff;
}

uint64_t NanosecsEpochFromISODate(std::string date_time) {
    double frac = 0;
    int pos = date_time.find_first_of('.');
    if (pos != std::string::npos) {
        std::string frac_str = date_time.substr(pos);
        if (sscanf(frac_str.c_str(), "%lf", &frac) != 1) {
            return 0;
        }
        date_time = date_time.substr(0, pos);
    }

    std::tm tm{};

    int year, month, day, hour, minute, second;
    hour = 0;
    minute = 0;
    second = 0;

    auto n = sscanf(date_time.c_str(), "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    if (!(year >= 1970 && month >= 1 && month <= 12 && day >= 1 && day <= 31) || (n != 3 && n != 6)) {
        return 0;
    }
    if ((n == 3 && date_time.size() != 10) || (n == 6 && date_time.size() != 19)) {
        return 0;
    }

    tm.tm_sec = second;
    tm.tm_min = minute;
    tm.tm_hour = hour;
    tm.tm_mday = day;
    tm.tm_mon = month - 1;
    tm.tm_year = year - 1900;

    system_clock::time_point tp = system_clock::from_time_t (timegm(&tm));
    uint64_t ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(tp).
                  time_since_epoch().count();

    ns = ns + uint64_t(frac * 1000000000) ;

    return ns > 0 ? ns : 1;
}

uint64_t EpochNanosecsFromNow() {
    return (uint64_t) std::chrono::duration_cast<std::chrono::nanoseconds>(system_clock::now().time_since_epoch()).count();
}

std::string StreamInfo::Json() const {
    std::string s = "{\"lastId\":" + std::to_string(last_id) + "}";
    return s;
}

bool StreamInfo::SetFromJson(const std::string& json_string) {
    JsonStringParser parser(json_string);
    uint64_t id;
    if (parser.GetUInt64("lastId", &id)) {
        return false;
    }
    last_id = id;
    return true;
}


}
