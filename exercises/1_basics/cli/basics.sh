#!/usr/bin/env bash

DATADIR="$( readlink -f "$( dirname -- "$0" )/../../datasets" )"
echo $DATADIR

for i in 1e-6 1e-5 1e-4 1e-3 1e-2 1e-1
do
    echo "bound=${i} metrics="
    pressio -b compressor=sz -o abs=$i -i ${DATADIR}/CLOUDf48.bin.f32 -d 500 -d 500 -d 100 -t float -m size  -m error_stat -m time -M all
done
