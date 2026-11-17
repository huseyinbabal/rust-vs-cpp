// word count: wc <file> <load|stream> <std|reserve|fast>
//   load   = whole file in memory first (not timed), words borrowed as &str
//   stream = 1 MiB read() chunks, timed end to end, words copied only when new
use rbench::*;
use std::collections::HashMap;
use std::hash::BuildHasher;
use std::io::Read;
use std::time::Instant;

fn finish<S: BuildHasher, K: AsRef<[u8]> + std::hash::Hash + Eq>(m: &HashMap<K, u64, S>, variant: &str, mode: &str, t: Instant) {
    let ms = t;
    let mut sum: u64 = 0; let mut words: u64 = 0;
    for (k, c) in m { sum = sum.wrapping_add(fnv(k.as_ref()).wrapping_mul(*c)); words += c; }
    report(&format!("wc_{mode}"), variant, "count", ms, &format!(",\"unique\":{},\"words\":{},\"checksum\":{}", m.len(), words, sum));
}

fn load<'a, S: BuildHasher + Default>(text: &'a [u8], mut m: HashMap<&'a [u8], u64, S>) -> HashMap<&'a [u8], u64, S> {
    for w in text.split(|b| b.is_ascii_whitespace()).filter(|w| !w.is_empty()) { *m.entry(w).or_insert(0) += 1; }
    m
}

fn stream<S: BuildHasher + Default>(path: &str, mut m: HashMap<Vec<u8>, u64, S>) -> HashMap<Vec<u8>, u64, S> {
    let mut f = std::fs::File::open(path).unwrap();
    let mut buf = vec![0u8; 1 << 20];
    let mut chunk: Vec<u8> = Vec::with_capacity(2 << 20);
    loop {
        let n = f.read(&mut buf).unwrap();
        if n == 0 { break; }
        chunk.extend_from_slice(&buf[..n]);
        let cut = chunk.iter().rposition(|b| b.is_ascii_whitespace()).map_or(0, |i| i + 1);
        for w in chunk[..cut].split(|b| b.is_ascii_whitespace()).filter(|w| !w.is_empty()) {
            if let Some(c) = m.get_mut(w) { *c += 1 } else { m.insert(w.to_vec(), 1); }
        }
        chunk.drain(..cut);
    }
    for w in chunk.split(|b| b.is_ascii_whitespace()).filter(|w| !w.is_empty()) {
        if let Some(c) = m.get_mut(w) { *c += 1 } else { m.insert(w.to_vec(), 1); }
    }
    m
}

fn main() {
    let a: Vec<String> = std::env::args().collect();
    let (path, mode, variant) = (&a[1], a[2].as_str(), a[3].as_str());
    let cap = a.get(4).map(|s| s.parse::<usize>().unwrap()).unwrap_or(0); // expected unique words, for "reserve"
    if mode == "load" {
        let text = read_all(path);
        let t = Instant::now();
        match variant {
            "std" => finish(&load(&text, HashMap::new()), variant, mode, t),
            "reserve" => finish(&load(&text, HashMap::with_capacity(cap)), variant, mode, t),
            "fast" => finish(&load(&text, HashMap::with_capacity_and_hasher(cap, FastBuild::default())), variant, mode, t),
            _ => panic!("variant"),
        }
    } else {
        let t = Instant::now();
        match variant {
            "std" => finish(&stream(path, HashMap::new()), variant, mode, t),
            "reserve" => finish(&stream(path, HashMap::with_capacity(cap)), variant, mode, t),
            "fast" => finish(&stream(path, HashMap::with_capacity_and_hasher(cap, FastBuild::default())), variant, mode, t),
            _ => panic!("variant"),
        }
    }
}
