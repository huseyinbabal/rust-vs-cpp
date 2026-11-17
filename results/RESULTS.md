# Rust vs C++ — results (DO c-16, Xeon Platinum 8358, 16 cores, Ubuntu 26.04.1; median of 10 runs, 100M intmap: 3)

Versions: rustc 1.100.0 (LLVM 23.1.5), clang 23.1.6, g++ 16.0.1 (experimental snapshot), Abseil 20260817.0, Boost 1.92.0

## 1. Sort 100M u64 (read from file, not timed)
| | Rust | clang | gcc |
|---|---|---|---|
| std default (sort_unstable / std::sort) | 2.70 s | 10.01 s | 10.23 s |
| C++ pdqsort | – | 4.02 s | 3.79 s |

## 2a. Word count, load (whole file in memory first, not timed) — 499 MB, 61.2M words, 871,375 unique
| variant | Rust | clang | gcc |
|---|---|---|---|
| default (SipHash / unordered_map) | 4.70 s | 7.11 s | 7.15 s |
| + reserve | 4.66 s | 6.95 s | 6.90 s |
| + same FNV-1a hash | 3.37 s | 6.93 s | 7.17 s |
| C++ absl::flat_hash_map (SwissTable, same hash) | – | 3.57 s | 3.90 s |
| C++ boost::unordered_flat_map (same hash) | – | 3.61 s | 4.00 s |

## 2b. Word count, stream (1 MiB read() chunks, timed end to end) — 499 MB, 61.2M words, 871,375 unique
| variant | Rust | clang | gcc |
|---|---|---|---|
| default (SipHash / unordered_map) | 4.92 s | 6.11 s | 6.63 s |
| + reserve | 5.09 s | 6.21 s | 6.33 s |
| + same FNV-1a hash | 3.45 s | 6.12 s | 6.49 s |
| C++ absl::flat_hash_map (SwissTable, same hash) | – | 2.54 s | 2.82 s |
| C++ boost::unordered_flat_map (same hash) | – | 2.81 s | 2.89 s |

## 3. Integer keys, 10M (insert / hit / miss / erase half)
| variant | Rust | clang | gcc |
|---|---|---|---|
| default (SipHash / unordered_map std::hash) | 879 ms / 905 ms / 312 ms / 422 ms | 3.33 s / 640 ms / 856 ms / 1.43 s | 3.33 s / 662 ms / 808 ms / 1.44 s |
| same Fx hash (Rust HashMap / unordered_map) | 667 ms / 397 ms / 121 ms / 224 ms | 3.60 s / 667 ms / 888 ms / 1.50 s | 3.60 s / 690 ms / 833 ms / 1.51 s |
| C++ absl::flat_hash_map + Fx | – | 645 ms / 387 ms / 357 ms / 280 ms | 718 ms / 377 ms / 349 ms / 279 ms |
| C++ boost::unordered_flat_map + Fx | – | 1.28 s / 387 ms / 109 ms / 485 ms | 587 ms / 379 ms / 106 ms / 197 ms |

## 3. Integer keys, 100M (insert / hit / miss / erase half)
| variant | Rust | clang | gcc |
|---|---|---|---|
| default (SipHash / unordered_map std::hash) | 12.63 s / 13.04 s / 6.97 s / 6.61 s | 44.66 s / 10.71 s / 13.61 s / 20.26 s | 45.07 s / 11.32 s / 13.32 s / 21.88 s |
| same Fx hash (Rust HashMap / unordered_map) | 9.21 s / 6.70 s / 3.29 s / 3.74 s | 49.31 s / 11.72 s / 15.86 s / 24.29 s | 48.34 s / 12.36 s / 15.51 s / 23.81 s |
| C++ absl::flat_hash_map + Fx | – | 9.26 s / 6.50 s / 6.83 s / 4.49 s | 11.17 s / 6.39 s / 6.51 s / 4.29 s |
| C++ boost::unordered_flat_map + Fx | – | 28.76 s / 6.26 s / 2.79 s / 9.81 s | 8.05 s / 6.22 s / 2.79 s / 3.28 s |

## 4. Parallel word count (64 chunks; count in parallel / single-thread merge)
| threads | Rust (Rayon) | clang (OpenMP) | gcc (OpenMP) |
|---|---|---|---|
| 1 | 2.37 s + 1.06 s | 2.63 s + 889 ms | 2.72 s + 919 ms |
| 2 | 1.21 s + 1.05 s | 1.36 s + 887 ms | 1.77 s + 937 ms |
| 4 | 639 ms + 1.06 s | 706 ms + 890 ms | 1.26 s + 923 ms |
| 8 | 385 ms + 1.06 s | 424 ms + 857 ms | 767 ms + 953 ms |
