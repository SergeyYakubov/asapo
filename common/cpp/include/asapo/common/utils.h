#ifndef ASAPO_COMMON_CPP_INCLUDE_ASAPO_COMMON_UTILS_H_
#define ASAPO_COMMON_CPP_INCLUDE_ASAPO_COMMON_UTILS_H_

#include <iomanip>
#include <sstream>


namespace asapo {

inline std::string EscapeJson(const std::string& s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        switch (*c) {
            case '"':
                o << "\\\"";
                break;
            case '\\':
                o << "\\\\";
                break;
            case '\b':
                o << "\\b";
                break;
            case '\f':
                o << "\\f";
                break;
            case '\n':
                o << "\\n";
                break;
            case '\r':
                o << "\\r";
                break;
            case '\t':
                o << "\\t";
                break;
            default:
                if ('\x00' <= *c && *c <= '\x1f') {
                    o << "\\u"
                      << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
                } else {
                    o << *c;
                }
        }
    }
    return o.str();
}

}

#endif //ASAPO_COMMON_CPP_INCLUDE_ASAPO_COMMON_UTILS_H_
