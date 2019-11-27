#ifndef ASAPO_MOCKDATABASE_H
#define ASAPO_MOCKDATABASE_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "database/database.h"
#include "common/error.h"

namespace asapo {

class MockDatabase : public Database {
  public:
    Error Connect(const std::string& address, const std::string& database) override {
        return Error{Connect_t(address, database)};

    }
    Error Insert(const std::string& collection, const FileInfo& file, bool ignore_duplicates) const override {
        return Error{Insert_t(collection, file, ignore_duplicates)};
    }

    Error InsertAsSubset(const std::string& collection, const FileInfo& file, uint64_t subset_id,
                         uint64_t subset_size, bool ignore_duplicates) const override {
        return Error{InsertAsSubset_t(collection, file, subset_id, subset_size, ignore_duplicates)};
    }


    MOCK_METHOD2(Connect_t, ErrorInterface * (const std::string&, const std::string&));
    MOCK_CONST_METHOD3(Insert_t, ErrorInterface * (const std::string&, const FileInfo&, bool));


    MOCK_CONST_METHOD5(InsertAsSubset_t, ErrorInterface * (const std::string&, const FileInfo&, uint64_t, uint64_t, bool));


    Error Upsert(const std::string& collection, uint64_t id, const uint8_t* data, uint64_t size) const override {
        return Error{Upsert_t(collection, id, data, size)};

    }
    MOCK_CONST_METHOD4(Upsert_t, ErrorInterface * (const std::string&, uint64_t id, const uint8_t* data, uint64_t size));



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
