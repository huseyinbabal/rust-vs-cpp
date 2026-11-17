//! Shared helpers: identical hash functions and file loading for every benchmark (same code in cpp/common.hpp).
use std::hash::{BuildHasherDefault, Hasher};
use std::time::Instant;

/// FNV-1a 64 for strings, Fx-style multiply for u64 keys. The C++ side uses the exact same functions.
#[derive(Default)]
pub struct Fast(u64);
impl Hasher for Fast {
    fn write(&mut self, bytes: &[u8]) {
        let mut h: u64 = if self.0 == 0 { 0xcbf29ce484222325 } else { self.0 };
        for &b in bytes { h ^= b as u64; h = h.wrapping_mul(0x100000001b3); }
        self.0 = h;
    }
    fn write_u8(&mut self, _: u8) {} // str's Hash impl appends 0xff; ignored so both languages hash the same bytes
    fn write_usize(&mut self, _: usize) {} // slices hash their length first; ignored for the same reason
    fn write_u64(&mut self, x: u64) { self.0 = (self.0.rotate_left(5) ^ x).wrapping_mul(0x517cc1b727220a95); }
    fn finish(&self) -> u64 { self.0 }
}
pub type FastBuild = BuildHasherDefault<Fast>;

pub fn fnv(s: &[u8]) -> u64 { let mut h = Fast::default(); h.write(s); h.0 }

pub fn read_all(path: &str) -> Vec<u8> { std::fs::read(path).expect("read input") }

pub fn read_u64s(path: &str) -> Vec<u64> {
    let b = read_all(path);
    b.chunks_exact(8).map(|c| u64::from_le_bytes(c.try_into().unwrap())).collect()
}

/// one JSON line per measured phase; the parent script collects them
pub fn report(test: &str, variant: &str, phase: &str, t: Instant, extra: &str) {
    println!("{{\"lang\":\"rust\",\"test\":\"{test}\",\"variant\":\"{variant}\",\"phase\":\"{phase}\",\"ms\":{:.3}{extra}}}",
             t.elapsed().as_secs_f64() * 1e3);
}
