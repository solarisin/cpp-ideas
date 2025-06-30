# syntax=docker/dockerfile:1
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# Install apt dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        autoconf \
        autoconf-archive \
        automake \
        bear \
        ca-certificates \
        ccache \
        cppcheck \
        curl \
        file \
        gnupg \
        ninja-build \
        patch \
        pkg-config \
        software-properties-common \
        unzip \
        valgrind \
        wget \
        xz-utils \
        zip && \
    rm -rf /var/lib/apt/lists/*

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libxinerama-dev \
        libxcursor-dev \
        libxrandr-dev \
        xorg-dev \
        libglu1-mesa-dev && \
    rm -rf /var/lib/apt/lists/*

# Build and install an appropriate version of CMake.
ARG CMAKE_VER=4.1.0-rc1
RUN NUM_CPUS=$(grep processor /proc/cpuinfo | wc -l) && \
    curl -OL https://github.com/Kitware/CMake/releases/download/v${CMAKE_VER}/cmake-${CMAKE_VER}.tar.gz && \
    tar xf cmake-${CMAKE_VER}.tar.gz && \
    cd cmake-${CMAKE_VER} && \
    ./bootstrap --parallel=${NUM_CPUS} -- -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_INSTALL_PREFIX=/usr && \
    make -j${NUM_CPUS} && \
    make install && \
    cd .. && \
    rm -rf cmake-${CMAKE_VER}*

ARG USERNAME="vscode"

# Persist bash history
RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
    && mkdir /commandhistory \
    && touch /commandhistory/.bash_history \
    && chown -R ${USERNAME} /commandhistory \
    && echo "$SNIPPET" >> "/home/${USERNAME}/.bashrc"

USER ${USERNAME}