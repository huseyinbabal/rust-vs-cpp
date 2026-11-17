// integer keys: intmap <keys.bin> <n> <std|fast|swiss>; first n u64 inserted, next n are misses
#include "common.hpp"
#include <absl/container/flat_hash_map.h>
#include <boost/unordered/unordered_flat_map.hpp>
#include <unordered_map>
template <class M> void run(const std::vector<uint64_t>& keys, size_t n, M m, const char* v) {
    auto N = ",\"n\":" + std::to_string(n);
    auto t = Clock::now();
    for (size_t i = 0; i < n; ++i) m.emplace(keys[i], i);
    report("intmap", v, "insert", t, N);
    t = Clock::now(); uint64_t s = 0;
    for (size_t i = 0; i < n; ++i) s += m.find(keys[i])->second;
    report("intmap", v, "hit", t, N + ",\"checksum\":" + std::to_string(s));
    t = Clock::now(); uint64_t c = 0;
    for (size_t i = n; i < 2 * n; ++i) c += m.contains(keys[i]);
    report("intmap", v, "miss", t, N + ",\"found\":" + std::to_string(c));
    t = Clock::now();
    for (size_t i = 0; i < n / 2; ++i) m.erase(keys[i]);
    report("intmap", v, "erase", t, N + ",\"left\":" + std::to_string(m.size()));
}
int main(int, char** argv) {
    auto keys = read_u64s(argv[1]); size_t n = std::stoull(argv[2]); std::string v = argv[3];
    if (v == "std") run(keys, n, std::unordered_map<uint64_t, uint64_t>{}, "std");
    else if (v == "fast") run(keys, n, std::unordered_map<uint64_t, uint64_t, FastU64>{}, "fast");
    else if (v == "swiss") run(keys, n, absl::flat_hash_map<uint64_t, uint64_t, FastU64>{}, "swiss");
    else if (v == "boost") run(keys, n, boost::unordered_flat_map<uint64_t, uint64_t, FastU64>{}, "boost");
}
