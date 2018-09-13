FROM urho3d-builder:v0.2 as builder
MAINTAINER Yue Kang yuek@chalmers.se
ADD . /opt/sources
WORKDIR /opt/sources
RUN mkdir build && \
    cd build && cmake .. && make && \
    cp -r ../Urho3D/bin/ /tmp && \
    cp ../data/*.png /tmp/bin/Data/Textures/ && \
    cp ./testRemoteControl /tmp/bin && \
    cp ./Listener /tmp/bin && \
    cp ./CameraViewer /tmp/bin

# Deploy
FROM ubuntu:18.04
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y \
    libglu1-mesa freeglut3 mesa-common-dev \
        libx11-dev libxrandr-dev libasound2 libsdl2-dev
WORKDIR /opt
COPY --from=builder /tmp/bin/ .

CMD ["/opt/testRemoteControl"]
