#!/bin/sh
/bin/rm -rf output
mkdir output
gdb --args ./morph /mnt/midas/www/group/biocomp/extra/morph/morph_config.yaml ../../joblist.yaml output 100
