#ifndef ASAPO_COMMON__NETWORKING_H
#define ASAPO_COMMON__NETWORKING_H

#include <cstdint>
#include <algorithm>
#include <string>
#include <cstring>

#include "data_structs.h"

namespace asapo {

typedef uint64_t NetworkRequestId;

enum class NetworkConnectionType : uint32_t {
    kUndefined,
    kAsapoTcp, // ASAPOs TCP (Multiple connections for parallel data transfers)
    kFabric, // Fabric connection (Primarily used for InfiniBand verbs)
};

// do not forget to add new codes to the end!
enum Opcode : uint8_t {
    kOpcodeUnknownOp = 1,
    kOpcodeTransferData,
    kOpcodeTransferDatasetData,
    kOpcodeStreamInfo,
    kOpcodeLastStream,
    kOpcodeGetBufferData,
    kOpcodeAuthorize,
    kOpcodeTransferMetaData,
    kOpcodeDeleteStream,
    kOpcodeGetMeta,
    kOpcodeCount,
};

inline std::string OpcodeToString(uint8_t code) {
    switch (code) {
    case kOpcodeTransferData:
        return "transfer data";
    case kOpcodeTransferDatasetData:
        return "transfer dataset data";
    case kOpcodeStreamInfo:
        return "stream info";
    case kOpcodeLastStream:
        return "last stream";
    case kOpcodeGetBufferData:
        return "get buffer data";
    case kOpcodeAuthorize:
        return "authorize";
    case kOpcodeTransferMetaData:
        return "transfer metadata";
    case kOpcodeDeleteStream:
        return "delete stream";
    case kOpcodeGetMeta:
        return "get meta";
    default:
        return "unknown op";
    }
}

enum NetworkErrorCode : uint16_t {
    kNetErrorNoError,
    kNetErrorReauthorize,
    kNetErrorWarning,
    kNetErrorWrongRequest,
    kNetErrorNotSupported,
    kNetErrorNoData,
    kNetAuthorizationError,
    kNetErrorInternalServerError = 65535,
};

//TODO need to use an serialization framework to ensure struct consistency on different computers

const std::size_t kMaxMessageSize = 1024;
const std::size_t kMaxVersionSize = 10;
const std::size_t kNCustomParams = 3;
using CustomRequestData = uint64_t[kNCustomParams];
const std::size_t kPosIngestMode = 0;
const std::size_t kPosDataSetId = 1;
const std::size_t kPosMetaIngestMode = 1;
const std::size_t kPosDataSetSize = 2;

struct GenericRequestHeader {
    GenericRequestHeader(const GenericRequestHeader& header) {
        op_code = header.op_code, data_id = header.data_id, data_size = header.data_size, meta_size = header.meta_size,
        memcpy(custom_data, header.custom_data, kNCustomParams * sizeof(uint64_t)),
        memcpy(message, header.message, kMaxMessageSize);
        strncpy(stream, header.stream, kMaxMessageSize);
        strncpy(api_version, header.api_version, kMaxVersionSize);
    }

    /* Keep in mind that the message here is just strncpy'ed, you can change the message later */
    GenericRequestHeader(Opcode i_op_code = kOpcodeUnknownOp, uint64_t i_data_id = 0,
                         uint64_t i_data_size = 0, uint64_t i_meta_size = 0, const std::string& i_message = "",
                         const std::string& i_stream = "") :
        op_code{i_op_code}, data_id{i_data_id}, data_size{i_data_size}, meta_size{i_meta_size} {
        strncpy(message, i_message.c_str(), kMaxMessageSize);
        strncpy(stream, i_stream.c_str(), kMaxMessageSize);
        strncpy(api_version, "v0.0", kMaxVersionSize);
    }

    Opcode op_code;
    uint64_t data_id;
    uint64_t data_size;
    uint64_t meta_size;
    CustomRequestData custom_data;
    char message[kMaxMessageSize]; /* Can also be a binary message (e.g. MemoryRegionDetails) */
    char stream[kMaxMessageSize]; /* Must be a string (strcpy is used) */
    char api_version[kMaxVersionSize]; /* Must be a string (strcpy is used) */
    std::string Json() {
        std::string s = "{\"id\":" + std::to_string(data_id) + ","
                        "\"buffer\":\"" + std::string(message) + "\"" + ","
                        "\"stream\":\""
                        + std::string(stream) + "\""
                        + "}";
        return s;
    };
};

struct GenericNetworkResponse {
    Opcode op_code;
    NetworkErrorCode error_code;
    char message[kMaxMessageSize];
};

struct SendResponse : GenericNetworkResponse {
};

}

#endif //ASAPO_COMMON__NETWORKING_H
