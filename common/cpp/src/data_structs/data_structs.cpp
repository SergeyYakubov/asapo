#include "asapo/common/data_structs.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

#include "asapo/json_parser/json_parser.h"
#include "asapo/preprocessor/definitions.h"

using std::chrono::system_clock;

#ifdef _WIN32
#define timegm _mkgmtime
#endif

namespace asapo {

const std::string SourceCredentials::kDefaultDataSource = "detector";
const std::string SourceCredentials::kDefaultBeamline = "auto";
const std::string SourceCredentials::kDefaultBeamtimeId = "auto";

std::string GetStringFromSourceType(SourceType type) {
    switch (type) {
        case SourceType::kRaw:return "raw";
        case SourceType::kProcessed:return "processed";
    }
    return "unknown";
}

Error GetSourceTypeFromString(std::string stype, SourceType* type) {
    Error err;
    if (stype == "raw") {
        *type = SourceType::kRaw;
        return nullptr;
    } else if (stype == "processed") {
        *type = SourceType::kProcessed;
        return nullptr;
    } else {
        return TextError("cannot parse/wrong source type: " + stype);
    }
}

std::string MessageMeta::Json() const {
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
                                                                                                                "\"timestamp\":"
        + std::to_string(nanoseconds_from_epoch) + ","
                                                   "\"source\":\"" + source + "\","
                                                                              "\"buf_id\":" + std::to_string(buf_id_int) + ","
         "\"dataset_substream\":" + std::to_string(dataset_substream) + ","
          "\"meta\":" + (metadata.size() == 0 ? std::string("{}") : metadata)
        + "}";
    return s;
}

std::chrono::system_clock::time_point TimePointfromNanosec(uint64_t nanoseconds_from_epoch) {
    std::chrono::nanoseconds ns = std::chrono::nanoseconds{nanoseconds_from_epoch};
    return std::chrono::system_clock::time_point
        {std::chrono::duration_cast<std::chrono::system_clock::duration>(ns)};
}

bool TimeFromJson(const JsonStringParser &parser, const std::string &name, std::chrono::system_clock::time_point* val) {
    uint64_t nanoseconds_from_epoch;
    if (parser.GetUInt64(name, &nanoseconds_from_epoch)) {
        return false;
    }
    *val = TimePointfromNanosec(nanoseconds_from_epoch);
    return true;
}

bool DataSet::SetFromJson(const std::string &json_string) {
    auto old = *this;

    auto parser = JsonStringParser(std::move(json_string));

    std::vector<std::string> vec_fi_endcoded;
    Error parse_err;
    (parse_err = parser.GetArrayRawStrings("messages", &vec_fi_endcoded)) ||
        (parse_err = parser.GetUInt64("size", &expected_size)) ||
        (parse_err = parser.GetUInt64("_id", &id));
    if (parse_err) {
        *this = old;
        return false;
    }
    for (auto fi_encoded : vec_fi_endcoded) {
        MessageMeta fi;
        if (!fi.SetFromJson(fi_encoded)) {
            *this = old;
            return false;
        }
        content.emplace_back(fi);
    }
    return true;
}

bool MessageMeta::SetFromJson(const std::string &json_string) {
    auto old = *this;

    JsonStringParser parser(json_string);

    if (parser.GetUInt64("_id", &id) ||
        parser.GetUInt64("size", &size) ||
        parser.GetString("name", &name) ||
        parser.GetString("source", &source) ||
        parser.GetUInt64("buf_id", &buf_id) ||
        parser.GetUInt64("dataset_substream", &dataset_substream) ||
        parser.Embedded("meta").GetRawString(&metadata) ||
        !TimeFromJson(parser, "timestamp", &timestamp)) {
        *this = old;
        return false;
    }

//ignore error if meta not found

    return true;
}

std::string MessageMeta::FullName(const std::string &base_path) const {
    std::string full_name;
    full_name = base_path.empty() ? "" : base_path + kPathSeparator;
    return full_name + name;
}

uint64_t EpochNanosecsFromNow() {
    return NanosecsEpochFromTimePoint(system_clock::now());
}

uint64_t NanosecsEpochFromTimePoint(std::chrono::system_clock::time_point time_point) {
    return (uint64_t) std::chrono::duration_cast<std::chrono::nanoseconds>(time_point.time_since_epoch()).count();
}

std::string StreamInfo::Json() const {
    auto nanoseconds_from_epoch = NanosecsEpochFromTimePoint(timestamp_created);
    auto nanoseconds_from_epoch_le = NanosecsEpochFromTimePoint(timestamp_lastentry);
    return ("{\"lastId\":" + std::to_string(last_id) + ","  +
        "\"name\":\"" + name + "\",\"timestampCreated\":" + std::to_string(nanoseconds_from_epoch)
        + std::string(",") + "\"timestampLast\":" + std::to_string(nanoseconds_from_epoch_le)
        + ",\"finished\":" + (finished?"true":"false")+ ",\"nextStream\":\"" + next_stream)
        + "\"}";
}

bool StreamInfo::SetFromJson(const std::string &json_string) {
    auto old = *this;
    JsonStringParser parser(json_string);
    if (parser.GetUInt64("lastId", &last_id) ||
        parser.GetBool("finished", &finished) ||
        parser.GetString("nextStream", &next_stream) ||
        !TimeFromJson(parser, "timestampLast", &timestamp_lastentry) ||
        parser.GetString("name", &name) ||
        !TimeFromJson(parser, "timestampCreated", &timestamp_created)) {
        *this = old;
        return false;
    }
    return true;
}

std::string IsoDateFromEpochNanosecs(uint64_t time_from_epoch_nanosec) {
    auto tp = TimePointfromNanosec(time_from_epoch_nanosec);
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
    auto pos = date_time.find_first_of('.');
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

    system_clock::time_point tp = system_clock::from_time_t(timegm(&tm));
    uint64_t ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(tp).
        time_since_epoch().count();

    ns = ns + uint64_t(frac * 1000000000);

    return ns > 0 ? ns : 1;
}

}
