# syntax=docker/dockerfile:1
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

# Install apt dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        autoconf \
        autoconf-archive \
        automake \
        bear \
        bison \
        ca-certificates \
        ccache \
        cppcheck \
        curl \
        file \
        flex \
        fontconfig \
        gdb \
        gnupg \
        libltdl-dev \
        libssl-dev \
        libtool \
        '^libxcb.*-dev' \
        libx11-xcb-dev \
        libglu1-mesa-dev \
        libxrender-dev \
        libxi-dev \
        libxkbcommon-dev \
        libxkbcommon-x11-dev \
        libegl1-mesa-dev \
        librsvg2-dev \
        libgts-dev \
        libgd-dev \
        libudev-dev \
        locales \
        lsb-release \
        m4 \
        ninja-build \
        patch \
        perl \
        pkg-config \
        python3-dev \
        python3-pip \
        software-properties-common \
        unzip \
        valgrind \
        wget \
        xz-utils \
        zip \
        '^libxcb.*-dev' && \
    rm -rf /var/lib/apt/lists/*

RUN pip install --no-cache-dir \
    jinja2 \
    --break-system-packages

# Set VCPKG environment variables
# ENV VCPKG_ROOT="/usr/local/vcpkg"
# ENV PATH="$VCPKG_ROOT:$PATH"
# ENV VCPKG_DEFAULT_BINARY_CACHE="/bincache"
# ENV X_VCPKG_ASSET_SOURCES="clear;x-azurl,file:///assetcache,,readwrite"

ARG USERNAME="vscode"

# Persist bash history
RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
    && mkdir /commandhistory \
    && touch /commandhistory/.bash_history \
    && chown -R ${USERNAME} /commandhistory \
    && echo "$SNIPPET" >> "/home/${USERNAME}/.bashrc"

USER ${USERNAME}