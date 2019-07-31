#ifndef ASAPO_DATABASE_H
#define ASAPO_DATABASE_H

#include <string>

#include "common/data_structs.h"
#include "common/error.h"

namespace asapo {

constexpr char kDBDataCollectionName[] = "data";
constexpr char kDBMetaCollectionName[] = "meta";


class Database {
  public:
    virtual Error Connect(const std::string& address, const std::string& database,
                          const std::string& collection ) = 0;
    virtual Error Insert(const FileInfo& file, bool ignore_duplicates) const = 0;
    virtual Error Upsert(uint64_t id, const uint8_t* data, uint64_t size) const = 0;
    virtual Error InsertAsSubset(const FileInfo& file, uint64_t subset_id, uint64_t subset_size,
                                 bool ignore_duplicates) const = 0;

    virtual ~Database() = default;
};

class DatabaseFactory {
  public:
    virtual std::unique_ptr<Database> Create(Error* err) const noexcept;
    virtual ~DatabaseFactory() = default;
};


}

#endif //ASAPO_DATABASE_H
