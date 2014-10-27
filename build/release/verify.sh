#!/bin/sh
pushd output
files=`ls`
popd
for f in $files
do
    echo $f
    dos2unix expected_output/$f
    diff output/$f expected_output/$f
done
