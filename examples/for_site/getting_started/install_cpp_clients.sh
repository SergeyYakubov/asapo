#!/usr/bin/env bash

# you can also install Linux/Windows packages if you have root access (or install locally).
# take a look at http://nims.desy.de/extra/asapo/linux_packages/ or http://nims.desy.de/extra/asapo/windows10 for your OS. E.g. for Debian 10.7
wget http://nims.desy.de/extra/asapo/linux_packages/debian10.7/asapo-dev-@ASAPO_VERSION@-debian10.7.x86_64.deb
sudo apt install ./asapo-dev-@ASAPO_VERSION@-debian10.7.x86_64.deb


