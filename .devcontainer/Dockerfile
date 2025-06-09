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
        gdb \
        gnupg \
        libglu1-mesa-dev \
        libssl-dev \
        libtool \
        libx11-xcb-dev \
        libxi-dev \
        libxkbcommon-x11-dev \
        libxrender-dev \
        locales \
        lsb-release \
        m4 \
        ninja-build \
        patch \
        perl \
        pkg-config \
        software-properties-common \
        unzip \
        valgrind \
        wget \
        xz-utils \
        zip \
        '^libxcb.*-dev' && \
    rm -rf /var/lib/apt/lists/*

# # Set up ccache
# RUN /usr/sbin/update-ccache-symlinks && \
#     echo 'export PATH="/usr/lib/ccache:$PATH"' >> /etc/bash.bashrc

# # Install simple C++ dependencies with vcpkg
# RUN /usr/local/vcpkg/vcpkg install \
#     fmt \
#     nlohmann-json

# # Install all boost libs in seperate commands using vcpkg according to their levels on the boost dependency report
# # Level 0:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-callable-traits \
#     boost-config \
#     boost-crc \
#     boost-headers \
#     boost-hof \
#     boost-lambda2 \
#     boost-leaf \
#     boost-mp11 \
#     boost-pfr \
#     boost-predef \
#     boost-preprocessor \
#     boost-qvm \
#     boost-ratio

# # Level 1:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-assert \
#     boost-describe \
#     boost-endian \
#     boost-io \
#     boost-move \
#     boost-polygon \
#     boost-static-assert \
#     boost-typeof \
#     boost-vmd \
#     boost-winapi

# # Level 2:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-container-hash \
#     boost-intrusive \
#     boost-throw-exception \
#     boost-timer \
#     boost-type-traits \
#     boost-variant2

# # Level 3:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-array \
#     boost-compat \
#     boost-concept-check \
#     boost-container \
#     boost-core \
#     boost-hash2 \
#     boost-stl-interfaces \
#     boost-system \
#     boost-uuid

# # Level 4:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-align \
#     boost-bind \
#     boost-charconv \
#     boost-circular-buffer \
#     boost-detail \
#     boost-integer \
#     boost-interprocess \
#     boost-lexical-cast \
#     boost-logic \
#     boost-optional \
#     boost-scope \
#     boost-smart-ptr \
#     boost-stacktrace \
#     boost-tuple \
#     boost-type-index \
#     boost-unordered \
#     boost-utility

# # Level 5:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-any \
#     boost-atomic \
#     boost-conversion \
#     boost-dynamic-bitset \
#     boost-exception \
#     boost-format \
#     boost-function \
#     boost-json \
#     boost-mpl \
#     boost-interval \
#     boost-pool \
#     boost-rational \
#     boost-safe-numerics \
#     boost-static-string \
#     boost-url

# # Level 6:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-chrono \
#     boost-context \
#     boost-function-types \
#     boost-metaparse \
#     boost-numeric-conversion \
#     boost-outcome \
#     boost-random \
#     boost-regex \
#     boost-scope-exit \
#     boost-variant

# # Level 7:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-coroutine \
#     boost-coroutine2 \
#     boost-functional \
#     boost-local-function \
#     boost-math \
#     boost-tti

# # Level 8:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-fusion \
#     boost-multiprecision

# # Level 9:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-hana \
#     boost-iterator \
#     boost-parameter

# # Level 10:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-filesystem \
#     boost-heap \
#     boost-lambda \
#     boost-lockfree \
#     boost-multi-array \
#     boost-multi-index \
#     boost-parser \
#     boost-property-map \
#     boost-range \
#     boost-signals2 \
#     boost-tokenizer \
#     boost-yap

# # Level 11:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-algorithm \
#     boost-bimap \
#     boost-dll \
#     boost-flyweight \
#     boost-foreach \
#     boost-gil \
#     boost-iostreams \
#     boost-nowide \
#     boost-program-options \
#     boost-proto \
#     boost-ptr-container \
#     boost-sort \
#     boost-units

# # Level 12:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-assign \
#     boost-date-time \
#     boost-fiber \
#     boost-phoenix \
#     boost-test \
#     boost-xpressive

# # Level 13:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-asio \
#     boost-icl \
#     boost-thread

# # Level 14:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-beast \
#     boost-cobalt \
#     boost-contract \
#     boost-locale \
#     boost-mysql \
#     boost-process \
#     boost-redis \
#     boost-spirit \
#     boost-statechart \
#     boost-type-erasure

# # Level 15:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-convert \
#     boost-mqtt5 \
#     boost-poly-collection \
#     boost-serialization

# # Level 16:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-geometry \
#     boost-histogram \
#     boost-msm \
#     boost-property-tree \
#     boost-wave

# # Level 17:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-compute \
#     boost-graph \
#     boost-log

# # Level 18:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-ublas \
#     boost-python

# # Level 19:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-accumulators \
#     boost-mpi \
#     boost-parameter-python

# # Level 20:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-odeint \
#     boost-property-map-parallel

# # Level 21:
# RUN /usr/local/vcpkg/vcpkg install \
#     boost-graph-parallel

# Install boost
# RUN /usr/local/vcpkg/vcpkg install \
#     boost


    #libxtst-dev \
    # libltdl-dev \
    # libarchive-dev \

# Install Qt5 dependencies
# RUN apt-get update && \
#     apt-get install -y --no-install-recommends \
#         perl \
#         libxkbcommon-x11-dev \
#         '^libxcb.*-dev' \
#         libx11-xcb-dev \
#         libglu1-mesa-dev \
#         libxrender-dev \
#         libxi-dev \
#         libxkbcommon-dev \
#         libxkbcommon-x11-dev && \
#     rm -rf /var/lib/apt/lists/*


# RUN pip install --no-cache-dir \
#     jinja2

# Install problematic vcpkg dependencies
# RUN /usr/local/vcpkg/vcpkg install \
#     qt5-base
# RUN /usr/local/vcpkg/vcpkg install \
#     qt5-tools
# RUN /usr/local/vcpkg/vcpkg install \
#     qt5-sqlite

# Set VCPKG environment variables
ENV VCPKG_ROOT="/usr/local/vcpkg"
ENV PATH="$VCPKG_ROOT:$PATH"
ENV VCPKG_DEFAULT_BINARY_CACHE="/bincache"
ENV X_VCPKG_ASSET_SOURCES="clear;x-azurl,file:///assetcache,,readwrite"

# Set user back to vscode (if needed)
USER vscode
