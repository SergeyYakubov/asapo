#include "tcp_client.h"

namespace asapo {

Error TcpClient::GetData(const FileInfo* info, FileData* data) const noexcept {
    return ErrorTemplates::kMemoryAllocationError.Generate();
}

}