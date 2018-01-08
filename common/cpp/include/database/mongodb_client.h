#ifndef HIDRA2_MONGO_DATABASE_H
#define HIDRA2_MONGO_DATABASE_H

#include "database.h"

namespace hidra2 {

class MongoDB final: public Database {
  public:
    virtual DBError Connect(const std::string& address, const std::string& database,
                            const std::string& collection ) override;
    virtual DBError Import(const FileInfos& files) const override;
    virtual ~MongoDB() override;

};

}

#endif //HIDRA2_MONGO_DATABASE_H
