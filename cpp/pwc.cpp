// parallel word count: 64 chunks cut at whitespace, one SwissTable map per chunk in parallel (OpenMP, dynamic), then single-thread merge
#include "common.hpp"
#include <absl/container/flat_hash_map.h>
#include <omp.h>
using M = absl::flat_hash_map<std::string_view, uint64_t, FastStr>;
int main(int, char** argv) {
    std::string text = read_all(argv[1]);
    auto t = Clock::now();
    std::vector<size_t> cuts{0};
    for (size_t k = 1; k < 64; ++k) {
        size_t p = text.size() * k / 64;
        while (p < text.size() && !is_ws(text[p])) ++p;
        cuts.push_back(p);
    }
    cuts.push_back(text.size());
    std::vector<M> parts(64);
    std::string_view tv(text);
    #pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < 64; ++i)
        for_each_word(tv.substr(cuts[i], cuts[i + 1] - cuts[i]), [&](std::string_view w) { ++parts[i][w]; });
    int threads = omp_get_max_threads();
    report("pwc", "openmp", "count", t, ",\"threads\":" + std::to_string(threads));
    t = Clock::now();
    M all = std::move(parts[0]);
    for (int i = 1; i < 64; ++i) for (auto& [k, c] : parts[i]) all[k] += c;
    uint64_t sum = 0; for (auto& [k, c] : all) sum += fnv(k) * c;
    report("pwc", "openmp", "merge", t, ",\"threads\":" + std::to_string(threads) + ",\"unique\":" + std::to_string(all.size()) + ",\"checksum\":" + std::to_string(sum));
}
