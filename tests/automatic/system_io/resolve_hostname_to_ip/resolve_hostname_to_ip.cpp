#include <iostream>
#include "asapo/io/io_factory.h"

#include "testing.h"

using asapo::Error;
using asapo::ErrorType;

void Check(const std::string& expected_ip_address, const std::string& hostname) {
    std::cout << "Checking: " << hostname << std::endl;
    Error err;
    auto io = std::unique_ptr<asapo::IO> {asapo::GenerateDefaultIO()};
    std::string ip_address = io->ResolveHostnameToIp(hostname, &err);
    if(expected_ip_address.empty()) {
        M_AssertEq(asapo::IOErrorTemplates::kUnableToResolveHostname, err);
    } else {
        M_AssertEq(nullptr, err);
    }
    M_AssertEq(expected_ip_address, ip_address);
}

int main() {
    Check("127.0.0.1", "localhost");
    Check("8.8.8.8", "google-public-dns-a.google.com");
    Check("8.8.4.4", "google-public-dns-b.google.com");
    Check("4.2.2.1", "a.resolvers.level3.net");
    Check("4.2.2.2", "b.resolvers.level3.net");
    Check("4.2.2.3", "c.resolvers.level3.net");
    Check("4.2.2.4", "d.resolvers.level3.net");

    Check("", "some-address-that-does-not-exists.ff");
    Check("", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.ff");

    // Fallthrough tests
    Check("123.123.123.123", "123.123.123.123");
    Check("8.8.8.8", "8.8.8.8");
    return 0;
}
