#!/bin/bash
echo "Now cloning git repository of Urho3D..."

git clone https://github.com/urho3d/Urho3D
cd Urho3D

echo "Now getting ready to compile Urho3D engine..."

./cmake_clean.sh
./cmake_generic.sh . \
    -DURHO3D_ANGELSCRIPT=0 -DURHO3D_LUA=0 -DURHO3D_URHO2D=0 \
    -DURHO3D_SAMPLES=0 -DURHO3D_TOOLS=0

echo "Now compiling Urho3D engine..."
make

echo "All done (for now )."
cd ..
