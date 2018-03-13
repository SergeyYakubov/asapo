#ifndef HIDRA2_COMMON__NETWORKING_H
#define HIDRA2_COMMON__NETWORKING_H

#include <cstdint>

namespace hidra2 {

typedef uint64_t NetworkRequestId;

enum Opcode : uint8_t {
    kNetOpcodeSendData,

    kNetOpcodeCount,
};

enum NetworkErrorCode : uint16_t {
    NET_ERR__NO_ERROR,
    NET_ERR__FILEID_ALREADY_IN_USE,
    NET_ERR__ALLOCATE_STORAGE_FAILED,
    NET_ERR__INTERNAL_SERVER_ERROR = 65535,
};

//TODO need to use an serialization framework to ensure struct consistency on different computers

/**
 * @defgroup RPC
 * RPC always return a response to a corresponding request
 * @{
 */
struct GenericNetworkRequest {
    Opcode              op_code;
    NetworkRequestId    request_id;
};

struct GenericNetworkResponse {
    Opcode              op_code;
    NetworkRequestId    request_id;
    NetworkErrorCode    error_code;
};

struct SendDataRequest : GenericNetworkRequest {
    uint64_t    file_id;
    uint64_t    file_size;
};

/**
 * Possible error codes:
 * - ::NET_ERR__FILENAME_ALREADY_IN_USE
 * - ::NET_ERR__ALLOCATE_STORAGE_FAILED
 */
struct SendDataResponse :  GenericNetworkResponse {
};
/** @} */

}

#endif //HIDRA2_COMMON__NETWORKING_H
