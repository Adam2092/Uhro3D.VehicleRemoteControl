# This Dockerfile is for generating an image containing necessary dependencies
# As a builder for compiling code of Urho3D applications
FROM ubuntu:18.04
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y\
        ca-certificates cmake g++ make nano git \
        libglu1-mesa-dev freeglut3-dev mesa-common-dev \
        libx11-dev libxrandr-dev libasound2-dev libsdl2-dev
ADD . /opt/sources
WORKDIR /opt/sources
RUN cd /opt/sources && ./Urho3D-build.sh && \
    cp ./sources/* ./Urho3D/bin/Data/Textures/
