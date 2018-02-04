#include "broker_helpers.h"

namespace hidra2 {

WorkerErrorCode MapIOError(IOErrors io_err) {
    WorkerErrorCode err;
    switch (io_err) { // we do not use map due to performance reasons
    case IOErrors::kNoError:
        err = WorkerErrorCode::kOK;
        break;
    case IOErrors::kFileNotFound:
        err = WorkerErrorCode::kSourceNotFound;
        break;
    case IOErrors::kPermissionDenied:
        err = WorkerErrorCode::kPermissionDenied;
        break;
    case IOErrors::kReadError:
        err = WorkerErrorCode::kErrorReadingSource;
        break;
    case IOErrors::kMemoryAllocationError:
        err = WorkerErrorCode::kMemoryError;
        break;
    default:
        err = WorkerErrorCode::kUnknownIOError;
        break;
    }

    return err;
}

}
