#!/usr/bin/env bash

if [ $# -eq 0 ]; then
    echo "Build script for mcc"
    echo "Usage: $0 build|clean|run"
    exit
fi

if [ $1 = build ]; then
    # Running CMake, out-of-source build
    cd ./build && cmake ..
    make
fi

if [ $1 = clean ]; then
    rm -rf ./build
    mkdir build
fi

if [ $1 = run ]; then
    ./build/mcc.out
fi

