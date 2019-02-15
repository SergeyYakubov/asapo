#ifndef ASAPO_COMMON__NETWORKING_H
#define ASAPO_COMMON__NETWORKING_H

#include <cstdint>
#include <algorithm>
#include <string>
#include <cstring>

namespace asapo {

typedef uint64_t NetworkRequestId;

enum Opcode : uint8_t {
    kOpcodeUnknownOp = 1,
    kOpcodeTransferData,
    kOpcodeGetBufferData,
    kOpcodeAuthorize,
    kOpcodeCount,
};

enum NetworkErrorCode : uint16_t {
    kNetErrorNoError,
    kNetAuthorizationError,
    kNetErrorFileIdAlreadyInUse,
    kNetErrorAllocateStorageFailed,
    kNetErrorInternalServerError = 65535,
};

//TODO need to use an serialization framework to ensure struct consistency on different computers

const std::size_t kMaxMessageSize = 1024;

struct GenericRequestHeader {
    GenericRequestHeader(Opcode i_op_code = kOpcodeUnknownOp, uint64_t i_data_id = 0,
                         uint64_t i_data_size = 0, const std::string& i_message = ""):
        op_code{i_op_code}, data_id{i_data_id}, data_size{i_data_size} {
        strncpy(message, i_message.c_str(), kMaxMessageSize);
    }
  GenericRequestHeader(const GenericRequestHeader &header) {op_code = header.op_code, data_id = header.data_id,data_size = header.data_size,
          strncpy(message, header.message, kMaxMessageSize);
    }

  Opcode      op_code;
    uint64_t    data_id;
    uint64_t    data_size;
    char        message[kMaxMessageSize];
};

struct GenericNetworkResponse {
    Opcode              op_code;
    NetworkRequestId    request_id;
    NetworkErrorCode    error_code;
    char        message[kMaxMessageSize];
};


struct SendDataResponse :  GenericNetworkResponse {
};

}

#endif //ASAPO_COMMON__NETWORKING_H
