# Deep Blue Genome 

Contains these CLI:

- database: to manage the database needed by the other commands
- morph: to run Morph
- coexpr: to run CoExpr

# MORPH v1.0.6

TODO short description

TODO longer description

Supported platforms: Linux

# Dependencies

TODO
- CMake
- boost: filesystem iostreams system serialization
- gsl
- yamlcpp

- boost 1.55

# Compilation

Note: Is known to compile on GCC 4.8 or older.

## For use at psb servers

Run:

  cd build/release
  source compile_vampire.sh


## For use elsewhere

Compilation:

  cd build/release
  ./cmake_
  make

# Installation

cp morph your/install/directory
  
# Usage

All supplied files must use unix line endings.

./morph # to get a synopsis

## License

LGPL3

# Developer info

Info for those working on this project.

## Directory structure

Below is a partial overview of the file structure:

- src: source code

  - deep_blue_genome
  
    - common: library common amongst the project's algorithms 
    
      - writer: Output functions/classes
      - reader: Input functions/classes, especially for importing to database
    
    - util: utilities that could be reused in other projects

