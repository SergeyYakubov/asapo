#ifndef HIDRA2_DATABASE_H
#define HIDRA2_DATABASE_H

#include <string>

#include "common/data_structs.h"

namespace hidra2 {

enum class DBError {
    KUnknownError,
    kConnectionError,
    kInsertError,
    kDuplicateID,
    kAlreadyConnected,
    kNotConnected,
    kBadAddress,
    kNoError,
    kMemoryError
};

constexpr char kDBCollectionName[] = "data";

class Database {
  public:
    virtual DBError Connect(const std::string& address, const std::string& database,
                            const std::string& collection ) = 0;
    virtual DBError Insert(const FileInfo& file, bool ignore_duplicates) const = 0;
    virtual ~Database() = default;
};

class DatabaseFactory {
  public:
    virtual std::unique_ptr<Database> Create(DBError* err) const noexcept;
    virtual ~DatabaseFactory() = default;
};


}

#endif //HIDRA2_DATABASE_H