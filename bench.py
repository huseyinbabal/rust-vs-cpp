# Runs every benchmark: 1 warm-up + RUNS measured runs, reads the JSON lines each program prints (file loading is outside
# the timed region), checks that Rust, clang and gcc builds produce the same checksums, writes results/summary.json.
import json, os, statistics, subprocess, sys
RUNS = int(os.environ.get('RUNS', 10)); D = '/root/data'; R = 'results'
os.makedirs(R, exist_ok=True)
BIN = {'rust': 'rust/target/release', 'clang': 'cpp/build/clang', 'gcc': 'cpp/build/gcc'}
def run(cmd, env=None, pin=True):
    full = (['taskset', '-c', '2'] if pin else []) + cmd
    out = subprocess.run(full, capture_output=True, text=True, env={**os.environ, **(env or {})}, check=True).stdout
    return [json.loads(l) for l in out.splitlines() if l.startswith('{')]
def bench(name, cmds, env=None, pin=True, runs=RUNS):
    res = {}
    for lang, cmd in cmds.items():
        run(cmd, env, pin)                                   # warm-up (also puts the input file in the page cache)
        rows = [run(cmd, env, pin) for _ in range(runs)]
        for phase in [r['phase'] for r in rows[0]]:
            ms = [next(x for x in r if x['phase'] == phase)['ms'] for r in rows]
            extra = {k: v for k, v in next(x for x in rows[0] if x['phase'] == phase).items() if k not in ('ms', 'lang')}
            res.setdefault(phase, {})[lang] = {'median': statistics.median(ms), 'mean': statistics.mean(ms), 'stdev': statistics.pstdev(ms), **extra}
        print(name, lang, {p: round(v[lang]['median'], 1) for p, v in res.items()}, flush=True)
    for phase, langs in res.items():                         # same answer in every language, or the run is invalid
        sums = {l: v.get('checksum', v.get('found', v.get('left'))) for l, v in langs.items()}
        if len(set(map(str, sums.values()))) > 1: print('CHECKSUM MISMATCH', name, phase, sums); sys.exit(1)
    return res
S = {}
c3 = lambda b, *a: {l: [f'{BIN[l]}/{b}', *a] for l in BIN}
S['sort'] = bench('sort', c3('sort', f'{D}/sort.bin'))
U = os.environ['UNIQUE']
for mode in ('load', 'stream'):
    for v in ('std', 'reserve', 'fast'):
        S[f'wc_{mode}_{v}'] = bench(f'wc_{mode}_{v}', c3('wc', f'{D}/corpus.txt', mode, v, U))
    S[f'wc_{mode}_swiss'] = bench(f'wc_{mode}_swiss', {l: [f'{BIN[l]}/wc', f'{D}/corpus.txt', mode, 'swiss', U] for l in ('clang', 'gcc')})
for n in (100_000, 10_000_000, 100_000_000):
    runs = RUNS if n < 100_000_000 else 3
    for v in ('std', 'fast'):
        S[f'intmap_{n}_{v}'] = bench(f'intmap_{n}_{v}', c3('intmap', f'{D}/keys.bin', str(n), v), runs=runs)
    S[f'intmap_{n}_swiss'] = bench(f'intmap_{n}_swiss', {l: [f'{BIN[l]}/intmap', f'{D}/keys.bin', str(n), 'swiss'] for l in ('clang', 'gcc')}, runs=runs)
for th in (1, 2, 4, 8):
    env = {'RAYON_NUM_THREADS': str(th), 'OMP_NUM_THREADS': str(th)}
    S[f'pwc_{th}'] = bench(f'pwc_{th}', c3('pwc', f'{D}/corpus.txt'), env=env, pin=False)
json.dump(S, open(f'{R}/summary.json', 'w'), indent=1)
print('ALL DONE')
