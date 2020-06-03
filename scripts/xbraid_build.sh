#!/bin/bash
pushd .
cd ..
git clone https://github.com/XBraid/xbraid.git xbraid
cd xbraid
CC=mpicc

make -j4
popd
