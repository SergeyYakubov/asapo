#ifndef HIDRA2_DATABASE_H
#define HIDRA2_DATABASE_H

#include <string>

#include "common/file_info.h"

namespace hidra2 {

enum class DBError {
    kConnectionError,
    kInsertError,
    kDuplicateID,
    kAlreadyConnected,
    kNotConnected,
    kBadAddress,
    kNoError
};

constexpr char kDBName[] = "data";

class Database {
  public:
    virtual DBError Connect(const std::string& address, const std::string& database,
                            const std::string& collection ) = 0;
    virtual DBError Insert(const FileInfo& file) const = 0;
    virtual ~Database() = default;
};

}

#endif //HIDRA2_DATABASE_H
