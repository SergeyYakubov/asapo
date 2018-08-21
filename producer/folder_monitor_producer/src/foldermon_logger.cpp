#include "foldermon_logger.h"

namespace asapo {

AbstractLogger* GetDefaultFolderMonLogger() {
    static Logger logger = asapo::CreateDefaultLoggerBin("producer ");
    return logger.get();
}

}
