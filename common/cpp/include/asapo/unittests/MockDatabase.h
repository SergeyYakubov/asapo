#ifndef ASAPO_MOCKDATABASE_H
#define ASAPO_MOCKDATABASE_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asapo/database/database.h"
#include "asapo/common/error.h"

namespace asapo {

class MockDatabase : public Database {
  public:
    Error Connect(const std::string& address, const std::string& database) override {
        return Error{Connect_t(address, database)};

    }
    Error Insert(const std::string& collection, const MessageMeta& file, bool ignore_duplicates) const override {
        return Error{Insert_t(collection, file, ignore_duplicates)};
    }

    Error InsertAsDatasetMessage(const std::string& collection, const MessageMeta& file,
                                 uint64_t dataset_size, bool ignore_duplicates) const override {
        return Error{InsertAsDatasetMessage_t(collection, file, dataset_size, ignore_duplicates)};
    }

    MOCK_METHOD2(Connect_t, ErrorInterface * (const std::string&, const std::string&));
    MOCK_CONST_METHOD3(Insert_t, ErrorInterface * (const std::string&, const MessageMeta&, bool));

    MOCK_CONST_METHOD4(InsertAsDatasetMessage_t,
                       ErrorInterface * (const std::string&, const MessageMeta&, uint64_t, bool));

    Error InsertMeta(const std::string& collection, const std::string& id, const uint8_t* data, uint64_t size,
                     MetaIngestMode mode) const override {
        return Error{InsertMeta_t(collection, id, data, size, mode)};

    }
    MOCK_CONST_METHOD5(InsertMeta_t, ErrorInterface * (const std::string&, const std::string& id, const uint8_t* data,
                       uint64_t size, MetaIngestMode mode));

    Error GetById(const std::string& collection, uint64_t id, MessageMeta* file) const override {
        return Error{GetById_t(collection, id, file)};
    }

    MOCK_CONST_METHOD3(GetById_t, ErrorInterface * (const std::string&, uint64_t id, MessageMeta*));

    Error GetDataSetById(const std::string& collection, uint64_t set_id, uint64_t id, MessageMeta* file) const override {
        return Error{GetSetById_t(collection, set_id, id, file)};
    }

    MOCK_CONST_METHOD4(GetSetById_t, ErrorInterface * (const std::string&, uint64_t set_id, uint64_t id, MessageMeta*));

    Error GetStreamInfo(const std::string& collection, StreamInfo* info) const override {
        return Error{GetStreamInfo_t(collection, info)};
    }

    MOCK_CONST_METHOD2(GetStreamInfo_t, ErrorInterface * (const std::string&, StreamInfo*));

    Error GetLastStream(StreamInfo* info) const override {
        return Error{GetLastStream_t(info)};
    }

    MOCK_CONST_METHOD1(DeleteStream_t, ErrorInterface * (const std::string&));

    Error DeleteStream(const std::string& stream) const override {
        return Error{DeleteStream_t(stream)};
    }

    MOCK_CONST_METHOD1(GetLastStream_t, ErrorInterface * (StreamInfo*));


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
