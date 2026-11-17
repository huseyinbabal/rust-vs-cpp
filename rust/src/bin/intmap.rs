// integer keys: intmap <keys.bin> <n> <std|fast>; first n u64 are inserted, the next n are misses
use rbench::*;
use std::collections::HashMap;
use std::hash::BuildHasher;
use std::time::Instant;
fn run<S: BuildHasher>(keys: &[u64], n: usize, mut m: HashMap<u64, u64, S>, variant: &str) {
    let (ins, miss) = (&keys[..n], &keys[n..2 * n]);
    let t = Instant::now();
    for (i, &k) in ins.iter().enumerate() { m.insert(k, i as u64); }
    report("intmap", variant, "insert", t, &format!(",\"n\":{n}"));
    let t = Instant::now(); let mut s: u64 = 0;
    for &k in ins { s = s.wrapping_add(*m.get(&k).unwrap()); }
    report("intmap", variant, "hit", t, &format!(",\"n\":{n},\"checksum\":{s}"));
    let t = Instant::now(); let mut c = 0u64;
    for &k in miss { if m.contains_key(&k) { c += 1; } }
    report("intmap", variant, "miss", t, &format!(",\"n\":{n},\"found\":{c}"));
    let t = Instant::now();
    for &k in &ins[..n / 2] { m.remove(&k); }
    report("intmap", variant, "erase", t, &format!(",\"n\":{n},\"left\":{}", m.len()));
}
fn main() {
    let a: Vec<String> = std::env::args().collect();
    let keys = read_u64s(&a[1]); let n: usize = a[2].parse().unwrap();
    match a[3].as_str() {
        "std" => run(&keys, n, HashMap::new(), "std"),
        "fast" => run(&keys, n, HashMap::with_hasher(FastBuild::default()), "fast"),
        _ => panic!(),
    }
}
