# Introduction

Shell++ is a programming language that aims bring features from modern languages, 
as facility to manipulate data structures, object oriented programming, 
functional programming and others, to shell script.
https://alexst07.github.io/shpp/

# How it looks like:
![syntax highlighting](https://github.com/alexst07/shpp/blob/gh-pages/img/shpp.png)

# Resources
 * Easy to manipulate data structure, as in python.
 * Lambda functions.
 * Classes.
 * Calls commands like in Bash.
 * Pipe, redirection to or from files, sub shell.

# Building

## Requirements:
  * A compiler that supports C++ 14 (gcc or clang)
  * Boost
  * Readline
  * CMake
  * Linux
  
## Compiling

### Fedora
```
# dnf install gcc-c++ clang
# dnf install boost boost-devel readline readline-devel cmake git
$ git clone https://github.com/alexst07/shpp.git
$ cd shpp
$ mkdir build && cd build
$ cmake ..
$ make
# make install
```

### Ubuntu
```
# apt-get install -y build-essential
# apt-get install -y libboost-all-dev libreadline6 libreadline6-dev git cmake
$ git clone https://github.com/alexst07/shpp.git
$ cd shpp
$ mkdir build && cd build
$ cmake ..
$ make
# make install
```

# Running
## Hello world
```
$ shpp
> echo hello world
```
## Running a file
```
$ shpp file.shpp
```
