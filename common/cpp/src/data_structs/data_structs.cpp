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

std::string IsoDateFromEpochNanosecs(uint64_t time_from_epoch_nanosec) {
    std::chrono::nanoseconds ns = std::chrono::nanoseconds {time_from_epoch_nanosec};
    auto tp = std::chrono::system_clock::time_point
    {std::chrono::duration_cast<std::chrono::system_clock::duration>(ns)};
    std::time_t time = std::chrono::system_clock::to_time_t(tp);
    std::tm timetm = *std::gmtime(&time);
    std::stringstream ssTp;
    auto zz = time_from_epoch_nanosec % 1000000000;
    ssTp << std::put_time(&timetm, "%Y-%m-%dT%H:%M:%S");
    if (zz > 0) {
        ssTp << "." << std::setw(9) << std::setfill('0') << zz;
    }
    return ssTp.str();
}

uint64_t NanosecsEpochFromISODate(std::string date_time) {
    std::istringstream iss{date_time};
    std::tm tm{};
    if (!(iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S"))) {
        return 0;
    }

    system_clock::time_point tp = system_clock::from_time_t (timegm(&tm));
    uint64_t ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(tp).
                  time_since_epoch().count();

    if (iss.eof()) {
        return ns > 0 ? ns : 1;
    }

    double zz = 0;
    if (iss.peek() != '.' || !(iss >> zz)) {
        return 0;
    }

    ns = ns + uint64_t(zz * 1000000000);
    return ns > 0 ? ns : 1;
}

uint64_t EpochNanosecsFromNow() {
    return (uint64_t) std::chrono::duration_cast<std::chrono::nanoseconds>(system_clock::now().time_since_epoch()).count();
}

}


