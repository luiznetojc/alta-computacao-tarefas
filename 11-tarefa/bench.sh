#!/bin/bash
REPS=5
THREADS=(1 2 4 8)
OUT="resultados_tempos.tsv"

echo -e "versao\tthreads\ttempo_medio\tvelmax" > "$OUT"

run_and_avg() {
    local bin=$1
    local thr=$2
    local total=0
    local velmax=0
    for _ in $(seq 1 $REPS); do
        line=$(./"$bin" "$thr")
        t=$(echo "$line" | grep -oE 'Tempo: [0-9.]+' | awk '{print $2}')
        v=$(echo "$line" | grep -oE 'VelMax: [0-9.]+' | awk '{print $2}')
        total=$(echo "$total + $t" | bc -l)
        velmax=$v
    done
    avg=$(echo "scale=6; $total / $REPS" | bc -l)
    echo -e "$bin\t$thr\t$avg\t$velmax"
}

echo "=== Serial ==="
total=0
for _ in $(seq 1 $REPS); do
    t=$(./navier_serial | grep -c "" )
    t=$( { time ./navier_serial > /dev/null; } 2>&1 | grep real | awk '{print $2}' | sed 's/m/:/g' | awk -F: '{print $1*60+$2}' )
    total=$(echo "$total + $t" | bc -l)
done
avg=$(echo "scale=6; $total / $REPS" | bc -l)
echo -e "navier_serial\t1\t$avg\t-" >> "$OUT"
echo "  serial tempo medio: $avg s"

for bin in navier_static_collapse navier_dynamic navier_guided navier_static; do
    echo "=== $bin ==="
    for thr in "${THREADS[@]}"; do
        row=$(run_and_avg "$bin" "$thr")
        echo "$row" >> "$OUT"
        echo "  $row"
    done
done

echo ""
echo "Resultados salvos em $OUT"
