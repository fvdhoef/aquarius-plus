#!/bin/sh
set -e

PROJECTS=$(cat projects)

for i in $PROJECTS; do
    rm -rf $i/build
    cmake -S $i -B $i/build -G Ninja
    ninja -C $i/build

    cp $i/build/$i.aq32 ~/Projects/aquarius-plus/EndUser/sdcard/cores/aq32/
done

cp -f basic/help/content/basic.hlp ~/Projects/aquarius-plus/EndUser/sdcard/cores/aq32/
