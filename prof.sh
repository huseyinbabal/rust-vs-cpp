#!/bin/bash
# "Why" layer: perf stat counters, flame graphs, allocation counts, peak memory, syscalls (results/prof/)
set -e
cd /root/bench; . /root/.cargo/env; D=/root/data; O=results/prof; mkdir -p $O; U=$(cat /root/unique)
R=rust/target/release; RP=rust/target/profiling; C=cpp/build/clang; G=cpp/build/gcc; CP=cpp/build/prof
EV=cycles,instructions,branch-misses,cache-references,cache-misses,L1-dcache-load-misses,LLC-load-misses,page-faults
st() { taskset -c 2 perf stat -x, -e $EV -o $O/stat_$1.csv -- "${@:2}" > /dev/null; }
for v in std fast; do st wc_load_${v}_rust $R/wc $D/corpus.txt load $v $U; st wc_load_${v}_clang $C/wc $D/corpus.txt load $v $U; done
st wc_load_swiss_clang $C/wc $D/corpus.txt load swiss $U
for v in std fast; do st wc_stream_${v}_rust $R/wc $D/corpus.txt stream $v $U; st wc_stream_${v}_clang $C/wc $D/corpus.txt stream $v $U; done
st intmap10m_std_rust $R/intmap $D/keys.bin 10000000 std; st intmap10m_std_clang $C/intmap $D/keys.bin 10000000 std
st intmap10m_fast_rust $R/intmap $D/keys.bin 10000000 fast; st intmap10m_swiss_clang $C/intmap $D/keys.bin 10000000 swiss
st sort_rust $R/sort $D/sort.bin; st sort_clang $C/sort $D/sort.bin; st sort_gcc $G/sort $D/sort.bin
# flame graphs (frame-pointer/debug builds)
fg() { perf record -F 999 -g -o /tmp/p.data -- "${@:2}" > /dev/null 2>&1; perf script -i /tmp/p.data 2>/dev/null | inferno-collapse-perf | inferno-flamegraph --title "$1" > $O/flame_$1.svg; }
fg wc_load_std_rust $RP/wc $D/corpus.txt load std $U; fg wc_load_std_cpp $CP/wc $D/corpus.txt load std $U
fg wc_stream_std_rust $RP/wc $D/corpus.txt stream std $U; fg wc_stream_std_cpp $CP/wc $D/corpus.txt stream std $U
fg sort_rust $RP/sort $D/sort.bin; fg sort_cpp $CP/sort $D/sort.bin
# allocations (heaptrack) and peak memory
for v in std fast; do
  heaptrack -o $O/heap_wc_stream_${v}_rust $R/wc $D/corpus.txt stream $v $U > /dev/null 2>&1
  heaptrack -o $O/heap_wc_stream_${v}_clang $C/wc $D/corpus.txt stream $v $U > /dev/null 2>&1
done
for f in $O/heap_*.zst; do heaptrack_print -f $f 2>/dev/null | grep -E "^total runtime|calls to allocation|peak heap" > ${f%.zst}.txt; done
mem() { /usr/bin/time -v "${@:2}" 2>&1 > /dev/null | grep "Maximum resident" | awk -v n=$1 '{print n, $NF}' >> $O/maxrss.txt; }
: > $O/maxrss.txt
for v in std fast; do mem wc_load_${v}_rust $R/wc $D/corpus.txt load $v $U; mem wc_load_${v}_clang $C/wc $D/corpus.txt load $v $U; done
mem wc_load_swiss_clang $C/wc $D/corpus.txt load swiss $U
for n in 10000000 100000000; do for v in std fast; do mem intmap${n}_${v}_rust $R/intmap $D/keys.bin $n $v; mem intmap${n}_${v}_clang $C/intmap $D/keys.bin $n $v; done; mem intmap${n}_swiss_clang $C/intmap $D/keys.bin $n swiss; done
# syscalls in stream mode: same number of read() calls on both sides
strace -c -o $O/strace_rust.txt $R/wc $D/corpus.txt stream fast $U > /dev/null
strace -c -o $O/strace_clang.txt $C/wc $D/corpus.txt stream fast $U > /dev/null
# cold page cache for the stream test
for l in rust clang; do b=$R; [ $l = clang ] && b=$C
  for i in 1 2 3; do sync; echo 3 > /proc/sys/vm/drop_caches; $b/wc $D/corpus.txt stream fast $U >> $O/cold_$l.jsonl; done; done
echo PROF_DONE
