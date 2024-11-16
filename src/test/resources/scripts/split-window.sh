#!/bin/bash
#
#
HOME_DIR=/home/andold
SOURCE_DIR=$HOME_DIR/src/eclipse-workspace/split-window
#
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
#
cd $SOURCE_DIR
g++ -o ./build/split-window ./src/main/cpp/split-window.cpp -L/usr/X11/lib -lX11 -lstdc++
#
#
#
$SOURCE_DIR/build/split-window left
$SOURCE_DIR/build/split-window left
$SOURCE_DIR/build/split-window right
$SOURCE_DIR/build/split-window right
