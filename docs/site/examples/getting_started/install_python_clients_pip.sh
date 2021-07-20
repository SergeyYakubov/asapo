#!/usr/bin/env bash

pip3 install --user --trusted-host nims.desy.de --find-links=http://nims.desy.de/extra/asapo/linux_wheels asapo_producer==@ASAPO_WHEEL_VERSION_IN_DOCS@
pip3 install --user --trusted-host nims.desy.de --find-links=http://nims.desy.de/extra/asapo/linux_wheels asapo_consumer==@ASAPO_WHEEL_VERSION_IN_DOCS@
# you might need to update pip if the above commands error: pip3 install --upgrade pip

# if that does not work (abi incompatibility, etc) you may try to install source packages
# take a look at http://nims.desy.de/extra/asapo/linux_packages/ or http://nims.desy.de/extra/asapo/windows10 for your OS. E.g. for Debian 10.7
# wget http://nims.desy.de/extra/asapo/linux_packages/debian10.7/asapo_producer-@ASAPO_VERSION_IN_DOCS@.tar.gz
# wget http://nims.desy.de/extra/asapo/linux_packages/debian10.7/asapo_consumer-@ASAPO_VERSION_IN_DOCS@.tar.gz

# pip3 install asapo_producer-@ASAPO_VERSION_IN_DOCS@.tar.gz
# pip3 install asapo_consumer-@ASAPO_VERSION_IN_DOCS@.tar.gz