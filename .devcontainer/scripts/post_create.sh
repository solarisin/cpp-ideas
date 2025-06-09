#!/bin/bash
set -e

# Set up ccache
/usr/sbin/update-ccache-symlinks && echo 'export PATH="/usr/lib/ccache:$PATH"' >> "/${HOME}/.bashrc"
ccache -o cache_dir="/ccache"

# Set environment variables for vcpkg to use LLVM/Clang and ccache
echo 'export CC=/usr/lib/ccache/clang' >> "/${HOME}/.bashrc"
echo 'export CXX=/usr/lib/ccache/clang++' >> "/${HOME}/.bashrc"

# sudo apt-get update && \
#     sudo apt-get install -y --no-install-recommends \
#         ninja-build && \
#     sudo rm -rf /var/lib/apt/lists/*