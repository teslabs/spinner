FROM ubuntu:22.04

# install dependencies
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    cmake \
    ninja-build \
    gperf \
    ccache \
    dfu-util \
    device-tree-compiler \
    wget \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-tk \
    python3-wheel \
    xz-utils \
    file \
    make \
    gcc \
    gcc-multilib \
    g++-multilib \
    libsdl2-dev \
    libmagic1 \
    doxygen \
 && rm -rf /var/lib/apt/lists/*

# install SDK
ARG ZSDK_VERSION=0.16.0
RUN mkdir /opt/toolchains && cd /opt/toolchains && \
    wget -q "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZSDK_VERSION}/zephyr-sdk-${ZSDK_VERSION}_linux-x86_64_minimal.tar.xz" && \
    tar xf zephyr-sdk-${ZSDK_VERSION}_linux-x86_64_minimal.tar.xz -C . && \
    ./zephyr-sdk-${ZSDK_VERSION}/setup.sh -t arm-zephyr-eabi -c

# install West
RUN pip3 install west
