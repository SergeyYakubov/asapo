#ifndef HIDRA2__COMMON_NETWORKING_H
#define HIDRA2__COMMON_NETWORKING_H

#include <cstdint>
#include "os.h"

namespace HIDRA2 {
enum OP_CODE : uint8_t {
  OP_CODE__HELLO,
};

enum ERROR_CODE : uint16_t {
  ERR__NO_ERROR,
  ERR__UNSUPPORTED_VERSION,
  ERR__INTERNAL_SERVER_ERROR = 65535,
};

struct NetworkRequest {
  OP_CODE     op_code;
  uint64_t    request_id;
  char        data[];
};

struct NetworkResponse {
  OP_CODE     op_code;
  uint64_t    request_id;
  ERROR_CODE  error_code;
  char        data[];
};

struct OP_HelloRequest {
  uint32_t client_version;

  OS_TYPE os : 4;
  bool is_x64 : 1;
};

struct OP_HelloResponse {
  uint32_t server_version;
};
}

#endif //HIDRA2__COMMON_NETWORKING_H
