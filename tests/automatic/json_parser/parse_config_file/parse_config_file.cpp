#include "json_parser/json_parser.h"

#include <iostream>
#include <string>
#include <vector>

#include "testing.h"

using namespace asapo;

struct Settings {
    uint64_t port;
    std::string server;
    std::vector<uint64_t> intarray;
    std::vector<std::string> stringarray;
    std::string embedded_string;
    uint64_t some_int;
};

void AssertSettings(const Settings& settings) {
    M_AssertEq(10, settings.port);

    M_AssertEq(3, settings.intarray.size());
    M_AssertEq(1, settings.intarray[0]);
    M_AssertEq(2, settings.intarray[1]);
    M_AssertEq(3, settings.intarray[2]);

    M_AssertEq(3, settings.stringarray.size());
    M_AssertEq("s1", settings.stringarray[0]);
    M_AssertEq("s2", settings.stringarray[1]);
    M_AssertEq("s3", settings.stringarray[2]);

    M_AssertEq("some_string", settings.server);
    M_AssertEq("embedded_string", settings.embedded_string);
    M_AssertEq(20, settings.some_int);
}

Settings Parse(const std::string& fname, Error* err) {

    asapo::JsonFileParser parser(fname);

    Settings settings;

    (*err = parser.GetUInt64("Port", &settings.port)) ||
    (*err = parser.GetArrayString("some_string_array", &settings.stringarray)) ||
    (*err = parser.GetArrayUInt64("some_array", &settings.intarray)) ||
    (*err = parser.Embedded("embedded_stuff").GetString("some_string", &settings.embedded_string)) ||
    (*err = parser.Embedded("embedded_stuff").GetUInt64("some_int", &settings.some_int)) ||
    (*err = parser.GetString("server", &settings.server)
    );

    return settings;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto fname = std::string{argv[1]};
    auto res = std::string{argv[2]};

    Error err;
    auto settings = Parse(fname, &err);

    if (err) {
        return ! (res == "FAIL");
    } else {
        AssertSettings(settings);
        return ! (res == "OK");
    }
}


