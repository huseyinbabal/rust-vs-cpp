// word count: wc <file> <load|stream> <std|reserve|fast|swiss> [unique]
#include "common.hpp"
#include <absl/container/flat_hash_map.h>
#include <boost/unordered/unordered_flat_map.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>

template <class M> void finish(const M& m, const char* variant, const std::string& test, Clock::time_point t) {
    auto ms = t; uint64_t sum = 0, words = 0;
    for (auto& [k, c] : m) { sum += fnv(k) * c; words += c; }
    report(test.c_str(), variant, "count", ms, ",\"unique\":" + std::to_string(m.size()) + ",\"words\":" + std::to_string(words) + ",\"checksum\":" + std::to_string(sum));
}
template <class M> void load(std::string_view text, M& m) { for_each_word(text, [&](std::string_view w) { ++m[w]; }); }
template <class M> void add(M& m, std::string_view w) {
    if (auto it = m.find(w); it != m.end()) ++it->second; else m.emplace(std::string(w), 1);
}
template <class M> void stream(const char* path, M& m) {
    int fd = ::open(path, O_RDONLY);
    std::vector<char> buf(1 << 20); std::string chunk; chunk.reserve(2 << 20);
    while (true) {
        ssize_t n = ::read(fd, buf.data(), buf.size());
        if (n <= 0) break;
        chunk.append(buf.data(), n);
        size_t cut = chunk.size();
        while (cut > 0 && !is_ws(chunk[cut - 1])) --cut;
        for_each_word(std::string_view(chunk).substr(0, cut), [&](std::string_view w) { add(m, w); });
        chunk.erase(0, cut);
    }
    for_each_word(chunk, [&](std::string_view w) { add(m, w); });
    ::close(fd);
}
int main(int argc, char** argv) {
    std::string mode = argv[2], v = argv[3]; size_t cap = argc > 4 ? std::stoull(argv[4]) : 0;
    std::string test = "wc_" + mode;
    if (mode == "load") {
        std::string text = read_all(argv[1]);
        auto t = Clock::now();
        if (v == "std") { std::unordered_map<std::string_view, uint64_t> m; load(text, m); finish(m, "std", test, t); }
        else if (v == "reserve") { std::unordered_map<std::string_view, uint64_t> m; m.reserve(cap); load(text, m); finish(m, "reserve", test, t); }
        else if (v == "fast") { std::unordered_map<std::string_view, uint64_t, FastStr> m; m.reserve(cap); load(text, m); finish(m, "fast", test, t); }
        else if (v == "swiss") { absl::flat_hash_map<std::string_view, uint64_t, FastStr> m; m.reserve(cap); load(text, m); finish(m, "swiss", test, t); }
        else if (v == "boost") { boost::unordered_flat_map<std::string_view, uint64_t, FastStr> m; m.reserve(cap); load(text, m); finish(m, "boost", test, t); }
    } else {
        auto t = Clock::now();
        if (v == "std") { std::unordered_map<std::string, uint64_t, StdStr, Eq> m; stream(argv[1], m); finish(m, "std", test, t); }
        else if (v == "reserve") { std::unordered_map<std::string, uint64_t, StdStr, Eq> m; m.reserve(cap); stream(argv[1], m); finish(m, "reserve", test, t); }
        else if (v == "fast") { std::unordered_map<std::string, uint64_t, FastStr, Eq> m; m.reserve(cap); stream(argv[1], m); finish(m, "fast", test, t); }
        else if (v == "swiss") { absl::flat_hash_map<std::string, uint64_t, FastStr, Eq> m; m.reserve(cap); stream(argv[1], m); finish(m, "swiss", test, t); }
        else if (v == "boost") { boost::unordered_flat_map<std::string, uint64_t, FastStr, Eq> m; m.reserve(cap); stream(argv[1], m); finish(m, "boost", test, t); }
    }
}
