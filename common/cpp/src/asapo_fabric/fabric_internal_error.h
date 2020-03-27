#ifndef ASAPO_FABRICERRORCONVERTER_H
#define ASAPO_FABRICERRORCONVERTER_H

#include <common/error.h>

namespace asapo {
namespace fabric {

/**
 * internalStatusCode must be a negative number
 * (Which all libfabric api calls usually return in an error case
 */
Error ErrorFromFabricInternal(const std::string& where, int internalStatusCode);

}
}

#endif //ASAPO_FABRICERRORCONVERTER_H
