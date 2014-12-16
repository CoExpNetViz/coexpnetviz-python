#!/bin/sh
/bin/rm -rf output
mkdir output
gdb --args ./morph ../../debug_config.yaml ../../joblist.yaml output 100
