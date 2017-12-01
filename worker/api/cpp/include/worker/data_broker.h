#ifndef HIDRA2_DATASOURCE_H
#define HIDRA2_DATASOURCE_H

#include <memory>
#include <string>
#include <iostream>

namespace hidra2 {

class DataBroker {
  public:
  inline virtual int connect()=0;
};

class DataBrokerFactory{
 public:
  static std::unique_ptr<DataBroker> create(std::string source_name) noexcept;
};




}
#endif //HIDRA2_DATASOURCE_H
