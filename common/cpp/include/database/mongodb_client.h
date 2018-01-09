#ifndef HIDRA2_MONGO_DATABASE_H
#define HIDRA2_MONGO_DATABASE_H

#include "database.h"

namespace hidra2 {

class MongoDB final: public Database {
  public:
    DBError Connect(const std::string& address, const std::string& database,
                    const std::string& collection ) override;
    DBError Import(const FileInfos& files) const override;
    ~MongoDB() override;

};

}

#endif //HIDRA2_MONGO_DATABASE_H
