#ifndef ASAPO_DEPRECATED_H
#define ASAPO_DEPRECATED_H

#if defined(__GNUC__) || defined(__clang__)
#define ASAPO_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
#define ASAPO_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define ASAPO_DEPRECATED(msg)
#endif

#endif //ASAPO_DEPRECATED_H
