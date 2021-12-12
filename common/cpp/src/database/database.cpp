#include "asapo/database/database.h"
#include "mongodb_client.h"

namespace asapo {

std::unique_ptr<Database> DatabaseFactory::Create(Error* err) const noexcept {
    std::unique_ptr<Database> p = nullptr;
    try {
        p.reset(new MongoDBClient());
        *err = nullptr;
    } catch (...) {
        *err = GeneralErrorTemplates::kMemoryAllocationError.Generate();
    }
    return p;
}

}