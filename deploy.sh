#!/bin/sh
set -e

pushd build/release
./cmake_
make
popd

dstdir=../website/bin/morph
/bin/rm -rf $dstdir
mkdir -m 0755 $dstdir
install build/release/morph $dstdir
