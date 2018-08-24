FROM ubuntu:18.04 as builder
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y\
        ca-certificates cmake g++ make nano git \
        libglu1-mesa-dev freeglut3-dev mesa-common-dev \
        libx11-dev libxrandr-dev libasound2-dev libsdl2-dev
ADD . /opt/sources
WORKDIR /opt/sources
RUN cd /opt/sources && ./Urho3D-build.sh
RUN mkdir build && \
    cd build && cmake .. && make && \
    cp -r ../Urho3D/bin/ /tmp && \
    cp ./testRemoteControl /tmp/bin && \
    cp ./Listener /tmp/bin && \
    cp ./CameraViewer /tmp/bin

# Deploy.
FROM ubuntu:18.04
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y \
    libglu1-mesa freeglut3 mesa-common-dev \
        libx11-dev libxrandr-dev libasound2 libsdl2-dev
WORKDIR /opt
COPY --from=builder /tmp/bin/ .

CMD ["/opt/testRemoteControl"]
