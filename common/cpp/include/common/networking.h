#ifndef ASAPO_COMMON__NETWORKING_H
#define ASAPO_COMMON__NETWORKING_H

#include <cstdint>
#include <algorithm>
#include <string>
#include <cstring>

#include "data_structs.h"

namespace asapo {

typedef uint64_t NetworkRequestId;

enum class NetworkConnectionType {
    kAsapoTcp, // ASAPOs TCP (Multiple connections for parallel data transfers)
};

enum Opcode : uint8_t {
    kOpcodeUnknownOp = 1,
    kOpcodeTransferData,
    kOpcodeTransferSubsetData,
    kOpcodeGetBufferData,
    kOpcodeAuthorize,
    kOpcodeTransferMetaData,
    kOpcodeCount,
};


enum NetworkErrorCode : uint16_t {
    kNetErrorNoError,
    kNetErrorReauthorize,
    kNetErrorWarning,
    kNetErrorWrongRequest,
    kNetErrorNoData,
    kNetAuthorizationError,
    kNetErrorInternalServerError = 65535,
};

//TODO need to use an serialization framework to ensure struct consistency on different computers

const std::size_t kMaxMessageSize = 1024;
const std::size_t kNCustomParams = 3;
using CustomRequestData = uint64_t[kNCustomParams];
const std::size_t kPosIngestMode = 0;
const std::size_t kPosDataSetId = 1;
const std::size_t kPosDataSetSize = 2;

struct GenericRequestHeader {
    GenericRequestHeader(const GenericRequestHeader& header) {
        op_code = header.op_code, data_id = header.data_id, data_size = header.data_size, meta_size = header.meta_size,
        memcpy(custom_data, header.custom_data, kNCustomParams * sizeof(uint64_t)),
        memcpy(message, header.message, kMaxMessageSize);
        strncpy(substream, header.substream, kMaxMessageSize);
    }

    /* THIS CONSTRUCTOR SHOULD ONLY BE CALLED FOR TESTS! (because the message is only being strncpy'ed */
    GenericRequestHeader(Opcode i_op_code = kOpcodeUnknownOp, uint64_t i_data_id = 0,
                         uint64_t i_data_size = 0, uint64_t i_meta_size = 0, const std::string& i_message = "",
                         const std::string& i_substream = ""):
        op_code{i_op_code}, data_id{i_data_id}, data_size{i_data_size}, meta_size{i_meta_size} {
        strncpy(message, i_message.c_str(), kMaxMessageSize);
        strncpy(substream, i_substream.c_str(), kMaxMessageSize);
    }

    Opcode      op_code;
    uint64_t    data_id;
    uint64_t    data_size;
    uint64_t    meta_size;
    CustomRequestData    custom_data;
    char        message[kMaxMessageSize]; /* Can also be a binary message (e.g. MemoryRegionDetails)*/
    char        substream[kMaxMessageSize]; /* Must be a string (strcpy is used) */
    std::string Json() {
        std::string s = "{\"id\":" + std::to_string(data_id) + ","
                        "\"buffer\":\"" + std::string(message) + "\""
                        + "}";
        return s;
    };

};



struct GenericNetworkResponse {
    Opcode              op_code;
    NetworkErrorCode    error_code;
    char        message[kMaxMessageSize];
};


struct SendDataResponse :  GenericNetworkResponse {
};

}

#endif //ASAPO_COMMON__NETWORKING_H
