#!/bin/sh
/bin/rm -rf output
mkdir output
gdb --args ./morph ../../joblist.yaml output 100
