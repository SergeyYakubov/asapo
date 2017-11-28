#ifndef HIDRA2__COMMON_NETWORKING_H
#define HIDRA2__COMMON_NETWORKING_H

#include <cstdint>
#include "os.h"

namespace HIDRA2
{
    namespace Networking {
        enum OP_CODE : uint8_t {
            OP_CODE__HELLO,
            OP_CODE__PREPARE_SEND_DATA,
            OP_CODE__SEND_DATA_CHUNK,
        };

        enum ERROR_CODE : uint16_t {
            ERR__NO_ERROR,
            ERR__UNSUPPORTED_VERSION,
            ERR__FILENAME_ALREADY_IN_USE,
            ERR__UNKNOWN_REFERENCE_ID,
            ERR__INTERNAL_SERVER_ERROR = 65535,
        };

        /**
         * @defgroup RPC
         * RPC always return a response to a corresponding request
         * @{
         */
        struct GenericNetworkRequest {
            OP_CODE  op_code;
            uint64_t request_id;
            char     data[];
        };

        struct GenericNetworkResponse {
            OP_CODE    op_code;
            uint64_t   request_id;
            ERROR_CODE error_code;
            char       data[];
        };

        struct HelloRequest {
            uint32_t client_version;

            OS_TYPE os : 4;
            bool    is_x64 : 1;
        };

        /**
         * Possible error codes:
         * - ::ERR__UNSUPPORTED_VERSION
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
         * - ::ERR__FILENAME_ALREADY_IN_USE
         */
        struct PrepareSendDataResponse {
            uint64_t file_reference_id;
        };

        struct SendDataChunkRequest {
            uint64_t file_reference_id;
            uint64_t start_byte;
            uint64_t chunk_size;
        };

        /**
         * Possible error codes:
         * - ::ERR__UNKNOWN_REFERENCE_ID
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
            OP_CODE     op_code;
            ERROR_CODE  error_code;
            char        data[];
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
}

#endif //HIDRA2__COMMON_NETWORKING_H
