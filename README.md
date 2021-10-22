# Tinycc
Uptodate Single header C99 compiler (TCC)
mob: 16456168430c9e185dd94b8215aa77d02bbb8a2c

Entire core of TCC compiler in one file. Including all architectures. 100000 LOC

# Why?

It may be painful to deal with complex build systems and files may harm code navigation and/or understanding. 
Many people prefer single header libraries, and this is just that, entire compiler is here. 

# Compiling Tcc 

The most simple command will be

For Unix

```
gcc tinycc.c -ldl -lpthread
or bootstrap:
tcc tinycc.c -ldl -lpthread
```

For win32

```
gcc tinycc.c 
or more explicit, both commands equalent, x86 is default.
gcc tinycc.c -DTCC_TARGET_PE -DTCC_TARGET_I386
```

For win64

```
gcc tinycc.c -DTCC_TARGET_PE -DTCC_TARGET_X86_64
```

You can add compile defines options youself, look into the file and see what is available or what you need.

# Using Tcc

```cpp
#include <windows.h> /*or on unix*/ #include <stdlib.h>

//This declaration is required to make printf external.
int printf(char *format, ...);

//See tinycc.c on how to do argc and argv
int _start()
{
	MessageBox(NULL, NULL, "Hello World", 0); //windows only
	printf("Hello World\n"); //crt is usable
	return 1;
}
```

To include CRT on windows link library using -luser32 for example. This will link to a dll using user32.def file in ./lib
For reference refer to tinycc.c file. 
```
tcc test.c -luser32
```

What is excluded from tinycc.c:

Bounds checker found in official repo's ./lib folder
Arm code found in official repo's ./lib folder

NOTES:
main() function is at the very end of tinycc.c it
will need modifications from the master branch, for
example setup the default include path "./include"
so that it can read those files.

There might be an aftifact in the source code
due to almagination inaccuracy, this probably 
looks like a typo thing in source code, just remove it
if it exists.

At the top of tinycc.c you will find va_args code,
that code is needed only if tcc is bootstrapping
itself (or at least this is how it used to be irc)
then a bit below that you find the extended asm
alloca and ctkchk implementations, again necessary
for bootstrap probably. After that you find libtcc1.c
source, lots of intrinsics needed for float conversions
and etc... Everything after is tcc code straight up
inlined.

This code does compile with gcc under linux+musl no
errors. As far as compiling on anything else I have 
no comment to give.

Tinycc version string with commit hash is included
in the source code.
