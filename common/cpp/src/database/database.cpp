#include "database/database.h"
#include "mongodb_client.h"

namespace hidra2 {

std::unique_ptr<Database> DatabaseFactory::Create(DBError* err) const noexcept {
    std::unique_ptr<Database> p = nullptr;
    try {
        p.reset(new MongoDBClient());
        *err = DBError::kNoError;
    } catch (...) {
        *err = DBError::kMemoryError;
    }
    return p;
};

}