#include <iostream>
#include "receiver.h"

int main (int argc, char* argv[]) {
    auto* receiver = new hidra2::Receiver();

    hidra2::ReceiverError err;

    receiver->StartListener("127.0.0.1", 8099, &err);
    if(err != hidra2::ReceiverError::NO_ERROR) {
        std::cerr << "Fail to start receiver" << std::endl;
        return 1;
    }
    std::cout << "StartListener" << std::endl;

    std::cout << "Press Enter to exit" << std::endl;
    getchar();

    std::cout << "Stop listener..." << std::endl;
    receiver->StopListener(&err);

    return 0;
}
