uppaal2octopus
==============

C++ program for converting UPPAAL traces to Octopus traces.

This tool works on any UPPAAL trace, as long as it originates from a timed model containing a global clock `c`.
The `octopus` format is used by the `ResVis` project; an internal project of the [Embedded Systems Institute](http://esi.nl).
The `uppaal2octopus` project can be used as a base for other tools that work with UPPAAL traces.

![Example of a trace in ResVis](https://raw.github.com/Wassasin/uppaal2octopus/master/docs/resvis.png)

Dependencies
============

`uppaal2octopus` is written in C++11 and can be compiled using clang 3.1 or higher.

Compilation has been tested with the following setup:
* Ubuntu 12.10, clang 3.1

Compilation requires:
* CMake 2.8
* A C++11 compiler like clang
* A compiler and stdlib containing `functional`
* Boost 1.49 or higher with `system`, `program_options` and `regex`

How to use it
=============

To build `uppaal2octopus`, run:

```
$ ./cmake-linux.sh
$ cd build
$ make
```

For instructions, run:

```
$ ./uppaal2octopus -h
Program for converting UPPAAL traces to Octopus traces. [https://github.com/Wassasin/uppaal2octopus]
Usage: ./uppaal2octopus [options] xtr <model> <trace>
       ./uppaal2octopus [options] hr <trace>

General options:
  -h [ --help ]         display this message
```

The xtr format is a non-humanreadable format for UPPAAL traces, exportable from the UPPAAL java GUI.
Identifiers in files in this format refer to elements in the UPPAAL intermediate format.
Thus when using xtr, `uppaal2octopus` requires the compiled UPPAAL model in the intermediate format.

To attain a model in this format, use the following command where the cwd is your UPPAAL package:

```
$ UPPAAL_COMPILE_ONLY ./bin-Linux/verifyta model.xml 1>model.if
```

The hr format is a humanreadable format exported by the UPPAAL verification server.
To attain a model in this format, use the following command where the cwd is your UPPAAL package:

```
$ ./bin-Linux/verifyta -y -t2 model.xml query.q 2>trace.hr
```

Note on `if` and `xtr` formats
==============================

The UPPAAL verification server produces buggy intermediate format `if` models.
The human readable format `hr` is better and should be preferred.

For more information, see:
* [UPPAAL bug 555](http://bugsy.grid.aau.dk/bugzilla3/show_bug.cgi?id=555)
* [UPPAAL bug 556](http://bugsy.grid.aau.dk/bugzilla3/show_bug.cgi?id=556)
