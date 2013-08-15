#!/bin/sh

cd ./src/zht
make

cd ../udt
make -e arch=POWERPC

cd ../ffsnet
make

cd ..
make

cd ../test
make

echo =====Compile succeed.========
