#ifndef ASAPO_DATABASE_H
#define ASAPO_DATABASE_H

#include <string>

#include "common/data_structs.h"
#include "common/error.h"

namespace asapo {

namespace DBError {
auto const KUnknownError = "Inknown error";
auto const kConnectionError = "Connection error";
auto const kInsertError = "Insert error";
auto const kDuplicateID = "Duplicate ID";
auto const kAlreadyConnected = "Already connected";
auto const kNotConnected = "Not connected";
auto const kBadAddress = "Bad address";
auto const kMemoryError = "Memory error";

}

constexpr char kDBCollectionName[] = "data";

class Database {
  public:
    virtual Error Connect(const std::string& address, const std::string& database,
                          const std::string& collection ) = 0;
    virtual Error Insert(const FileInfo& file, bool ignore_duplicates) const = 0;
    virtual ~Database() = default;
};

class DatabaseFactory {
  public:
    virtual std::unique_ptr<Database> Create(Error* err) const noexcept;
    virtual ~DatabaseFactory() = default;
};


}

#endif //ASAPO_DATABASE_H
