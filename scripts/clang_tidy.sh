#!/bin/bash
# clang-tidy has segfaulted on me trying to do all the files at once; do them
# a directory at a time
# TODO parallelize this?
for i in $(ls ../src); do
    files=$(find ../src/$i | grep .cpp)
    echo $files
    clang-tidy $files >> src.tidy
done
