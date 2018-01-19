#include <iostream>
#include "receiver.h"

int main (int argc, char* argv[]) {
    static const std::string address = "0.0.0.0";
    static const uint16_t port = 4420;

    auto* receiver = new hidra2::Receiver();

    hidra2::ReceiverError err;

    receiver->StartListener(address, port, &err);
    if(err != hidra2::ReceiverError::NO_ERROR) {
        std::cerr << "Fail to start receiver" << std::endl;
        return 1;
    }
    std::cout << "StartListener on " << address <<  ":" << port << std::endl;

    std::cout << "Press Enter to exit" << std::endl;
    getchar();

    std::cout << "Stop listener..." << std::endl;
    receiver->StopListener(&err);

    return 0;
}
