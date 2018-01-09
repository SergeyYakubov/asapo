#ifndef HIDRA2_DATABASE_H
#define HIDRA2_DATABASE_H

#include <string>

#include "common/file_info.h"

namespace hidra2 {

enum class DBError {
    kNoError,
    kConnectionError,
    kImportError,
    kAlreadyConnected,
    kNotConnected,
    kBadAddress
};

constexpr char kDBName[] = "data";

class Database {
  public:
    virtual DBError Connect(const std::string& address, const std::string& database,
                            const std::string& collection ) = 0;
    virtual DBError Import(const FileInfos& files) const = 0;
    virtual ~Database() = default;
};

}

#endif //HIDRA2_DATABASE_H
