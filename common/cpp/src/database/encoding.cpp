#include "encoding.h"
#include <regex>
#include <string.h>
#include <stdio.h>

namespace asapo {

bool ShouldEscape(char c, bool db) {
    if (c == '$' || c == ' ') {
        return true;
    }
    if (!db) {
        return false;
    }

    switch (c) {
        case '/':
        case '\\':
        case '.':
        case '"':return true;
    }
    return false;
}

const std::string upperhex = "0123456789ABCDEF";

std::string Escape(const std::string &s, bool db) {
    auto hexCount = 0;
    for (auto i = 0; i < s.size(); i++) {
        char c = s[i];
        if (ShouldEscape(c, db)) {
            hexCount++;
        }
    }

    if (hexCount == 0) {
        return s;
    }

    char t[s.size() + 2 * hexCount + 1];
    t[s.size() + 2 * hexCount] = 0;
    auto j = 0;
    for (auto i = 0; i < s.size(); i++) {
        auto c = s[i];
        if (ShouldEscape(c, db)) {
            t[j] = '%';
            t[j + 1] = upperhex[c >> 4];
            t[j + 2] = upperhex[c & 15];
            j += 3;

        } else {
            t[j] = c;
            j++;
        }
    }
    return t;
}


inline int ishex(int x)
{
    return	(x >= '0' && x <= '9')	||
        (x >= 'a' && x <= 'f')	||
        (x >= 'A' && x <= 'F');
}

int decode(const char *s, char *dec)
{
    char *o;
    const char *end = s + strlen(s);
    int c;

    for (o = dec; s <= end; o++) {
        c = *s++;
//        if (c == '+') c = ' ';
        if (c == '%' && (	!ishex(*s++)	||
            !ishex(*s++)	||
            !sscanf(s - 2, "%2x", &c)))
            return -1;
        if (dec) *o = c;
    }

    return o - dec;
}


std::string EncodeDbName(const std::string &dbname) {
    return Escape(dbname, true);
}

std::string EncodeColName(const std::string &colname) {
    return Escape(colname, false);
}

std::string DecodeName(const std::string &name) {
    char decoded[name.size()];
    auto res = decode(name.c_str(),decoded);
    return res>=0?decoded:"";
}

std::string EscapeQuery(const std::string& query) {
    std::regex specialChars { R"([-[\]{}()*+?\\.,\^$|#\s])" };
    return std::regex_replace( query, specialChars, std::string(R"(\$&)" ));
}

}
