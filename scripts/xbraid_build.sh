#!/bin/bash
cd ..
git clone https://github.com/XBraid/xbraid.git .
cd xbraid
CC=mpicc

make -j4
make install
