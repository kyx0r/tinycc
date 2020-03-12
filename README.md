# Tinycc
Single header C99 compiler (TCC)

Entire core of TCC compiler in one file. Including all architectures. 43000 LOC

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

------------------------------------------------------------------------
# Notes & TODO
Have not tested on Arm, riscv64 but it should work unless i screwed up.

- Add intel assembly support and vector instructions properly. 
