#!/bin/bash

sudo apt install cmake -y

cd setup/TinyMAT
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
sudo make install .
cd ../
sudo rm -rf build
cd ../../