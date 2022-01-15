#!/bin/bash
cmake -B build
cd build || exit
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./leptjson_test