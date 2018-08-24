#!/bin/bash
echo "Now cloning git repository of Urho3D..."

git clone https://github.com/urho3d/Urho3D
cd Urho3D

echo "Now getting ready to compile Urho3D engine..."
cd script

./cmake_clean.sh
./cmake_generic.sh .. \
    -DURHO3D_ANGELSCRIPT=0 -DURHO3D_LUA=0 -DURHO3D_URHO2D=0 \
    -DURHO3D_SAMPLES=0 -DURHO3D_TOOLS=0 -DURHO3D_PCH=0 -DURHO3D_MINIDUMPS=0 \
    -DURHO3D_FILEWATCHER=0

echo "Now compiling Urho3D engine..."
cd ..
make

echo "All done (for now )."
cd ..
