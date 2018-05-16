FROM ubuntu:16.04 as builder
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y\
        ca-certificates cmake g++ make git \
        libglu1-mesa-dev freeglut3-dev mesa-common-dev \
        libx11-dev libxrandr-dev libasound2-dev
ADD . /opt/sources
WORKDIR /opt/sources
RUN cd /opt/sources && \
    git clone https://github.com/urho3d/Urho3D && \
    cd Urho3D && ./cmake_clean.sh && \
    ./cmake_generic.sh . \
    -DURHO3D_ANGELSCRIPT=0 -DURHO3D_LUA=0 -DURHO3D_URHO2D=0 \
    -DURHO3D_SAMPLES=0 -DURHO3D_TOOLS=0 && \
    make && cd .. && \
    mkdir build && \
    cd build && \
    cmake .. && make && \
    cp -r ../bin /tmp && \
    cp ./testRemoteControl /tmp/bin

# Deploy.
FROM ubuntu:16.04
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y \
    libglu1-mesa freeglut3 mesa-common-dev \
        libx11-dev libxrandr-dev libasound2
ADD ./bin /opt
WORKDIR /opt
COPY --from=builder /tmp/bin/testRemoteControl .
CMD ["/opt/testRemoteControl"]
