#ifndef ASAPO_MOCKDATABASE_H
#define ASAPO_MOCKDATABASE_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "database/database.h"
#include "common/error.h"

namespace asapo {

class MockDatabase : public Database {
  public:
    Error Connect(const std::string& address, const std::string& database,
                  const std::string& collection ) override {
        return Error{Connect_t(address, database, collection)};

    }
    Error Insert(const FileInfo& file, bool ignore_duplicates) const override {
        return Error{Insert_t(file, ignore_duplicates)};
    }

    MOCK_METHOD3(Connect_t, SimpleError * (const std::string&, const std::string&, const std::string&));
    MOCK_CONST_METHOD2(Insert_t, SimpleError * (const FileInfo&, bool));

    Error Upsert(uint64_t id, const uint8_t* data, uint64_t size) const override {
        return Error{Upsert_t(id, data, size)};

    }
    MOCK_CONST_METHOD3(Upsert_t, SimpleError * (uint64_t id, const uint8_t* data, uint64_t size));



    // stuff to test db destructor is called and avoid "uninteresting call" messages
    MOCK_METHOD0(Die, void());
    virtual ~MockDatabase() override {
        if (check_destructor)
            Die();
    }
    bool check_destructor{false};
};

}

#endif //ASAPO_MOCKDATABASE_H
