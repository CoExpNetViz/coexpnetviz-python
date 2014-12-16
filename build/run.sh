#!/bin/sh
/bin/rm -rf output
mkdir output
./morph ../../debug_config.yaml ../../joblist.yaml output 100
