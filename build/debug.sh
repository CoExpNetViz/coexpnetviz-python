#!/bin/sh
/bin/rm -rf output
mkdir output
gdb --args ./morph ~/doc/internship/midas_morph/data/morph_config.yaml ../../joblist.yaml output 100
