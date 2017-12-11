#include <iostream>
#include "receiver.h"

int main (int argc, char* argv[]) {
    auto* receiver = new hidra2::Receiver();

    receiver->start_listener("127.0.0.1", 8099);

    std::cout << "started listener" << std::endl;

    getchar();

    receiver->stop_listener();
    getchar();

    return 0;
}
