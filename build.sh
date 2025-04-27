#!/bin/bash

clear
rm -rf build
mkdir -p build
cd build
cmake ..
make
cd ..