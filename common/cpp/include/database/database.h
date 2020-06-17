#ifndef ASAPO_DATABASE_H
#define ASAPO_DATABASE_H

#include <string>
#include <ostream>
#include "common/data_structs.h"
#include "common/error.h"

namespace asapo {

constexpr char kDBDataCollectionNamePrefix[] = "data";
constexpr char kDBMetaCollectionName[] = "meta";


class Database {
  public:
    virtual Error Connect(const std::string& address, const std::string& database) = 0;
    virtual Error Insert(const std::string& collection, const FileInfo& file, bool ignore_duplicates) const = 0;
    virtual Error Upsert(const std::string& collection, uint64_t id, const uint8_t* data, uint64_t size) const = 0;
    virtual Error InsertAsSubset(const std::string& collection, const FileInfo& file, uint64_t subset_id,
                                 uint64_t subset_size,
                                 bool ignore_duplicates) const = 0;

    virtual Error GetById(const std::string& collection, uint64_t id, FileInfo* file) const = 0;
    virtual Error GetDataSetById(const std::string& collection, uint64_t set_id, uint64_t id, FileInfo* file) const = 0;
    virtual Error GetStreamInfo(const std::string& collection, StreamInfo* info) const  = 0;
    virtual ~Database() = default;
};

class DatabaseFactory {
  public:
    virtual std::unique_ptr<Database> Create(Error* err) const noexcept;
    virtual ~DatabaseFactory() = default;
};


}

#endif //ASAPO_DATABASE_H
