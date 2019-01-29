#include "request.h"

namespace asapo {

Request::Request(uint64_t net_id, const NetServer* server) : net_id{net_id}, server{server} {

}
}