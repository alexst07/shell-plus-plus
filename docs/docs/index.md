# Introduction

Shell++ is a programming language that aims bring features from modern languages,
as facility to manipulate data structures, object oriented programming,
functional programming and others, to shell script.

# Building

## Requirements:
  * A compiler that supports C++ 14 (gcc or clang)
  * Boost
  * Readline
  * CMake
  * Linux

## Compiling
In the root of the project:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
## Installing
```
$ sudo make install
```
# Running
## Hello world
```
$ shell++
> echo hello world
```
## Running a file
```
$ shell++ file.shell++
```
