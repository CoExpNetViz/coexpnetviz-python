#!/bin/sh
dstdir=../website/bin/morph
/bin/rm -rf $dstdir
mkdir -m 0755 $dstdir
install build/release/morph $dstdir
install -m 0444 config.yaml $dstdir
