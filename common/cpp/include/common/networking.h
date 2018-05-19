#ifndef ASAPO_COMMON__NETWORKING_H
#define ASAPO_COMMON__NETWORKING_H

#include <cstdint>
#include <algorithm>
#include <string>

namespace asapo {

typedef uint64_t NetworkRequestId;

enum Opcode : uint8_t {
    kOpcodeUnknownOp,
    kOpcodeTransferData,
    kOpcodeCount,
};

enum NetworkErrorCode : uint16_t {
    kNetErrorNoError,
    kNetErrorFileIdAlreadyInUse,
    kNetErrorAllocateStorageFailed,
    kNetErrorInternalServerError = 65535,
};

//TODO need to use an serialization framework to ensure struct consistency on different computers

/**
 * @defgroup RPC
 * RPC always return a response to a corresponding request
 * @{
 */

const std::size_t kMaxFileNameSize = 1024;
struct GenericRequestHeader {
    GenericRequestHeader(Opcode i_op_code = kOpcodeUnknownOp,uint64_t i_data_id = 0,
                         uint64_t i_data_size = 0,const std::string& i_file_name = ""):
        op_code{i_op_code},data_id{i_data_id},data_size{i_data_size} {
        auto size = std::min(i_file_name.size()+1,kMaxFileNameSize);
        memcpy(file_name, i_file_name.c_str(), size);
    }
    Opcode      op_code;
    uint64_t    data_id;
    uint64_t    data_size;
    char        file_name[kMaxFileNameSize];
};

struct GenericNetworkResponse {
    Opcode              op_code;
    NetworkRequestId    request_id;
    NetworkErrorCode    error_code;
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

#endif //ASAPO_COMMON__NETWORKING_H
