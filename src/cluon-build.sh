#!/bin/bash
if [ -z "$1" ]
then
    echo "Please indicate the name of your cpp/hpp files (without extention)."
    exit
fi

fileName="$1.odvd"
if [ -f "$fileName" ]
then
    echo "$1.odvd file found."
else
    echo "$1.odvd file not found!"
    exit
fi

echo "Now generating cluon-complete.cpp..."
ln -sf cluon-complete.hpp cluon-complete.cpp

echo "Now compiling cluon-msc..."
g++ -DHAVE_CLUON_MSC -std=c++14 -pthread -o cluon-msc cluon-complete.cpp

echo "Now generating $1.hpp..."
./cluon-msc --cpp-headers --out=$1.hpp $1.odvd

echo "Now generating $1.cpp..."
./cluon-msc --cpp-sources --out=$1.cpp --cpp-add-include-file=$1.hpp $1.odvd

echo "Now cleaning up..."
rm -f cluon-msc cluon-complete.cpp
