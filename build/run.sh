#!/bin/sh
/bin/rm -rf output
mkdir output
./morph ../../config.yaml ../../joblist.yaml output 100
