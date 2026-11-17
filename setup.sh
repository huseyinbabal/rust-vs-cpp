#!/bin/bash
# Toolchain for the Rust vs C++ benchmark, latest of everything (Ubuntu 26.04 droplet)
set -ex
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y build-essential cmake ninja-build git curl wget lsb-release software-properties-common gnupg python3-numpy \
  linux-tools-common linux-tools-$(uname -r) heaptrack time pkg-config
# Rust: latest stable
curl -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal --default-toolchain stable
. "$HOME/.cargo/env"
rustc -vV
LLVM=$(rustc -vV | sed -n 's/LLVM version: \([0-9]*\).*/\1/p')
# clang with the same LLVM major as rustc (apt.llvm.org)
wget -qO /tmp/llvm.sh https://apt.llvm.org/llvm.sh && chmod +x /tmp/llvm.sh && /tmp/llvm.sh $LLVM all
# newest GCC in the archive
GCC=$(apt-cache search --names-only '^g\+\+-[0-9]+$' | sed 's/g++-\([0-9]*\).*/\1/' | sort -n | tail -1)
apt-get install -y g++-$GCC
echo "LLVM=$LLVM GCC=$GCC" > /root/toolchain.env
# Abseil: latest release, built with -O3 for this CPU
TAG=$(curl -s https://api.github.com/repos/abseil/abseil-cpp/releases/latest | python3 -c 'import json,sys;print(json.load(sys.stdin)["tag_name"])')
git clone --depth 1 -b $TAG https://github.com/abseil/abseil-cpp /root/abseil
cmake -S /root/abseil -B /root/abseil/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_FLAGS="-O3 -march=native" -DABSL_PROPAGATE_CXX_STD=ON -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build /root/abseil/build && cmake --install /root/abseil/build
echo "ABSEIL=$TAG" >> /root/toolchain.env
# hyperfine + flame graphs (latest from crates.io)
cargo install hyperfine inferno
echo SETUP_DONE
