#ifndef ASAPO_TESTING_H
#define ASAPO_TESTING_H

#include <asapo/common/error.h>
#include <string>

namespace asapo {

void _M_INTERNAL_PrintComment(const std::string& comment);
void _M_INTERNAL_ThrowError(const std::string& expected, const std::string& got, const std::string& comment);

void _M_AssertTrue(bool value, const std::string& comment);
void _M_AssertContains(const std::string& whole, const std::string& sub, const std::string& comment);

void _M_AssertEq(const std::string& expected, const std::string& got, const std::string& comment);
void _M_AssertEq(uint64_t expected, uint64_t got, const std::string& comment);

// Error checks
void _M_AssertEq(const Error& expected, const Error& got, const std::string& comment);
// decltype(nullptr) == nullptr_t but we are using an old bamboo compiler that does not know this
void _M_AssertEq(const decltype(nullptr)& expected, const Error& got, const std::string& comment);
template<class ErrorTemplateType>
inline void _M_AssertEq(const ErrorTemplateType& expected, const Error& got, const std::string& comment) {
    _M_INTERNAL_PrintComment(comment);
    if (expected != got) {
        _M_INTERNAL_ThrowError(expected.Text(), got ? got->Explain() : "No error", comment);
    }
}

// Function that helps to convert the __LINE__ to a string
#define _M_INTERNAL_TO_STRING_WRAPPER(x) #x
#define _M_INTERNAL_TO_STRING(x) _M_INTERNAL_TO_STRING_WRAPPER(x)
#define _M_INTERNAL_COMMENT_PREFIX "Line " _M_INTERNAL_TO_STRING(__LINE__) ": "

// These macros are handling optional arguments
// https://stackoverflow.com/a/3048361/3338196
#define _M_AssertEq_2_ARGS(e, g) \
    asapo::_M_AssertEq(e, g, _M_INTERNAL_COMMENT_PREFIX "Expect " # g " to be " # e)
#define _M_AssertEq_3_ARGS(e, g, c) \
    asapo::_M_AssertEq(e, g, std::string(_M_INTERNAL_COMMENT_PREFIX) + std::string(c))

#define _M_AssertContains_2_ARGS(whole, sub) \
    asapo::_M_AssertContains(whole, sub, _M_INTERNAL_COMMENT_PREFIX "Expect " # whole " to contain substring " # sub)
#define _M_AssertContains_3_ARGS(whole, sub, c) \
    asapo::_M_AssertContains(whole, sub, std::string(_M_INTERNAL_COMMENT_PREFIX) + std::string(c))

#define _M_AssertTrue_1_ARGS(value) \
    asapo::_M_AssertTrue(value, _M_INTERNAL_COMMENT_PREFIX "Expect " # value " to be true")
#define _M_AssertTrue_2_ARGS(value, c) \
    asapo::_M_AssertTrue(value, std::string(_M_INTERNAL_COMMENT_PREFIX) + std::string(c))

#define _M_GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define _M_MACRO_CHOOSER_2_3(func, ...) _M_GET_4TH_ARG(__VA_ARGS__,  _ ## func ## _3_ARGS, _ ## func ## _2_ARGS, )
#define _M_GET_3TH_ARG(arg1, arg2, arg3, ...) arg3
#define _M_MACRO_CHOOSER_1_2(func, ...) _M_GET_3TH_ARG(__VA_ARGS__,  _ ## func ## _2_ARGS, _ ## func ## _1_ARGS, )

// MSVC Fix... >:(
// https://stackoverflow.com/a/5134656/3338196
#define _M_EXPAND( x ) x

// Calls available to the user
// Can be used like the real _M_Assert... functions, the comment is optional
#define M_AssertEq(...) _M_EXPAND(_M_MACRO_CHOOSER_2_3(M_AssertEq, __VA_ARGS__)(__VA_ARGS__))
#define M_AssertContains(...) _M_EXPAND(_M_MACRO_CHOOSER_2_3(M_AssertContains, __VA_ARGS__)(__VA_ARGS__))
#define M_AssertTrue(...) _M_EXPAND(_M_MACRO_CHOOSER_1_2(M_AssertTrue, __VA_ARGS__)(__VA_ARGS__))

}

#endif //ASAPO_TESTING_H
