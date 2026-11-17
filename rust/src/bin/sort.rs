// sort: read N u64 from a file (not timed), sort them (timed), checksum
use rbench::*;
use std::time::Instant;
fn main() {
    let a: Vec<String> = std::env::args().collect();
    let mut v = read_u64s(&a[1]);
    let t = Instant::now();
    v.sort_unstable();
    let ms = t;
    let mut sum: u64 = 0;
    for (i, x) in v.iter().enumerate() { sum = sum.wrapping_add(x.wrapping_mul(i as u64 + 1)); }
    assert!(v.windows(2).all(|w| w[0] <= w[1]));
    report("sort", "sort_unstable", "sort", ms, &format!(",\"n\":{},\"checksum\":{}", v.len(), sum));
}
