#ifndef HIDRA2_DATASOURCE_H
#define HIDRA2_DATASOURCE_H

#include <memory>
#include <string>

namespace hidra2 {

class DataBroker {
  public:
    virtual int Connect()=0;
};

class DataBrokerFactory {
  public:
    static std::unique_ptr<DataBroker> Create(std::string);
};


}
#endif //HIDRA2_DATASOURCE_H
