# One-time input files (run before any benchmark, never inside one):
#   corpus.txt  ~600 MB of words, 1M-word vocabulary with a Zipf distribution (a few very common words, a long tail)
#   keys.bin    200M little-endian u64 (first half inserted, second half used as misses)
#   sort.bin    100M little-endian u64
import numpy as np, sys
out = sys.argv[1] if len(sys.argv) > 1 else '.'
rng = np.random.default_rng(20261116)
V = 1_000_000
lens = rng.integers(3, 13, V)
letters = rng.integers(97, 123, int(lens.sum()), dtype=np.uint8)
ends = np.cumsum(lens); starts = ends - lens
vocab = [letters[s:e].tobytes() for s, e in zip(starts, ends)]
with open(f'{out}/corpus.txt', 'wb') as f:
    for _ in range(16):                                   # 16 blocks × 5M words
        idx = rng.zipf(1.1, 5_000_000) - 1
        idx = idx[idx < V]
        line = b' '.join(vocab[i] for i in idx)
        f.write(line.replace(b' ', b'\n', 0)); f.write(b'\n')
rng.integers(0, 2**63, 200_000_000, dtype=np.uint64).astype('<u8').tofile(f'{out}/keys.bin')
rng.integers(0, 2**63, 100_000_000, dtype=np.uint64).astype('<u8').tofile(f'{out}/sort.bin')
print('done')
