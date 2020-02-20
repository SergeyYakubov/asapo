#include "file_processor.h"

#include "io/io_factory.h"
#include "receiver_logger.h"

namespace asapo {

FileProcessor::FileProcessor(): io__{GenerateDefaultIO()}, log__{GetDefaultReceiverLogger()} {

}

}