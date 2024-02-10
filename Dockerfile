FROM gcc:latest

RUN  \
     dpkg --add-architecture arm64 \
     && dpkg --add-architecture armel \
     && apt update \
     && apt upgrade -yy \
     && apt install -yy \
     cmake \
     g++-arm-linux-gnueabi \
     g++-aarch64-linux-gnu \
     socat
