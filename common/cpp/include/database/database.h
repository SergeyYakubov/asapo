#ifndef HIDRA2_DATABASE_H
#define HIDRA2_DATABASE_H

#include <string>

namespace hidra2 {

enum class DBError {
    kNoError,
    kConnectionError
};

constexpr char kDBName[] = "data";

class Database {
  public:
    virtual DBError Connect(const std::string& address, const std::string& database,
                            const std::string& collection ) = 0;
    virtual ~Database() = default;
};

}

#endif //HIDRA2_DATABASE_H
