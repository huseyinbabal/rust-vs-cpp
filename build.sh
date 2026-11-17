#!/bin/bash
# Builds every benchmark: Rust (release + profiling), C++ with clang (same LLVM major as rustc) and with the newest GCC
set -e
cd /root/bench; . /root/.cargo/env; . /root/toolchain.env
(cd rust && cargo build --release -q && cargo build --profile profiling -q)
ABSL="$(pkg-config --libs absl_flat_hash_map absl_hash absl_raw_hash_set) -I/root/boost"
mkdir -p cpp/build/clang cpp/build/gcc cpp/build/prof
for p in sort wc intmap pwc; do
  clang++-$LLVM -O3 -DNDEBUG -march=native -flto -std=c++23 -fopenmp -DCOMPILER='"clang"' cpp/$p.cpp -o cpp/build/clang/$p $ABSL
  g++-$GCC     -O3 -DNDEBUG -march=native -flto -std=c++23 -fopenmp -DCOMPILER='"gcc"'   cpp/$p.cpp -o cpp/build/gcc/$p   $ABSL
  clang++-$LLVM -O3 -DNDEBUG -march=native -g -fno-omit-frame-pointer -std=c++23 -fopenmp -DCOMPILER='"clang"' cpp/$p.cpp -o cpp/build/prof/$p $ABSL
done
{ rustc --version; rustc -vV | grep LLVM; clang++-$LLVM --version | head -1; g++-$GCC --version | head -1; echo "abseil $ABSEIL"; } > results/versions.txt
cat results/versions.txt
