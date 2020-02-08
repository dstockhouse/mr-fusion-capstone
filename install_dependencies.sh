#!/usr/bin/env bash

sudo apt-get update

# For makefiles
sudo apt install make

# For C code
sudo apt-get install gcc

# For Rust
curl https://sh.rustup.rs -sSf | sh

echo Ignore line above
echo Restart to finish installation