#!/bin/sh
/bin/rm -rf output
gdb --args ./morph ../../joblist.yaml output 100
