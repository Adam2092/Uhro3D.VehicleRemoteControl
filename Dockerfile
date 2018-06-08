FROM ubuntu:18.04 as builder
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y\
        ca-certificates cmake g++ make git \
        libglu1-mesa-dev freeglut3-dev mesa-common-dev \
        libx11-dev libxrandr-dev libasound2-dev
ADD . /opt/sources
WORKDIR /opt/sources
RUN cd /opt/sources && ./Urho3D-build.sh
RUN mkdir build && \
    cd build && \
    cmake .. && make && \
    cp -r ../Urho3D/bin/Data/ /tmp && \ 
    cp -r ../Urho3D/bin/CoreData /tmp &&\
    cp ./testRemoteControl /tmp &&\
    cp ./Listener /tmp

# Deploy.
FROM ubuntu:18.04
MAINTAINER Yue Kang yuek@chalmers.se
RUN apt update && \
    apt install -y \
    libglu1-mesa freeglut3 mesa-common-dev \
        libx11-dev libxrandr-dev libasound2
# ADD ./bin /opt
WORKDIR /opt
# TODO: MODIFY THE FOLLOWING LINES!!!
# COPY --from=builder /tmp/Data .
# COPY --from=builder /tmp/CoreData .
# COPY --from=builder /tmp/testRemoteControl .
COPY --from=builder /tmp .
CMD ["/opt/testRemoteControl"]
