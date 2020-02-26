# tinycc
Single header C99 compiler (TCC)

Entire core of TCC compiler in one file. Including all achitectures. 43000 LOC

# Why?

It may be painful to deal with complex build systems and files may harm code navigation and/or understanding. 
Many people prefer single header libraries, and this is just that, entire compiler is here. 

# Usage 

The most simple command will be

```
gcc tinycc.h -ldl -lpthread
or bootstrap:
tcc tinycc.h -ldl -lpthread
```

You can add compile defines options youself, look into the file and see what is available or what you need.

# TODO

Have not tested on Arm, i386, riscv64 but it should work unless i screwed up.

Move the rest of compiler files needed to actually start using the compiler here.
They are not part of the compiler, but need to be scoped when you compile C programs.
