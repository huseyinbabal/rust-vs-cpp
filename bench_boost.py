# Extra C++ tier: boost::unordered_flat_map (same inputs, runs and checksums as bench.py); merged into results/summary.json
import json, os, statistics, subprocess, sys
RUNS = 10; D = '/root/data'; U = open('/root/unique').read().strip()
BIN = {'clang': 'cpp/build/clang', 'gcc': 'cpp/build/gcc'}
def run(cmd):
    out = subprocess.run(['taskset', '-c', '2'] + cmd, capture_output=True, text=True, check=True).stdout
    return [json.loads(l) for l in out.splitlines() if l.startswith('{')]
def bench(name, args, runs=RUNS):
    res = {}
    for lang, b in BIN.items():
        cmd = [f'{b}/{args[0]}', *args[1:]]; run(cmd)
        rows = [run(cmd) for _ in range(runs)]
        for phase in [r['phase'] for r in rows[0]]:
            ms = [next(x for x in r if x['phase'] == phase)['ms'] for r in rows]
            extra = {k: v for k, v in next(x for x in rows[0] if x['phase'] == phase).items() if k not in ('ms', 'lang')}
            res.setdefault(phase, {})[lang] = {'median': statistics.median(ms), 'mean': statistics.mean(ms), 'stdev': statistics.pstdev(ms), **extra}
        print(name, lang, {p: round(v[lang]['median'], 1) for p, v in res.items()}, flush=True)
    return res
S = json.load(open('results/summary.json'))
for mode in ('load', 'stream'):
    S[f'wc_{mode}_boost'] = bench(f'wc_{mode}_boost', ['wc', f'{D}/corpus.txt', mode, 'boost', U])
for n in (100_000, 10_000_000, 100_000_000):
    S[f'intmap_{n}_boost'] = bench(f'intmap_{n}_boost', ['intmap', f'{D}/keys.bin', str(n), 'boost'], runs=RUNS if n < 100_000_000 else 3)
# checksums must equal the Rust run of the same test
ref = {'wc_load_boost': ('wc_load_fast', 'count'), 'wc_stream_boost': ('wc_stream_fast', 'count')}
for k, (rk, ph) in ref.items():
    for lang in BIN:
        if str(S[k][ph][lang]['checksum']) != str(S[rk][ph]['rust']['checksum']): print('CHECKSUM MISMATCH', k, lang); sys.exit(1)
for n in (100_000, 10_000_000, 100_000_000):
    for lang in BIN:
        if str(S[f'intmap_{n}_boost']['hit'][lang]['checksum']) != str(S[f'intmap_{n}_fast']['hit']['rust']['checksum']): print('CHECKSUM MISMATCH intmap', n, lang); sys.exit(1)
json.dump(S, open('results/summary.json', 'w'), indent=1); print('BOOST DONE')
