#!/bin/bash
#
#
if [ ! -d "./build" ]; then
	mkdir ./build
fi
if [ -f "./build" ]; then
	rm ./build/split-window
fi
#
#
g++ -o ./build/split-window ./src/main/cpp/split-window.cpp -L/usr/X11/lib -lX11 -lstdc++
#
#
./build/split-window left
./build/split-window left
./build/split-window right

/home/andold/eclipse-workspace/split-window/build/split-window right