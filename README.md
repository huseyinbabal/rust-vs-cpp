# Rust vs C++: same programs, same machine

The same four programs written in Rust and in C++, built with the same LLVM version, run on one 16-core DigitalOcean server.
Every program reads its input from a file outside the timed region and prints a checksum; a run only counts if Rust, Clang and GCC
builds print the same checksum. No async anywhere: parallel code uses plain operating system threads (Rayon / OpenMP).

| test | what it does |
|---|---|
| `sort` | sort 100M random `u64` with each standard library sort (`sort_unstable`, `std::sort`), plus C++ `pdqsort` |
| `wc` | count every word in a ~500 MB text file: whole file in memory (`load`) and 1 MiB `read()` chunks (`stream`); default hash map, `reserve`, same FNV-1a hash, Abseil `flat_hash_map`, Boost `unordered_flat_map` |
| `intmap` | 10M and 100M integer keys: insert / hit / miss / erase half |
| `pwc` | the word count split over 1–8 threads: Rayon in Rust, OpenMP in C++ |

Results (median of 10 runs) are in [results/RESULTS.md](results/RESULTS.md), raw numbers in `results/summary.json` and the
`*.jsonl` files, perf / heaptrack / flame graph output in `results/prof/`. Tool versions: [results/versions.txt](results/versions.txt).

## Run it

On a fresh Ubuntu 26.04 machine, as root, with this repo at `/root/bench`:

```sh
./setup.sh                 # Rust stable, clang with rustc's LLVM major, newest GCC, Abseil, hyperfine, inferno
./setup_boost.sh           # Boost (unordered_flat_map)
python3 data/gen.py /root/data
./build.sh                 # Rust release, C++ with clang and gcc: -O3 -DNDEBUG -march=native -flto -std=c++23
UNIQUE=871375 python3 bench.py   # unique words in the generated corpus (reserve size)
python3 bench_boost.py
./prof.sh                  # perf stat, flame graphs, heaptrack
```

Numbers hold for this machine (Intel Xeon Platinum 8358, 16 vCPU) and this code; other CPUs, compilers or flags can move them.

`cpp/pdqsort.h` is Orson Peters' pattern-defeating quicksort (zlib license, header kept).
