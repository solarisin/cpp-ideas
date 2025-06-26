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

ARG USERNAME="vscode"

# Persist bash history
RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
    && mkdir /commandhistory \
    && touch /commandhistory/.bash_history \
    && chown -R ${USERNAME} /commandhistory \
    && echo "$SNIPPET" >> "/home/${USERNAME}/.bashrc"

USER ${USERNAME}