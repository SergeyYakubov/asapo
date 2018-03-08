#include <iostream>
#include <system_wrappers/system_io.h>

#include "testing.h"

using hidra2::SystemIO;
using hidra2::Error;
using hidra2::ErrorType;

using hidra2::M_AssertEq;
using hidra2::M_AssertTrue;

SystemIO systemIO;

void Check(const std::string& expected_ip_address, const std::string& hostname) {
    std::cout << "Checking: " << hostname << std::endl;
    Error err;
    std::string ip_address = systemIO.ResolveHostnameToIp(hostname, &err);
    M_AssertEq(expected_ip_address, ip_address);
    if(expected_ip_address.empty()) {
        M_AssertTrue(err != nullptr && (*err).GetErrorType() == ErrorType::kUnableToResolveHostname);
        return;
    }
    M_AssertTrue(err == nullptr);
}

int main(int argc, char* argv[]) {
    Check("127.0.0.1", "localhost");
    Check("8.8.8.8", "google-public-dns-a.google.com");
    Check("8.8.4.4", "google-public-dns-b.google.com");
    Check("4.2.2.1", "a.resolvers.level3.net");
    Check("4.2.2.2", "b.resolvers.level3.net");
    Check("4.2.2.3", "c.resolvers.level3.net");
    Check("4.2.2.4", "d.resolvers.level3.net");

    Check("", "some-address-that-does-not-exists.ff");
    Check("", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.ff");
    return 0;
}
