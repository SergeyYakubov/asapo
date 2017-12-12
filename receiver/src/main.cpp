#include <iostream>
#include "receiver.h"

int main (int argc, char* argv[]) {
    auto* receiver = new hidra2::Receiver();

    hidra2::ReceiverError err;

    receiver->start_listener("127.0.0.1", 8099, &err);
    if(err != hidra2::ReceiverError::NO_ERROR) {
        std::cerr << "Fail to start receiver" << std::endl;
        return 1;
    }
    std::cout << "start_listener" << std::endl;

    getchar();

    receiver->stop_listener(&err);
    std::cout << "stop_listener" << std::endl;
    getchar();

    return 0;
}
