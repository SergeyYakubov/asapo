#ifndef ASAPO_DEFINITIONS_H
#define ASAPO_DEFINITIONS_H

#ifdef UNIT_TESTS
#define ASAPO_VIRTUAL virtual
#define ASAPO_FINAL
#else
#define ASAPO_VIRTUAL
#define ASAPO_FINAL final
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
