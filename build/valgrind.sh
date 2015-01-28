#!/bin/sh
/bin/rm -rf output
mkdir output
. ../$1_command.sh
valgrind ${command[@]}
