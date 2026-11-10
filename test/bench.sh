#!/bin/bash
cd "$(dirname "$0")/.."
cores=$(taskset -cp $$ | sed 's/.*: //')
first=$(echo $cores | cut -d, -f1 | cut -d- -f1)
echo "cores: $cores"
taskset -c $first test/run.sh
for t in 2 4 8 16 24 32; do
  THREADS=$t OMP_PROC_BIND=close OMP_PLACES=cores test/run.sh opt7
done
test/check.sh
