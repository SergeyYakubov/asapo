#ifndef ASAPO_FABRIC_INTERNAL_IMPL_COMMON_H
#define ASAPO_FABRIC_INTERNAL_IMPL_COMMON_H

/*
 * This file contains common features used in ASAPO's integration of libfabric.
 * Only include this file into *.cpp files, never in *.h files
 */

#ifndef EXPECTED_FI_VERSION
#define EXPECTED_FI_VERSION FI_VERSION(1, 9)
#endif

#pragma pack(push, 1)
struct HandshakePayload {
    // Hostnames can be up to 256 Bytes long. We also need to store the port number.
    char hostnameAndPort[512];
};

#pragma pack(pop)

#define TODO_UNKNOWN_ADDRESS (FI_ADDR_NOTAVAIL - 1)

#endif //ASAPO_FABRIC_INTERNAL_IMPL_COMMON_H
