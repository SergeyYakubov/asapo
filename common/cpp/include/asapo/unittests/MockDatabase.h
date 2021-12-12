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
    Error Insert(const std::string& collection, const MessageMeta& file, bool ignore_duplicates,
                 uint64_t* id_inserted) const override {
        return Error{Insert_t(collection, file, ignore_duplicates, id_inserted)};
    }

    Error InsertAsDatasetMessage(const std::string& collection, const MessageMeta& file,
                                 uint64_t dataset_size, bool ignore_duplicates) const override {
        return Error{InsertAsDatasetMessage_t(collection, file, dataset_size, ignore_duplicates)};
    }

    MOCK_METHOD(ErrorInterface *, Connect_t, (const std::string&, const std::string&), ());
    MOCK_METHOD(ErrorInterface *, Insert_t, (const std::string&, const MessageMeta&, bool, uint64_t*), (const));

    MOCK_METHOD(ErrorInterface *, InsertAsDatasetMessage_t, (const std::string&, const MessageMeta&, uint64_t, bool), (const));

    Error InsertMeta(const std::string& collection, const std::string& id, const uint8_t* data, uint64_t size,
                     MetaIngestMode mode) const override {
        return Error{InsertMeta_t(collection, id, data, size, mode)};

    }
    MOCK_METHOD(ErrorInterface *, InsertMeta_t, (const std::string&, const std::string& id, const uint8_t* data, uint64_t size, MetaIngestMode mode), (const));

    Error GetById(const std::string& collection, uint64_t id, MessageMeta* file) const override {
        return Error{GetById_t(collection, id, file)};
    }

    MOCK_METHOD(ErrorInterface *, GetById_t, (const std::string&, uint64_t id, MessageMeta*), (const));

    Error GetDataSetById(const std::string& collection, uint64_t set_id, uint64_t id, MessageMeta* file) const override {
        return Error{GetSetById_t(collection, set_id, id, file)};
    }

    Error GetMetaFromDb(const std::string& collection, const std::string& id, std::string* res) const override {
        return Error{GetMetaFromDb_t(collection, id, res)};
    }
    MOCK_METHOD(ErrorInterface *, GetMetaFromDb_t, (const std::string&, const std::string&, std::string* res), (const));

    MOCK_METHOD(ErrorInterface *, GetSetById_t, (const std::string&, uint64_t set_id, uint64_t id, MessageMeta*), (const));

    Error GetStreamInfo(const std::string& collection, StreamInfo* info) const override {
        return Error{GetStreamInfo_t(collection, info)};
    }

    MOCK_METHOD(ErrorInterface *, GetStreamInfo_t, (const std::string&, StreamInfo*), (const));

    Error GetLastStream(StreamInfo* info) const override {
        return Error{GetLastStream_t(info)};
    }

    MOCK_METHOD(ErrorInterface *, DeleteStream_t, (const std::string&), (const));

    Error DeleteStream(const std::string& stream) const override {
        return Error{DeleteStream_t(stream)};
    }

    MOCK_METHOD(ErrorInterface *, GetLastStream_t, (StreamInfo*), (const));


    // stuff to test db destructor is called and avoid "uninteresting call" messages
    MOCK_METHOD(void, Die, (), ());
    virtual ~MockDatabase() override {
        if (check_destructor)
            Die();
    }
    bool check_destructor{false};
};

}

#endif //ASAPO_MOCKDATABASE_H
