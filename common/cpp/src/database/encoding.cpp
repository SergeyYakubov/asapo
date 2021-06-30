#include "encoding.h"
#include <string.h>
#include <stdio.h>
#include <memory>

namespace asapo {

bool ShouldEscape(char c, bool db) {
    if (c == '$' || c == ' ' || c == '%') {
        return true;
    }
    if (!db) {
        return false;
    }

    switch (c) {
    case '/':
    case '\\':
    case '.':
    case '"':
        return true;
    }
    return false;
}

const std::string upperhex = "0123456789ABCDEF";

std::string Escape(const std::string& s, bool db) {
    auto hexCount = 0;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (ShouldEscape(c, db)) {
            hexCount++;
        }
    }

    if (hexCount == 0) {
        return s;
    }

    std::string res;
    res.reserve(s.size() + 2 * hexCount);
    for (size_t i = 0; i < s.size(); i++) {
        auto c = s[i];
        if (ShouldEscape(c, db)) {
            res.push_back('%');
            res.push_back(upperhex[c >> 4]);
            res.push_back(upperhex[c & 15]);
        } else {
            res.push_back(c);
        }
    }
    return res;
}

inline int ishex(int x) {
    return (x >= '0' && x <= '9') ||
           (x >= 'a' && x <= 'f') ||
           (x >= 'A' && x <= 'F');
}

int decode(const char* s, char* dec) {
    char* o;
    const char* end = s + strlen(s);
    int c;

    for (o = dec; s <= end; o++) {
        c = *s++;
//        if (c == '+') c = ' ';
        if (c == '%' && (!ishex(*s++) ||
                         !ishex(*s++) ||
                         !sscanf(s - 2, "%2x", &c)))
            return -1;
        if (dec) *o = c;
    }

    return o - dec;
}

std::string EncodeDbName(const std::string& dbname) {
    return Escape(dbname, true);
}

std::string EncodeColName(const std::string& colname) {
    return Escape(colname, false);
}

std::string DecodeName(const std::string& name) {
    char* decoded = new char[name.size() + 1];
    auto res = decode(name.c_str(), decoded);
    if (res < 0) {
        return "";
    }
    std::string str = std::string{decoded};
    delete[] decoded;
    return str;
}

bool ShouldEscapeQuery(char c) {
    char chars[] = "-[]{}()*+?\\.,^$|#";
    for (size_t i = 0; i < strlen(chars); i++) {
        if (c == chars[i]) {
            return true;
        }
    };
    return false;
}

std::string EscapeQuery(const std::string& s) {
    auto count = 0;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (ShouldEscapeQuery(c)) {
            count++;
        }
    }

    if (count == 0) {
        return s;
    }

    std::string res;
    res.reserve(s.size() + count);
    for (size_t i = 0; i < s.size(); i++) {
        auto c = s[i];
        if (ShouldEscapeQuery(c)) {
            res.push_back('\\');
            res.push_back(c);
        } else {
            res.push_back(c);
        }
    }
    return res;

}

}
