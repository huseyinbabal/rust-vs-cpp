// sort: read N u64 from a file (not timed), sort them (timed), checksum
#include "common.hpp"
#include <algorithm>
#include <cassert>
int main(int, char** argv) {
    auto v = read_u64s(argv[1]);
    auto t = Clock::now();
    std::sort(v.begin(), v.end());
    auto ms = t;
    uint64_t sum = 0;
    for (size_t i = 0; i < v.size(); ++i) sum += v[i] * (i + 1);
    if (!std::is_sorted(v.begin(), v.end())) return 1;
    report("sort", "std::sort", "sort", ms, ",\"n\":" + std::to_string(v.size()) + ",\"checksum\":" + std::to_string(sum));
}
