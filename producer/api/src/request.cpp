#include "request.h"

namespace asapo {

Error Request::ReadDataFromFileIfNeeded(const IO* io) {
    if (data != nullptr || original_filepath.empty()) {
        return nullptr;
    }
    Error err;
    data = io->GetDataFromFile(original_filepath, &header.data_size, &err);
    return err;
}

}