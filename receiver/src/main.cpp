#include <iostream>
#include "receiver.h"

int main (int argc, char* argv[]) {
    static const std::string address = "0.0.0.0:4200";

    auto* receiver = new hidra2::Receiver();

    hidra2::Error err;

    receiver->StartListener(address, &err);
    if(err) {
        std::cerr << "Fail to start receiver: " << err << std::endl;
        return 1;
    }
    std::cout << "StartListener on " << address << std::endl;

    std::cout << "Press Enter to exit" << std::endl;
    getchar();

    std::cout << "Stop listener..." << std::endl;
    receiver->StopListener(&err);//TODO might not work since Accept is a blocking call :/
    if(err) {
        std::cerr << "Fail to stop receiver: " << err << std::endl;
        return 1;
    }

    return 0;
}
