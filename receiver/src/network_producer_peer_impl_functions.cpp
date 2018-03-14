#include "network_producer_peer_impl.h"

namespace hidra2 {
void NetworkProducerPeerImpl::ReceiveAndSaveFile(uint64_t file_id, size_t file_size, Error* err) {
    if(!CheckIfValidFileSize(file_size)) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        return;
    }

    FileDescriptor fd = CreateAndOpenFileByFileId(file_id, err);
    if(*err) {
        if(*err != IOErrorTemplates::kFileAlreadyExists) {
            return; //Unexpected error
        }
        Error skipErr;//Create a new error instance so that the original error will not be overwritten
        io->Skip(socket_fd_, file_size, &skipErr);//Skip the file payload so it will not get out of sync
        return;
    }

    FileData buffer;
    try {
        buffer.reset(new uint8_t[file_size]);
    } catch(std::exception& e) {
        *err = ErrorTemplates::kMemoryAllocationError.Generate();
        (*err)->Append(e.what());
        return;
    }

    io->Receive(socket_fd_, buffer.get(), file_size, err);
    if(*err) {
        return;
    }

    io->Write(fd, buffer.get(), file_size, err);
    if(*err) {
        return;
    }
}
}
