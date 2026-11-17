// sort with pdqsort (Orson Peters), the algorithm Rust's sort_unstable grew out of: same input, same checksum as sort.cpp
#include "common.hpp"
#include "pdqsort.h"
#include <algorithm>
int main(int, char** argv) {
    auto v = read_u64s(argv[1]);
    auto t = Clock::now();
    pdqsort(v.begin(), v.end());
    auto ms = t;
    uint64_t sum = 0;
    for (size_t i = 0; i < v.size(); ++i) sum += v[i] * (i + 1);
    if (!std::is_sorted(v.begin(), v.end())) return 1;
    report("sort", "pdqsort", "sort", ms, ",\"n\":" + std::to_string(v.size()) + ",\"checksum\":" + std::to_string(sum));
}
