#ifndef ASAPO_ENCODING_H
#define ASAPO_ENCODING_H

#include <string>

namespace asapo {

std::string EncodeDbName(const std::string& dbname);
std::string EncodeColName(const std::string& colname);
std::string DecodeName(const std::string& name);
std::string EscapeQuery(const std::string& name);
}

#endif //ASAPO_ENCODING_H
