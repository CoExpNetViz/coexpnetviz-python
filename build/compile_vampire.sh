#!/bin/sh

# Compiles morph on a system like vampire (i.e. with the module system)
module purge
module load gsl
module load cmake
module load boost
module load gmp
module load mpfr
module load mpc
module load yaml-cpp
module load gcc
./cmake_
make
