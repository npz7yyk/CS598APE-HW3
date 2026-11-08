#!/bin/bash
cd "$(dirname "$0")/.."
names=${*:-$(grep -v '^#' test/commits.txt | cut -d' ' -f1)}
N=${N:-1001}
RUNS=${RUNS:-3}
THREADS=${THREADS:-1}
mkdir -p test/results
[ -f test/results/times.csv ] || echo "commit,n,threads,run,seconds" > test/results/times.csv

for name in $names; do
  test/build.sh $name
  for run in $(seq $RUNS); do
    dir=$(mktemp -d)
    start=$(date +%s.%N)
    (cd $dir && OMP_NUM_THREADS=$THREADS "$OLDPWD/test/build/$name/sandpile.exe" $N > /dev/null)
    end=$(date +%s.%N)
    secs=$(echo "$end - $start" | bc)
    mv $dir/identity.png test/results/$name-$N.png
    rm -rf $dir
    echo "$name,$N,$THREADS,$run,$secs" | tee -a test/results/times.csv
  done
done
