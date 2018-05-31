#ifndef ASAPO_DEFINITIONS_H
#define ASAPO_DEFINITIONS_H

#ifdef UNIT_TESTS
#define VIRTUAL virtual
#else
#define VIRTUAL
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
