#!/usr/bin/env bash

# C unit testing library (cgreen)
wget https://github.com/cgreen-devs/cgreen/releases/download/1.2.0/cgreen_1.2.0_amd64.deb -P ~/Downloads
sudo dpkg -i ~/Downloads/cgreen_1.2.0_amd64.deb

# For determining the ip address of our R Pi's using their MAC address
sudo apt update
sudo apt install arp-scan

