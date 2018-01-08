#include "database/mongodb_client.h"

namespace hidra2 {

DBError MongoDB::Connect(const std::string& address, const std::string& database,
                         const std::string& collection ) {
    return DBError::kNoError;
}

DBError MongoDB::Import(const FileInfos& files) const {
    return DBError::kNoError;
}


MongoDB::~MongoDB() {

}



}
