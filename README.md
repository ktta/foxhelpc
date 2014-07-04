foxhelpc : Help Compiler for the FOX Toolkit
========

This program compiles help documents into a C++ source file. This file
can be compiled with a C++ compiler and linked into your executable
to provide online help.

Installation
------------

Change into the build directory and run make. You can set three variables
on the command line: STATIC, SELF and DBG.

make STATIC=1 SELF=.. DBG=..

  creates a static binary instead of a dynamically linked binary.
  The library list in the Makefile is based on my Sabotage Linux
  environment and might require some changes.

make STATIC=.. SELF=1 DBG=..

  Compiles the help document for the compiler into the foxhelpc program.
  The resulting program must be executed in an X11 environment in order
  to view the compiled-in help.

make STATIC=.. SELF=.. DBG=1

  Creates a debugging binary. The default is to create an -O2 optimized
  binary.

There are no installation files other than the binary 'foxhelpc'.
Just copy it somewhere in your $PATH.

Usage
-----

If you have built foxhelpc with self help, then run it with the -H 
option to see the documentation.

Otherwise, you can view build/self.ref in a text editor, it's just a
plain text file.

