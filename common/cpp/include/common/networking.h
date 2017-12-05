#ifndef HIDRA2__COMMON_NETWORKING_H
#define HIDRA2__COMMON_NETWORKING_H

#include <cstdint>
#include "os.h"

namespace hidra2 {
typedef uint64_t FileReferenceId;

enum OpCode : uint8_t {
    OP_CODE__HELLO,
    OP_CODE__PREPARE_SEND_DATA,
    OP_CODE__SEND_DATA_CHUNK,
};

enum NetworkErrorCode : uint16_t {
    NET_ERR__NO_ERROR,
    NET_ERR__UNSUPPORTED_VERSION,
    NET_ERR__FILENAME_ALREADY_IN_USE,
    NET_ERR__UNKNOWN_REFERENCE_ID,
    NET_ERR__INTERNAL_SERVER_ERROR = 65535,
};

/**
 * @defgroup RPC
 * RPC always return a response to a corresponding request
 * @{
 */
struct GenericNetworkRequest {
    OpCode      op_code;
    uint64_t    request_id;
    char        data[];
};

struct GenericNetworkResponse {
    OpCode              op_code;
    uint64_t            request_id;
    NetworkErrorCode    error_code;
    char                data[];
};

struct HelloRequest {
    uint32_t    client_version;
    OS_TYPE     os;

    //Flags
    bool        is_x64 : 1;
};

/**
 * Possible error codes:
 * - ::NET_ERR__UNSUPPORTED_VERSION
 */
struct HelloResponse {
    uint32_t server_version;
};

struct PrepareSendDataRequest {
    char     filename[255];
    uint64_t file_size;
};

/**
 * Possible error codes:
 * - ::NET_ERR__FILENAME_ALREADY_IN_USE
 */
struct PrepareSendDataResponse {
    FileReferenceId file_reference_id;
};

struct SendDataChunkRequest {
    FileReferenceId file_reference_id;
    uint64_t        start_byte;
    uint64_t        chunk_size;
};

/**
 * Possible error codes:
 * - ::NET_ERR__UNKNOWN_REFERENCE_ID
 */
struct SendDataChunkResponse {
};
/** @} */

/**
 * @defgroup EVENT
 * Events cannot be requests, they will be send by the server spontaneously
 * @{
 */
struct GenericNetworkEvent {
    OpCode              op_code;
    NetworkErrorCode    error_code;
    char                data[];
};

/**
 * Possible error codes:
 * - TODO
 */
struct FileStatusEvent {
    uint64_t    file_reference_id;
};
/** @} */
}

#endif //HIDRA2__COMMON_NETWORKING_H
