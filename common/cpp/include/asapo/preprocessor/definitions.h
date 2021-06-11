#ifndef ASAPO_DEFINITIONS_H
#define ASAPO_DEFINITIONS_H

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#define FINAL
#else
#define VIRTUAL
#define FINAL final
#endif

#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated(msg))
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(msg)
#endif



namespace  asapo {
const char kPathSeparator =
#ifdef WIN32
    '\\';
#else
    '/';
#endif
}

#endif //ASAPO_DEFINITIONS_H
