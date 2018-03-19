#include <iostream>
#include "receiver.h"

int main (int argc, char* argv[]) {
    static const std::string address = "0.0.0.0:4200";

    auto* receiver = new hidra2::Receiver();

    hidra2::Error err;

    std::cout << "StartListener on " << address << std::endl;
    receiver->StartListener(address, &err);
    if(err) {
        std::cerr << "Failed to start receiver: " << err << std::endl;
        return 1;
    }

    return 0;
}
