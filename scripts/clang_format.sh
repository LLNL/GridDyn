#!/bin/bash
# clang-tidy has segfaulted on me trying to do all the files at once; do them
# a directory at a time
# TODO parallelize this?
for i in $(ls ../src); do
    echo "Directory: $i"
    files=$(find ../src/$i | grep -E '\.cpp$|\.hpp$|\.h$|\.cc$|\.cxx$')
    for f in $files; do
        clang-format -i $f
    done
done
