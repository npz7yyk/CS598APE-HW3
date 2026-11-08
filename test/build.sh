#!/bin/bash
cd "$(dirname "$0")/.."
name=$1
sha=$(grep "^$name " test/commits.txt | awk '{print $2}')
[ -n "$sha" ] || { echo "$name: not in test/commits.txt"; exit 1; }
dir=test/build/$name

rm -rf $dir
mkdir -p $dir
git archive $sha | tar -x -C $dir
make -C $dir all > $dir/build.log 2>&1 || echo "$name: build failed, see $dir/build.log"
