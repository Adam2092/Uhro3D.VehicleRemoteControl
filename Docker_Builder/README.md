The scripts in this folder are designed for generating a "builder" Docker image which contains (by far) necessary dependencies for compiling Urho3D applications.

The purpose of this specific "builder" image is to avoid duplication of cloning / compiling Urho3D library before compiling application code each time, thus reducing compilation time.
