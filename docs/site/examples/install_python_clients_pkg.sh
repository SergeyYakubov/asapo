#!/usr/bin/env bash

# you can also install Linux/Windows packages if you have root access (or install locally).
# take a look at http://nims.desy.de/extra/asapo/linux_packages/ or http://nims.desy.de/extra/asapo/windows10 for your OS. E.g. for Debian 10.7
wget http://nims.desy.de/extra/asapo/linux_packages/debian10.7/python-asapo-producer_100.0~develop-debian10.7_amd64.deb
wget http://nims.desy.de/extra/asapo/linux_packages/debian10.7/python-asapo-consumer_100.0~develop-debian10.7_amd64.deb

sudo apt install ./python3-asapo-producer_100.0~develop-debian10.7_amd64.deb
sudo apt install ./python3-asapo_consumer_100.0~develop-debian10.7_amd64.deb
