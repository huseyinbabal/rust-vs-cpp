// parallel word count: pwc <file> ; threads from RAYON_NUM_THREADS. 64 chunks cut at whitespace,
// one map per chunk in parallel (timed "count"), then a single-thread merge (timed "merge")
use rbench::*;
use rayon::prelude::*;
use std::collections::HashMap;
use std::time::Instant;
type M<'a> = HashMap<&'a [u8], u64, FastBuild>;
fn main() {
    let a: Vec<String> = std::env::args().collect();
    let text = read_all(&a[1]);
    let t = Instant::now();
    let mut cuts = vec![0usize];
    for k in 1..64 {
        let mut p = text.len() * k / 64;
        while p < text.len() && !text[p].is_ascii_whitespace() { p += 1; }
        cuts.push(p);
    }
    cuts.push(text.len());
    let parts: Vec<M> = (0..64).into_par_iter().map(|i| {
        let mut m = M::default();
        for w in text[cuts[i]..cuts[i + 1]].split(|b| b.is_ascii_whitespace()).filter(|w| !w.is_empty()) { *m.entry(w).or_insert(0) += 1; }
        m
    }).collect();
    let threads = rayon::current_num_threads();
    report("pwc", "rayon", "count", t, &format!(",\"threads\":{threads}"));
    let t = Instant::now();
    let mut it = parts.into_iter(); let mut all = it.next().unwrap();
    for m in it { for (k, c) in m { *all.entry(k).or_insert(0) += c; } }
    let mut sum: u64 = 0; for (k, c) in &all { sum = sum.wrapping_add(fnv(k).wrapping_mul(*c)); }
    report("pwc", "rayon", "merge", t, &format!(",\"threads\":{threads},\"unique\":{},\"checksum\":{sum}", all.len()));
}
