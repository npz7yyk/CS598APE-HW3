#!/bin/bash
cd "$(dirname "$0")/.."
names=${*:-$(grep -v '^#' test/commits.txt | cut -d' ' -f1 | grep -v '^base$')}
SIZES=${SIZES:-"1 2 3 4 5 6 7 50 51 100 101 200 201 300 301"}
mkdir -p test/results/small
[ -x test/build/base/sandpile.exe ] || test/build.sh base

out() {
  f=test/results/small/$1-$2.png
  if [ ! -f $f ]; then
    [ -x test/build/$1/sandpile.exe ] || test/build.sh $1
    dir=$(mktemp -d)
    (cd $dir && OMP_NUM_THREADS=${THREADS:-4} "$OLDPWD/test/build/$1/sandpile.exe" $2 > /dev/null)
    mv $dir/identity.png $f
    rm -rf $dir
  fi
  echo $f
}

for name in $names; do
  rm -f test/results/small/$name-*.png
  bad=
  for n in $SIZES; do
    cmp -s $(out base $n) $(out $name $n) || bad="$bad $n"
  done
  if [ -f test/results/base-1001.png ] && [ -f test/results/$name-1001.png ]; then
    cmp -s test/results/base-1001.png test/results/$name-1001.png || bad="$bad 1001"
  fi
  [ -z "$bad" ] && echo "$name: same as base" || echo "$name: DIFFERENT from base at N =$bad"
done
