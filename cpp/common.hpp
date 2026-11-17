// Shared helpers: identical hash functions and file loading (same code as rust/src/lib.rs)
#pragma once
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

// FNV-1a 64 for strings, Fx-style multiply for u64 keys
inline uint64_t fnv(std::string_view s) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (unsigned char b : s) { h ^= b; h *= 0x100000001b3ULL; }
    return h;
}
struct FastStr {
    using is_transparent = void;
    size_t operator()(std::string_view s) const { return fnv(s); }
};
struct FastU64 {
    size_t operator()(uint64_t x) const { return (std::rotl(uint64_t{0}, 5) ^ x) * 0x517cc1b727220a95ULL; }
};
struct StdStr {  // std::hash, made transparent so lookups by string_view don't allocate (Rust's HashMap<Vec<u8>> does the same)
    using is_transparent = void;
    size_t operator()(std::string_view s) const { return std::hash<std::string_view>{}(s); }
};
struct Eq { using is_transparent = void; bool operator()(std::string_view a, std::string_view b) const { return a == b; } };

inline std::string read_all(const char* path) {
    std::ifstream f(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(f), {});
}
inline std::vector<uint64_t> read_u64s(const char* path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    auto n = static_cast<size_t>(f.tellg()) / 8;
    std::vector<uint64_t> v(n);
    f.seekg(0);
    f.read(reinterpret_cast<char*>(v.data()), n * 8);
    return v;
}
inline bool is_ws(unsigned char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v'; }
template <class F> void for_each_word(std::string_view t, F&& f) {
    size_t i = 0, n = t.size();
    while (i < n) {
        while (i < n && is_ws(t[i])) ++i;
        size_t j = i;
        while (j < n && !is_ws(t[j])) ++j;
        if (j > i) f(t.substr(i, j - i));
        i = j;
    }
}
using Clock = std::chrono::steady_clock;
#ifndef COMPILER
#define COMPILER "cpp"
#endif
inline void report(const char* test, const char* variant, const char* phase, Clock::time_point t, const std::string& extra) {
    double ms = std::chrono::duration<double, std::milli>(Clock::now() - t).count();
    std::printf("{\"lang\":\"%s\",\"test\":\"%s\",\"variant\":\"%s\",\"phase\":\"%s\",\"ms\":%.3f%s}\n", COMPILER, test, variant, phase, ms, extra.c_str());
}
