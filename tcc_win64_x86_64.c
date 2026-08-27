#define TCC_VERSION "2ba12e83b3599ca8f5d50c179fe5138fe956f0c9"
/*
 *  TCC - Tiny C Compiler
 *
 *  Copyright (c) 2001-2004 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* ==================== tcc.c ==================== */

/* ==================== tcc.h ==================== */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
/* gnu headers use to #define __attribute__ to empty for non-gcc compilers */
#ifdef __TINYC__

#undef __attribute__
#endif
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <setjmp.h>
#include <time.h>

#define WIN32_LEAN_AND_MEAN 1
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x502 /* AddVectoredExceptionHandler */

#endif
#include <windows.h>
#include <io.h> /* open, close etc. */
/* open, close etc. */
#include <direct.h> /* getcwd */
/* getcwd */
#include <malloc.h> /* alloca */
/* alloca */
#include <stdint.h>
#define inline __inline
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#ifndef __GNUC__

#define strtold (long double)strtod
#define strtof (float)strtod
#define strtoll _strtoi64
#define strtoull _strtoui64
#endif
/* some compilers don't know ldexpl and windows doesn't have long doubles anyway */

#undef ldexpl
#define ldexpl ldexp
#ifdef LIBTCC_AS_DLL

#define LIBTCCAPI __declspec(dllexport)
#define PUB_FUNC LIBTCCAPI
#endif
#ifndef va_copy
#define va_copy(a,b) a = b
#endif
#ifndef O_BINARY

#define O_BINARY 0
#endif
#ifndef offsetof

#define offsetof(type,field) ((size_t) &((type *)0)->field)
#endif
#ifndef countof
#endif

#define NORETURN __attribute__((noreturn))
#define PRINTF_LIKE(x,y) __attribute__ ((format (printf, (x), (y))))

#define IS_DIRSEP(c) (c == '/' || c == '\\')
#define IS_ABSPATH(p) (IS_DIRSEP(p[0]) || (p[0] && p[1] == ':' && IS_DIRSEP(p[2])))
#define PATHCMP stricmp
#define PATHSEP ";"
/* -------------------------------------------- */
/* parser debug */
/* #define PARSE_DEBUG */
/* preprocessor debug */
/* #define PP_DEBUG */
/* include file debug */
/* #define INC_DEBUG */
/* memory leak debug (only for single threaded usage) */
/* #define MEM_DEBUG 1,2,3 */
/* assembler debug */
/* #define ASM_DEBUG */
/* target selection */
/* #define TCC_TARGET_I386   */
/* i386 code generator */
/* #define TCC_TARGET_X86_64 */
/* x86-64 code generator */
/* #define TCC_TARGET_ARM    */
/* ARMv4 code generator */
/* #define TCC_TARGET_ARM64  */
/* ARMv8 code generator */
/* #define TCC_TARGET_C67    */
/* TMS320C67xx code generator */
/* #define TCC_TARGET_RISCV64 */
/* risc-v code generator */
/* default target is I386 */
/* only native compiler supports -run */
/* enable builtin stack backtraces */

/* No ten-byte long doubles on window and macos except in
   cross-compilers made by a mingw-GCC */
/* support using libtcc from threads */
/* ------------ path configuration ------------ */
#ifndef CONFIG_SYSROOT

#define CONFIG_SYSROOT ""
#endif
/* path to find crt1.o, crti.o and crtn.o */
#ifndef CONFIG_TCC_CRTPREFIX
#endif
/* Below: {B} is substituted by CONFIG_TCCDIR (rsp. -B option) */
/* system include paths */
#ifndef CONFIG_TCC_SYSINCLUDEPATHS
#define CONFIG_TCC_SYSINCLUDEPATHS     "{B}/include" PATHSEP "{B}/include/winapi"
#endif
/* library search paths */
#ifndef CONFIG_TCC_LIBPATHS
#define CONFIG_TCC_LIBPATHS     "{B}/lib"
#endif
/* name of ELF interpreter */
#ifndef CONFIG_TCC_ELFINTERP
#endif
/* (target specific) libtcc1.a */
#ifndef TCC_LIBTCC1
#define TCC_LIBTCC1 "libtcc1.a"
#endif
/* <cross-prefix-to->libtcc1.a */
#ifndef CONFIG_TCC_CROSSPREFIX

#define CONFIG_TCC_CROSSPREFIX ""
#endif
/* -------------------------------------------- */

/* ==================== libtcc.h ==================== */

#define LIBTCCAPI
/**/
/* set custom allocator for all allocations (optional), NULL for default. */
typedef void *TCCReallocFunc(void *ptr, unsigned long size);
LIBTCCAPI void tcc_set_realloc(TCCReallocFunc *my_realloc);
/**/

typedef struct TCCState TCCState;
/* create a new TCC compilation context */

LIBTCCAPI TCCState *tcc_new(void);
/* free a TCC compilation context */

LIBTCCAPI void tcc_delete(TCCState *s);
/* set CONFIG_TCCDIR at runtime */

LIBTCCAPI void tcc_set_lib_path(TCCState *s, const char *path);
/* set error/warning callback (optional) */

typedef void TCCErrorFunc(void *opaque, const char *msg);
LIBTCCAPI void tcc_set_error_func(TCCState *s, void *error_opaque,
				  TCCErrorFunc *error_func);
/* set options as from command line (multiple supported) */

LIBTCCAPI int tcc_set_options(TCCState *s, const char *str);
/**/
/* preprocessor */
/* add include path */

LIBTCCAPI int tcc_add_include_path(TCCState *s, const char *pathname);
/* add in system include path */

LIBTCCAPI int tcc_add_sysinclude_path(TCCState *s, const char *pathname);
/* define preprocessor symbol 'sym'. value can be NULL, sym can be "sym=val" */

LIBTCCAPI void tcc_define_symbol(TCCState *s, const char *sym,
				 const char *value);
/* undefine preprocess symbol 'sym' */

LIBTCCAPI void tcc_undefine_symbol(TCCState *s, const char *sym);
/**/
/* compiling */
/* add a file (C file, dll, object, library, ld script). Return -1 if error. */

LIBTCCAPI int tcc_add_file(TCCState *s, const char *filename);
/* compile a string containing a C source. Return -1 if error. */

LIBTCCAPI int tcc_compile_string(TCCState *s, const char *buf);
/* Tip: to have more specific errors/warnings from tcc_compile_string(),
   you can prefix the string with "#line <num> \"<filename>\"\n" */
/**/
/* linking commands */
/* set output type. MUST BE CALLED before any compilation */
LIBTCCAPI int tcc_set_output_type(TCCState *s, int output_type);
/* output will be run in memory */
#define TCC_OUTPUT_MEMORY 1
/* executable file */
#define TCC_OUTPUT_EXE 2
/* dynamic library */
#define TCC_OUTPUT_DLL 4
/* object file */
#define TCC_OUTPUT_OBJ 3
/* only preprocess */
#define TCC_OUTPUT_PREPROCESS 5
/* equivalent to -Lpath option */

LIBTCCAPI int tcc_add_library_path(TCCState *s, const char *pathname);
/* the library name is the same as the argument of the '-l' option */

LIBTCCAPI int tcc_add_library(TCCState *s, const char *libraryname);
/* add a symbol to the compiled program */

LIBTCCAPI int tcc_add_symbol(TCCState *s, const char *name, const void *val);
/* output an executable, library or object file. DO NOT call
   tcc_relocate() before. */

LIBTCCAPI int tcc_output_file(TCCState *s, const char *filename);
/* link and run main() function and return its value. DO NOT call
   tcc_relocate() before. */

LIBTCCAPI int tcc_run(TCCState *s, int argc, char **argv);
/* do all relocations (needed before using tcc_get_symbol()) */

LIBTCCAPI int tcc_relocate(TCCState *s1);
/* return symbol value or NULL if not found */

LIBTCCAPI void *tcc_get_symbol(TCCState *s, const char *name);
/* list all (global) symbols and their values via 'symbol_cb()' */

LIBTCCAPI void tcc_list_symbols(TCCState *s, void *ctx,
				void (*symbol_cb)(void *ctx, const char *name, const void *val));
/* experimental/advanced section (see libtcc_test_mt.c for an example) */
/* catch runtime exceptions (optionally limit backtraces at top_func),
   when using tcc_set_options("-bt") and when not using tcc_run() */

LIBTCCAPI void *_tcc_setjmp(TCCState *s1, void *jmp_buf, void *top_func,
			    void *longjmp);
#define tcc_setjmp(s1,jb,f) setjmp(_tcc_setjmp(s1, jb, f, longjmp))
/* custom error printer for runtime exceptions. Returning 0 stops backtrace */

typedef int TCCBtFunc(void *udata, void *pc, const char *file, int line,
		      const char *func, const char *msg);
LIBTCCAPI void tcc_set_backtrace_func(TCCState *s1, void *userdata,
				      TCCBtFunc *);
/* ==================== elf.h ==================== */

#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
#endif
/* Standard ELF types.  */
/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;
/* Types for signed and unsigned 32-bit quantities.  */

typedef uint32_t Elf32_Word;
typedef	int32_t Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef	int32_t Elf64_Sword;
/* Types for signed and unsigned 64-bit quantities.  */

typedef uint64_t Elf32_Xword;
typedef	int64_t Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef	int64_t Elf64_Sxword;
/* Type of addresses.  */

typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;
/* Type of file offsets.  */

typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;
/* Type for section indices, which are 16-bit quantities.  */

typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;
/* Type for version symbol information.  */

typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;
/* The ELF file header.  This appears at the start of every ELF file.  */

#define EI_NIDENT (16)

typedef struct {
	unsigned char	e_ident[EI_NIDENT];/* Magic number and other info */

	Elf64_Half	e_type;/* Object file type */

	Elf64_Half	e_machine;/* Architecture */

	Elf64_Word	e_version;/* Object file version */

	Elf64_Addr	e_entry;/* Entry point virtual address */

	Elf64_Off	e_phoff;/* Program header table file offset */

	Elf64_Off	e_shoff;/* Section header table file offset */

	Elf64_Word	e_flags;/* Processor-specific flags */

	Elf64_Half	e_ehsize;/* ELF header size in bytes */

	Elf64_Half	e_phentsize;/* Program header table entry size */

	Elf64_Half	e_phnum;/* Program header table entry count */

	Elf64_Half	e_shentsize;/* Section header table entry size */

	Elf64_Half	e_shnum;/* Section header table entry count */

	Elf64_Half	e_shstrndx;/* Section header string table index */

} Elf64_Ehdr;
/* Fields in the e_ident array.  The EI_* macros are indices into the
   array.  The macros under each EI_* macro are the values the byte
   may have.  */
/* Magic number byte 0 */
#define ELFMAG0 0x7f
/* Magic number byte 1 */
#define ELFMAG1 'E'
/* Magic number byte 2 */
#define ELFMAG2 'L'
/* Magic number byte 3 */
#define ELFMAG3 'F'
/* Conglomeration of the identification bytes, for easy testing as a word.  */

#define ELFMAG "\177ELF"
/* 64-bit objects */
#define ELFCLASS64 2
/* 2's complement, little endian */
#define ELFDATA2LSB 1
/* Relocatable file */
#define ET_REL 1
/* Executable file */
#define ET_EXEC 2
/* Shared object file */
#define ET_DYN 3
/* AMD x86-64 architecture */
#define EM_X86_64 62
/* If it is necessary to assign new unofficial EM_* values, please
   pick large random numbers (0x8523, 0xa7f2, etc.) to minimize the
   chances of collision with official or non-GNU unofficial values.  */
/* Current version */
#define EV_CURRENT 1
/* Section header.  */

typedef struct {
	Elf64_Word	sh_name;/* Section name (string tbl index) */

	Elf64_Word	sh_type;/* Section type */

	Elf64_Xword	sh_flags;/* Section flags */

	Elf64_Addr	sh_addr;/* Section virtual addr at execution */

	Elf64_Off	sh_offset;/* Section file offset */

	Elf64_Xword	sh_size;/* Section size in bytes */

	Elf64_Word	sh_link;/* Link to another section */

	Elf64_Word	sh_info;/* Additional section information */

	Elf64_Xword	sh_addralign;/* Section alignment */

	Elf64_Xword	sh_entsize;/* Entry size if section holds table */

} Elf64_Shdr;
/* Special section indices.  */
/* Undefined section */

#define SHN_UNDEF 0
/* Start of reserved indices */
#define SHN_LORESERVE 0xff00
/* Order section before all others
					   (Solaris).  */
/* Order section after all others
					   (Solaris).  */
/* Associated symbol is absolute */
#define SHN_ABS 0xfff1
/* Associated symbol is common */
#define SHN_COMMON 0xfff2
/* Program data */
#define SHT_PROGBITS 1
/* Symbol table */
#define SHT_SYMTAB 2
/* String table */
#define SHT_STRTAB 3
/* Relocation entries with addends */
#define SHT_RELA 4
/* Symbol hash table */
#define SHT_HASH 5
/* Dynamic linking information */
#define SHT_DYNAMIC 6
/* Notes */
#define SHT_NOTE 7
/* Program space with no data (bss) */
#define SHT_NOBITS 8
/* Relocation entries, no addends */
#define SHT_REL 9
/* Dynamic linker symbol table */
#define SHT_DYNSYM 11
/* Array of constructors */
#define SHT_INIT_ARRAY 14
/* Array of destructors */
#define SHT_FINI_ARRAY 15
/* Array of pre-constructors */
#define SHT_PREINIT_ARRAY 16
/* GNU-style hash table.  */
#define SHT_GNU_HASH 0x6ffffff6
/* Version definition section.  */
#define SHT_GNU_verdef 0x6ffffffd
/* Version needs section.  */
#define SHT_GNU_verneed 0x6ffffffe
/* Version symbol table.  */
#define SHT_GNU_versym 0x6fffffff
/* Legal values for sh_flags (section flags).  */
/* Writable */

#define SHF_WRITE (1 << 0)
/* Occupies memory during execution */
#define SHF_ALLOC (1 << 1)
/* Executable */
#define SHF_EXECINSTR (1 << 2)
/* Might be merged */
#define SHF_MERGE (1 << 4)
/* Contains nul-terminated strings */
#define SHF_STRINGS (1 << 5)
/* Non-standard OS specific handling
					   required */
/* Section is member of a group.  */
#define SHF_GROUP (1 << 9)
/* Section hold thread-local data.  */
#define SHF_TLS (1 << 10)
/* Section with compressed data. */
#define SHF_COMPRESSED (1 << 11)
/* Special ordering requirement
					   (Solaris).  */
/* Section is excluded unless
					   referenced or allocated (Solaris).*/
/* Symbol table entry.  */

typedef struct {
	Elf64_Word	st_name;/* Symbol name (string tbl index) */

	unsigned char	st_info;/* Symbol type and binding */

	unsigned char st_other;/* Symbol visibility */

	Elf64_Section	st_shndx;/* Section index */

	Elf64_Addr	st_value;/* Symbol value */

	Elf64_Xword	st_size;/* Symbol size */

} Elf64_Sym;
/* The syminfo section if available contains additional information about
   every dynamic symbol.  */

/* Symbol bound to object to be lazy
					   loaded */
/* How to extract and insert information held in the st_info field.  */

#define ELF32_ST_BIND(val) (((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val) ((val) & 0xf)
#define ELF32_ST_INFO(bind,type) (((bind) << 4) + ((type) & 0xf))
/* Both Elf32_Sym and Elf64_Sym use the same one-byte st_info field.  */

#define ELF64_ST_BIND(val) ELF32_ST_BIND (val)
#define ELF64_ST_TYPE(val) ELF32_ST_TYPE (val)
#define ELF64_ST_INFO(bind,type) ELF32_ST_INFO ((bind), (type))
/* Legal values for ST_BIND subfield of st_info (symbol binding).  */
/* Local symbol */

#define STB_LOCAL 0
/* Global symbol */
#define STB_GLOBAL 1
/* Weak symbol */
#define STB_WEAK 2
/* Legal values for ST_TYPE subfield of st_info (symbol type).  */
/* Symbol type is unspecified */

#define STT_NOTYPE 0
/* Symbol is a data object */
#define STT_OBJECT 1
/* Symbol is a code object */
#define STT_FUNC 2
/* Symbol associated with a section */
#define STT_SECTION 3
/* Symbol's name is file name */
#define STT_FILE 4
/* Symbol is thread-local data object*/
#define STT_TLS 6
/* Symbol table indices are found in the hash buckets and chain table
   of a symbol hash table section.  This special index value indicates
   the end of a chain, meaning no further symbols are found in that bucket.  */
/* How to extract and insert information held in the st_other field.  */

#define ELF32_ST_VISIBILITY(o) ((o) & 0x03)
/* For ELF64 the definitions are the same.  */

#define ELF64_ST_VISIBILITY(o) ELF32_ST_VISIBILITY (o)
/* Symbol visibility specification encoded in the st_other field.  */
/* Default symbol visibility rules */

#define STV_DEFAULT 0
/* Processor specific hidden class */
#define STV_INTERNAL 1
/* Sym unavailable in other modules */
#define STV_HIDDEN 2
/* Not preemptible, not exported */
#define STV_PROTECTED 3
/* Relocation table entry without addend (in section of type SHT_REL).  */

/* I have seen two different definitions of the Elf64_Rel and
   Elf64_Rela structures, so we'll leave them out until Novell (or
   whoever) gets their act together.  */
/* The following, at least, is used on Sparc v9, MIPS, and Alpha.  */

/* Relocation table entry with addend (in section of type SHT_RELA).  */

typedef struct {
	Elf64_Addr	r_offset;/* Address */

	Elf64_Xword	r_info;/* Relocation type and symbol index */

	Elf64_Sxword	r_addend;/* Addend */

} Elf64_Rela;

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xffffffff)
#define ELF64_R_INFO(sym,type) ((((Elf64_Xword) (sym)) << 32) + (type))
/* Program segment header.  */

typedef struct {
	Elf64_Word	p_type;/* Segment type */

	Elf64_Word	p_flags;/* Segment flags */

	Elf64_Off	p_offset;/* Segment file offset */

	Elf64_Addr	p_vaddr;/* Segment virtual address */

	Elf64_Addr	p_paddr;/* Segment physical address */

	Elf64_Xword	p_filesz;/* Segment size in file */

	Elf64_Xword	p_memsz;/* Segment size in memory */

	Elf64_Xword	p_align;/* Segment alignment */

} Elf64_Phdr;
/* Special value for e_phnum.  This indicates that the real number of
   program headers is too large to fit into e_phnum.  Instead the real
   value is in the field sh_info of section 0.  */
/* Dynamic section entry.  */

/* DT_* entries which fall between DT_VALRNGHI & DT_VALRNGLO use the
   Dyn.d_un.d_val field of the Elf*_Dyn structure.  This follows Sun's
   approach.  */
/* Flags for DT_* entries, effecting
					   the following DT_* entry.  */
/* DT_* entries which fall between DT_ADDRRNGHI & DT_ADDRRNGLO use the
   Dyn.d_un.d_ptr field of the Elf*_Dyn structure.

   If any adjustment is made to the ELF object after it has been
   built these entries will need to be adjusted.  */
/* The versioning entry types.  The next are defined as part of the
   GNU extension.  */
/* Address of version definition
					   table */
/* Address of table with needed
					   versions */
/* Sun added these machine-independent extensions in the "processor-specific"
   range.  Be compatible.  */
/* State flags selectable in the `d_un.d_val' element of the DT_FLAGS_1
   entry in the dynamic section.  */
/* Symbols from next object are not
					   generally available.  */
/* Version definition sections.  */

/* Auxiliary version information.  */

/* Version dependency section.  */

/* Auxiliary needed version information.  */

/* Auxiliary vector.  */
/* This vector is normally only used by the program interpreter.  The
   usual definition in an ABI supplement uses the name auxv_t.  The
   vector is not usually defined in a standard <elf.h> file, but it
   can't hurt.  We rename it to avoid conflicts.  The sizes of these
   types are an arrangement between the exec server and the program
   interpreter, so we don't fully specify them here.  */

/* Machine dependent hints about
					   processor capabilities.  */
/* This entry gives some i\nformation about the FPU initialization
   performed by the kernel.  */
/* A special ignored value for PPC, used by the kernel to control the
   interpretation of the AUXV. Must be > 16.  */
/* Pointer to the global system page used for system calls and other
   nice things.  */
/* Shapes of the caches.  Bits 0-3 contains associativity; bits 4-7 contains
   log2 of line size; mask those to get cache size.  */
/* Note section contents.  Each entry in the note section begins with
   a header of a fixed form.  */

/* Defined note types for GNU systems.  */
/* ABI information.  The descriptor consists of words:
   word 0: OS descriptor
   word 1: major version of the ABI
   word 2: minor version of the ABI
   word 3: subminor version of the ABI
*/
/* Known OSes.  These values can appear in word 0 of an
   NT_GNU_ABI_TAG note section entry.  */
/* Synthetic hwcap information.  The descriptor begins with two words:
   word 0: number of entries
   word 1: bitmask of enabled entries
   Then follow variable-length entries, one byte followed by a
   '\0'-terminated hwcap name string.  The byte gives the bit
   number to test if enabled, (1U << bit) & bitmask.  */
/* Build ID bits as generated by ld --build-id.
   The descriptor consists of any nonzero number of bytes.  */
/* Move records.  */

/* 32 bit offset relative to
					   static TLS block */
/* 16 bit offset relative to
					   static TLS block */
/* 8 bit offset relative to
					   static TLS block */
/* Address of GOT entry for static TLS
					   block offset */
/* GOT entry for static TLS block
					   offset */
/* Offset relative to static TLS
					   block */
/* Direct 32 bit for GNU version of
					   general dynamic thread local data */
/* Direct 32 bit for GNU version of
					   local dynamic thread local data
					   in LE code */
/* Direct 32 bit for general dynamic
					   thread local data */
/* Relocation for call to
					   __tls_get_addr() */
/* Direct 32 bit for local dynamic
					   thread local data in LE code */
/* Relocation for call to
					   __tls_get_addr() in LDM code */
/* GOT entry for negated static TLS
					   block offset */
/* Negated offset relative to static
					   TLS block */
/* Marker of call through TLS
					   descriptor for
					   relaxation.  */
/* TLS descriptor containing
					   pointer to code and to
					   argument, returning the TLS
					   offset for the symbol.  */
/* Entries found in sections of type SHT_MIPS_GPTAB.  */

/* Entry found in sections of type SHT_MIPS_REGINFO.  */

/* Entries found in sections of type SHT_MIPS_OPTIONS.  */

/* Entry found in `.options' section.  */

/* Number of entries in
						DT_MIPS_DELTA_CLASS.  */
/* Number of entries in
						DT_MIPS_DELTA_INSTANCE.  */
/* Number of entries in
					     DT_MIPS_DELTA_RELOC.  */
/* Delta symbols that Delta
					   relocations refer to.  */
/* Number of entries in
					   DT_MIPS_DELTA_SYM.  */
/* Delta symbols that hold the
					     class declaration.  */
/* Number of entries in
						DT_MIPS_DELTA_CLASSSYM.  */
/* Address of rld_text_rsolve
						    function stored in GOT.  */
/* Default suffix of dso to be added
					   by rld on dlopen() calls.  */
/* The base of the PLT in an executable using the new non-PIC ABI if that
   PLT is writable.  For a non-writable PLT, this is omitted or has a zero
   value.  */
/* Entries found in sections of type SHT_MIPS_LIBLIST.  */

/* Entries found in sections of type SHT_MIPS_CONFLICT.  */

typedef Elf32_Addr Elf32_Conflict;
/* No kernel assisted branch
					      prediction.  */
/* Additional section indices.  */
/* Section for tentatively declared
					      symbols in ANSI C.  */
/* PowerPC -mrelocatable-lib
						   flag */
/* The remaining relocs are from the Embedded ELF ABI, and are not
   in the SVR4 ELF ABI.  */
/* This is a phony reloc to handle any old fashioned TOC16 references
   that may still be in object files.  */
/* Section may be multiply defined
					      in the input to a link step.  */
/* ARM-specific program header flags */
/* Segment contains the location
					      addressed by the static base. */
/* PC-rel 32 bit for global dynamic
					   thread local data */
/* PC-rel 32 bit for local dynamic
					   thread local data */
/* 32 bit offset relative to TLS
					   block */
/* PC-rel 32 bit for GOT entry of
					   static TLS block offset */
/* 32 bit offset relative to static
					   TLS block */
/* Tag for function call in general
					   dynamic TLS code. */
/* Tag for function call in local
					   dynamic TLS code. */
/* Direct 32 bit for general dynamic
					   thread local data.  */
/* Direct 64 bit for general dynamic
					  thread local data.  */
/* 12 bit GOT offset for static TLS
					   block offset.  */
/* 32 bit GOT offset for static TLS
					   block offset.  */
/* 64 bit GOT offset for static TLS
					   block offset. */
/* Direct 32 bit for local dynamic
					   thread local data in LE code.  */
/* Direct 64 bit for local dynamic
					   thread local data in LE code.  */
/* 32 bit address of GOT entry for
					   negated static TLS block offset.  */
/* 64 bit address of GOT entry for
					   negated static TLS block offset.  */
/* 32 bit rel. offset to GOT entry for
					   negated static TLS block offset.  */
/* 32 bit negated offset relative to
					   static TLS block.  */
/* 64 bit negated offset relative to
					   static TLS block.  */
/* 32 bit offset relative to TLS
					   block.  */
/* 64 bit offset relative to TLS
					   block.  */
/* Negated offset in static TLS
					   block.  */
/* 20 bit GOT offset for static TLS
					   block offset.  */
/* AMD x86-64 relocations.  */
/* No reloc */

#define R_X86_64_NONE 0
/* Direct 64 bit  */
#define R_X86_64_64 1
/* PC relative 32 bit signed */
#define R_X86_64_PC32 2
/* 32 bit GOT entry */
#define R_X86_64_GOT32 3
/* 32 bit PLT address */
#define R_X86_64_PLT32 4
/* Copy symbol at runtime */
#define R_X86_64_COPY 5
/* Create GOT entry */
#define R_X86_64_GLOB_DAT 6
/* Create PLT entry */
#define R_X86_64_JUMP_SLOT 7
/* Adjust by program base */
#define R_X86_64_RELATIVE 8
/* 32 bit signed PC relative
					   offset to GOT */

#define R_X86_64_GOTPCREL 9
/* Direct 32 bit zero extended */
#define R_X86_64_32 10
/* Direct 32 bit sign extended */
#define R_X86_64_32S 11
/* Offset in module's TLS block */
#define R_X86_64_DTPOFF64 17
/* Offset in initial TLS block */
#define R_X86_64_TPOFF64 18
/* 32 bit signed PC relative offset
					   to two GOT entries for GD symbol */

#define R_X86_64_TLSGD 19
/* 32 bit signed PC relative offset
					   to two GOT entries for LD symbol */

#define R_X86_64_TLSLD 20
/* Offset in TLS block */
#define R_X86_64_DTPOFF32 21
/* 32 bit signed PC relative offset
					   to GOT entry for IE symbol */

#define R_X86_64_GOTTPOFF 22
/* Offset in initial TLS block */
#define R_X86_64_TPOFF32 23
/* PC relative 64 bit */
#define R_X86_64_PC64 24
/* 64 bit offset to GOT */
#define R_X86_64_GOTOFF64 25
/* 32 bit signed pc relative
					   offset to GOT */

#define R_X86_64_GOTPC32 26
/* 64-bit GOT entry offset */
#define R_X86_64_GOT64 27
/* 64-bit PC relative offset
					   to GOT entry */
/* 64-bit PC relative offset to GOT */
#define R_X86_64_GOTPC64 29
/* 64-bit GOT relative offset
					   to PLT entry */

#define R_X86_64_PLTOFF64 31
/* Marker for call through TLS
					   descriptor.  */
/* like GOTPCREL, but optionally with
					   linker optimizations */

#define R_X86_64_GOTPCRELX 41
/* like GOTPCRELX, but a REX prefix
					   is present */

#define R_X86_64_REX_GOTPCRELX 42
/* GOT offset for static TLS block
					   offset.  */
/* GOT address for static TLS block
					   offset.  */
/* Offset relative to static TLS
					   block.  */
/* Adjustment for next reloc as needed
					   by linker relaxation.  */
/* Alignment requirement for linker
					   relaxation.  */
/* High 16 bit GOT entry with unsigned
					   low */
/* High 16 bit GOT entry with signed
					   low */
/* High 16 bit PC relative offset to
					   GOT with unsigned low */
/* High 16 bit PC relative offset to
					   GOT with signed low */
/* Low 16 bit PC relative offset to
					   GOT */
/* High 16 bit offset to GOT
					   with unsigned low */
/* High 16 bit offset to GOT
					   with signed low */
/* elf.h */
/* ==================== stab.h ==================== */

#define __define_stab(NAME,CODE,STRING) NAME=CODE,

enum __stab_debug_code {
	/* ==================== stab.def ==================== */

	/* This contains contribution from Cygnus Support.  */
	/* Global variable.  Only the name is significant.
	   To find the address, look in the corresponding external symbol.  */
	__define_stab (N_GSYM, 0x20, "GSYM")
	/* Function name for BSD Fortran.  Only the name is significant.
	   To find the address, look in the corresponding external symbol.  */

	__define_stab (N_FNAME, 0x22, "FNAME")
	/* Function name or text-segment variable for C.  Value is its address.
	   Desc is supposedly starting line number, but GCC doesn't set it
	   and DBX seems not to miss it.  */

	__define_stab (N_FUN, 0x24, "FUN")
	/* Data-segment variable with internal linkage.  Value is its address.
	   "Static Sym".  */

	__define_stab (N_STSYM, 0x26, "STSYM")
	/* BSS-segment variable with internal linkage.  Value is its address.  */

	__define_stab (N_LCSYM, 0x28, "LCSYM")
	/* Name of main routine.  Only the name is significant.
	   This is not used in C.  */

	__define_stab (N_MAIN, 0x2a, "MAIN")
	/* Global symbol in Pascal.
	   Supposedly the value is its line number; I'm skeptical.  */

	__define_stab (N_PC, 0x30, "PC")
	/* Number of symbols:  0, files,,funcs,lines according to Ultrix V4.0. */

	__define_stab (N_NSYMS, 0x32, "NSYMS")
	/* "No DST map for sym: name, ,0,type,ignored"  according to Ultrix V4.0. */

	__define_stab (N_NOMAP, 0x34, "NOMAP")
	/* New stab from Solaris.  I don't know what it means, but it
	   don't seem to contain useful information.  */

	__define_stab (N_OBJ, 0x38, "OBJ")
	/* New stab from Solaris.  I don't know what it means, but it
	   don't seem to contain useful information.  Possibly related to the
	   optimization flags used in this module.  */

	__define_stab (N_OPT, 0x3c, "OPT")
	/* Register variable.  Value is number of register.  */

	__define_stab (N_RSYM, 0x40, "RSYM")
	/* Modula-2 compilation unit.  Can someone say what info it contains?  */

	__define_stab (N_M2C, 0x42, "M2C")
	/* Line number in text segment.  Desc is the line number;
	   value is corresponding address.  */

	__define_stab (N_SLINE, 0x44, "SLINE")
	/* Similar, for data segment.  */

	__define_stab (N_DSLINE, 0x46, "DSLINE")
	/* Similar, for bss segment.  */

	__define_stab (N_BSLINE, 0x48, "BSLINE")
	/* Sun's source-code browser stabs.  ?? Don't know what the fields are.
	   Supposedly the field is "path to associated .cb file".  THIS VALUE
	   OVERLAPS WITH N_BSLINE!  */

	__define_stab (N_BROWS, 0x48, "BROWS")
	/* GNU Modula-2 definition module dependency.  Value is the modification time
	   of the definition file.  Other is non-zero if it is imported with the
	   GNU M2 keyword %INITIALIZE.  Perhaps N_M2C can be used if there
	   are enough empty fields? */

	__define_stab(N_DEFD, 0x4a, "DEFD")
	/* THE FOLLOWING TWO STAB VALUES CONFLICT.  Happily, one is for Modula-2
	   and one is for C++.   Still,... */
	/* GNU C++ exception variable.  Name is variable name.  */

	__define_stab (N_EHDECL, 0x50, "EHDECL")
	/* Modula2 info "for imc":  name,,0,0,0  according to Ultrix V4.0.  */

	__define_stab (N_MOD2, 0x50, "MOD2")
	/* GNU C++ `catch' clause.  Value is its address.  Desc is nonzero if
	   this entry is immediately followed by a CAUGHT stab saying what exception
	   was caught.  Multiple CAUGHT stabs means that multiple exceptions
	   can be caught here.  If Desc is 0, it means all exceptions are caught
	   here.  */

	__define_stab (N_CATCH, 0x54, "CATCH")
	/* Structure or union element.  Value is offset in the structure.  */

	__define_stab (N_SSYM, 0x60, "SSYM")
	/* Name of main source file.
	   Value is starting text address of the compilation.  */

	__define_stab (N_SO, 0x64, "SO")
	/* Automatic variable in the stack.  Value is offset from frame pointer.
	   Also used for type descriptions.  */

	__define_stab (N_LSYM, 0x80, "LSYM")
	/* Beginning of an include file.  Only Sun uses this.
	   In an object file, only the name is significant.
	   The Sun linker puts data into some of the other fields.  */

	__define_stab (N_BINCL, 0x82, "BINCL")
	/* Name of sub-source file (#include file).
	   Value is starting text address of the compilation.  */

	__define_stab (N_SOL, 0x84, "SOL")
	/* Parameter variable.  Value is offset from argument pointer.
	   (On most machines the argument pointer is the same as the frame pointer.  */

	__define_stab (N_PSYM, 0xa0, "PSYM")
	/* End of an include file.  No name.
	   This and N_BINCL act as brackets around the file's output.
	   In an object file, there is no significant data in this entry.
	   The Sun linker puts data into some of the fields.  */

	__define_stab (N_EINCL, 0xa2, "EINCL")
	/* Alternate entry point.  Value is its address.  */

	__define_stab (N_ENTRY, 0xa4, "ENTRY")
	/* Beginning of lexical block.
	   The desc is the nesting level in lexical blocks.
	   The value is the address of the start of the text for the block.
	   The variables declared inside the block *precede* the N_LBRAC symbol.  */

	__define_stab (N_LBRAC, 0xc0, "LBRAC")
	/* Place holder for deleted include file.  Replaces a N_BINCL and everything
	   up to the corresponding N_EINCL.  The Sun linker generates these when
	   it finds multiple identical copies of the symbols from an include file.
	   This appears only in output from the Sun linker.  */

	__define_stab (N_EXCL, 0xc2, "EXCL")
	/* Modula-2 scope information.  Can someone say what info it contains?  */

	__define_stab (N_SCOPE, 0xc4, "SCOPE")
	/* End of a lexical block.  Desc matches the N_LBRAC's desc.
	   The value is the address of the end of the text for the block.  */

	__define_stab (N_RBRAC, 0xe0, "RBRAC")
	/* Begin named common block.  Only the name is significant.  */

	__define_stab (N_BCOMM, 0xe2, "BCOMM")
	/* End named common block.  Only the name is significant
	   (and it should match the N_BCOMM).  */

	__define_stab (N_ECOMM, 0xe4, "ECOMM")
	/* End common (local name): value is address.
	   I'm not sure how this is used.  */

	__define_stab (N_ECOML, 0xe8, "ECOML")
	/* These STAB's are used on Gould systems for Non-Base register symbols
	   or something like that.  FIXME.  I have assigned the values at random
	   since I don't have a Gould here.  Fixups from Gould folk welcome... */

	__define_stab (N_NBTEXT, 0xF0, "NBTEXT")
	__define_stab (N_NBDATA, 0xF2, "NBDATA")
	__define_stab (N_NBBSS, 0xF4, "NBBSS")
	__define_stab (N_NBSTS, 0xF6, "NBSTS")
	__define_stab (N_NBLCS, 0xF8, "NBLCS")
	/* Second symbol entry containing a length-value for the preceding entry.
	   The value is the length.  */

	__define_stab (N_LENG, 0xfe, "LENG")
	/* The above information, in matrix format.

				STAB MATRIX
		_________________________________________________
		| 00 - 1F are not dbx stab symbols		|
		| In most cases, the low bit is the EXTernal bit|

		| 00 UNDEF  | 02 ABS	| 04 TEXT   | 06 DATA	|
		| 01  |EXT  | 03  |EXT	| 05  |EXT  | 07  |EXT	|

		| 08 BSS    | 0A INDR	| 0C FN_SEQ | 0E   	|
		| 09  |EXT  | 0B 	| 0D	    | 0F	|

		| 10 	    | 12 COMM	| 14 SETA   | 16 SETT	|
		| 11	    | 13	| 15 	    | 17	|

		| 18 SETD   | 1A SETB	| 1C SETV   | 1E WARNING|
		| 19	    | 1B	| 1D 	    | 1F FN	|

		|_______________________________________________|
		| Debug entries with bit 01 set are unused.	|
		| 20 GSYM   | 22 FNAME	| 24 FUN    | 26 STSYM	|
		| 28 LCSYM  | 2A MAIN	| 2C	    | 2E	|
		| 30 PC	    | 32 NSYMS	| 34 NOMAP  | 36	|
		| 38 OBJ    | 3A	| 3C OPT    | 3E	|
		| 40 RSYM   | 42 M2C	| 44 SLINE\  | 46 DSLINE |
		| 48 BSLINE*| 4A DEFD	| 4C        | 4E	|
		| 50 EHDECL*| 52	| 54 CATCH  | 56        |
		| 58        | 5A        | 5C        | 5E	|
		| 60 SSYM   | 62	| 64 SO	    | 66 	|
		| 68 	    | 6A	| 6C	    | 6E	|
		| 70	    | 72	| 74	    | 76	|
		| 78	    | 7A	| 7C	    | 7E	|
		| 80 LSYM   | 82 BINCL	| 84 SOL    | 86	|
		| 88	    | 8A	| 8C	    | 8E	|
		| 90	    | 92	| 94	    | 96	|
		| 98	    | 9A	| 9C	    | 9E	|
		| A0 PSYM   | A2 EINCL	| A4 ENTRY  | A6	|
		| A8	    | AA	| AC	    | AE	|
		| B0	    | B2	| B4	    | B6	|
		| B8	    | BA	| BC	    | BE	|
		| C0 LBRAC  | C2 EXCL	| C4 SCOPE  | C6	|
		| C8	    | CA	| CC	    | CE	|
		| D0	    | D2	| D4	    | D6	|
		| D8	    | DA	| DC	    | DE	|
		| E0 RBRAC  | E2 BCOMM	| E4 ECOMM  | E6	|
		| E8 ECOML  | EA	| EC	    | EE	|
		| F0	    | F2	| F4	    | F6	|
		| F8	    | FA	| FC	    | FE LENG	|
		+-----------------------------------------------+
	 * 50 EHDECL is also MOD2.
	 * 48 BSLINE is also BROWS.
	 */
	LAST_UNUSED_STAB_CODE
};

#undef __define_stab
/* __GNU_STAB_ */
/* ==================== dwarf.h ==================== */

/* DWARF Unit Header Types.  */

enum {
	DW_UT_compile = 0x01
};
/* DWARF tags.  */

enum {
	DW_TAG_array_type = 0x01,
	DW_TAG_enumeration_type = 0x04,
	DW_TAG_formal_parameter = 0x05,
	DW_TAG_lexical_block = 0x0b,
	/* 0x0c reserved.  */

	DW_TAG_member = 0x0d,
	/* 0x0e reserved.  */

	DW_TAG_pointer_type = 0x0f,
	DW_TAG_compile_unit = 0x11,
	DW_TAG_structure_type = 0x13,
	/* 0x14 reserved.  */

	DW_TAG_subroutine_type = 0x15,
	DW_TAG_typedef = 0x16,
	DW_TAG_union_type = 0x17,
	DW_TAG_subrange_type = 0x21,
	DW_TAG_base_type = 0x24,
	DW_TAG_enumerator = 0x28,
	DW_TAG_subprogram = 0x2e,
	DW_TAG_variable = 0x34
};
/* DWARF attributes encodings.  */

enum {
	DW_AT_sibling = 0x01,
	DW_AT_location = 0x02,
	DW_AT_name = 0x03,
	/* 0x0a reserved.  */

	DW_AT_byte_size = 0x0b,

	DW_AT_bit_size = 0x0d,
	/* 0x0e reserved.  */
	/* 0x0f reserved.  */

	DW_AT_stmt_list = 0x10,
	DW_AT_low_pc = 0x11,
	DW_AT_high_pc = 0x12,
	DW_AT_language = 0x13,
	DW_AT_comp_dir = 0x1b,
	DW_AT_const_value = 0x1c,
	/* 0x23 reserved.  */
	/* 0x24 reserved.  */

	DW_AT_producer = 0x25,
	DW_AT_upper_bound = 0x2f,
	DW_AT_data_member_location = 0x38,
	DW_AT_decl_file = 0x3a,
	DW_AT_decl_line = 0x3b,
	DW_AT_encoding = 0x3e,
	DW_AT_external = 0x3f,
	DW_AT_frame_base = 0x40,
	DW_AT_type = 0x49,
	DW_AT_data_bit_offset = 0x6b
};
/* Old unofficially attribute names.  Should not be used.
   Will not appear in known-dwarf.h  */
/* DWARF form encodings.  */

enum {
	DW_FORM_addr = 0x01,
	DW_FORM_data4 = 0x06,
	DW_FORM_data8 = 0x07,
	DW_FORM_block1 = 0x0a,
	DW_FORM_data1 = 0x0b,
	DW_FORM_flag = 0x0c,
	DW_FORM_sdata = 0x0d,
	DW_FORM_strp = 0x0e,
	DW_FORM_udata = 0x0f,
	DW_FORM_ref4 = 0x13,
	DW_FORM_sec_offset = 0x17,
	DW_FORM_exprloc = 0x18,
	DW_FORM_line_strp = 0x1f,

};
/* DWARF location operation encodings.  */

enum {
	DW_OP_addr = 0x03,/* Constant address.  */

	DW_OP_reg6 = 0x56,/* Register 6.  */

	DW_OP_fbreg = 0x91,/* Signed LEB128 offset.  */

};
/* DWARF base type encodings.  */

enum {
	DW_ATE_boolean = 0x2,
	DW_ATE_float = 0x4,
	DW_ATE_signed = 0x5,
	DW_ATE_signed_char = 0x6,
	DW_ATE_unsigned = 0x7,
	DW_ATE_unsigned_char = 0x8
};
/* DWARF language encodings.  */

enum {

	DW_LANG_C99 = 0x000c,/* ISO C:1999 */

	DW_LANG_C11 = 0x001d,/* ISO C:2011 */
};
/* DWARF calling conventions encodings.
   Used as values of DW_AT_calling_convention for subroutines
   (normal, program or nocall) or structures, unions and class types
   (normal, reference or value).  */
/* DWARF line content descriptions.  */

enum {
	DW_LNCT_path = 0x1,
	DW_LNCT_directory_index = 0x2
};
/* DWARF standard opcode encodings.  */

enum {
	DW_LNS_advance_pc = 2,
	DW_LNS_advance_line = 3,
	DW_LNS_set_file = 4,
	DW_LNS_set_prologue_end = 10,
	DW_LNS_set_epilogue_begin = 11
};
/* DWARF extended opcode encodings.  */

enum {
	DW_LNE_end_sequence = 1,
	DW_LNE_set_address = 2,

	DW_LNE_hi_user = 255
};
/* Old GNU extension names for DWARF5 debug_macro type encodings.
   There are no equivalents for the supplementary object file (sup)
   and indirect string references (strx).  */
/* Section 7.2.2 of the DWARF3 specification defines a range of escape
   codes that can appear in the length field of certain DWARF structures.

   These defines enumerate the minimum and maximum values of this range.
   Currently only the maximum value is used (to indicate that 64-bit
   values are going to be used in the dwarf data that accompanies the
   structure).  The other values are reserved.

   Note: There is a typo in DWARF3 spec (published Dec 20, 2005).  In
   sections 7.4, 7.5.1, 7.19, 7.20 the minimum escape code is referred to
   as 0xffffff00 whereas in fact it should be 0xfffffff0.  */
/* dwarf.h */
/* -------------------------------------------- */
/* functions used by tcc.c but not in libtcc.h */

#define PUB_FUNC

#define ST_INLN static inline
#define ST_FUNC static
#define ST_DATA static
/* -------------------------------------------- */
/* include the target specific definitions */

/* ==================== x86_64-gen.c ==================== */

/* number of available registers */
#define NB_REGS 25
#define NB_ASM_REGS 16
/* a register can belong to several classes. The classes must be
   sorted from more general to more precise (see gv2() code which does
   assumptions on it). */
/* generic integer register */

#define RC_INT 0x0001
/* generic float register */
#define RC_FLOAT 0x0002
#define RC_RAX 0x0004
#define RC_RDX 0x0008
#define RC_RCX 0x0010
#define RC_RSI 0x0020
#define RC_RDI 0x0040
/* only for long double */
#define RC_ST0 0x0080
#define RC_R8 0x0100
#define RC_R9 0x0200
#define RC_R10 0x0400
#define RC_R11 0x0800
#define RC_XMM0 0x1000
#define RC_XMM1 0x2000
#define RC_XMM2 0x4000
#define RC_XMM3 0x8000
#define RC_XMM4 0x10000
#define RC_XMM5 0x20000
#define RC_XMM6 0x40000
#define RC_XMM7 0x80000
/* function return: integer register */
#define RC_IRET RC_RAX
/* function return: second integer register */
#define RC_IRE2 RC_RDX
/* function return: float register */
#define RC_FRET RC_XMM0
/* function return: second float register */
#define RC_FRE2 RC_XMM1
/* pretty names for the registers */

enum {
	TREG_RAX = 0,
	TREG_RCX = 1,
	TREG_RDX = 2,
	TREG_RSP = 4,

	TREG_R8 = 8,
	TREG_R9 = 9,
	TREG_R11 = 11,

	TREG_XMM0 = 16,
	TREG_XMM1 = 17,
	TREG_XMM7 = 23,

	TREG_ST0 = 24,

	TREG_MEM = 0x20
};

#define REX_BASE(reg) (((reg) >> 3) & 1)
#define REG_VALUE(reg) ((reg) & 7)
/* return registers for function */
/* single word int return register */

#define REG_IRET TREG_RAX
/* second word return register (for long long) */
#define REG_IRE2 TREG_RDX
/* float return register */
#define REG_FRET TREG_XMM0
/* second float return register */
#define REG_FRE2 TREG_XMM1
/* pointer size, in bytes */

#define PTR_SIZE 8
/* long double size and alignment, in bytes */

#define LDOUBLE_SIZE 16
#define LDOUBLE_ALIGN 16
/* maximum alignment (for aligned attribute support) */

#define MAX_ALIGN 16
/* define if return values need to be extended explicitely
   at caller side (for interfacing with non-TCC compilers) */
ST_FUNC void gen_struct_copy(int size);
/**/
/**/
/* ==================== x86_64-link.c ==================== */

#define EM_TCC_TARGET EM_X86_64
/* relocation type for 32 bit data relocation */

#define R_DATA_32 R_X86_64_32S
#define R_DATA_PTR R_X86_64_64
#define R_JMP_SLOT R_X86_64_JUMP_SLOT
#define R_GLOB_DAT R_X86_64_GLOB_DAT
#define R_RELATIVE R_X86_64_RELATIVE

#define PCRELATIVE_DLLPLT 1

/* -------------------------------------------- */

#define ELFCLASSW ELFCLASS64
#define ElfW(type) Elf ## 64 ## _ ## type
#define ELFW(type) ELF ## 64 ## _ ## type
#define ElfW_Rel ElfW(Rela)
#define SHT_RELX SHT_RELA
#define REL_SECTION_FMT ".rela%s"
/* target address type */
#define addr_t ElfW(Addr)
#define ElfSym ElfW(Sym)

#define LONG_SIZE 4
/* -------------------------------------------- */

#define INCLUDE_STACK_SIZE 32
#define IFDEF_STACK_SIZE 64
#define VSTACK_SIZE 512
#define STRING_MAX_SIZE 1024
#define TOKSTR_MAX_SIZE 256
#define PACK_STACK_SIZE 8
/* must be a power of two */

#define TOK_HASH_SIZE 16384
/* must be a power of two */
#define TOK_ALLOC_INCR 512
/* token max size in int unit when stored in string */
#define TOK_MAX_SIZE 4
/* token symbol management */

typedef struct TokenSym {
	struct TokenSym *hash_next;
	struct Sym *sym_define;/* direct pointer to define */

	struct Sym *sym_label;/* direct pointer to label */

	struct Sym *sym_struct;/* direct pointer to structure */

	struct Sym *sym_identifier;/* direct pointer to identifier */

	int tok;/* token number */

	int len;
	char str[1];
} TokenSym;

typedef unsigned short nwchar_t;

typedef struct CString {
	int size;/* size in bytes */

	int size_allocated;
	char *data;/* nwchar_t* in cases */

} CString;
/* type definition */

typedef struct CType {
	int t;
	struct Sym *ref;
} CType;
/* long double words on host(!) platform */

#define LDOUBLE_WORDS ((sizeof(long double)+3)/4)
/* constant value */

typedef union CValue {
	long double ld;
	double d;
	float f;
	uint64_t i;
	struct {
		char *data;
		int size;
	} str;
	int tab[LDOUBLE_WORDS];
} CValue;
/* value on stack */

typedef struct SValue {
	CType type;/* type */

	unsigned short r;/* register + flags */

	unsigned short r2;/* second register, used for 'long long'
                              type. If not used, set to VT_CONST */

	union {
		struct {
			int jtrue, jfalse;
		};/* forward jmps */

		CValue c;/* constant, if VT_CONST */

	};
	union {
		struct {
			unsigned short cmp_op, cmp_r;
		};/* VT_CMP operation */

		struct Sym *sym;/* symbol, if (VT_SYM | VT_CONST), or if */

	};/* result of unary() for an identifier. */

} SValue;
/* symbol attributes */

struct SymAttr {
	unsigned short
	aligned : 5,/* alignment as log2+1 (0 == unspecified) */

		    packed : 1,
		    weak : 1,
		    visibility : 2,
		    dllexport : 1,
		    nodecorate : 1,
		    dllimport : 1,
		    addrtaken : 1,
		    nodebug : 1,
		    xxxx : 2;/* not used */

};
/* function attributes or temporary attributes for parsing */

struct FuncAttr {
	unsigned
	func_call : 3,/* calling convention (0..5), see below */

		    func_type : 2,/* FUNC_OLD/NEW/ELLIPSIS */

		    func_noreturn : 1,/* attribute((noreturn)) */

		    func_ctor : 1,/* attribute((constructor)) */

		    func_dtor : 1,/* attribute((destructor)) */

		    func_args : 8,/* PE __stdcall args */

		    func_alwinl : 1,/* always_inline */

		    xxxx : 15;
};
/* symbol management */

typedef struct Sym {
	int v;/* symbol token */

	unsigned short r;/* associated register or VT_CONST/VT_LOCAL and LVAL type */

	struct SymAttr a;/* symbol attributes */

	union {
		struct {
			int c;/* associated number or Elf symbol index */

			union {
				int sym_scope;/* scope level for locals */

				int jnext;/* next jump label */

				int jind;/* label position */

				struct FuncAttr f;/* function attributes */

				int auxtype;/* bitfield access type */

			};
		};
		long long enum_val;/* enum constant if IS_ENUM_VAL */

		int *d;/* define token stream */

		struct Sym *cleanup_func;
	};

	CType type;/* associated type */

	union {
		struct Sym *next;/* next related symbol (for fields and anoms) */

		int *e;/* expanded token stream with preprocessor macros */

		int asm_label;/* associated asm label */

		struct Sym *cleanupstate;/* in defined labels */

		int *vla_array_str;/* vla array code */

	};
	struct Sym *prev;/* prev symbol in stack */

	union {
		struct Sym *prev_tok;/* previous symbol for this token */

		struct Sym *cleanup_sym;/* symbol from __attribute__((cleanup())) */

		struct Sym *cleanup_label;/* label in 'pending_gotos' chain */

	};
} Sym;
/* section definition */

typedef struct Section {
	unsigned long data_offset;/* current data offset */

	unsigned char *data;/* section data */

	unsigned long data_allocated;/* used for realloc() handling */

	TCCState *s1;
	int sh_name;/* elf section name (only used during output) */

	int sh_num;/* elf section number */

	int sh_type;/* elf section type */

	int sh_flags;/* elf section flags */

	int sh_info;/* elf section info */

	int sh_addralign;/* elf section alignment */

	int sh_entsize;/* elf entry size */

	unsigned long sh_size;/* section size (only used d\uring output) */

	addr_t sh_addr;/* address at which the section is relocated */

	unsigned long sh_offset;/* file offset */

	int nb_hashed_syms;/* used to resize the hash table */

	struct Section *link;/* link to another section */

	struct Section *reloc;/* corresponding section for relocation, if any */

	struct Section *hash;/* hash table for symbols */

	struct Section *prev;/* previous section on section stack */

	char name[1];/* section name */

} Section;

typedef struct DLLReference {
	int level;
	void *handle;
	unsigned char found, index;
	char name[1];
} DLLReference;
/* -------------------------------------------------- */
/* struct/union/enum symbol space */

#define SYM_STRUCT 0x40000000
/* struct/union field symbol space */
#define SYM_FIELD 0x20000000
/* first anonymous sym */
#define SYM_FIRST_ANOM 0x10000000
/* stored in 'Sym->f.func_type' field */
/* ansi function prototype */

#define FUNC_NEW 1
/* old function prototype */
#define FUNC_OLD 2
/* ansi function prototype with ... */
#define FUNC_ELLIPSIS 3
/* stored in 'Sym->f.func_call' field */
/* standard c call */

#define FUNC_CDECL 0
/* pascal c call */
#define FUNC_STDCALL 1
/* field 'Sym.t' for macros */
/* object like macro */

#define MACRO_OBJ 0
/* function like macro */
#define MACRO_FUNC 1
/* macro uses ## */
#define MACRO_JOIN 2
/* field 'Sym.r' for C labels */
/* label is defined */

#define LABEL_DEFINED 0
/* label is forward defined */
#define LABEL_FORWARD 1
/* label is declared but never used */
#define LABEL_DECLARED 2
/* label isn't in scope, but not yet popped
                            from local_label_stack (stmt exprs) */

#define LABEL_GONE 3
/* type_decl() types */
/* type without variable */

#define TYPE_ABSTRACT 1
/* type with variable */
#define TYPE_DIRECT 2
/* type declares function parameter */
#define TYPE_PARAM 4
/* nested call to post_type */
#define TYPE_NEST 8

#define IO_BUF_SIZE 8192

typedef struct BufferedFile {
	uint8_t *buf_ptr;
	uint8_t *buf_end;
	int fd;
	struct BufferedFile *prev;
	int line_num;/* current line number - here to simplify code */

	int line_ref;/* tcc -E: last printed line */

	int ifndef_macro;/* #ifndef macro / #endif search */

	int ifndef_macro_saved;/* saved ifndef_macro */

	int *ifdef_stack_ptr;/* ifdef_stack value at the start of the file */

	int include_next_index;/* next search path */

	int prev_tok_flags;/* saved tok_flags */

	char filename[1024];/* filename */

	char *true_filename;/* filename not modified by # line directive */

	unsigned char unget[4];
	unsigned char buffer[1];/* extra size for CH_EOB char */

} BufferedFile;
/* end of buffer or '\0' char in file */

#define CH_EOB '\\'
/* end of file */
#define CH_EOF (-1)
/* used to record tokens */

typedef struct TokenString {
	int *str;
	int len;
	int need_spc;
	int allocated_len;
	int last_line_num;
	int save_line_num;
	/* used to chain token-strings with begin/end_macro() */

	struct TokenString *prev;
	const int *prev_ptr;
	char alloc;
} TokenString;
/* GNUC attribute definition */

typedef struct AttributeDef {
	struct SymAttr a;
	struct FuncAttr f;
	struct Section *section;
	Sym *cleanup_func;
	int alias_target;/* token */

	int asm_label;/* associated asm label */

	char attr_mode;/* __attribute__((__mode__(...))) */

	char new_section;/* section is new */

} AttributeDef;
/* inline functions */

typedef struct InlineFunc {
	TokenString *func_str;
	Sym *sym;
	char filename[1];
} InlineFunc;
/* include file cache, used to find files faster and also to eliminate
   inclusion if the include file is protected by #ifndef ... #endif */

typedef struct CachedInclude {
	int ifndef_macro;
	int once;
	int hash_next;/* -1 if none */

	char filename[1];/* path specified in #include */

} CachedInclude;

#define CACHED_INCLUDES_HASH_SIZE 32

typedef struct ExprValue {
	uint64_t v;
	Sym *sym;
	int pcrel;
} ExprValue;

#define MAX_ASM_OPERANDS 30
typedef struct ASMOperand {
	int id;/* GCC 3 optional identifier (0 if number only supported) */

	char constraint[16];
	char asm_str[16];/* computed asm string for operand */

	SValue *vt;/* C value of the expression */

	int ref_index;/* if >= 0, gives reference to a output constraint */

	int input_index;/* if >= 0, gives reference to an input constraint */

	int priority;/* priority, used to assign registers */

	int reg;/* if >= 0, register number used for this operand */

	int is_llong;/* true if double register value */

	int is_memory;/* true if memory operand */

	int is_rw;/* for '+' modifier */

	int is_label;/* for asm goto */

} ASMOperand;
/* extra symbol attributes (not in symbol table) */

struct sym_attr {
	unsigned got_offset;
	unsigned plt_offset;
	int plt_sym;
	int dyn_index;
	unsigned char linker_sym:1;

};

struct TCCState {
	unsigned char verbose;/* if true, display some information during compilation */

	unsigned char nostdinc;/* if true, no standard headers are added */

	unsigned char nostdlib;/* if true, no standard libraries are added */

	unsigned char
	nostdlib_paths;/* if true, the default paths are not searched for libraries */

	unsigned char nocommon;/* if true, do not use common symbols for .bss data */

	unsigned char static_link;/* if true, static linking is performed */

	unsigned char rdynamic;/* if true, all symbols are exported */

	unsigned char
	symbolic;/* if true, resolve symbols in the current module first */

	unsigned char znodelete;/* Set DF_1_NODELETE in dynamic section */

	unsigned char filetype;/* file type for compilation (NONE,C,ASM) */

	unsigned char optimize;/* only to #define __OPTIMIZE__ */

	unsigned char option_pthread;/* -pthread option */

	unsigned char enable_new_dtags;/* -Wl,--enable-new-dtags */

	unsigned int
	cversion;/* supported C ISO version, 199901 (the default), 201112, ... */

	/* C language options */

	unsigned char char_is_unsigned;
	unsigned char leading_underscore;
	unsigned char
	ms_extensions;/* allow nested named struct w/o identifier behave like unnamed */

	unsigned char dollars_in_identifiers;/* allows '$' char in identifiers */

	unsigned char
	ms_bitfields;/* if true, emulate MS algorithm for aligning bitfields */

	unsigned char reverse_funcargs;/* if true, evaluate last function arg first */

	unsigned char gnu89_inline;/* treat 'extern inline' like 'static inline' */

	unsigned char unwind_tables;/* create eh_frame section */

	/* warning switches */

	unsigned char warn_none;
	unsigned char warn_all;
	unsigned char warn_error;
	unsigned char warn_write_strings;
	unsigned char warn_unsupported;
	unsigned char warn_implicit_function_declaration;
	unsigned char warn_discarded_qualifiers;
	/* warning is on (-Woption) */
#define WARN_ON 1
	unsigned char warn_num;/* temp var for tcc_warning_c() */

	unsigned char option_r;/* option -r */

	unsigned char do_bench;/* option -bench */

	unsigned char just_deps;/* option -M  */

	unsigned char gen_deps;/* option -MD  */

	unsigned char include_sys_deps;/* option -MD  */

	unsigned char gen_phony_deps;/* option -MP */

	/* compile with debug symbol (and use them if error during execution) */

	unsigned char do_debug;
	unsigned char dwarf;
	unsigned char do_backtrace;

	unsigned char test_coverage;/* generate\ test coverage code */

	/* use GNU C extensions */

	unsigned char gnu_ext;
	/* use TinyCC extensions */

	unsigned char tcc_ext;

	unsigned char dflag;/* -dX value */

	unsigned char Pflag;/* -P switch (LINE_MACRO_OUTPUT_FORMAT) */

	unsigned char nosse;/* For -mno-sse support. */

	unsigned char has_text_addr;
	addr_t text_addr;/* address of text section */

	unsigned section_align;/* section alignment */

	char *tcc_lib_path;/* CONFIG_TCCDIR or -B option */

	char *soname;/* as specified on the command line (-soname) */

	char *rpath;/* as specified on the command line (-Wl,-rpath=) */

	char *elfint;/* -Wl,-I... on command line */

	char *elf_entryname;/* "_start" unless set */

	char *init_symbol;/* symbols to call at load-time (not used currently) */

	char *fini_symbol;/* symbols to call at unload-time (not used currently) */

	char *mapfile;/* create a mapfile (not used currently) */

	/* output type, see TCC_OUTPUT_XXX */

	int output_type;
	/* output format, see TCC_OUTPUT_FORMAT_xxx */

	int output_format;
	/* nth test to run with -dt -run */

	int run_test;
	/* array of all loaded dlls (including those referenced by loaded dlls) */

	DLLReference **loaded_dlls;
	int nb_loaded_dlls;
	/* include paths */

	char **include_paths;
	int nb_include_paths;

	char **sysinclude_paths;
	int nb_sysinclude_paths;
	/* library paths */

	char **library_paths;
	int nb_library_paths;
	/* crt?.o object path */

	char **crt_paths;
	int nb_crt_paths;
	/* -D / -U options */

	CString cmdline_defs;
	/* -include options */

	CString cmdline_incl;
	/* error handling */

	void *error_opaque;
	void (*error_func)(void *opaque, const char *msg);
	int error_set_jmp_enabled;
	jmp_buf error_jmp_buf;
	int nb_errors;
	/* output file for preprocessing (-E) */

	FILE *ppfp;
	/* for -MD/-MF: collected dependencies for this compilation */

	char **target_deps;
	int nb_target_deps;
	/* compilation */

	BufferedFile *include_stack[INCLUDE_STACK_SIZE];
	BufferedFile **include_stack_ptr;

	int ifdef_stack[IFDEF_STACK_SIZE];
	int *ifdef_stack_ptr;
	/* included files enclosed with #ifndef MACRO */

	int cached_includes_hash[CACHED_INCLUDES_HASH_SIZE];
	CachedInclude **cached_includes;
	int nb_cached_includes;
	/* #pragma pack stack */

	int pack_stack[PACK_STACK_SIZE];
	int *pack_stack_ptr;
	char **pragma_libs;
	int nb_pragma_libs;
	/* inline functions are stored as token lists and compiled last
	       only if referenced */

	struct InlineFunc **inline_fns;
	int nb_inline_fns;
	/* sections */

	Section **sections;
	int nb_sections;/* number of sections, including first dummy section */

	Section **priv_sections;
	int nb_priv_sections;/* number of private sections */

	/* predefined sections */

	Section *text_section, *data_section, *rodata_section, *bss_section;
	Section *common_section;
	Section *cur_text_section;/* current section where function code is generated */

	/* symbol section */

	union {
		Section *symtab_section, *symtab;
	};/* historical alias */

	/* temporary dynamic symbol sections (for dll loading) */

	Section *dynsymtab_section;
	/* exported dynamic symbol section */

	Section *dynsym;
	/* got & plt handling */

	Section *got, *plt;
	/* exception handling */

	Section *eh_frame_section;
	Section *eh_frame_hdr_section;
	unsigned long eh_start;
	/* debug sections */

	Section *stab_section;
	Section *dwarf_info_section;
	Section *dwarf_abbrev_section;
	Section *dwarf_line_section;
	Section *dwarf_aranges_section;
	Section *dwarf_str_section;
	Section *dwarf_line_str_section;
	int dwlo, dwhi;/* dwarf section range */

	/* test coverage */

	Section *tcov_section;
	/* debug state */

	struct _tccdbg *dState;
	/* extra attributes (eg. GOT/PLT value) for symtab symbols */

	struct sym_attr *sym_attrs;
	int nb_sym_attrs;
	/* ptr to next reloc entry reused */

	ElfW_Rel *qrel;
#define qrel s1->qrel

	addr_t tls_start, tls_end;

	/* PE info */

	int pe_subsystem;
	unsigned pe_characteristics;
	unsigned pe_dll_characteristics;
	unsigned pe_file_align;
	unsigned pe_stack_size;
	addr_t pe_imagebase;
	Section *uw_pdata;
	int uw_sym;
	int uw_xsym;
	unsigned uw_offs;

	const char *run_main;/* entry for tcc_run() */

	void *run_ptr;/* runtime_memory */

	unsigned run_size;/* size of runtime_memory  */

	const char *run_stdin;/* custom stdin file for run_main */

	void *run_function_table; /* unwind data */

	struct TCCState *next;
	struct rt_context *rc;/* pointer to backtrace info block */

	void *run_lj, *run_jb;/* sj/lj for tcc_setjmp()/tcc_run() */

	TCCBtFunc *bt_func;
	void *bt_data;
	/* benchmark info */

	int total_idents;
	int total_lines;
	unsigned int total_bytes;
	unsigned int total_output[4];
	/* used by tcc_load_ldscript */

	unsigned char *ld_p;/* text pointer */

	/* for warnings/errors for object files */

	const char *current_filename;
	/* used by main and tcc_parse_args only */

	struct filespec **files;/* files seen on command line */

	int nb_files;/* number thereof */

	int nb_libraries;/* number of libs thereof */

	char *outfile;/* output filename */

	char *deps_outfile;/* option -MF */

	int argc;
	char **argv;
	/* -Wl options */

	char **link_argv;
	int link_argc, link_optind;
};

struct filespec {
	char type;
	char name[1];
};
/* The current value can be: */
/* mask for value location, register or: */

#define VT_VALMASK 0x003f
/* constant in vc (must be first non register value) */
#define VT_CONST 0x0030
/* lvalue, offset on stack */
#define VT_LLOCAL 0x0031
/* offset on stack */
#define VT_LOCAL 0x0032
/* the value is stored in processor flags (in vc) */
#define VT_CMP 0x0033
/* value is the consequence of jmp true (even) */
#define VT_JMP 0x0034
/* value is the consequence of jmp false (odd) */
#define VT_JMPI 0x0035
/* var is an lvalue */
#define VT_LVAL 0x0100
/* a symbol value is added */
#define VT_SYM 0x0200
/* value must be casted to be correct (used for
                                char/short stored in integer registers) */

#define VT_MUSTCAST 0x0C00
/* VT_CONST, but not an (C standard) integer
                                constant expression */

#define VT_NONCONST 0x1000
/* bound checking must be done before
                                dereferencing value */
/* value is bounded. The address of the
                                bounding function call point is in vc */

#define VT_BOUNDED 0x8000
/* types */
/* mask for basic type */

#define VT_BTYPE 0x000f
/* void type */
#define VT_VOID 0
/* signed byte type */
#define VT_BYTE 1
/* short type */
#define VT_SHORT 2
/* integer type */
#define VT_INT 3
/* 64 bit integer */
#define VT_LLONG 4
/* pointer */
#define VT_PTR 5
/* function type */
#define VT_FUNC 6
/* struct/union definition */
#define VT_STRUCT 7
/* IEEE float */
#define VT_FLOAT 8
/* IEEE double */
#define VT_DOUBLE 9
/* IEEE long double */
#define VT_LDOUBLE 10
/* ISOC99 boolean type */
#define VT_BOOL 11
/* 128-bit integer. Only used for x86-64 ABI */
#define VT_QLONG 13
/* 128-bit float. Only used for x86-64 ABI */
#define VT_QFLOAT 14
/* unsigned type */

#define VT_UNSIGNED 0x0010
/* explicitly signed or unsigned */
#define VT_DEFSIGN 0x0020
/* array type (also has VT_PTR) */
#define VT_ARRAY 0x0040
/* bitfield modifier */
#define VT_BITFIELD 0x0080
/* const modifier */
#define VT_CONSTANT 0x0100
/* volatile modifier */
#define VT_VOLATILE 0x0200
/* VLA type (also has VT_PTR and VT_ARRAY) */
#define VT_VLA 0x0400
/* long type (also has VT_INT rsp. VT_LLONG) */
#define VT_LONG 0x0800
/* storage */
/* extern definition */

#define VT_EXTERN 0x00001000
/* static variable */
#define VT_STATIC 0x00002000
/* typedef definition */
#define VT_TYPEDEF 0x00004000
/* inline definition */
#define VT_INLINE 0x00008000
/* thread-local storage */
#define VT_TLS 0x00010000
/* currently unused: 0x000[248]0000  */
/* shift for bitfield shift values (32 - 2*6) */

#define VT_STRUCT_SHIFT 20
#define VT_STRUCT_MASK (((1U << (6+6)) - 1) << VT_STRUCT_SHIFT | VT_BITFIELD)
#define BIT_POS(t) (((t) >> VT_STRUCT_SHIFT) & 0x3f)
#define BIT_SIZE(t) (((t) >> (VT_STRUCT_SHIFT + 6)) & 0x3f)

#define VT_UNION (1 << VT_STRUCT_SHIFT | VT_STRUCT)
/* integral type is an enum really */
#define VT_ENUM (2 << VT_STRUCT_SHIFT)
/* integral type is an enum constant really */
#define VT_ENUM_VAL (3 << VT_STRUCT_SHIFT)

#define IS_ENUM(t) ((t & VT_STRUCT_MASK) == VT_ENUM)
#define IS_ENUM_VAL(t) ((t & VT_STRUCT_MASK) == VT_ENUM_VAL)
#define IS_UNION(t) ((t & (VT_STRUCT_MASK|VT_BTYPE)) == VT_UNION)

#define VT_ATOMIC VT_VOLATILE
/* type mask (except storage) */

#define VT_STORAGE (VT_EXTERN | VT_STATIC | VT_TYPEDEF | VT_INLINE | VT_TLS)
#define VT_TYPE (~(VT_STORAGE|VT_STRUCT_MASK))
/* symbol was created by tccasm.c first */

#define VT_ASM (VT_VOID | 4 << VT_STRUCT_SHIFT)
#define VT_ASM_FUNC (VT_VOID | 5 << VT_STRUCT_SHIFT)
#define IS_ASM_SYM(sym) (((sym)->type.t & ((VT_BTYPE|VT_STRUCT_MASK) & ~(1<<VT_STRUCT_SHIFT))) == VT_ASM)
#define IS_ASM_FUNC(t) ((t & (VT_BTYPE|VT_STRUCT_MASK)) == VT_ASM_FUNC)
/* base type is array (from typedef/typeof) */

#define VT_BT_ARRAY (6 << VT_STRUCT_SHIFT)
#define IS_BT_ARRAY(t) ((t & VT_STRUCT_MASK) == VT_BT_ARRAY)
/* general: set/get the pseudo-bitfield value for bit-mask M */

#define BFVAL(M,N) ((unsigned)((M) & ~((M) << 1)) * (N))
#define BFGET(X,M) (((X) & (M)) / BFVAL(M,1))
/* token values */
/* conditional ops */

#define TOK_LAND 0x90
#define TOK_LOR 0x91
/* warning: the following compare tokens depend on i386 asm code */

#define TOK_ULT 0x92
#define TOK_UGE 0x93
#define TOK_EQ 0x94
#define TOK_NE 0x95
#define TOK_ULE 0x96
#define TOK_UGT 0x97
#define TOK_LT 0x9c
#define TOK_GE 0x9d
#define TOK_LE 0x9e
#define TOK_GT 0x9f

#define TOK_ISCOND(t) (t >= TOK_LAND && t <= TOK_GT)
/* -- */

#define TOK_DEC 0x80
/* inc/dec, to void constant */
#define TOK_MID 0x81
/* ++ */
#define TOK_INC 0x82
/* unsigned division */
#define TOK_UDIV 0x83
/* unsigned modulo */
#define TOK_UMOD 0x84
/* fast division with undefined rounding for pointers */
#define TOK_PDIV 0x85
/* add with carry generation */
#define TOK_ADDC1 0x87
/* add with carry use */
#define TOK_ADDC2 0x88
/* add with carry generation */
#define TOK_SUBC1 0x89
/* add with carry use */
#define TOK_SUBC2 0x8a
/* shift left */
#define TOK_SHL '<'
/* signed shift right */
#define TOK_SAR '>'
/* unsigned shift right */
#define TOK_SHR 0x8b
/* unary minus operation (for floats) */
#define TOK_NEG TOK_MID
/* -> */

#define TOK_ARROW 0xa0
/* three dots */
#define TOK_DOTS 0xa1
/* C++ token ? */
#define TOK_TWODOTS 0xa2
/* ## preprocessing token */
#define TOK_TWOSHARPS 0xa3
/* placeholder token as defined in C99 */
#define TOK_PLCHLDR 0xa4
/* A '##' in a macro to mean pasting */
#define TOK_PPJOIN (TOK_TWOSHARPS | SYM_FIELD)
/* alias of '(' for parsing sizeof (type) */
#define TOK_SOTYPE 0xa7
/* assignment operators */

#define TOK_A_ADD 0xb0
#define TOK_A_SUB 0xb1
#define TOK_A_MUL 0xb2
#define TOK_A_DIV 0xb3
#define TOK_A_MOD 0xb4
#define TOK_A_AND 0xb5
#define TOK_A_OR 0xb6
#define TOK_A_XOR 0xb7
#define TOK_A_SHL 0xb8
#define TOK_A_SAR 0xb9

#define TOK_ASSIGN(t) (t >= TOK_A_ADD && t <= TOK_A_SAR)
#define TOK_ASSIGN_OP(t) ("+-*/%&|^<>"[t - TOK_A_ADD])
/* tokens that carry values (in additional token string space / tokc) --> */
/* char constant in tokc */

#define TOK_CCHAR 0xc0
#define TOK_LCHAR 0xc1
/* number in tokc */
#define TOK_CINT 0xc2
/* unsigned int constant */
#define TOK_CUINT 0xc3
/* long long constant */
#define TOK_CLLONG 0xc4
/* unsigned long long constant */
#define TOK_CULLONG 0xc5
/* long constant */
#define TOK_CLONG 0xc6
/* unsigned long constant */
#define TOK_CULONG 0xc7
/* pointer to string in tokc */
#define TOK_STR 0xc8
#define TOK_LSTR 0xc9
/* float constant */
#define TOK_CFLOAT 0xca
/* double constant */
#define TOK_CDOUBLE 0xcb
/* long double constant */
#define TOK_CLDOUBLE 0xcc
/* preprocessor number */
#define TOK_PPNUM 0xcd
/* preprocessor string */
#define TOK_PPSTR 0xce
/* line number info */
#define TOK_LINENUM 0xcf

#define TOK_HAS_VALUE(t) (t >= TOK_CCHAR && t <= TOK_LINENUM)
/* end of file */

#define TOK_EOF (-1)
/* line feed */
#define TOK_LINEFEED 10
/* all identifiers and strings have token above that */

#define TOK_IDENT 256

enum tcc_token {
	TOK_LAST = TOK_IDENT - 1
#define DEF(id,str) ,id
		   /* ==================== tcctok.h ==================== */
		   /**/
		   /* keywords */

		   DEF(TOK_IF, "if")
		   DEF(TOK_ELSE, "else")
		   DEF(TOK_WHILE, "while")
		   DEF(TOK_FOR, "for")
		   DEF(TOK_DO, "do")
		   DEF(TOK_CONTINUE, "continue")
		   DEF(TOK_BREAK, "break")
		   DEF(TOK_RETURN, "return")
		   DEF(TOK_GOTO, "goto")
		   DEF(TOK_SWITCH, "switch")
		   DEF(TOK_CASE, "case")
		   DEF(TOK_DEFAULT, "default")
		   DEF(TOK_ASM1, "asm")
		   DEF(TOK_ASM2, "__asm")
		   DEF(TOK_ASM3, "__asm__")

		   DEF(TOK_EXTERN, "extern")
		   DEF(TOK_STATIC, "static")
		   DEF(TOK_UNSIGNED, "unsigned")
		   DEF(TOK__Atomic, "_Atomic")
		   DEF(TOK_CONST1, "const")
		   DEF(TOK_CONST2, "__const")/* gcc keyword */

		   DEF(TOK_CONST3, "__const__")/* gcc keyword */

		   DEF(TOK_VOLATILE1, "volatile")
		   DEF(TOK_VOLATILE2, "__volatile")/* gcc keyword */

		   DEF(TOK_VOLATILE3, "__volatile__")/* gcc keyword */

		   DEF(TOK_REGISTER, "register")
		   DEF(TOK_SIGNED1, "signed")
		   DEF(TOK_SIGNED2, "__signed")/* gcc keyword */

		   DEF(TOK_SIGNED3, "__signed__")/* gcc keyword */

		   DEF(TOK_AUTO, "auto")
		   DEF(TOK_INLINE1, "inline")
		   DEF(TOK_INLINE2, "__inline")/* gcc keyword */

		   DEF(TOK_INLINE3, "__inline__")/* gcc keyword */

		   DEF(TOK_RESTRICT1, "restrict")
		   DEF(TOK_RESTRICT2, "__restrict")
		   DEF(TOK_RESTRICT3, "__restrict__")
		   DEF(TOK_EXTENSION, "__extension__")/* gcc keyword */

		   DEF(TOK_THREAD_LOCAL, "_Thread_local")/* C11 thread-local storage */

		   DEF(TOK___thread, "__thread")/* GCC thread-local storage extension */

		   DEF(TOK_GENERIC, "_Generic")
		   DEF(TOK_STATIC_ASSERT, "_Static_assert")

		   DEF(TOK_VOID, "void")
		   DEF(TOK_CHAR, "char")
		   DEF(TOK_INT, "int")
		   DEF(TOK_FLOAT, "float")
		   DEF(TOK_DOUBLE, "double")
		   DEF(TOK_BOOL, "_Bool")
		   DEF(TOK_COMPLEX, "_Complex")
		   DEF(TOK_SHORT, "short")
		   DEF(TOK_LONG, "long")
		   DEF(TOK_STRUCT, "struct")
		   DEF(TOK_UNION, "union")
		   DEF(TOK_TYPEDEF, "typedef")
		   DEF(TOK_ENUM, "enum")
		   DEF(TOK_SIZEOF, "sizeof")
		   DEF(TOK_ATTRIBUTE1, "__attribute")
		   DEF(TOK_ATTRIBUTE2, "__attribute__")
		   DEF(TOK_ALIGNOF1, "__alignof")
		   DEF(TOK_ALIGNOF2, "__alignof__")
		   DEF(TOK_ALIGNOF3, "_Alignof")
		   DEF(TOK_ALIGNAS, "_Alignas")
		   DEF(TOK_TYPEOF1, "typeof")
		   DEF(TOK_TYPEOF2, "__typeof")
		   DEF(TOK_TYPEOF3, "__typeof__")
		   DEF(TOK_LABEL, "__label__")
		   /**/
		   /* the following are not keywords. They are included to ease parsing */
		   /* preprocessor only */

		   DEF(TOK_DEFINE, "define")
		   DEF(TOK_INCLUDE, "include")
		   DEF(TOK_INCLUDE_NEXT, "include_next")
		   DEF(TOK_IFDEF, "ifdef")
		   DEF(TOK_IFNDEF, "ifndef")
		   DEF(TOK_ELIF, "elif")
		   DEF(TOK_ENDIF, "endif")
		   DEF(TOK_DEFINED, "defined")
		   DEF(TOK_UNDEF, "undef")
		   DEF(TOK_ERROR, "error")
		   DEF(TOK_WARNING, "warning")
		   DEF(TOK_LINE, "line")
		   DEF(TOK_PRAGMA, "pragma")
		   DEF(TOK___LINE__, "__LINE__")
		   DEF(TOK___FILE__, "__FILE__")
		   DEF(TOK___DATE__, "__DATE__")
		   DEF(TOK___TIME__, "__TIME__")
		   DEF(TOK___FUNCTION__, "__FUNCTION__")
		   DEF(TOK___VA_ARGS__, "__VA_ARGS__")
		   DEF(TOK___COUNTER__, "__COUNTER__")
		   DEF(TOK___HAS_INCLUDE, "__has_include")
		   DEF(TOK___HAS_INCLUDE_NEXT, "__has_include_next")
		   /* special identifiers */

		   DEF(TOK___FUNC__, "__func__")
		   /* special floating point values */

		   DEF(TOK___NAN__, "__nan__")
		   DEF(TOK___SNAN__, "__snan__")
		   DEF(TOK___INF__, "__inf__")
		   /* attribute identifiers */
		   /* XXX: handle all tokens generically since speed is not critical */

		   DEF(TOK_SECTION1, "section")
		   DEF(TOK_SECTION2, "__section__")
		   DEF(TOK_ALIGNED1, "aligned")
		   DEF(TOK_ALIGNED2, "__aligned__")
		   DEF(TOK_PACKED1, "packed")
		   DEF(TOK_PACKED2, "__packed__")
		   DEF(TOK_WEAK1, "weak")
		   DEF(TOK_WEAK2, "__weak__")
		   DEF(TOK_ALIAS1, "alias")
		   DEF(TOK_ALIAS2, "__alias__")
		   DEF(TOK_USED1, "used")
		   DEF(TOK_USED2, "__used__")
		   DEF(TOK_UNUSED1, "unused")
		   DEF(TOK_UNUSED2, "__unused__")
		   DEF(TOK_FORMAT1, "format")
		   DEF(TOK_FORMAT2, "__format__")
		   DEF(TOK_NODEBUG1, "nodebug")
		   DEF(TOK_NODEBUG2, "__nodebug__")
		   DEF(TOK_CDECL1, "cdecl")
		   DEF(TOK_CDECL2, "__cdecl")
		   DEF(TOK_CDECL3, "__cdecl__")
		   DEF(TOK_STDCALL1, "stdcall")
		   DEF(TOK_STDCALL2, "__stdcall")
		   DEF(TOK_STDCALL3, "__stdcall__")
		   DEF(TOK_FASTCALL1, "fastcall")
		   DEF(TOK_FASTCALL2, "__fastcall")
		   DEF(TOK_FASTCALL3, "__fastcall__")
		   DEF(TOK_THISCALL1, "thiscall")
		   DEF(TOK_THISCALL2, "__thiscall")
		   DEF(TOK_THISCALL3, "__thiscall__")
		   DEF(TOK_REGPARM1, "regparm")
		   DEF(TOK_REGPARM2, "__regparm__")
		   DEF(TOK_CLEANUP1, "cleanup")
		   DEF(TOK_CLEANUP2, "__cleanup__")
		   DEF(TOK_CONSTRUCTOR1, "constructor")
		   DEF(TOK_CONSTRUCTOR2, "__constructor__")
		   DEF(TOK_DESTRUCTOR1, "destructor")
		   DEF(TOK_DESTRUCTOR2, "__destructor__")
		   DEF(TOK_ALWAYS_INLINE1, "always_inline")
		   DEF(TOK_ALWAYS_INLINE2, "__always_inline__")
		   DEF(TOK_NOINLINE, "__noinline__")
		   DEF(TOK_PURE1, "pure")
		   DEF(TOK_PURE2, "__pure__")

		   DEF(TOK_MODE, "__mode__")
		   DEF(TOK_MODE_QI, "__QI__")
		   DEF(TOK_MODE_DI, "__DI__")
		   DEF(TOK_MODE_HI, "__HI__")
		   DEF(TOK_MODE_SI, "__SI__")
		   DEF(TOK_MODE_word, "__word__")

		   DEF(TOK_DLLEXPORT, "dllexport")
		   DEF(TOK_DLLIMPORT, "dllimport")
		   DEF(TOK_NODECORATE, "nodecorate")
		   DEF(TOK_NORETURN1, "noreturn")
		   DEF(TOK_NORETURN2, "__noreturn__")
		   DEF(TOK_NORETURN3, "_Noreturn")
		   DEF(TOK_VISIBILITY1, "visibility")
		   DEF(TOK_VISIBILITY2, "__visibility__")

		   DEF(TOK_builtin_types_compatible_p, "__builtin_types_compatible_p")
		   DEF(TOK_builtin_choose_expr, "__builtin_choose_expr")
		   DEF(TOK_builtin_constant_p, "__builtin_constant_p")
		   DEF(TOK_builtin_frame_address, "__builtin_frame_address")
		   DEF(TOK_builtin_return_address, "__builtin_return_address")
		   DEF(TOK_builtin_expect, "__builtin_expect")
		   DEF(TOK_builtin_unreachable, "__builtin_unreachable")
		   /*DEF(TOK_builtin_va_list, "__builtin_va_list")*/
		   DEF(TOK_builtin_va_start, "__builtin_va_start")
		   /* atomic operations */
#define DEF_ATOMIC(ID) DEF(TOK_ ## __ ## ID, "__"#ID)
		   DEF_ATOMIC(atomic_store)
		   DEF_ATOMIC(atomic_load)
		   DEF_ATOMIC(atomic_exchange)
		   DEF_ATOMIC(atomic_compare_exchange)
		   DEF_ATOMIC(atomic_fetch_add)
		   DEF_ATOMIC(atomic_fetch_sub)
		   DEF_ATOMIC(atomic_fetch_or)
		   DEF_ATOMIC(atomic_fetch_xor)
		   DEF_ATOMIC(atomic_fetch_and)
		   DEF_ATOMIC(atomic_fetch_nand)
		   DEF_ATOMIC(atomic_add_fetch)
		   DEF_ATOMIC(atomic_sub_fetch)
		   DEF_ATOMIC(atomic_or_fetch)
		   DEF_ATOMIC(atomic_xor_fetch)
		   DEF_ATOMIC(atomic_and_fetch)
		   DEF_ATOMIC(atomic_nand_fetch)
		   /* pragma */

		   DEF(TOK_pack, "pack")

		   DEF(TOK_comment, "comment")
		   DEF(TOK_lib, "lib")
		   DEF(TOK_push_macro, "push_macro")
		   DEF(TOK_pop_macro, "pop_macro")
		   DEF(TOK_once, "once")
		   DEF(TOK_option, "option")
		   /* builtin functions or variables */

		   DEF(TOK_memcpy, "memcpy")
		   DEF(TOK_memmove, "memmove")
		   DEF(TOK_memset, "memset")
		   DEF(TOK___divdi3, "__divdi3")
		   DEF(TOK___moddi3, "__moddi3")
		   DEF(TOK___udivdi3, "__udivdi3")
		   DEF(TOK___umoddi3, "__umoddi3")
		   DEF(TOK___ashrdi3, "__ashrdi3")
		   DEF(TOK___lshrdi3, "__lshrdi3")
		   DEF(TOK___ashldi3, "__ashldi3")
		   DEF(TOK___floatundisf, "__floatundisf")
		   DEF(TOK___floatundidf, "__floatundidf")

		   DEF(TOK___floatundixf, "__floatundixf")
		   DEF(TOK___fixunsxfdi, "__fixunsxfdi")

		   DEF(TOK___fixunssfdi, "__fixunssfdi")
		   DEF(TOK___fixunsdfdi, "__fixunsdfdi")
		   DEF(TOK___fixxfdi, "__fixxfdi")

		   DEF(TOK_alloca, "alloca")
		   DEF(TOK___chkstk, "__chkstk")
		   DEF(TOK___tls_index, "__tls_index")
		   /* bound checking symbols */
		   /**/
		   /* Tiny Assembler */
#define DEF_ASM(x) DEF(TOK_ASM_ ## x, #x)
#define DEF_ASMDIR(x) DEF(TOK_ASMDIR_ ## x, "." #x)

		   DEF_ASMDIR(byte)/* must be first directive */

		   DEF_ASMDIR(word)
		   DEF_ASMDIR(align)
		   DEF_ASMDIR(balign)
		   DEF_ASMDIR(p2align)
		   DEF_ASMDIR(set)
		   DEF_ASMDIR(skip)
		   DEF_ASMDIR(space)
		   DEF_ASMDIR(string)
		   DEF_ASMDIR(asciz)
		   DEF_ASMDIR(ascii)
		   DEF_ASMDIR(file)
		   DEF_ASMDIR(globl)
		   DEF_ASMDIR(global)
		   DEF_ASMDIR(weak)
		   DEF_ASMDIR(hidden)
		   DEF_ASMDIR(ident)
		   DEF_ASMDIR(size)
		   DEF_ASMDIR(type)
		   DEF_ASMDIR(text)
		   DEF_ASMDIR(data)
		   DEF_ASMDIR(bss)
		   DEF_ASMDIR(previous)
		   DEF_ASMDIR(pushsection)
		   DEF_ASMDIR(popsection)
		   DEF_ASMDIR(fill)
		   DEF_ASMDIR(rept)
		   DEF_ASMDIR(endr)
		   DEF_ASMDIR(org)
		   DEF_ASMDIR(quad)

		   DEF_ASMDIR(code64)

		   DEF_ASMDIR(short)
		   DEF_ASMDIR(long)
		   DEF_ASMDIR(int)
		   DEF_ASMDIR(symver)
		   DEF_ASMDIR(reloc)
		   DEF_ASMDIR(section)/* must be last directive */

		   /* ==================== i386-tok.h ==================== */
		   /* ------------------------------------------------------------------ */
		   /* WARNING: relative order of tokens is important. */

#define DEF_BWL(x) DEF(TOK_ASM_ ## x ## b, #x "b") DEF(TOK_ASM_ ## x ## w, #x "w") DEF(TOK_ASM_ ## x ## l, #x "l") DEF(TOK_ASM_ ## x, #x)

#define DEF_BWLQ(x) DEF(TOK_ASM_ ## x ## b, #x "b") DEF(TOK_ASM_ ## x ## w, #x "w") DEF(TOK_ASM_ ## x ## l, #x "l") DEF(TOK_ASM_ ## x ## q, #x "q") DEF(TOK_ASM_ ## x, #x)

#define DEF_WLQ(x) DEF(TOK_ASM_ ## x ## w, #x "w") DEF(TOK_ASM_ ## x ## l, #x "l") DEF(TOK_ASM_ ## x ## q, #x "q") DEF(TOK_ASM_ ## x, #x)
#define DEF_BWLX DEF_BWLQ
#define DEF_WLX DEF_WLQ
#define DEF_FP1(x) DEF(TOK_ASM_ ## f ## x ## s, "f" #x "s") DEF(TOK_ASM_ ## fi ## x ## l, "fi" #x "l") DEF(TOK_ASM_ ## f ## x ## l, "f" #x "l") DEF(TOK_ASM_ ## fi ## x ## s, "fi" #x "s")

#define DEF_FP(x) DEF(TOK_ASM_ ## f ## x, "f" #x ) DEF(TOK_ASM_ ## f ## x ## p, "f" #x "p") DEF_FP1(x)
#define DEF_ASMTEST(x,suffix) DEF_ASM(x ## o ## suffix) DEF_ASM(x ## no ## suffix) DEF_ASM(x ## b ## suffix) DEF_ASM(x ## c ## suffix) DEF_ASM(x ## nae ## suffix) DEF_ASM(x ## nb ## suffix) DEF_ASM(x ## nc ## suffix) DEF_ASM(x ## ae ## suffix) DEF_ASM(x ## e ## suffix) DEF_ASM(x ## z ## suffix) DEF_ASM(x ## ne ## suffix) DEF_ASM(x ## nz ## suffix) DEF_ASM(x ## be ## suffix) DEF_ASM(x ## na ## suffix) DEF_ASM(x ## nbe ## suffix) DEF_ASM(x ## a ## suffix) DEF_ASM(x ## s ## suffix) DEF_ASM(x ## ns ## suffix) DEF_ASM(x ## p ## suffix) DEF_ASM(x ## pe ## suffix) DEF_ASM(x ## np ## suffix) DEF_ASM(x ## po ## suffix) DEF_ASM(x ## l ## suffix) DEF_ASM(x ## nge ## suffix) DEF_ASM(x ## nl ## suffix) DEF_ASM(x ## ge ## suffix) DEF_ASM(x ## le ## suffix) DEF_ASM(x ## ng ## suffix) DEF_ASM(x ## nle ## suffix) DEF_ASM(x ## g ## suffix)
		   /* ------------------------------------------------------------------ */
		   /* register */

		   DEF_ASM(al)
		   DEF_ASM(cl)
		   DEF_ASM(dl)
		   DEF_ASM(bl)
		   DEF_ASM(ah)
		   DEF_ASM(ch)
		   DEF_ASM(dh)
		   DEF_ASM(bh)
		   DEF_ASM(ax)
		   DEF_ASM(cx)
		   DEF_ASM(dx)
		   DEF_ASM(bx)
		   DEF_ASM(sp)
		   DEF_ASM(bp)
		   DEF_ASM(si)
		   DEF_ASM(di)
		   DEF_ASM(eax)
		   DEF_ASM(ecx)
		   DEF_ASM(edx)
		   DEF_ASM(ebx)
		   DEF_ASM(esp)
		   DEF_ASM(ebp)
		   DEF_ASM(esi)
		   DEF_ASM(edi)

		   DEF_ASM(rax)
		   DEF_ASM(rcx)
		   DEF_ASM(rdx)
		   DEF_ASM(rbx)
		   DEF_ASM(rsp)
		   DEF_ASM(rbp)
		   DEF_ASM(rsi)
		   DEF_ASM(rdi)

		   DEF_ASM(mm0)
		   DEF_ASM(mm1)
		   DEF_ASM(mm2)
		   DEF_ASM(mm3)
		   DEF_ASM(mm4)
		   DEF_ASM(mm5)
		   DEF_ASM(mm6)
		   DEF_ASM(mm7)
		   DEF_ASM(xmm0)
		   DEF_ASM(xmm1)
		   DEF_ASM(xmm2)
		   DEF_ASM(xmm3)
		   DEF_ASM(xmm4)
		   DEF_ASM(xmm5)
		   DEF_ASM(xmm6)
		   DEF_ASM(xmm7)
		   DEF_ASM(cr0)
		   DEF_ASM(cr1)
		   DEF_ASM(cr2)
		   DEF_ASM(cr3)
		   DEF_ASM(cr4)
		   DEF_ASM(cr5)
		   DEF_ASM(cr6)
		   DEF_ASM(cr7)
		   DEF_ASM(tr0)
		   DEF_ASM(tr1)
		   DEF_ASM(tr2)
		   DEF_ASM(tr3)
		   DEF_ASM(tr4)
		   DEF_ASM(tr5)
		   DEF_ASM(tr6)
		   DEF_ASM(tr7)
		   DEF_ASM(db0)
		   DEF_ASM(db1)
		   DEF_ASM(db2)
		   DEF_ASM(db3)
		   DEF_ASM(db4)
		   DEF_ASM(db5)
		   DEF_ASM(db6)
		   DEF_ASM(db7)
		   DEF_ASM(dr0)
		   DEF_ASM(dr1)
		   DEF_ASM(dr2)
		   DEF_ASM(dr3)
		   DEF_ASM(dr4)
		   DEF_ASM(dr5)
		   DEF_ASM(dr6)
		   DEF_ASM(dr7)
		   DEF_ASM(es)
		   DEF_ASM(cs)
		   DEF_ASM(ss)
		   DEF_ASM(ds)
		   DEF_ASM(fs)
		   DEF_ASM(gs)
		   DEF_ASM(st)
		   DEF_ASM(rip)
		   /* The four low parts of sp/bp/si/di that exist only on
		       x86-64 (encoding aliased to ah,ch,dh,dh when not using REX). */

		   DEF_ASM(spl)
		   DEF_ASM(bpl)
		   DEF_ASM(sil)
		   DEF_ASM(dil)
		   /* generic two operands */

		   DEF_BWLX(mov)

		   DEF_BWLX(add)
		   DEF_BWLX(or)
		   DEF_BWLX(adc)
		   DEF_BWLX(sbb)
		   DEF_BWLX(and)
		   DEF_BWLX(sub)
		   DEF_BWLX(xor)
		   DEF_BWLX(cmp)
		   /* unary ops */

		   DEF_BWLX(inc)
		   DEF_BWLX(dec)
		   DEF_BWLX(not)
		   DEF_BWLX(neg)
		   DEF_BWLX(mul)
		   DEF_BWLX(imul)
		   DEF_BWLX(div)
		   DEF_BWLX(idiv)

		   DEF_BWLX(xchg)
		   DEF_BWLX(test)
		   /* shifts */

		   DEF_BWLX(rol)
		   DEF_BWLX(ror)
		   DEF_BWLX(rcl)
		   DEF_BWLX(rcr)
		   DEF_BWLX(shl)
		   DEF_BWLX(shr)
		   DEF_BWLX(sar)

		   DEF_WLX(shld)
		   DEF_WLX(shrd)

		   DEF_ASM(pushw)
		   DEF_ASM(pushl)

		   DEF_ASM(pushq)

		   DEF_ASM(push)

		   DEF_ASM(popw)
		   DEF_ASM(popl)

		   DEF_ASM(popq)

		   DEF_ASM(pop)

		   DEF_BWL(in)
		   DEF_BWL(out)

		   DEF_WLX(movzb)
		   DEF_ASM(movzwl)
		   DEF_ASM(movsbw)
		   DEF_ASM(movsbl)
		   DEF_ASM(movswl)

		   DEF_ASM(movsbq)
		   DEF_ASM(movswq)
		   DEF_ASM(movzwq)
		   DEF_ASM(movslq)

		   DEF_WLX(lea)

		   DEF_ASM(les)
		   DEF_ASM(lds)
		   DEF_ASM(lss)
		   DEF_ASM(lfs)
		   DEF_ASM(lgs)

		   DEF_ASM(call)
		   DEF_ASM(jmp)
		   DEF_ASM(lcall)
		   DEF_ASM(ljmp)

		   DEF_ASMTEST(j,)

		   DEF_ASMTEST(set,)
		   DEF_ASMTEST(set,b)
		   DEF_ASMTEST(cmov,)

		   DEF_WLX(bsf)
		   DEF_WLX(bsr)
		   DEF_WLX(bt)
		   DEF_WLX(bts)
		   DEF_WLX(btr)
		   DEF_WLX(btc)
		   DEF_WLX(popcnt)
		   DEF_WLX(tzcnt)
		   DEF_WLX(lzcnt)

		   DEF_WLX(lar)
		   DEF_WLX(lsl)
		   /* generic FP ops */

		   DEF_FP(add)
		   DEF_FP(mul)

		   DEF_ASM(fcom)
		   DEF_ASM(fcom_1)/* non existent op, just to have a regular table */

		   DEF_FP1(com)

		   DEF_FP(comp)
		   DEF_FP(sub)
		   DEF_FP(subr)
		   DEF_FP(div)
		   DEF_FP(divr)

		   DEF_BWLX(xadd)
		   DEF_BWLX(cmpxchg)
		   /* string ops */

		   DEF_BWLX(cmps)
		   DEF_BWLX(scmp)
		   DEF_BWL(ins)
		   DEF_BWL(outs)
		   DEF_BWLX(lods)
		   DEF_BWLX(slod)
		   DEF_BWLX(movs)
		   DEF_BWLX(smov)
		   DEF_BWLX(scas)
		   DEF_BWLX(ssca)
		   DEF_BWLX(stos)
		   DEF_BWLX(ssto)
		   /* generic asm ops */

#define ALT(x)
#define DEF_ASM_OP0(name,opcode) DEF_ASM(name)
#define DEF_ASM_OP0L(name,opcode,group,instr_type)
#define DEF_ASM_OP1(name,opcode,group,instr_type,op0)
#define DEF_ASM_OP2(name,opcode,group,instr_type,op0,op1)

		   /* ==================== x86_64-asm.h ==================== */
		   DEF_ASM_OP0(clc, 0xf8)/* must be first OP0 */

		   DEF_ASM_OP0(cld, 0xfc)
		   DEF_ASM_OP0(cli, 0xfa)
		   DEF_ASM_OP0(clts, 0x0f06)
		   DEF_ASM_OP0(cmc, 0xf5)
		   DEF_ASM_OP0(lahf, 0x9f)
		   DEF_ASM_OP0(sahf, 0x9e)
		   DEF_ASM_OP0(pushfq, 0x9c)
		   DEF_ASM_OP0(popfq, 0x9d)
		   DEF_ASM_OP0(pushf, 0x9c)
		   DEF_ASM_OP0(popf, 0x9d)
		   DEF_ASM_OP0(stc, 0xf9)
		   DEF_ASM_OP0(std, 0xfd)
		   DEF_ASM_OP0(sti, 0xfb)
		   DEF_ASM_OP0(aaa, 0x37)
		   DEF_ASM_OP0(aas, 0x3f)
		   DEF_ASM_OP0(daa, 0x27)
		   DEF_ASM_OP0(das, 0x2f)
		   DEF_ASM_OP0(aad, 0xd50a)
		   DEF_ASM_OP0(aam, 0xd40a)
		   DEF_ASM_OP0(cbw, 0x6698)
		   DEF_ASM_OP0(cwd, 0x6699)
		   DEF_ASM_OP0(cwde, 0x98)
		   DEF_ASM_OP0(cdq, 0x99)
		   DEF_ASM_OP0(cbtw, 0x6698)
		   DEF_ASM_OP0(cwtl, 0x98)
		   DEF_ASM_OP0(cwtd, 0x6699)
		   DEF_ASM_OP0(cltd, 0x99)
		   DEF_ASM_OP0(cqto, 0x4899)
		   DEF_ASM_OP0(int3, 0xcc)
		   DEF_ASM_OP0(into, 0xce)
		   DEF_ASM_OP0(iret, 0xcf)
		   DEF_ASM_OP0(iretw, 0x66cf)
		   DEF_ASM_OP0(iretl, 0xcf)
		   DEF_ASM_OP0(iretq, 0x48cf)
		   DEF_ASM_OP0(rsm, 0x0faa)
		   DEF_ASM_OP0(hlt, 0xf4)
		   DEF_ASM_OP0(wait, 0x9b)
		   DEF_ASM_OP0(nop, 0x90)
		   DEF_ASM_OP0(pause, 0xf390)
		   DEF_ASM_OP0(xlat, 0xd7)

		   DEF_ASM_OP0L(vmcall, 0xc1, 0, OPC_0F01)
		   DEF_ASM_OP0L(vmlaunch, 0xc2, 0, OPC_0F01)
		   DEF_ASM_OP0L(vmresume, 0xc3, 0, OPC_0F01)
		   DEF_ASM_OP0L(vmxoff, 0xc4, 0, OPC_0F01)
		   /* strings */

		   ALT(DEF_ASM_OP0L(cmpsb, 0xa6, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(scmpb, 0xa6, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(insb, 0x6c, 0, OPC_BWL))
		   ALT(DEF_ASM_OP0L(outsb, 0x6e, 0, OPC_BWL))

		   ALT(DEF_ASM_OP0L(lodsb, 0xac, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(slodb, 0xac, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(movsb, 0xa4, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(smovb, 0xa4, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(scasb, 0xae, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(sscab, 0xae, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(stosb, 0xaa, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(sstob, 0xaa, 0, OPC_BWLX))
		   /* bits */

		   ALT(DEF_ASM_OP2(bsfw, 0x0fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(bsrw, 0x0fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

		   ALT(DEF_ASM_OP2(btw, 0x0fa3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btw, 0x0fba, 4, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(btsw, 0x0fab, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btsw, 0x0fba, 5, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(btrw, 0x0fb3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btrw, 0x0fba, 6, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(btcw, 0x0fbb, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btcw, 0x0fba, 7, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(popcntw, 0xf30fb8, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

		   ALT(DEF_ASM_OP2(tzcntw, 0xf30fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(lzcntw, 0xf30fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
		   /* prefixes */

		   DEF_ASM_OP0(lock, 0xf0)
		   DEF_ASM_OP0(rep, 0xf3)
		   DEF_ASM_OP0(repe, 0xf3)
		   DEF_ASM_OP0(repz, 0xf3)
		   DEF_ASM_OP0(repne, 0xf2)
		   DEF_ASM_OP0(repnz, 0xf2)

		   DEF_ASM_OP0(invd, 0x0f08)
		   DEF_ASM_OP0(wbinvd, 0x0f09)
		   DEF_ASM_OP0(cpuid, 0x0fa2)
		   DEF_ASM_OP0(wrmsr, 0x0f30)
		   DEF_ASM_OP0(rdtsc, 0x0f31)
		   DEF_ASM_OP0(rdmsr, 0x0f32)
		   DEF_ASM_OP0(rdpmc, 0x0f33)

		   DEF_ASM_OP0(syscall, 0x0f05)
		   DEF_ASM_OP0(sysret, 0x0f07)
		   DEF_ASM_OP0L(sysretq, 0x480f07, 0, 0)
		   DEF_ASM_OP0(ud2, 0x0f0b)
		   /* NOTE: we took the same order as gas opcode definition order */
		   /* Right now we can't express the fact that 0xa1/0xa3 can't use $eax and a
		      32 bit moffset as operands.
		   ALT(DEF_ASM_OP2(movb, 0xa0, 0, OPC_BWLX, OPT_ADDR, OPT_EAX))
		   ALT(DEF_ASM_OP2(movb, 0xa2, 0, OPC_BWLX, OPT_EAX, OPT_ADDR)) */

		   ALT(DEF_ASM_OP2(movb, 0x88, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(movb, 0x8a, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
		   /* The moves are special: the 0xb8 form supports IM64 (the only insn that
		      does) with REG64.  It doesn't support IM32 with REG64, it would use
		      the full movabs form (64bit immediate).  For IM32->REG64 we prefer
		      the 0xc7 opcode.  So disallow all 64bit forms and code the rest by hand. */

		   ALT(DEF_ASM_OP2(movb, 0xb0, 0, OPC_REG | OPC_BWLX, OPT_IM, OPT_REG))
		   ALT(DEF_ASM_OP2(mov, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
		   ALT(DEF_ASM_OP2(movq, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
		   ALT(DEF_ASM_OP2(movb, 0xc6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP2(movw, 0x8c, 0, OPC_MODRM | OPC_WLX, OPT_SEG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(movw, 0x8e, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_SEG))

		   ALT(DEF_ASM_OP2(movw, 0x0f20, 0, OPC_MODRM | OPC_WLX, OPT_CR, OPT_REG64))
		   ALT(DEF_ASM_OP2(movw, 0x0f21, 0, OPC_MODRM | OPC_WLX, OPT_DB, OPT_REG64))
		   ALT(DEF_ASM_OP2(movw, 0x0f22, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_CR))
		   ALT(DEF_ASM_OP2(movw, 0x0f23, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_DB))

		   ALT(DEF_ASM_OP2(movsbw, 0x660fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG16))
		   ALT(DEF_ASM_OP2(movsbl, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(movsbq, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(movswl, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(movswq, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))
		   ALT(DEF_ASM_OP2(movslq, 0x63, 0, OPC_MODRM, OPT_REG32 | OPT_EA, OPT_REG))
		   ALT(DEF_ASM_OP2(movzbw, 0x0fb6, 0, OPC_MODRM | OPC_WLX, OPT_REG8 | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(movzwl, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(movzwq, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))

		   ALT(DEF_ASM_OP1(pushq, 0x6a, 0, 0, OPT_IM8S))
		   ALT(DEF_ASM_OP1(push, 0x6a, 0, 0, OPT_IM8S))
		   ALT(DEF_ASM_OP1(pushw, 0x666a, 0, 0, OPT_IM8S))
		   ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG64))
		   ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG16))
		   ALT(DEF_ASM_OP1(pushw, 0xff, 6, OPC_MODRM | OPC_WLX, OPT_REG64 | OPT_EA))
		   ALT(DEF_ASM_OP1(pushw, 0x6668, 0, 0, OPT_IM16))
		   ALT(DEF_ASM_OP1(pushw, 0x68, 0, OPC_WLX, OPT_IM32))
		   ALT(DEF_ASM_OP1(pushw, 0x06, 0, OPC_WLX, OPT_SEG))

		   ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG64))
		   ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG16))
		   ALT(DEF_ASM_OP1(popw, 0x8f, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP1(popw, 0x07, 0, OPC_WLX, OPT_SEG))

		   ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_REGW, OPT_EAX))
		   ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_EAX, OPT_REGW))
		   ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))

		   ALT(DEF_ASM_OP2(inb, 0xe4, 0, OPC_BWL, OPT_IM8, OPT_EAX))
		   ALT(DEF_ASM_OP1(inb, 0xe4, 0, OPC_BWL, OPT_IM8))
		   ALT(DEF_ASM_OP2(inb, 0xec, 0, OPC_BWL, OPT_DX, OPT_EAX))
		   ALT(DEF_ASM_OP1(inb, 0xec, 0, OPC_BWL, OPT_DX))

		   ALT(DEF_ASM_OP2(outb, 0xe6, 0, OPC_BWL, OPT_EAX, OPT_IM8))
		   ALT(DEF_ASM_OP1(outb, 0xe6, 0, OPC_BWL, OPT_IM8))
		   ALT(DEF_ASM_OP2(outb, 0xee, 0, OPC_BWL, OPT_EAX, OPT_DX))
		   ALT(DEF_ASM_OP1(outb, 0xee, 0, OPC_BWL, OPT_DX))

		   ALT(DEF_ASM_OP2(leaw, 0x8d, 0, OPC_MODRM | OPC_WLX, OPT_EA, OPT_REG))

		   ALT(DEF_ASM_OP2(les, 0xc4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lds, 0xc5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lss, 0x0fb2, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lfs, 0x0fb4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lgs, 0x0fb5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   /* arith */

		   ALT(DEF_ASM_OP2(addb, 0x00, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))/* XXX: use D bit ? */

		   ALT(DEF_ASM_OP2(addb, 0x02, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
		   ALT(DEF_ASM_OP2(addb, 0x04, 0, OPC_ARITH | OPC_BWLX, OPT_IM, OPT_EAX))
		   ALT(DEF_ASM_OP2(addw, 0x83, 0, OPC_ARITH | OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP2(addb, 0x80, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

		   ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
		   ALT(DEF_ASM_OP2(testb, 0xa8, 0, OPC_BWLX, OPT_IM, OPT_EAX))
		   ALT(DEF_ASM_OP2(testb, 0xf6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

		   ALT(DEF_ASM_OP1(incb, 0xfe, 0, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP1(decb, 0xfe, 1, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP1(notb, 0xf6, 2, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP1(negb, 0xf6, 3, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP1(mulb, 0xf6, 4, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP1(imulb, 0xf6, 5, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP2(imulw, 0x0faf, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
		   ALT(DEF_ASM_OP3(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW))
		   ALT(DEF_ASM_OP3(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW))

		   ALT(DEF_ASM_OP1(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP2(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
		   ALT(DEF_ASM_OP1(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP2(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
		   /* shifts */

		   ALT(DEF_ASM_OP2(rolb, 0xc0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_IM8, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(rolb, 0xd2, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_CL, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP1(rolb, 0xd0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_EA | OPT_REG))

		   ALT(DEF_ASM_OP3(shldw, 0x0fa4, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP3(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP2(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP3(shrdw, 0x0fac, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP3(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP2(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))

		   ALT(DEF_ASM_OP1(call, 0xff, 2, OPC_MODRM, OPT_INDIR))
		   ALT(DEF_ASM_OP1(call, 0xe8, 0, 0, OPT_DISP))
		   DEF_ASM_OP1(callq, 0xff, 2, OPC_MODRM, OPT_INDIR)
		   ALT(DEF_ASM_OP1(callq, 0xe8, 0, 0, OPT_DISP))
		   ALT(DEF_ASM_OP1(jmp, 0xff, 4, OPC_MODRM, OPT_INDIR))
		   ALT(DEF_ASM_OP1(jmp, 0xeb, 0, 0, OPT_DISP8))

		   ALT(DEF_ASM_OP1(lcall, 0xff, 3, OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(ljmp, 0xff, 5, OPC_MODRM, OPT_EA))
		   DEF_ASM_OP1(ljmpw, 0x66ff, 5, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(ljmpl, 0xff, 5, OPC_MODRM, OPT_EA)

		   ALT(DEF_ASM_OP1(int, 0xcd, 0, 0, OPT_IM8))
		   ALT(DEF_ASM_OP1(seto, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
		   ALT(DEF_ASM_OP1(setob, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
		   DEF_ASM_OP2(enter, 0xc8, 0, 0, OPT_IM16, OPT_IM8)
		   DEF_ASM_OP0(leave, 0xc9)
		   DEF_ASM_OP0(ret, 0xc3)
		   DEF_ASM_OP0(retq, 0xc3)
		   ALT(DEF_ASM_OP1(retq, 0xc2, 0, 0, OPT_IM16))
		   ALT(DEF_ASM_OP1(ret, 0xc2, 0, 0, OPT_IM16))
		   DEF_ASM_OP0(lret, 0xcb)
		   ALT(DEF_ASM_OP1(lret, 0xca, 0, 0, OPT_IM16))

		   ALT(DEF_ASM_OP1(jo, 0x70, 0, OPC_TEST, OPT_DISP8))
		   DEF_ASM_OP1(loopne, 0xe0, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loopnz, 0xe0, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loope, 0xe1, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loopz, 0xe1, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loop, 0xe2, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(jecxz, 0x67e3, 0, 0, OPT_DISP8)
		   /* float */
		   /* specific fcomp handling */

		   ALT(DEF_ASM_OP0L(fcomp, 0xd8d9, 0, 0))

		   ALT(DEF_ASM_OP1(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST))
		   ALT(DEF_ASM_OP2(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
		   ALT(DEF_ASM_OP2(fadd, 0xdcc0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
		   ALT(DEF_ASM_OP2(fmul, 0xdcc8, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
		   ALT(DEF_ASM_OP0L(fadd, 0xdec1, 0, OPC_FARITH))
		   ALT(DEF_ASM_OP1(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST))
		   ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
		   ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
		   ALT(DEF_ASM_OP0L(faddp, 0xdec1, 0, OPC_FARITH))
		   ALT(DEF_ASM_OP1(fadds, 0xd8, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(fiaddl, 0xda, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(faddl, 0xdc, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(fiadds, 0xde, 0, OPC_FARITH | OPC_MODRM, OPT_EA))

		   DEF_ASM_OP0(fucompp, 0xdae9)
		   DEF_ASM_OP0(ftst, 0xd9e4)
		   DEF_ASM_OP0(fxam, 0xd9e5)
		   DEF_ASM_OP0(fld1, 0xd9e8)
		   DEF_ASM_OP0(fldl2t, 0xd9e9)
		   DEF_ASM_OP0(fldl2e, 0xd9ea)
		   DEF_ASM_OP0(fldpi, 0xd9eb)
		   DEF_ASM_OP0(fldlg2, 0xd9ec)
		   DEF_ASM_OP0(fldln2, 0xd9ed)
		   DEF_ASM_OP0(fldz, 0xd9ee)

		   DEF_ASM_OP0(f2xm1, 0xd9f0)
		   DEF_ASM_OP0(fyl2x, 0xd9f1)
		   DEF_ASM_OP0(fptan, 0xd9f2)
		   DEF_ASM_OP0(fpatan, 0xd9f3)
		   DEF_ASM_OP0(fxtract, 0xd9f4)
		   DEF_ASM_OP0(fprem1, 0xd9f5)
		   DEF_ASM_OP0(fdecstp, 0xd9f6)
		   DEF_ASM_OP0(fincstp, 0xd9f7)
		   DEF_ASM_OP0(fprem, 0xd9f8)
		   DEF_ASM_OP0(fyl2xp1, 0xd9f9)
		   DEF_ASM_OP0(fsqrt, 0xd9fa)
		   DEF_ASM_OP0(fsincos, 0xd9fb)
		   DEF_ASM_OP0(frndint, 0xd9fc)
		   DEF_ASM_OP0(fscale, 0xd9fd)
		   DEF_ASM_OP0(fsin, 0xd9fe)
		   DEF_ASM_OP0(fcos, 0xd9ff)
		   DEF_ASM_OP0(fchs, 0xd9e0)
		   DEF_ASM_OP0(fabs, 0xd9e1)
		   DEF_ASM_OP0(fninit, 0xdbe3)
		   DEF_ASM_OP0(fnclex, 0xdbe2)
		   DEF_ASM_OP0(fnop, 0xd9d0)
		   DEF_ASM_OP0(fwait, 0x9b)
		   /* fp load */

		   DEF_ASM_OP1(fld, 0xd9c0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fldl, 0xd9c0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(flds, 0xd9, 0, OPC_MODRM, OPT_EA)
		   ALT(DEF_ASM_OP1(fldl, 0xdd, 0, OPC_MODRM, OPT_EA))
		   DEF_ASM_OP1(fildl, 0xdb, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fildq, 0xdf, 5, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fildll, 0xdf, 5, OPC_MODRM,OPT_EA)
		   DEF_ASM_OP1(fldt, 0xdb, 5, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fbld, 0xdf, 4, OPC_MODRM, OPT_EA)
		   /* fp store */

		   DEF_ASM_OP1(fst, 0xddd0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fstl, 0xddd0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fsts, 0xd9, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fstps, 0xd9, 3, OPC_MODRM, OPT_EA)
		   ALT(DEF_ASM_OP1(fstl, 0xdd, 2, OPC_MODRM, OPT_EA))
		   DEF_ASM_OP1(fstpl, 0xdd, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fist, 0xdf, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistp, 0xdf, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistl, 0xdb, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistpl, 0xdb, 3, OPC_MODRM, OPT_EA)

		   DEF_ASM_OP1(fstp, 0xddd8, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fistpq, 0xdf, 7, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistpll, 0xdf, 7, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fstpt, 0xdb, 7, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fbstp, 0xdf, 6, OPC_MODRM, OPT_EA)
		   /* exchange */

		   DEF_ASM_OP0(fxch, 0xd9c9)
		   ALT(DEF_ASM_OP1(fxch, 0xd9c8, 0, OPC_REG, OPT_ST))
		   /* misc FPU */

		   DEF_ASM_OP1(fucom, 0xdde0, 0, OPC_REG, OPT_ST )
		   DEF_ASM_OP1(fucomp, 0xdde8, 0, OPC_REG, OPT_ST )

		   DEF_ASM_OP0L(finit, 0xdbe3, 0, OPC_FWAIT)
		   DEF_ASM_OP1(fldcw, 0xd9, 5, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fnstcw, 0xd9, 7, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fstcw, 0xd9, 7, OPC_MODRM | OPC_FWAIT, OPT_EA )
		   DEF_ASM_OP0(fnstsw, 0xdfe0)
		   ALT(DEF_ASM_OP1(fnstsw, 0xdfe0, 0, 0, OPT_EAX ))
		   ALT(DEF_ASM_OP1(fnstsw, 0xdd, 7, OPC_MODRM, OPT_EA ))
		   DEF_ASM_OP1(fstsw, 0xdfe0, 0, OPC_FWAIT, OPT_EAX )
		   ALT(DEF_ASM_OP0L(fstsw, 0xdfe0, 0, OPC_FWAIT))
		   ALT(DEF_ASM_OP1(fstsw, 0xdd, 7, OPC_MODRM | OPC_FWAIT, OPT_EA ))
		   DEF_ASM_OP0L(fclex, 0xdbe2, 0, OPC_FWAIT)
		   DEF_ASM_OP1(fnstenv, 0xd9, 6, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fstenv, 0xd9, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
		   DEF_ASM_OP1(fldenv, 0xd9, 4, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fnsave, 0xdd, 6, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fsave, 0xdd, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
		   DEF_ASM_OP1(frstor, 0xdd, 4, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(ffree, 0xddc0, 4, OPC_REG, OPT_ST )
		   DEF_ASM_OP1(ffreep, 0xdfc0, 4, OPC_REG, OPT_ST )
		   DEF_ASM_OP1(fxsave, 0x0fae, 0, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fxrstor, 0x0fae, 1, OPC_MODRM, OPT_EA )
		   /* The *q forms of fxrstor/fxsave use a REX prefix.
		          If the operand would use extended registers we would have to modify
		          it instead of generating a second one.  Currently that's no
		          problem with TCC, we don't use extended registers.  */

		   DEF_ASM_OP1(fxsaveq, 0x0fae, 0, OPC_MODRM | OPC_48, OPT_EA )
		   DEF_ASM_OP1(fxrstorq, 0x0fae, 1, OPC_MODRM | OPC_48, OPT_EA )
		   /* segments */

		   DEF_ASM_OP2(arpl, 0x63, 0, OPC_MODRM, OPT_REG16, OPT_REG16 | OPT_EA)
		   ALT(DEF_ASM_OP2(larw, 0x0f02, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
		   DEF_ASM_OP1(lgdt, 0x0f01, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lgdtq, 0x0f01, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lidt, 0x0f01, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lidtq, 0x0f01, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lldt, 0x0f00, 2, OPC_MODRM, OPT_EA | OPT_REG)
		   DEF_ASM_OP1(lmsw, 0x0f01, 6, OPC_MODRM, OPT_EA | OPT_REG)
		   ALT(DEF_ASM_OP2(lslw, 0x0f03, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_REG))
		   DEF_ASM_OP1(ltr, 0x0f00, 3, OPC_MODRM, OPT_EA | OPT_REG16)
		   DEF_ASM_OP1(sgdt, 0x0f01, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sgdtq, 0x0f01, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sidt, 0x0f01, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sidtq, 0x0f01, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sldt, 0x0f00, 0, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP1(smsw, 0x0f01, 4, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM, OPT_REG32 | OPT_EA)
		   ALT(DEF_ASM_OP1(str, 0x660f00, 1, OPC_MODRM, OPT_REG16))
		   ALT(DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM | OPC_48, OPT_REG64))
		   DEF_ASM_OP1(verr, 0x0f00, 4, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP1(verw, 0x0f00, 5, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP0L(swapgs, 0x0f01, 7, OPC_MODRM)
		   /* 486 */
		   /* bswap can't be applied to 16bit regs */

		   DEF_ASM_OP1(bswap, 0x0fc8, 0, OPC_REG, OPT_REG32 )
		   DEF_ASM_OP1(bswapl, 0x0fc8, 0, OPC_REG, OPT_REG32 )
		   DEF_ASM_OP1(bswapq, 0x0fc8, 0, OPC_REG | OPC_48, OPT_REG64 )

		   ALT(DEF_ASM_OP2(xaddb, 0x0fc0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
		   ALT(DEF_ASM_OP2(cmpxchgb, 0x0fb0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
		   DEF_ASM_OP1(invlpg, 0x0f01, 7, OPC_MODRM, OPT_EA )
		   /* pentium */

		   DEF_ASM_OP1(cmpxchg8b, 0x0fc7, 1, OPC_MODRM, OPT_EA )
		   /* AMD 64 */

		   DEF_ASM_OP1(cmpxchg16b, 0x0fc7, 1, OPC_MODRM | OPC_48, OPT_EA )
		   /* pentium pro */

		   ALT(DEF_ASM_OP2(cmovo, 0x0f40, 0, OPC_MODRM | OPC_TEST | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

		   DEF_ASM_OP2(fcmovb, 0xdac0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmove, 0xdac8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovbe, 0xdad0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovu, 0xdad8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovnb, 0xdbc0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovne, 0xdbc8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovnbe, 0xdbd0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovnu, 0xdbd8, 0, OPC_REG, OPT_ST, OPT_ST0 )

		   DEF_ASM_OP2(fucomi, 0xdbe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcomi, 0xdbf0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fucomip, 0xdfe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcomip, 0xdff0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   /* mmx */

		   DEF_ASM_OP0(emms, 0x0f77)/* must be last OP0 */

		   DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_MMXSSE )
		   /* movd shouldn't accept REG64, but AMD64 spec uses it for 32 and 64 bit
		          moves, so let's be compatible. */

		   ALT(DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG64, OPT_MMXSSE ))
		   ALT(DEF_ASM_OP2(movq, 0x0f6e, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_MMXSSE ))
		   ALT(DEF_ASM_OP2(movq, 0x0f6f, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_MMX ))
		   ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG32 ))
		   ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))
		   ALT(DEF_ASM_OP2(movq, 0x0f7f, 0, OPC_MODRM, OPT_MMX, OPT_EA | OPT_MMX ))
		   ALT(DEF_ASM_OP2(movq, 0x660fd6, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_SSE ))
		   ALT(DEF_ASM_OP2(movq, 0xf30f7e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE ))
		   ALT(DEF_ASM_OP2(movq, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))

		   DEF_ASM_OP2(packssdw, 0x0f6b, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(packsswb, 0x0f63, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(packuswb, 0x0f67, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddb, 0x0ffc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddw, 0x0ffd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddd, 0x0ffe, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddsb, 0x0fec, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddsw, 0x0fed, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddusb, 0x0fdc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddusw, 0x0fdd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pand, 0x0fdb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pandn, 0x0fdf, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpeqb, 0x0f74, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpeqw, 0x0f75, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpeqd, 0x0f76, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpgtb, 0x0f64, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpgtw, 0x0f65, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpgtd, 0x0f66, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmaddwd, 0x0ff5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmulhw, 0x0fe5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmullw, 0x0fd5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(por, 0x0feb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psllw, 0x0ff1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psllw, 0x0f71, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(pslld, 0x0ff2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(pslld, 0x0f72, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psllq, 0x0ff3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psllq, 0x0f73, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psraw, 0x0fe1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psraw, 0x0f71, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrad, 0x0fe2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrad, 0x0f72, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrlw, 0x0fd1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrlw, 0x0f71, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrld, 0x0fd2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrld, 0x0f72, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrlq, 0x0fd3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrlq, 0x0f73, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psubb, 0x0ff8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubw, 0x0ff9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubd, 0x0ffa, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubsb, 0x0fe8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubsw, 0x0fe9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubusb, 0x0fd8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubusw, 0x0fd9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckhbw, 0x0f68, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckhwd, 0x0f69, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckhdq, 0x0f6a, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpcklbw, 0x0f60, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpcklwd, 0x0f61, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckldq, 0x0f62, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pxor, 0x0fef, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   /* sse */

		   DEF_ASM_OP1(ldmxcsr, 0x0fae, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(stmxcsr, 0x0fae, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP2(movups, 0x0f10, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
		   ALT(DEF_ASM_OP2(movups, 0x0f11, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
		   DEF_ASM_OP2(movaps, 0x0f28, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
		   ALT(DEF_ASM_OP2(movaps, 0x0f29, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
		   DEF_ASM_OP2(movhps, 0x0f16, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
		   ALT(DEF_ASM_OP2(movhps, 0x0f17, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
		   DEF_ASM_OP2(addps, 0x0f58, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(cvtpi2ps, 0x0f2a, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_SSE )
		   DEF_ASM_OP2(cvtps2pi, 0x0f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
		   DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
		   ALT(DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
		   DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
		   ALT(DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
		   DEF_ASM_OP2(cvttps2pi, 0x0f2c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
		   DEF_ASM_OP2(andps, 0x0f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(divps, 0x0f5e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(maxps, 0x0f5f, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(minps, 0x0f5d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(mulps, 0x0f59, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(pavgb, 0x0fe0, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(pavgw, 0x0fe3, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(pmaxsw, 0x0fee, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmaxub, 0x0fde, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pminsw, 0x0fea, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pminub, 0x0fda, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(rcpss, 0x0f53, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(rsqrtps, 0x0f52, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(sqrtps, 0x0f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(sqrtss, 0xf30f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(subps, 0x0f5c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   /* sse2 */

		   DEF_ASM_OP2(andpd, 0x660f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
		   DEF_ASM_OP2(sqrtsd, 0xf20f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
		   /* movnti should only accept REG32 and REG64, we accept more */

		   DEF_ASM_OP2(movnti, 0x0fc3, 0, OPC_MODRM, OPT_REG, OPT_EA)
		   DEF_ASM_OP2(movntil, 0x0fc3, 0, OPC_MODRM, OPT_REG32, OPT_EA)
		   DEF_ASM_OP2(movntiq, 0x0fc3, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_EA)
		   DEF_ASM_OP1(prefetchnta, 0x0f18, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetcht0, 0x0f18, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetcht1, 0x0f18, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetcht2, 0x0f18, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetchw, 0x0f0d, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP0L(lfence, 0x0fae, 5, OPC_MODRM)
		   DEF_ASM_OP0L(mfence, 0x0fae, 6, OPC_MODRM)
		   DEF_ASM_OP0L(sfence, 0x0fae, 7, OPC_MODRM)
		   DEF_ASM_OP1(clflush, 0x0fae, 7, OPC_MODRM, OPT_EA)
		   /* Control-Flow Enforcement */

		   DEF_ASM_OP0L(endbr64, 0xf30f1e, 7, OPC_MODRM)
#undef ALT
#undef DEF_ASM_OP0
#undef DEF_ASM_OP0L
#undef DEF_ASM_OP1
#undef DEF_ASM_OP2
#undef DEF_ASM_OP3

#define ALT(x)
#define DEF_ASM_OP0(name,opcode)
#define DEF_ASM_OP0L(name,opcode,group,instr_type) DEF_ASM(name)
#define DEF_ASM_OP1(name,opcode,group,instr_type,op0) DEF_ASM(name)
#define DEF_ASM_OP2(name,opcode,group,instr_type,op0,op1) DEF_ASM(name)

		   DEF_ASM_OP0(clc, 0xf8)/* must be first OP0 */

		   DEF_ASM_OP0(cld, 0xfc)
		   DEF_ASM_OP0(cli, 0xfa)
		   DEF_ASM_OP0(clts, 0x0f06)
		   DEF_ASM_OP0(cmc, 0xf5)
		   DEF_ASM_OP0(lahf, 0x9f)
		   DEF_ASM_OP0(sahf, 0x9e)
		   DEF_ASM_OP0(pushfq, 0x9c)
		   DEF_ASM_OP0(popfq, 0x9d)
		   DEF_ASM_OP0(pushf, 0x9c)
		   DEF_ASM_OP0(popf, 0x9d)
		   DEF_ASM_OP0(stc, 0xf9)
		   DEF_ASM_OP0(std, 0xfd)
		   DEF_ASM_OP0(sti, 0xfb)
		   DEF_ASM_OP0(aaa, 0x37)
		   DEF_ASM_OP0(aas, 0x3f)
		   DEF_ASM_OP0(daa, 0x27)
		   DEF_ASM_OP0(das, 0x2f)
		   DEF_ASM_OP0(aad, 0xd50a)
		   DEF_ASM_OP0(aam, 0xd40a)
		   DEF_ASM_OP0(cbw, 0x6698)
		   DEF_ASM_OP0(cwd, 0x6699)
		   DEF_ASM_OP0(cwde, 0x98)
		   DEF_ASM_OP0(cdq, 0x99)
		   DEF_ASM_OP0(cbtw, 0x6698)
		   DEF_ASM_OP0(cwtl, 0x98)
		   DEF_ASM_OP0(cwtd, 0x6699)
		   DEF_ASM_OP0(cltd, 0x99)
		   DEF_ASM_OP0(cqto, 0x4899)
		   DEF_ASM_OP0(int3, 0xcc)
		   DEF_ASM_OP0(into, 0xce)
		   DEF_ASM_OP0(iret, 0xcf)
		   DEF_ASM_OP0(iretw, 0x66cf)
		   DEF_ASM_OP0(iretl, 0xcf)
		   DEF_ASM_OP0(iretq, 0x48cf)
		   DEF_ASM_OP0(rsm, 0x0faa)
		   DEF_ASM_OP0(hlt, 0xf4)
		   DEF_ASM_OP0(wait, 0x9b)
		   DEF_ASM_OP0(nop, 0x90)
		   DEF_ASM_OP0(pause, 0xf390)
		   DEF_ASM_OP0(xlat, 0xd7)

		   DEF_ASM_OP0L(vmcall, 0xc1, 0, OPC_0F01)
		   DEF_ASM_OP0L(vmlaunch, 0xc2, 0, OPC_0F01)
		   DEF_ASM_OP0L(vmresume, 0xc3, 0, OPC_0F01)
		   DEF_ASM_OP0L(vmxoff, 0xc4, 0, OPC_0F01)
		   /* strings */

		   ALT(DEF_ASM_OP0L(cmpsb, 0xa6, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(scmpb, 0xa6, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(insb, 0x6c, 0, OPC_BWL))
		   ALT(DEF_ASM_OP0L(outsb, 0x6e, 0, OPC_BWL))

		   ALT(DEF_ASM_OP0L(lodsb, 0xac, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(slodb, 0xac, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(movsb, 0xa4, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(smovb, 0xa4, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(scasb, 0xae, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(sscab, 0xae, 0, OPC_BWLX))

		   ALT(DEF_ASM_OP0L(stosb, 0xaa, 0, OPC_BWLX))
		   ALT(DEF_ASM_OP0L(sstob, 0xaa, 0, OPC_BWLX))
		   /* bits */

		   ALT(DEF_ASM_OP2(bsfw, 0x0fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(bsrw, 0x0fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

		   ALT(DEF_ASM_OP2(btw, 0x0fa3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btw, 0x0fba, 4, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(btsw, 0x0fab, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btsw, 0x0fba, 5, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(btrw, 0x0fb3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btrw, 0x0fba, 6, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(btcw, 0x0fbb, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP2(btcw, 0x0fba, 7, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

		   ALT(DEF_ASM_OP2(popcntw, 0xf30fb8, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

		   ALT(DEF_ASM_OP2(tzcntw, 0xf30fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(lzcntw, 0xf30fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
		   /* prefixes */

		   DEF_ASM_OP0(lock, 0xf0)
		   DEF_ASM_OP0(rep, 0xf3)
		   DEF_ASM_OP0(repe, 0xf3)
		   DEF_ASM_OP0(repz, 0xf3)
		   DEF_ASM_OP0(repne, 0xf2)
		   DEF_ASM_OP0(repnz, 0xf2)

		   DEF_ASM_OP0(invd, 0x0f08)
		   DEF_ASM_OP0(wbinvd, 0x0f09)
		   DEF_ASM_OP0(cpuid, 0x0fa2)
		   DEF_ASM_OP0(wrmsr, 0x0f30)
		   DEF_ASM_OP0(rdtsc, 0x0f31)
		   DEF_ASM_OP0(rdmsr, 0x0f32)
		   DEF_ASM_OP0(rdpmc, 0x0f33)

		   DEF_ASM_OP0(syscall, 0x0f05)
		   DEF_ASM_OP0(sysret, 0x0f07)
		   DEF_ASM_OP0L(sysretq, 0x480f07, 0, 0)
		   DEF_ASM_OP0(ud2, 0x0f0b)
		   /* NOTE: we took the same order as gas opcode definition order */
		   /* Right now we can't express the fact that 0xa1/0xa3 can't use $eax and a
		      32 bit moffset as operands.
		   ALT(DEF_ASM_OP2(movb, 0xa0, 0, OPC_BWLX, OPT_ADDR, OPT_EAX))
		   ALT(DEF_ASM_OP2(movb, 0xa2, 0, OPC_BWLX, OPT_EAX, OPT_ADDR)) */

		   ALT(DEF_ASM_OP2(movb, 0x88, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(movb, 0x8a, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
		   /* The moves are special: the 0xb8 form supports IM64 (the only insn that
		      does) with REG64.  It doesn't support IM32 with REG64, it would use
		      the full movabs form (64bit immediate).  For IM32->REG64 we prefer
		      the 0xc7 opcode.  So disallow all 64bit forms and code the rest by hand. */

		   ALT(DEF_ASM_OP2(movb, 0xb0, 0, OPC_REG | OPC_BWLX, OPT_IM, OPT_REG))
		   ALT(DEF_ASM_OP2(mov, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
		   ALT(DEF_ASM_OP2(movq, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
		   ALT(DEF_ASM_OP2(movb, 0xc6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP2(movw, 0x8c, 0, OPC_MODRM | OPC_WLX, OPT_SEG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(movw, 0x8e, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_SEG))

		   ALT(DEF_ASM_OP2(movw, 0x0f20, 0, OPC_MODRM | OPC_WLX, OPT_CR, OPT_REG64))
		   ALT(DEF_ASM_OP2(movw, 0x0f21, 0, OPC_MODRM | OPC_WLX, OPT_DB, OPT_REG64))
		   ALT(DEF_ASM_OP2(movw, 0x0f22, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_CR))
		   ALT(DEF_ASM_OP2(movw, 0x0f23, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_DB))

		   ALT(DEF_ASM_OP2(movsbw, 0x660fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG16))
		   ALT(DEF_ASM_OP2(movsbl, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(movsbq, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(movswl, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(movswq, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))
		   ALT(DEF_ASM_OP2(movslq, 0x63, 0, OPC_MODRM, OPT_REG32 | OPT_EA, OPT_REG))
		   ALT(DEF_ASM_OP2(movzbw, 0x0fb6, 0, OPC_MODRM | OPC_WLX, OPT_REG8 | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(movzwl, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(movzwq, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))

		   ALT(DEF_ASM_OP1(pushq, 0x6a, 0, 0, OPT_IM8S))
		   ALT(DEF_ASM_OP1(push, 0x6a, 0, 0, OPT_IM8S))
		   ALT(DEF_ASM_OP1(pushw, 0x666a, 0, 0, OPT_IM8S))
		   ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG64))
		   ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG16))
		   ALT(DEF_ASM_OP1(pushw, 0xff, 6, OPC_MODRM | OPC_WLX, OPT_REG64 | OPT_EA))
		   ALT(DEF_ASM_OP1(pushw, 0x6668, 0, 0, OPT_IM16))
		   ALT(DEF_ASM_OP1(pushw, 0x68, 0, OPC_WLX, OPT_IM32))
		   ALT(DEF_ASM_OP1(pushw, 0x06, 0, OPC_WLX, OPT_SEG))

		   ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG64))
		   ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG16))
		   ALT(DEF_ASM_OP1(popw, 0x8f, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA))
		   ALT(DEF_ASM_OP1(popw, 0x07, 0, OPC_WLX, OPT_SEG))

		   ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_REGW, OPT_EAX))
		   ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_EAX, OPT_REGW))
		   ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))

		   ALT(DEF_ASM_OP2(inb, 0xe4, 0, OPC_BWL, OPT_IM8, OPT_EAX))
		   ALT(DEF_ASM_OP1(inb, 0xe4, 0, OPC_BWL, OPT_IM8))
		   ALT(DEF_ASM_OP2(inb, 0xec, 0, OPC_BWL, OPT_DX, OPT_EAX))
		   ALT(DEF_ASM_OP1(inb, 0xec, 0, OPC_BWL, OPT_DX))

		   ALT(DEF_ASM_OP2(outb, 0xe6, 0, OPC_BWL, OPT_EAX, OPT_IM8))
		   ALT(DEF_ASM_OP1(outb, 0xe6, 0, OPC_BWL, OPT_IM8))
		   ALT(DEF_ASM_OP2(outb, 0xee, 0, OPC_BWL, OPT_EAX, OPT_DX))
		   ALT(DEF_ASM_OP1(outb, 0xee, 0, OPC_BWL, OPT_DX))

		   ALT(DEF_ASM_OP2(leaw, 0x8d, 0, OPC_MODRM | OPC_WLX, OPT_EA, OPT_REG))

		   ALT(DEF_ASM_OP2(les, 0xc4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lds, 0xc5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lss, 0x0fb2, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lfs, 0x0fb4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   ALT(DEF_ASM_OP2(lgs, 0x0fb5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
		   /* arith */

		   ALT(DEF_ASM_OP2(addb, 0x00, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))/* XXX: use D bit ? */

		   ALT(DEF_ASM_OP2(addb, 0x02, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
		   ALT(DEF_ASM_OP2(addb, 0x04, 0, OPC_ARITH | OPC_BWLX, OPT_IM, OPT_EAX))
		   ALT(DEF_ASM_OP2(addw, 0x83, 0, OPC_ARITH | OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP2(addb, 0x80, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

		   ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
		   ALT(DEF_ASM_OP2(testb, 0xa8, 0, OPC_BWLX, OPT_IM, OPT_EAX))
		   ALT(DEF_ASM_OP2(testb, 0xf6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

		   ALT(DEF_ASM_OP1(incb, 0xfe, 0, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP1(decb, 0xfe, 1, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP1(notb, 0xf6, 2, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP1(negb, 0xf6, 3, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP1(mulb, 0xf6, 4, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP1(imulb, 0xf6, 5, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

		   ALT(DEF_ASM_OP2(imulw, 0x0faf, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
		   ALT(DEF_ASM_OP3(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW))
		   ALT(DEF_ASM_OP3(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW | OPT_EA, OPT_REGW))
		   ALT(DEF_ASM_OP2(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW))

		   ALT(DEF_ASM_OP1(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP2(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
		   ALT(DEF_ASM_OP1(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
		   ALT(DEF_ASM_OP2(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
		   /* shifts */

		   ALT(DEF_ASM_OP2(rolb, 0xc0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_IM8, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP2(rolb, 0xd2, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_CL, OPT_EA | OPT_REG))
		   ALT(DEF_ASM_OP1(rolb, 0xd0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_EA | OPT_REG))

		   ALT(DEF_ASM_OP3(shldw, 0x0fa4, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP3(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP2(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP3(shrdw, 0x0fac, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP3(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
		   ALT(DEF_ASM_OP2(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))

		   ALT(DEF_ASM_OP1(call, 0xff, 2, OPC_MODRM, OPT_INDIR))
		   ALT(DEF_ASM_OP1(call, 0xe8, 0, 0, OPT_DISP))
		   DEF_ASM_OP1(callq, 0xff, 2, OPC_MODRM, OPT_INDIR)
		   ALT(DEF_ASM_OP1(callq, 0xe8, 0, 0, OPT_DISP))
		   ALT(DEF_ASM_OP1(jmp, 0xff, 4, OPC_MODRM, OPT_INDIR))
		   ALT(DEF_ASM_OP1(jmp, 0xeb, 0, 0, OPT_DISP8))

		   ALT(DEF_ASM_OP1(lcall, 0xff, 3, OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(ljmp, 0xff, 5, OPC_MODRM, OPT_EA))
		   DEF_ASM_OP1(ljmpw, 0x66ff, 5, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(ljmpl, 0xff, 5, OPC_MODRM, OPT_EA)

		   ALT(DEF_ASM_OP1(int, 0xcd, 0, 0, OPT_IM8))
		   ALT(DEF_ASM_OP1(seto, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
		   ALT(DEF_ASM_OP1(setob, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
		   DEF_ASM_OP2(enter, 0xc8, 0, 0, OPT_IM16, OPT_IM8)
		   DEF_ASM_OP0(leave, 0xc9)
		   DEF_ASM_OP0(ret, 0xc3)
		   DEF_ASM_OP0(retq, 0xc3)
		   ALT(DEF_ASM_OP1(retq, 0xc2, 0, 0, OPT_IM16))
		   ALT(DEF_ASM_OP1(ret, 0xc2, 0, 0, OPT_IM16))
		   DEF_ASM_OP0(lret, 0xcb)
		   ALT(DEF_ASM_OP1(lret, 0xca, 0, 0, OPT_IM16))

		   ALT(DEF_ASM_OP1(jo, 0x70, 0, OPC_TEST, OPT_DISP8))
		   DEF_ASM_OP1(loopne, 0xe0, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loopnz, 0xe0, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loope, 0xe1, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loopz, 0xe1, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(loop, 0xe2, 0, 0, OPT_DISP8)
		   DEF_ASM_OP1(jecxz, 0x67e3, 0, 0, OPT_DISP8)
		   /* float */
		   /* specific fcomp handling */

		   ALT(DEF_ASM_OP0L(fcomp, 0xd8d9, 0, 0))

		   ALT(DEF_ASM_OP1(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST))
		   ALT(DEF_ASM_OP2(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
		   ALT(DEF_ASM_OP2(fadd, 0xdcc0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
		   ALT(DEF_ASM_OP2(fmul, 0xdcc8, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
		   ALT(DEF_ASM_OP0L(fadd, 0xdec1, 0, OPC_FARITH))
		   ALT(DEF_ASM_OP1(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST))
		   ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
		   ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
		   ALT(DEF_ASM_OP0L(faddp, 0xdec1, 0, OPC_FARITH))
		   ALT(DEF_ASM_OP1(fadds, 0xd8, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(fiaddl, 0xda, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(faddl, 0xdc, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
		   ALT(DEF_ASM_OP1(fiadds, 0xde, 0, OPC_FARITH | OPC_MODRM, OPT_EA))

		   DEF_ASM_OP0(fucompp, 0xdae9)
		   DEF_ASM_OP0(ftst, 0xd9e4)
		   DEF_ASM_OP0(fxam, 0xd9e5)
		   DEF_ASM_OP0(fld1, 0xd9e8)
		   DEF_ASM_OP0(fldl2t, 0xd9e9)
		   DEF_ASM_OP0(fldl2e, 0xd9ea)
		   DEF_ASM_OP0(fldpi, 0xd9eb)
		   DEF_ASM_OP0(fldlg2, 0xd9ec)
		   DEF_ASM_OP0(fldln2, 0xd9ed)
		   DEF_ASM_OP0(fldz, 0xd9ee)

		   DEF_ASM_OP0(f2xm1, 0xd9f0)
		   DEF_ASM_OP0(fyl2x, 0xd9f1)
		   DEF_ASM_OP0(fptan, 0xd9f2)
		   DEF_ASM_OP0(fpatan, 0xd9f3)
		   DEF_ASM_OP0(fxtract, 0xd9f4)
		   DEF_ASM_OP0(fprem1, 0xd9f5)
		   DEF_ASM_OP0(fdecstp, 0xd9f6)
		   DEF_ASM_OP0(fincstp, 0xd9f7)
		   DEF_ASM_OP0(fprem, 0xd9f8)
		   DEF_ASM_OP0(fyl2xp1, 0xd9f9)
		   DEF_ASM_OP0(fsqrt, 0xd9fa)
		   DEF_ASM_OP0(fsincos, 0xd9fb)
		   DEF_ASM_OP0(frndint, 0xd9fc)
		   DEF_ASM_OP0(fscale, 0xd9fd)
		   DEF_ASM_OP0(fsin, 0xd9fe)
		   DEF_ASM_OP0(fcos, 0xd9ff)
		   DEF_ASM_OP0(fchs, 0xd9e0)
		   DEF_ASM_OP0(fabs, 0xd9e1)
		   DEF_ASM_OP0(fninit, 0xdbe3)
		   DEF_ASM_OP0(fnclex, 0xdbe2)
		   DEF_ASM_OP0(fnop, 0xd9d0)
		   DEF_ASM_OP0(fwait, 0x9b)
		   /* fp load */

		   DEF_ASM_OP1(fld, 0xd9c0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fldl, 0xd9c0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(flds, 0xd9, 0, OPC_MODRM, OPT_EA)
		   ALT(DEF_ASM_OP1(fldl, 0xdd, 0, OPC_MODRM, OPT_EA))
		   DEF_ASM_OP1(fildl, 0xdb, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fildq, 0xdf, 5, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fildll, 0xdf, 5, OPC_MODRM,OPT_EA)
		   DEF_ASM_OP1(fldt, 0xdb, 5, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fbld, 0xdf, 4, OPC_MODRM, OPT_EA)
		   /* fp store */

		   DEF_ASM_OP1(fst, 0xddd0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fstl, 0xddd0, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fsts, 0xd9, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fstps, 0xd9, 3, OPC_MODRM, OPT_EA)
		   ALT(DEF_ASM_OP1(fstl, 0xdd, 2, OPC_MODRM, OPT_EA))
		   DEF_ASM_OP1(fstpl, 0xdd, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fist, 0xdf, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistp, 0xdf, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistl, 0xdb, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistpl, 0xdb, 3, OPC_MODRM, OPT_EA)

		   DEF_ASM_OP1(fstp, 0xddd8, 0, OPC_REG, OPT_ST)
		   DEF_ASM_OP1(fistpq, 0xdf, 7, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fistpll, 0xdf, 7, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fstpt, 0xdb, 7, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(fbstp, 0xdf, 6, OPC_MODRM, OPT_EA)
		   /* exchange */

		   DEF_ASM_OP0(fxch, 0xd9c9)
		   ALT(DEF_ASM_OP1(fxch, 0xd9c8, 0, OPC_REG, OPT_ST))
		   /* misc FPU */

		   DEF_ASM_OP1(fucom, 0xdde0, 0, OPC_REG, OPT_ST )
		   DEF_ASM_OP1(fucomp, 0xdde8, 0, OPC_REG, OPT_ST )

		   DEF_ASM_OP0L(finit, 0xdbe3, 0, OPC_FWAIT)
		   DEF_ASM_OP1(fldcw, 0xd9, 5, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fnstcw, 0xd9, 7, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fstcw, 0xd9, 7, OPC_MODRM | OPC_FWAIT, OPT_EA )
		   DEF_ASM_OP0(fnstsw, 0xdfe0)
		   ALT(DEF_ASM_OP1(fnstsw, 0xdfe0, 0, 0, OPT_EAX ))
		   ALT(DEF_ASM_OP1(fnstsw, 0xdd, 7, OPC_MODRM, OPT_EA ))
		   DEF_ASM_OP1(fstsw, 0xdfe0, 0, OPC_FWAIT, OPT_EAX )
		   ALT(DEF_ASM_OP0L(fstsw, 0xdfe0, 0, OPC_FWAIT))
		   ALT(DEF_ASM_OP1(fstsw, 0xdd, 7, OPC_MODRM | OPC_FWAIT, OPT_EA ))
		   DEF_ASM_OP0L(fclex, 0xdbe2, 0, OPC_FWAIT)
		   DEF_ASM_OP1(fnstenv, 0xd9, 6, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fstenv, 0xd9, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
		   DEF_ASM_OP1(fldenv, 0xd9, 4, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fnsave, 0xdd, 6, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fsave, 0xdd, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
		   DEF_ASM_OP1(frstor, 0xdd, 4, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(ffree, 0xddc0, 4, OPC_REG, OPT_ST )
		   DEF_ASM_OP1(ffreep, 0xdfc0, 4, OPC_REG, OPT_ST )
		   DEF_ASM_OP1(fxsave, 0x0fae, 0, OPC_MODRM, OPT_EA )
		   DEF_ASM_OP1(fxrstor, 0x0fae, 1, OPC_MODRM, OPT_EA )
		   /* The *q forms of fxrstor/fxsave use a REX prefix.
		          If the operand would use extended registers we would have to modify
		          it instead of generating a second one.  Currently that's no
		          problem with TCC, we don't use extended registers.  */

		   DEF_ASM_OP1(fxsaveq, 0x0fae, 0, OPC_MODRM | OPC_48, OPT_EA )
		   DEF_ASM_OP1(fxrstorq, 0x0fae, 1, OPC_MODRM | OPC_48, OPT_EA )
		   /* segments */

		   DEF_ASM_OP2(arpl, 0x63, 0, OPC_MODRM, OPT_REG16, OPT_REG16 | OPT_EA)
		   ALT(DEF_ASM_OP2(larw, 0x0f02, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
		   DEF_ASM_OP1(lgdt, 0x0f01, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lgdtq, 0x0f01, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lidt, 0x0f01, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lidtq, 0x0f01, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(lldt, 0x0f00, 2, OPC_MODRM, OPT_EA | OPT_REG)
		   DEF_ASM_OP1(lmsw, 0x0f01, 6, OPC_MODRM, OPT_EA | OPT_REG)
		   ALT(DEF_ASM_OP2(lslw, 0x0f03, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_REG))
		   DEF_ASM_OP1(ltr, 0x0f00, 3, OPC_MODRM, OPT_EA | OPT_REG16)
		   DEF_ASM_OP1(sgdt, 0x0f01, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sgdtq, 0x0f01, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sidt, 0x0f01, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sidtq, 0x0f01, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(sldt, 0x0f00, 0, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP1(smsw, 0x0f01, 4, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM, OPT_REG32 | OPT_EA)
		   ALT(DEF_ASM_OP1(str, 0x660f00, 1, OPC_MODRM, OPT_REG16))
		   ALT(DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM | OPC_48, OPT_REG64))
		   DEF_ASM_OP1(verr, 0x0f00, 4, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP1(verw, 0x0f00, 5, OPC_MODRM, OPT_REG | OPT_EA)
		   DEF_ASM_OP0L(swapgs, 0x0f01, 7, OPC_MODRM)
		   /* 486 */
		   /* bswap can't be applied to 16bit regs */

		   DEF_ASM_OP1(bswap, 0x0fc8, 0, OPC_REG, OPT_REG32 )
		   DEF_ASM_OP1(bswapl, 0x0fc8, 0, OPC_REG, OPT_REG32 )
		   DEF_ASM_OP1(bswapq, 0x0fc8, 0, OPC_REG | OPC_48, OPT_REG64 )

		   ALT(DEF_ASM_OP2(xaddb, 0x0fc0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
		   ALT(DEF_ASM_OP2(cmpxchgb, 0x0fb0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
		   DEF_ASM_OP1(invlpg, 0x0f01, 7, OPC_MODRM, OPT_EA )
		   /* pentium */

		   DEF_ASM_OP1(cmpxchg8b, 0x0fc7, 1, OPC_MODRM, OPT_EA )
		   /* AMD 64 */

		   DEF_ASM_OP1(cmpxchg16b, 0x0fc7, 1, OPC_MODRM | OPC_48, OPT_EA )
		   /* pentium pro */

		   ALT(DEF_ASM_OP2(cmovo, 0x0f40, 0, OPC_MODRM | OPC_TEST | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

		   DEF_ASM_OP2(fcmovb, 0xdac0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmove, 0xdac8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovbe, 0xdad0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovu, 0xdad8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovnb, 0xdbc0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovne, 0xdbc8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovnbe, 0xdbd0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcmovnu, 0xdbd8, 0, OPC_REG, OPT_ST, OPT_ST0 )

		   DEF_ASM_OP2(fucomi, 0xdbe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcomi, 0xdbf0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fucomip, 0xdfe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   DEF_ASM_OP2(fcomip, 0xdff0, 0, OPC_REG, OPT_ST, OPT_ST0 )
		   /* mmx */

		   DEF_ASM_OP0(emms, 0x0f77)/* must be last OP0 */

		   DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_MMXSSE )
		   /* movd shouldn't accept REG64, but AMD64 spec uses it for 32 and 64 bit
		          moves, so let's be compatible. */

		   ALT(DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG64, OPT_MMXSSE ))
		   ALT(DEF_ASM_OP2(movq, 0x0f6e, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_MMXSSE ))
		   ALT(DEF_ASM_OP2(movq, 0x0f6f, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_MMX ))
		   ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG32 ))
		   ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))
		   ALT(DEF_ASM_OP2(movq, 0x0f7f, 0, OPC_MODRM, OPT_MMX, OPT_EA | OPT_MMX ))
		   ALT(DEF_ASM_OP2(movq, 0x660fd6, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_SSE ))
		   ALT(DEF_ASM_OP2(movq, 0xf30f7e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE ))
		   ALT(DEF_ASM_OP2(movq, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))

		   DEF_ASM_OP2(packssdw, 0x0f6b, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(packsswb, 0x0f63, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(packuswb, 0x0f67, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddb, 0x0ffc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddw, 0x0ffd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddd, 0x0ffe, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddsb, 0x0fec, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddsw, 0x0fed, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddusb, 0x0fdc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(paddusw, 0x0fdd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pand, 0x0fdb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pandn, 0x0fdf, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpeqb, 0x0f74, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpeqw, 0x0f75, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpeqd, 0x0f76, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpgtb, 0x0f64, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpgtw, 0x0f65, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pcmpgtd, 0x0f66, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmaddwd, 0x0ff5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmulhw, 0x0fe5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmullw, 0x0fd5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(por, 0x0feb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psllw, 0x0ff1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psllw, 0x0f71, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(pslld, 0x0ff2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(pslld, 0x0f72, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psllq, 0x0ff3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psllq, 0x0f73, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psraw, 0x0fe1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psraw, 0x0f71, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrad, 0x0fe2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrad, 0x0f72, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrlw, 0x0fd1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrlw, 0x0f71, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrld, 0x0fd2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrld, 0x0f72, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psrlq, 0x0fd3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   ALT(DEF_ASM_OP2(psrlq, 0x0f73, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
		   DEF_ASM_OP2(psubb, 0x0ff8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubw, 0x0ff9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubd, 0x0ffa, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubsb, 0x0fe8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubsw, 0x0fe9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubusb, 0x0fd8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(psubusw, 0x0fd9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckhbw, 0x0f68, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckhwd, 0x0f69, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckhdq, 0x0f6a, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpcklbw, 0x0f60, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpcklwd, 0x0f61, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(punpckldq, 0x0f62, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pxor, 0x0fef, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   /* sse */

		   DEF_ASM_OP1(ldmxcsr, 0x0fae, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(stmxcsr, 0x0fae, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP2(movups, 0x0f10, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
		   ALT(DEF_ASM_OP2(movups, 0x0f11, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
		   DEF_ASM_OP2(movaps, 0x0f28, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
		   ALT(DEF_ASM_OP2(movaps, 0x0f29, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
		   DEF_ASM_OP2(movhps, 0x0f16, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
		   ALT(DEF_ASM_OP2(movhps, 0x0f17, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
		   DEF_ASM_OP2(addps, 0x0f58, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(cvtpi2ps, 0x0f2a, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_SSE )
		   DEF_ASM_OP2(cvtps2pi, 0x0f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
		   DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
		   ALT(DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
		   DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
		   ALT(DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
		   DEF_ASM_OP2(cvttps2pi, 0x0f2c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
		   DEF_ASM_OP2(andps, 0x0f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(divps, 0x0f5e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(maxps, 0x0f5f, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(minps, 0x0f5d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(mulps, 0x0f59, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(pavgb, 0x0fe0, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(pavgw, 0x0fe3, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(pmaxsw, 0x0fee, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pmaxub, 0x0fde, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pminsw, 0x0fea, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(pminub, 0x0fda, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
		   DEF_ASM_OP2(rcpss, 0x0f53, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(rsqrtps, 0x0f52, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(sqrtps, 0x0f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(sqrtss, 0xf30f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   DEF_ASM_OP2(subps, 0x0f5c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
		   /* sse2 */

		   DEF_ASM_OP2(andpd, 0x660f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
		   DEF_ASM_OP2(sqrtsd, 0xf20f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
		   /* movnti should only accept REG32 and REG64, we accept more */

		   DEF_ASM_OP2(movnti, 0x0fc3, 0, OPC_MODRM, OPT_REG, OPT_EA)
		   DEF_ASM_OP2(movntil, 0x0fc3, 0, OPC_MODRM, OPT_REG32, OPT_EA)
		   DEF_ASM_OP2(movntiq, 0x0fc3, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_EA)
		   DEF_ASM_OP1(prefetchnta, 0x0f18, 0, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetcht0, 0x0f18, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetcht1, 0x0f18, 2, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetcht2, 0x0f18, 3, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP1(prefetchw, 0x0f0d, 1, OPC_MODRM, OPT_EA)
		   DEF_ASM_OP0L(lfence, 0x0fae, 5, OPC_MODRM)
		   DEF_ASM_OP0L(mfence, 0x0fae, 6, OPC_MODRM)
		   DEF_ASM_OP0L(sfence, 0x0fae, 7, OPC_MODRM)
		   DEF_ASM_OP1(clflush, 0x0fae, 7, OPC_MODRM, OPT_EA)
		   /* Control-Flow Enforcement */

		   DEF_ASM_OP0L(endbr64, 0xf30f1e, 7, OPC_MODRM)
#undef ALT
#undef DEF_ASM_OP0
#undef DEF_ASM_OP0L
#undef DEF_ASM_OP1
#undef DEF_ASM_OP2
#undef DEF_ASM_OP3
#undef DEF
};
/* keywords: tok >= TOK_IDENT && tok < TOK_UIDENT */

#define TOK_UIDENT TOK_DEFINE
/* ------------ libtcc.c ------------ */

ST_DATA struct TCCState *tcc_state;
ST_DATA void **stk_data;
ST_DATA int nb_stk_data;
ST_DATA int g_debug;
/* public functions currently used by the tcc main function */

ST_FUNC char *pstrcpy(char *buf, size_t buf_size, const char *s);
ST_FUNC char *pstrcat(char *buf, size_t buf_size, const char *s);
ST_FUNC char *pstrncpy(char *out, size_t buf_size, const char *s, size_t num);
PUB_FUNC char *tcc_basename(const char *name);
PUB_FUNC char *tcc_fileextension (const char *name);
/* all allocations - even MEM_DEBUG - use these */

PUB_FUNC void tcc_free(void *ptr);
PUB_FUNC void *tcc_malloc(unsigned long size);
PUB_FUNC void *tcc_mallocz(unsigned long size);
PUB_FUNC void *tcc_realloc(void *ptr, unsigned long size);
PUB_FUNC char *tcc_strdup(const char *str);
ST_FUNC void libc_free(void *ptr);
#undef strdup
PUB_FUNC int _tcc_error_noabort(const char *fmt, ...) PRINTF_LIKE(1,2);
PUB_FUNC NORETURN void _tcc_error(const char *fmt, ...) PRINTF_LIKE(1,2);
PUB_FUNC void _tcc_warning(const char *fmt, ...) PRINTF_LIKE(1,2);

#define tcc_internal_error(msg) tcc_error("internal compiler error in %s:%d: %s", __FUNCTION__,__LINE__,msg)
/* other utilities */

ST_FUNC void dynarray_add(void *ptab, int *nb_ptr, void *data);
ST_FUNC void dynarray_reset(void *pp, int *n);
ST_INLN void cstr_ccat(CString *cstr, int ch);
ST_FUNC void cstr_cat(CString *cstr, const char *str, int len);
ST_FUNC void cstr_wccat(CString *cstr, int ch);
ST_FUNC void cstr_new(CString *cstr);
ST_FUNC void cstr_free(CString *cstr);
ST_FUNC int cstr_printf(CString *cs, const char *fmt, ...) PRINTF_LIKE(2,3);
ST_FUNC int cstr_vprintf(CString *cstr, const char *fmt, va_list ap);
ST_FUNC void cstr_reset(CString *cstr);
ST_FUNC void tcc_open_bf(TCCState *s1, const char *filename, int initlen);
ST_FUNC int tcc_open(TCCState *s1, const char *filename);
ST_FUNC void tcc_close(void);
/* mark a memory pointer on stack for cleanup after errors */

#define stk_push(p) dynarray_add(&stk_data, &nb_stk_data, p)
#define stk_pop() (--nb_stk_data)
/* mark CString on stack for cleanup errors */

#define cstr_new_s(cstr) (cstr_new(cstr), stk_push(&(cstr)->data))
#define cstr_free_s(cstr) (cstr_free(cstr), stk_pop())

ST_FUNC int tcc_add_file_internal(TCCState *s1, const char *filename,
				  int flags);
/* flags: */
/* print error if file not found */

#define AFF_PRINT_ERROR 0x10
/* file to add is binary */
#define AFF_TYPE_BIN 0x40
/* load all objects from archive */
#define AFF_WHOLE_ARCHIVE 0x80
/* s->filetype: */

#define AFF_TYPE_NONE 0
#define AFF_TYPE_C 1
#define AFF_TYPE_ASM 2
#define AFF_TYPE_ASMPP 4
#define AFF_TYPE_LIB 8
#define AFF_TYPE_MASK (7 | AFF_TYPE_BIN)
/* values from tcc_object_type(...) */

#define AFF_BINTYPE_REL 1
#define AFF_BINTYPE_DYN 2
#define AFF_BINTYPE_AR 3
/* return value of tcc_add_file_internal(): 0, -1, or FILE_NOT_FOUND */

#define FILE_NOT_FOUND -2
/* unrecognized file type */
#define FILE_NOT_RECOGNIZED -3

ST_FUNC int tcc_add_dll(TCCState *s, const char *filename, int flags);
ST_FUNC int tcc_add_support(TCCState *s1, const char *filename);

ST_FUNC void tcc_add_pragma_libs(TCCState *s1);
PUB_FUNC int tcc_add_library_err(TCCState *s, const char *f);
PUB_FUNC void tcc_print_stats(TCCState *s, unsigned total_time);
PUB_FUNC int tcc_parse_args(TCCState *s, int *argc, char ***argv);

ST_FUNC char *normalize_slashes(char *path);
PUB_FUNC FILE *tcc_fopen(const char *f, const char *m);
PUB_FUNC int tcc_fclose(FILE *f);

ST_FUNC DLLReference *tcc_add_dllref(TCCState *s1, const char *dllname,
				     int level);
ST_FUNC char *tcc_load_text(int fd);
/* for #pragma once */

ST_FUNC int normalized_PATHCMP(const char *f1, const char *f2);
/* tcc_parse_args return codes: */

#define OPT_HELP 1
#define OPT_HELP2 2
#define OPT_V 3
#define OPT_PRINT_DIRS 4
#define OPT_AR 5
#define OPT_IMPDEF 6
#define OPT_M32 32
#define OPT_M64 64
/* ------------ tccpp.c ------------ */

ST_DATA struct BufferedFile *file;
ST_DATA int tok;
ST_DATA CValue tokc;
ST_DATA const int *macro_ptr;
ST_DATA int parse_flags;
ST_DATA int tok_flags;
ST_DATA CString tokcstr;/* current parsed string, if any */

/* display benchmark infos */

ST_DATA int tok_ident;
ST_DATA TokenSym **table_ident;
ST_DATA int pp_expr;
/* beginning of line before */

#define TOK_FLAG_BOL 0x0001
/* beginning of file before */
#define TOK_FLAG_BOF 0x0002
/* a endif was found matching starting #ifdef */
#define TOK_FLAG_ENDIF 0x0004
/* activate preprocessing */

#define PARSE_FLAG_PREPROCESS 0x0001
/* return numbers instead of TOK_PPNUM */
#define PARSE_FLAG_TOK_NUM 0x0002
/* line feed is returned as a
                                        token. line feed is also
                                        returned at eof */

#define PARSE_FLAG_LINEFEED 0x0004
/* we processing an asm file: '#' can be used for line comment, etc. */
#define PARSE_FLAG_ASM_FILE 0x0008
/* next() returns space tokens (for -E) */
#define PARSE_FLAG_SPACES 0x0010
/* next() returns '\\' token */
#define PARSE_FLAG_ACCEPT_STRAYS 0x0020
/* return parsed strings instead of TOK_PPSTR */
#define PARSE_FLAG_TOK_STR 0x0040
/* isidnum_table flags: */

#define IS_SPC 1
#define IS_ID 2
#define IS_NUM 4

enum line_macro_output_format {
	LINE_MACRO_OUTPUT_FORMAT_GCC,
	LINE_MACRO_OUTPUT_FORMAT_NONE,
	LINE_MACRO_OUTPUT_FORMAT_STD,
	LINE_MACRO_OUTPUT_FORMAT_P10 = 11
};

ST_FUNC TokenSym *tok_alloc(const char *str, int len);
ST_FUNC int tok_alloc_const(const char *str);
ST_FUNC const char *get_tok_str(int v, CValue *cv);
ST_FUNC void begin_macro(TokenString *str, int alloc);
ST_FUNC void end_macro(void);
ST_FUNC int set_idnum(int c, int val);
ST_INLN void tok_str_new(TokenString *s);
ST_FUNC TokenString *tok_str_alloc(void);
ST_FUNC void tok_str_free(TokenString *s);
ST_FUNC void tok_str_free_str(int *str);
ST_FUNC void tok_str_add(TokenString *s, int t);
ST_FUNC void tok_str_add_tok(TokenString *s);
ST_INLN void define_push(int v, int macro_type, int *str, Sym *first_arg);
ST_FUNC void define_undef(Sym *s);
ST_INLN Sym *define_find(int v);
ST_FUNC void free_defines(Sym *b);
ST_FUNC void parse_define(void);
ST_FUNC void skip_to_eol(int warn);
ST_FUNC void preprocess(int is_bof);
ST_FUNC void next(void);
ST_INLN void unget_tok(int last_tok);
ST_FUNC void preprocess_start(TCCState *s1, int filetype);
ST_FUNC void preprocess_end(TCCState *s1);
ST_FUNC void tccpp_new(TCCState *s);
ST_FUNC void tccpp_delete(TCCState *s);
ST_FUNC void tccpp_putfile(const char *filename);
ST_FUNC int tcc_preprocess(TCCState *s1);
ST_FUNC void skip(int c);
ST_FUNC NORETURN void expect(const char *msg);
ST_FUNC void pp_error(CString *cs);
/* space excluding newline */

static inline int is_space(int ch)
{
	return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f' || ch == '\r';
}
static inline int isid(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static inline int isnum(int c)
{
	return c >= '0' && c <= '9';
}
static inline int isoct(int c)
{
	return c >= '0' && c <= '7';
}
static inline int toup(int c)
{
	return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}
/* ------------ tccgen.c ------------ */

#define SYM_POOL_NB (8192 / sizeof(Sym))

ST_DATA Sym *global_stack;
ST_DATA Sym *local_stack;
ST_DATA Sym *local_label_stack;
ST_DATA Sym *global_label_stack;
ST_DATA Sym *define_stack;
ST_DATA CType int_type, func_old_type, char_pointer_type;
ST_DATA SValue *vtop;
ST_DATA int rsym, anon_sym, ind, loc;
ST_DATA char debug_modes;

ST_DATA int
nocode_wanted;/* true if no code generation wanted for an expression */

ST_DATA int
global_expr;/* true if compound literals must be allocated globally (used during initializers parsing */

ST_DATA CType
func_vt;/* current function return type (used by return instruction) */

ST_DATA int func_var;/* true if current function is variadic */

ST_DATA int func_vc;/* stack address for implicit struct return storage */

ST_DATA int func_ind;/* function start address */

ST_DATA const char *funcname;

ST_FUNC void tccgen_init(TCCState *s1);
ST_FUNC int tccgen_compile(TCCState *s1);
ST_FUNC void tccgen_finish(TCCState *s1);
ST_FUNC void check_vstack(void);

ST_INLN int is_float(int t);
ST_FUNC int ieee_finite(double d);
ST_FUNC int exact_log2p1(int i);
ST_FUNC void test_lvalue(void);

ST_FUNC ElfSym *elfsym(Sym *);
ST_FUNC void update_storage(Sym *sym);
ST_FUNC void put_extern_sym2(Sym *sym, int sh_num, addr_t value,
			     unsigned long size, int can_add_underscore);
ST_FUNC void put_extern_sym(Sym *sym, Section *section, addr_t value,
			    unsigned long size);

ST_FUNC void greloca(Section *s, Sym *sym, unsigned long offset, int type,
		     addr_t addend);

ST_INLN void sym_free(Sym *sym);
ST_FUNC Sym *sym_push(int v, CType *type, int r, int c);
ST_FUNC void sym_pop(Sym **ptop, Sym *b, int keep);
ST_FUNC Sym *sym_push2(Sym **ps, int v, int t, int c);
ST_FUNC Sym *sym_find2(Sym *s, int v);
ST_INLN Sym *sym_find(int v);
ST_FUNC Sym *label_find(int v);
ST_FUNC Sym *label_push(Sym **ptop, int v, int flags);
ST_FUNC void label_pop(Sym **ptop, Sym *slast, int keep);
ST_INLN Sym *struct_find(int v);

ST_FUNC Sym *global_identifier_push(int v, int t, int c);
ST_FUNC Sym *external_global_sym(int v, CType *type);
ST_FUNC Sym *external_helper_sym(int v);
ST_FUNC void vpush_helper_func(int v);
ST_FUNC void vset(CType *type, int r, int v);
ST_FUNC void vset_VT_CMP(int op);
ST_FUNC void vpushi(int v);
ST_FUNC void vpushv(SValue *v);
ST_FUNC void vpushsym(CType *type, Sym *sym);
ST_FUNC void vswap(void);
ST_FUNC void vrott(int n);
ST_FUNC void vrotb(int n);
ST_FUNC void vrev(int n);
ST_FUNC void vpop(void);

ST_FUNC void save_reg(int r);
ST_FUNC void save_reg_upstack(int r, int n);
ST_FUNC int get_reg(int rc);
ST_FUNC void save_regs(int n);
ST_FUNC void gaddrof(void);
ST_FUNC int gv(int rc);
ST_FUNC void gv2(int rc1, int rc2);
ST_FUNC void gen_op(int op);
ST_FUNC int type_size(CType *type, int *a);
ST_FUNC void mk_pointer(CType *type);
ST_FUNC void vstore(void);
ST_FUNC void inc(int post, int c);
ST_FUNC CString *parse_mult_str(const char *msg);
ST_FUNC CString *parse_asm_str(void);
ST_FUNC void indir(void);
ST_FUNC void unary(void);
ST_FUNC void gexpr(void);
ST_FUNC int expr_const(void);

ST_FUNC Sym *gfunc_set_param(Sym *s, int c, int byref);
/* ------------ tccelf.c ------------ */
/* default output format: ELF */

#define TCC_OUTPUT_FORMAT_ELF 0
/* binary image output */
#define TCC_OUTPUT_FORMAT_BINARY 1
#define TCC_OUTPUT_DYN TCC_OUTPUT_DLL
/* For COFF and a.out archives */

#define ARMAG "!<arch>\n"

typedef struct {
	unsigned int n_strx;/* index into string table of name */

	unsigned char n_type;/* type of symbol */

	unsigned char n_other;/* misc info (usually empty) */

	unsigned short n_desc;/* description field */

	unsigned int n_value;/* value of symbol */

} Stab_Sym;

ST_FUNC void tccelf_new(TCCState *s);
ST_FUNC void tccelf_delete(TCCState *s);
ST_FUNC void tccelf_begin_file(TCCState *s1);
ST_FUNC void tccelf_end_file(TCCState *s1);
ST_FUNC Section *new_section(TCCState *s1, const char *name, int sh_type,
			     int sh_flags);
ST_FUNC void section_realloc(Section *sec, unsigned long new_size);
ST_FUNC size_t section_add(Section *sec, addr_t size, int align);
ST_FUNC void *section_ptr_add(Section *sec, addr_t size);
ST_FUNC Section *find_section(TCCState *s1, const char *name);
ST_FUNC void free_section(Section *s);
ST_FUNC Section *new_symtab(TCCState *s1, const char *symtab_name, int sh_type,
			    int sh_flags, const char *strtab_name, const char *hash_name,
			    int hash_sh_flags);
ST_FUNC void init_symtab(Section *s);

ST_FUNC int put_elf_str(Section *s, const char *sym);
ST_FUNC int put_elf_sym(Section *s, addr_t value, unsigned long size, int info,
			int other, int shndx, const char *name);
ST_FUNC int set_elf_sym(Section *s, addr_t value, unsigned long size, int info,
			int other, int shndx, const char *name);
ST_FUNC int find_elf_sym(Section *s, const char *name);
ST_FUNC void put_elf_reloc(Section *symtab, Section *s, unsigned long offset,
			   int type, int symbol);
ST_FUNC void put_elf_reloca(Section *symtab, Section *s, unsigned long offset,
			    int type, int symbol, addr_t addend);

ST_FUNC void resolve_common_syms(TCCState *s1);
ST_FUNC void relocate_syms(TCCState *s1, Section *symtab, int do_resolve);
ST_FUNC void relocate_sections(TCCState *s1);

ST_FUNC ssize_t full_read(int fd, void *buf, size_t count);
ST_FUNC void *load_data(int fd, unsigned long file_offset, unsigned long size);
ST_FUNC int tcc_object_type(int fd, ElfW(Ehdr) *h);
ST_FUNC int tcc_load_object_file(TCCState *s1, int fd,
				 unsigned long file_offset);
ST_FUNC int tcc_load_archive(TCCState *s1, int fd, int alacarte);
ST_FUNC void add_array(TCCState *s1, const char *sec, int c);

ST_FUNC struct sym_attr *get_sym_attr(TCCState *s1, int index, int alloc);
ST_FUNC addr_t get_sym_addr(TCCState *s, const char *name, int err, int forc);
ST_FUNC void list_elf_symbols(TCCState *s, void *ctx,
			      void (*symbol_cb)(void *ctx, const char *name, const void *val));
ST_FUNC int set_global_sym(TCCState *s1, const char *name, Section *sec,
			   addr_t offs);
/* Browse each elem of type <type> in section <sec> starting at elem <startoff>
   using variable <elem> */

#define for_each_elem(sec,startoff,elem,type) for (elem = (type *) sec->data + startoff; elem < (type *) (sec->data + sec->data_offset); elem++)
/* ------------ xxx-link.c ------------ */

ST_FUNC void relocate(TCCState *s1, ElfW_Rel *rel, int type, unsigned char *ptr,
		      addr_t addr, addr_t val);
/* ------------ xxx-gen.c ------------ */

ST_DATA const char *const target_machine_defs;
ST_DATA const int reg_classes[NB_REGS];

ST_FUNC void gsym_addr(int t, int a);
ST_FUNC void gsym(int t);
ST_FUNC void load(int r, SValue *sv);
ST_FUNC void store(int r, SValue *v);
ST_FUNC int gfunc_sret(CType *vt, int variadic, CType *ret, int *align,
		       int *regsize);
ST_FUNC void gfunc_call(int nb_args);
ST_FUNC void gfunc_prolog(Sym *func_sym);
ST_FUNC void gfunc_epilog(void);
ST_FUNC void gen_fill_nops(int);
ST_FUNC int gjmp(int t);
ST_FUNC void gjmp_addr(int a);
ST_FUNC int gjmp_cond(int op, int t);
ST_FUNC int gjmp_append(int n, int t);
ST_FUNC void gen_opi(int op);
ST_FUNC void gen_opf(int op);
ST_FUNC void gen_cvt_ftoi(int t);
ST_FUNC void gen_cvt_itof(int t);
ST_FUNC void gen_cvt_ftof(int t);
ST_FUNC void ggoto(void);

ST_FUNC void o(unsigned int c);

ST_FUNC void gen_vla_sp_save(int addr);
ST_FUNC void gen_vla_sp_restore(int addr);
ST_FUNC void gen_vla_alloc(CType *type, int align);

static inline uint16_t read16le(unsigned char *p)
{
	return p[0] | (uint16_t)p[1] << 8;
}
static inline void write16le(unsigned char *p, uint16_t x)
{
	p[0] = x & 255;
	p[1] = x >> 8 & 255;
}
static inline uint32_t read32le(unsigned char *p)
{
	return read16le(p) | (uint32_t)read16le(p + 2) << 16;
}
static inline void write32le(unsigned char *p, uint32_t x)
{
	write16le(p, x);
	write16le(p + 2, x >> 16);
}
static inline void add32le(unsigned char *p, int32_t x)
{
	write32le(p, read32le(p) + x);
}
static inline uint64_t read64le(unsigned char *p)
{
	return read32le(p) | (uint64_t)read32le(p + 4) << 32;
}
static inline void write64le(unsigned char *p, uint64_t x)
{
	write32le(p, x);
	write32le(p + 4, x >> 32);
}
static inline void add64le(unsigned char *p, int64_t x)
{
	write64le(p, read64le(p) + x);
}
/* ------------ i386-gen.c ------------ */

ST_FUNC void g(int c);
ST_FUNC void gen_le16(int c);
ST_FUNC void gen_le32(int c);

ST_FUNC void gen_addr32(int r, Sym *sym, int c);
ST_FUNC void gen_addrpc32(int r, Sym *sym, int c);
ST_FUNC void gen_cvt_csti(int t);
ST_FUNC void gen_increment_tcov (SValue *sv);
/* ------------ x86_64-gen.c ------------ */

ST_FUNC void gen_opl(int op);

ST_FUNC void gen_vla_result(int addr);

ST_FUNC void gen_cvt_sxtw(void);
ST_FUNC void gen_cvt_csti(int t);
/* ------------ arm-gen.c ------------ */
/* ------------ arm64-gen.c ------------ */
/* ------------ riscv64-gen.c ------------ */
/* ------------ c67-gen.c ------------ */
/* ------------ tcccoff.c ------------ */
/* ------------ tccasm.c ------------ */
ST_FUNC void asm_instr(void);
ST_FUNC void asm_global_instr(void);
ST_FUNC int tcc_assemble(TCCState *s1, int do_preprocess);

ST_FUNC int find_constraint(ASMOperand *operands, int nb_operands,
			    const char *name, const char **pp);
ST_FUNC Sym *get_asm_sym(int name, Sym *csym);
ST_FUNC void asm_expr(TCCState *s1, ExprValue *pe);
ST_FUNC int asm_int_expr(TCCState *s1);

ST_FUNC void gen_expr64(ExprValue *pe);
/* ------------ i386-asm.c ------------ */

ST_FUNC void gen_expr32(ExprValue *pe);
ST_FUNC void asm_opcode(TCCState *s1, int opcode);
ST_FUNC int asm_parse_regvar(int t);
ST_FUNC void asm_compute_constraints(ASMOperand *operands, int nb_operands,
				     int nb_outputs, const uint8_t *clobber_regs, int *pout_reg);
ST_FUNC void subst_asm_operand(CString *add_str, SValue *sv, int modifier);
ST_FUNC void asm_gen_code(ASMOperand *operands, int nb_operands, int nb_outputs,
			  int is_output, uint8_t *clobber_regs, int out_reg);
ST_FUNC void asm_clobber(uint8_t *clobber_regs, const char *str);
/* ------------ tccpe.c -------------- */

ST_FUNC int pe_load_file(struct TCCState *s1, int fd, const char *filename);
ST_FUNC int pe_output_file(TCCState * s1, const char *filename);
ST_FUNC int pe_putimport(TCCState *s1, int dllindex, const char *name,
			 addr_t value);
ST_FUNC int pe_setsubsy(TCCState *s1, const char *arg);
ST_FUNC void pe_add_unwind_data(unsigned start, unsigned end, unsigned stack);
PUB_FUNC int tcc_get_dllexports(const char *filename, char **pp);
/* symbol properties stored in Elf32_Sym->st_other */

#define ST_PE_EXPORT 0x10
#define ST_PE_IMPORT 0x20
#define ST_PE_STDCALL 0x40
#define ST_ASM_SET 0x04
/* ------------ tccmacho.c ----------------- */
/* ------------ tccrun.c ----------------- */
ST_FUNC void tcc_run_free(TCCState *s1);
/* ------------ tcctools.c ----------------- */
/* ------------ tccdbg.c ------------ */
ST_FUNC void tcc_debug_new(TCCState *s);

ST_FUNC void tcc_debug_start(TCCState *s1);
ST_FUNC void tcc_debug_end(TCCState *s1);
ST_FUNC void tcc_debug_bincl(TCCState *s1);
ST_FUNC void tcc_debug_eincl(TCCState *s1);
ST_FUNC void tcc_debug_newfile(TCCState *s1);

ST_FUNC void tcc_debug_line(TCCState *s1);
ST_FUNC void tcc_add_debug_info(TCCState *s1, Sym *s, Sym *e);
ST_FUNC void tcc_debug_funcstart(TCCState *s1, Sym *sym);
ST_FUNC void tcc_debug_prolog_epilog(TCCState *s1, int value);
ST_FUNC void tcc_debug_funcend(TCCState *s1, int size);
ST_FUNC void tcc_debug_extern_sym(TCCState *s1, Sym *sym, int sh_num,
				  int sym_bind, int sym_type);
ST_FUNC void tcc_debug_typedef(TCCState *s1, Sym *sym);
ST_FUNC void tcc_debug_stabn(TCCState *s1, int type, int value);
ST_FUNC void tcc_debug_fix_forw(TCCState *s1, CType *t);

ST_FUNC void tcc_tcov_start(TCCState *s1);
ST_FUNC void tcc_tcov_end(TCCState *s1);
ST_FUNC void tcc_tcov_check_line(TCCState *s1, int start);
ST_FUNC void tcc_tcov_block_end(TCCState *s1, int line);
ST_FUNC void tcc_tcov_block_begin(TCCState *s1);
ST_FUNC void tcc_tcov_reset_ind(TCCState *s1);

#define stab_section s1->stab_section
#define tcov_section s1->tcov_section
#define eh_frame_section s1->eh_frame_section
#define dwarf_info_section s1->dwarf_info_section
#define dwarf_abbrev_section s1->dwarf_abbrev_section
#define dwarf_line_section s1->dwarf_line_section
#define dwarf_aranges_section s1->dwarf_aranges_section
#define dwarf_str_section s1->dwarf_str_section
#define dwarf_line_str_section s1->dwarf_line_str_section

#define DWARF_MAX_128 ((8 * sizeof (int64_t) + 6) / 7)

#define dwarf_read_1(ln,end) ((ln) < (end) ? *(ln)++ : 0)
static inline uint64_t
dwarf_read_uleb128(unsigned char **ln, unsigned char *end)
{
	unsigned char *cp = *ln;
	uint64_t retval = 0;
	int i;
	for (i = 0; i < DWARF_MAX_128; i++) {
		uint64_t byte = dwarf_read_1(cp, end);
		retval |= (byte & 0x7f) << (i * 7);
		if ((byte & 0x80) == 0)
			break;
	}
	*ln = cp;
	return retval;
}
static inline int64_t
dwarf_read_sleb128(unsigned char **ln, unsigned char *end)
{
	unsigned char *cp = *ln;
	int64_t retval = 0;
	int i;
	for (i = 0; i < DWARF_MAX_128; i++) {
		uint64_t byte = dwarf_read_1(cp, end);
		retval |= (byte & 0x7f) << (i * 7);
		if ((byte & 0x80) == 0) {
			if ((byte & 0x40) && (i + 1) * 7 < 64)
				retval |= (uint64_t)-1LL << ((i + 1) * 7);
			break;
		}
	}
	*ln = cp;
	return retval;
}
/* default dwarf version for "-gdwarf" */

#define DEFAULT_DWARF_VERSION 5
/* default dwarf version for "-g". use 0 to emit stab debug infos */
#ifndef CONFIG_DWARF_VERSION

#define CONFIG_DWARF_VERSION 0
#endif

#define R_DATA_32DW R_X86_64_32
/**/
typedef struct {
	volatile LONG init;
	CRITICAL_SECTION cs;
} TCCSem;
static inline void wait_sem(TCCSem *p)
{
	if (InterlockedCompareExchange(&p->init, 1, 0) == 0) {
		InitializeCriticalSection(&p->cs);
		InterlockedExchange(&p->init, 2);
	} else {
		while (InterlockedCompareExchange(&p->init, 2, 2) != 2)
			Sleep(0);
	}
	EnterCriticalSection(&p->cs);
}
static inline void post_sem(TCCSem *p)
{
	LeaveCriticalSection(&p->cs);
}

#define TCC_SEM(s) TCCSem s
#define WAIT_SEM wait_sem
#define POST_SEM post_sem
/**/

#undef ST_DATA

#define ST_DATA static
/**/

#define text_section TCC_STATE_VAR(text_section)
#define data_section TCC_STATE_VAR(data_section)
#define rodata_section TCC_STATE_VAR(rodata_section)
#define bss_section TCC_STATE_VAR(bss_section)
#define common_section TCC_STATE_VAR(common_section)
#define cur_text_section TCC_STATE_VAR(cur_text_section)
#define symtab_section TCC_STATE_VAR(symtab_section)
#define gnu_ext TCC_STATE_VAR(gnu_ext)
#define tcc_error_noabort TCC_SET_STATE(_tcc_error_noabort)
#define tcc_error TCC_SET_STATE(_tcc_error)
#define tcc_warning TCC_SET_STATE(_tcc_warning)

#define total_idents TCC_STATE_VAR(total_idents)
#define total_lines TCC_STATE_VAR(total_lines)
#define total_bytes TCC_STATE_VAR(total_bytes)

PUB_FUNC void tcc_enter_state(TCCState *s1);
PUB_FUNC void tcc_exit_state(TCCState *s1);
/* conditional warning depending on switch */

#define tcc_warning_c(sw) TCC_SET_STATE(( tcc_state->warn_num = offsetof(TCCState, sw) - offsetof(TCCState, warn_none), _tcc_warning))
/**/
/* _TCC_H */

#undef TCC_STATE_VAR
#undef TCC_SET_STATE

/* ==================== libtcc.c ==================== */

/* ==================== tccpp.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) tcc_state->sym
#define TCC_SET_STATE(fn) fn
#undef _tcc_error
/* #define to 1 to enable (see parse_pp_string()) */

#define ACCEPT_LF_IN_STRINGS 0
/**/
/* global variables */

ST_DATA int tok_flags;
ST_DATA int parse_flags;

ST_DATA struct BufferedFile *file;
ST_DATA int tok;
ST_DATA CValue tokc;
ST_DATA const int *macro_ptr;
ST_DATA CString tokcstr;/* current parsed string, if any */

/* display benchmark infos */

ST_DATA int tok_ident;
ST_DATA TokenSym **table_ident;
ST_DATA int pp_expr;
/* ------------------------------------------------------------------------- */

static TokenSym *hash_ident[TOK_HASH_SIZE];
static char token_buf[STRING_MAX_SIZE + 1];
static CString cstr_buf;
static TokenString tokstr_buf;
static TokenString unget_buf;
static unsigned char isidnum_table[256 - CH_EOF];
static int pp_debug_tok, pp_debug_symv;
static int pp_counter;
static void tok_print(const int *str, const char *msg, ...);
static void next_nomacro(void);
static void parse_number(const char *p);
static void parse_string(const char *p, int len);

static struct TinyAlloc *toksym_alloc;
static struct TinyAlloc *tokstr_alloc;

static TokenString *macro_stack;

static const char tcc_keywords[] =
#define DEF(id,str) str "\0"
	/**/
	/* keywords */

	DEF(TOK_IF, "if")
	DEF(TOK_ELSE, "else")
	DEF(TOK_WHILE, "while")
	DEF(TOK_FOR, "for")
	DEF(TOK_DO, "do")
	DEF(TOK_CONTINUE, "continue")
	DEF(TOK_BREAK, "break")
	DEF(TOK_RETURN, "return")
	DEF(TOK_GOTO, "goto")
	DEF(TOK_SWITCH, "switch")
	DEF(TOK_CASE, "case")
	DEF(TOK_DEFAULT, "default")
	DEF(TOK_ASM1, "asm")
	DEF(TOK_ASM2, "__asm")
	DEF(TOK_ASM3, "__asm__")

	DEF(TOK_EXTERN, "extern")
	DEF(TOK_STATIC, "static")
	DEF(TOK_UNSIGNED, "unsigned")
	DEF(TOK__Atomic, "_Atomic")
	DEF(TOK_CONST1, "const")
	DEF(TOK_CONST2, "__const")/* gcc keyword */

	DEF(TOK_CONST3, "__const__")/* gcc keyword */

	DEF(TOK_VOLATILE1, "volatile")
	DEF(TOK_VOLATILE2, "__volatile")/* gcc keyword */

	DEF(TOK_VOLATILE3, "__volatile__")/* gcc keyword */

	DEF(TOK_REGISTER, "register")
	DEF(TOK_SIGNED1, "signed")
	DEF(TOK_SIGNED2, "__signed")/* gcc keyword */

	DEF(TOK_SIGNED3, "__signed__")/* gcc keyword */

	DEF(TOK_AUTO, "auto")
	DEF(TOK_INLINE1, "inline")
	DEF(TOK_INLINE2, "__inline")/* gcc keyword */

	DEF(TOK_INLINE3, "__inline__")/* gcc keyword */

	DEF(TOK_RESTRICT1, "restrict")
	DEF(TOK_RESTRICT2, "__restrict")
	DEF(TOK_RESTRICT3, "__restrict__")
	DEF(TOK_EXTENSION, "__extension__")/* gcc keyword */

	DEF(TOK_THREAD_LOCAL, "_Thread_local")/* C11 thread-local storage */

	DEF(TOK___thread, "__thread")/* GCC thread-local storage extension */

	DEF(TOK_GENERIC, "_Generic")
	DEF(TOK_STATIC_ASSERT, "_Static_assert")

	DEF(TOK_VOID, "void")
	DEF(TOK_CHAR, "char")
	DEF(TOK_INT, "int")
	DEF(TOK_FLOAT, "float")
	DEF(TOK_DOUBLE, "double")
	DEF(TOK_BOOL, "_Bool")
	DEF(TOK_COMPLEX, "_Complex")
	DEF(TOK_SHORT, "short")
	DEF(TOK_LONG, "long")
	DEF(TOK_STRUCT, "struct")
	DEF(TOK_UNION, "union")
	DEF(TOK_TYPEDEF, "typedef")
	DEF(TOK_ENUM, "enum")
	DEF(TOK_SIZEOF, "sizeof")
	DEF(TOK_ATTRIBUTE1, "__attribute")
	DEF(TOK_ATTRIBUTE2, "__attribute__")
	DEF(TOK_ALIGNOF1, "__alignof")
	DEF(TOK_ALIGNOF2, "__alignof__")
	DEF(TOK_ALIGNOF3, "_Alignof")
	DEF(TOK_ALIGNAS, "_Alignas")
	DEF(TOK_TYPEOF1, "typeof")
	DEF(TOK_TYPEOF2, "__typeof")
	DEF(TOK_TYPEOF3, "__typeof__")
	DEF(TOK_LABEL, "__label__")
	/**/
	/* the following are not keywords. They are included to ease parsing */
	/* preprocessor only */

	DEF(TOK_DEFINE, "define")
	DEF(TOK_INCLUDE, "include")
	DEF(TOK_INCLUDE_NEXT, "include_next")
	DEF(TOK_IFDEF, "ifdef")
	DEF(TOK_IFNDEF, "ifndef")
	DEF(TOK_ELIF, "elif")
	DEF(TOK_ENDIF, "endif")
	DEF(TOK_DEFINED, "defined")
	DEF(TOK_UNDEF, "undef")
	DEF(TOK_ERROR, "error")
	DEF(TOK_WARNING, "warning")
	DEF(TOK_LINE, "line")
	DEF(TOK_PRAGMA, "pragma")
	DEF(TOK___LINE__, "__LINE__")
	DEF(TOK___FILE__, "__FILE__")
	DEF(TOK___DATE__, "__DATE__")
	DEF(TOK___TIME__, "__TIME__")
	DEF(TOK___FUNCTION__, "__FUNCTION__")
	DEF(TOK___VA_ARGS__, "__VA_ARGS__")
	DEF(TOK___COUNTER__, "__COUNTER__")
	DEF(TOK___HAS_INCLUDE, "__has_include")
	DEF(TOK___HAS_INCLUDE_NEXT, "__has_include_next")
	/* special identifiers */

	DEF(TOK___FUNC__, "__func__")
	/* special floating point values */

	DEF(TOK___NAN__, "__nan__")
	DEF(TOK___SNAN__, "__snan__")
	DEF(TOK___INF__, "__inf__")
	/* attribute identifiers */
	/* XXX: handle all tokens generically since speed is not critical */

	DEF(TOK_SECTION1, "section")
	DEF(TOK_SECTION2, "__section__")
	DEF(TOK_ALIGNED1, "aligned")
	DEF(TOK_ALIGNED2, "__aligned__")
	DEF(TOK_PACKED1, "packed")
	DEF(TOK_PACKED2, "__packed__")
	DEF(TOK_WEAK1, "weak")
	DEF(TOK_WEAK2, "__weak__")
	DEF(TOK_ALIAS1, "alias")
	DEF(TOK_ALIAS2, "__alias__")
	DEF(TOK_USED1, "used")
	DEF(TOK_USED2, "__used__")
	DEF(TOK_UNUSED1, "unused")
	DEF(TOK_UNUSED2, "__unused__")
	DEF(TOK_FORMAT1, "format")
	DEF(TOK_FORMAT2, "__format__")
	DEF(TOK_NODEBUG1, "nodebug")
	DEF(TOK_NODEBUG2, "__nodebug__")
	DEF(TOK_CDECL1, "cdecl")
	DEF(TOK_CDECL2, "__cdecl")
	DEF(TOK_CDECL3, "__cdecl__")
	DEF(TOK_STDCALL1, "stdcall")
	DEF(TOK_STDCALL2, "__stdcall")
	DEF(TOK_STDCALL3, "__stdcall__")
	DEF(TOK_FASTCALL1, "fastcall")
	DEF(TOK_FASTCALL2, "__fastcall")
	DEF(TOK_FASTCALL3, "__fastcall__")
	DEF(TOK_THISCALL1, "thiscall")
	DEF(TOK_THISCALL2, "__thiscall")
	DEF(TOK_THISCALL3, "__thiscall__")
	DEF(TOK_REGPARM1, "regparm")
	DEF(TOK_REGPARM2, "__regparm__")
	DEF(TOK_CLEANUP1, "cleanup")
	DEF(TOK_CLEANUP2, "__cleanup__")
	DEF(TOK_CONSTRUCTOR1, "constructor")
	DEF(TOK_CONSTRUCTOR2, "__constructor__")
	DEF(TOK_DESTRUCTOR1, "destructor")
	DEF(TOK_DESTRUCTOR2, "__destructor__")
	DEF(TOK_ALWAYS_INLINE1, "always_inline")
	DEF(TOK_ALWAYS_INLINE2, "__always_inline__")
	DEF(TOK_NOINLINE, "__noinline__")
	DEF(TOK_PURE1, "pure")
	DEF(TOK_PURE2, "__pure__")

	DEF(TOK_MODE, "__mode__")
	DEF(TOK_MODE_QI, "__QI__")
	DEF(TOK_MODE_DI, "__DI__")
	DEF(TOK_MODE_HI, "__HI__")
	DEF(TOK_MODE_SI, "__SI__")
	DEF(TOK_MODE_word, "__word__")

	DEF(TOK_DLLEXPORT, "dllexport")
	DEF(TOK_DLLIMPORT, "dllimport")
	DEF(TOK_NODECORATE, "nodecorate")
	DEF(TOK_NORETURN1, "noreturn")
	DEF(TOK_NORETURN2, "__noreturn__")
	DEF(TOK_NORETURN3, "_Noreturn")
	DEF(TOK_VISIBILITY1, "visibility")
	DEF(TOK_VISIBILITY2, "__visibility__")

	DEF(TOK_builtin_types_compatible_p, "__builtin_types_compatible_p")
	DEF(TOK_builtin_choose_expr, "__builtin_choose_expr")
	DEF(TOK_builtin_constant_p, "__builtin_constant_p")
	DEF(TOK_builtin_frame_address, "__builtin_frame_address")
	DEF(TOK_builtin_return_address, "__builtin_return_address")
	DEF(TOK_builtin_expect, "__builtin_expect")
	DEF(TOK_builtin_unreachable, "__builtin_unreachable")
	/*DEF(TOK_builtin_va_list, "__builtin_va_list")*/
	DEF(TOK_builtin_va_start, "__builtin_va_start")
	/* atomic operations */
#define DEF_ATOMIC(ID) DEF(TOK_ ## __ ## ID, "__"#ID)
	DEF_ATOMIC(atomic_store)
	DEF_ATOMIC(atomic_load)
	DEF_ATOMIC(atomic_exchange)
	DEF_ATOMIC(atomic_compare_exchange)
	DEF_ATOMIC(atomic_fetch_add)
	DEF_ATOMIC(atomic_fetch_sub)
	DEF_ATOMIC(atomic_fetch_or)
	DEF_ATOMIC(atomic_fetch_xor)
	DEF_ATOMIC(atomic_fetch_and)
	DEF_ATOMIC(atomic_fetch_nand)
	DEF_ATOMIC(atomic_add_fetch)
	DEF_ATOMIC(atomic_sub_fetch)
	DEF_ATOMIC(atomic_or_fetch)
	DEF_ATOMIC(atomic_xor_fetch)
	DEF_ATOMIC(atomic_and_fetch)
	DEF_ATOMIC(atomic_nand_fetch)
	/* pragma */

	DEF(TOK_pack, "pack")

	DEF(TOK_comment, "comment")
	DEF(TOK_lib, "lib")
	DEF(TOK_push_macro, "push_macro")
	DEF(TOK_pop_macro, "pop_macro")
	DEF(TOK_once, "once")
	DEF(TOK_option, "option")
	/* builtin functions or variables */

	DEF(TOK_memcpy, "memcpy")
	DEF(TOK_memmove, "memmove")
	DEF(TOK_memset, "memset")
	DEF(TOK___divdi3, "__divdi3")
	DEF(TOK___moddi3, "__moddi3")
	DEF(TOK___udivdi3, "__udivdi3")
	DEF(TOK___umoddi3, "__umoddi3")
	DEF(TOK___ashrdi3, "__ashrdi3")
	DEF(TOK___lshrdi3, "__lshrdi3")
	DEF(TOK___ashldi3, "__ashldi3")
	DEF(TOK___floatundisf, "__floatundisf")
	DEF(TOK___floatundidf, "__floatundidf")

	DEF(TOK___floatundixf, "__floatundixf")
	DEF(TOK___fixunsxfdi, "__fixunsxfdi")

	DEF(TOK___fixunssfdi, "__fixunssfdi")
	DEF(TOK___fixunsdfdi, "__fixunsdfdi")
	DEF(TOK___fixxfdi, "__fixxfdi")

	DEF(TOK_alloca, "alloca")
	DEF(TOK___chkstk, "__chkstk")
	DEF(TOK___tls_index, "__tls_index")
	/* bound checking symbols */
	/**/
	/* Tiny Assembler */
#define DEF_ASM(x) DEF(TOK_ASM_ ## x, #x)
#define DEF_ASMDIR(x) DEF(TOK_ASMDIR_ ## x, "." #x)
#define TOK_ASM_int TOK_INT

#define TOK_ASMDIR_FIRST TOK_ASMDIR_byte
#define TOK_ASMDIR_LAST TOK_ASMDIR_section

	DEF_ASMDIR(byte)/* must be first directive */

	DEF_ASMDIR(word)
	DEF_ASMDIR(align)
	DEF_ASMDIR(balign)
	DEF_ASMDIR(p2align)
	DEF_ASMDIR(set)
	DEF_ASMDIR(skip)
	DEF_ASMDIR(space)
	DEF_ASMDIR(string)
	DEF_ASMDIR(asciz)
	DEF_ASMDIR(ascii)
	DEF_ASMDIR(file)
	DEF_ASMDIR(globl)
	DEF_ASMDIR(global)
	DEF_ASMDIR(weak)
	DEF_ASMDIR(hidden)
	DEF_ASMDIR(ident)
	DEF_ASMDIR(size)
	DEF_ASMDIR(type)
	DEF_ASMDIR(text)
	DEF_ASMDIR(data)
	DEF_ASMDIR(bss)
	DEF_ASMDIR(previous)
	DEF_ASMDIR(pushsection)
	DEF_ASMDIR(popsection)
	DEF_ASMDIR(fill)
	DEF_ASMDIR(rept)
	DEF_ASMDIR(endr)
	DEF_ASMDIR(org)
	DEF_ASMDIR(quad)

	DEF_ASMDIR(code64)

	DEF_ASMDIR(short)
	DEF_ASMDIR(long)
	DEF_ASMDIR(int)
	DEF_ASMDIR(symver)
	DEF_ASMDIR(reloc)
	DEF_ASMDIR(section)/* must be last directive */

	/* ------------------------------------------------------------------ */
	/* WARNING: relative order of tokens is important. */

#define DEF_BWL(x) DEF(TOK_ASM_ ## x ## b, #x "b") DEF(TOK_ASM_ ## x ## w, #x "w") DEF(TOK_ASM_ ## x ## l, #x "l") DEF(TOK_ASM_ ## x, #x)

#define DEF_BWLQ(x) DEF(TOK_ASM_ ## x ## b, #x "b") DEF(TOK_ASM_ ## x ## w, #x "w") DEF(TOK_ASM_ ## x ## l, #x "l") DEF(TOK_ASM_ ## x ## q, #x "q") DEF(TOK_ASM_ ## x, #x)

#define DEF_WLQ(x) DEF(TOK_ASM_ ## x ## w, #x "w") DEF(TOK_ASM_ ## x ## l, #x "l") DEF(TOK_ASM_ ## x ## q, #x "q") DEF(TOK_ASM_ ## x, #x)
#define DEF_BWLX DEF_BWLQ
#define DEF_WLX DEF_WLQ
	/* number of sizes + 1 */

#define NBWLX 5
#define DEF_FP1(x) DEF(TOK_ASM_ ## f ## x ## s, "f" #x "s") DEF(TOK_ASM_ ## fi ## x ## l, "fi" #x "l") DEF(TOK_ASM_ ## f ## x ## l, "f" #x "l") DEF(TOK_ASM_ ## fi ## x ## s, "fi" #x "s")

#define DEF_FP(x) DEF(TOK_ASM_ ## f ## x, "f" #x ) DEF(TOK_ASM_ ## f ## x ## p, "f" #x "p") DEF_FP1(x)
#define DEF_ASMTEST(x,suffix) DEF_ASM(x ## o ## suffix) DEF_ASM(x ## no ## suffix) DEF_ASM(x ## b ## suffix) DEF_ASM(x ## c ## suffix) DEF_ASM(x ## nae ## suffix) DEF_ASM(x ## nb ## suffix) DEF_ASM(x ## nc ## suffix) DEF_ASM(x ## ae ## suffix) DEF_ASM(x ## e ## suffix) DEF_ASM(x ## z ## suffix) DEF_ASM(x ## ne ## suffix) DEF_ASM(x ## nz ## suffix) DEF_ASM(x ## be ## suffix) DEF_ASM(x ## na ## suffix) DEF_ASM(x ## nbe ## suffix) DEF_ASM(x ## a ## suffix) DEF_ASM(x ## s ## suffix) DEF_ASM(x ## ns ## suffix) DEF_ASM(x ## p ## suffix) DEF_ASM(x ## pe ## suffix) DEF_ASM(x ## np ## suffix) DEF_ASM(x ## po ## suffix) DEF_ASM(x ## l ## suffix) DEF_ASM(x ## nge ## suffix) DEF_ASM(x ## nl ## suffix) DEF_ASM(x ## ge ## suffix) DEF_ASM(x ## le ## suffix) DEF_ASM(x ## ng ## suffix) DEF_ASM(x ## nle ## suffix) DEF_ASM(x ## g ## suffix)
	/* ------------------------------------------------------------------ */
	/* register */

	DEF_ASM(al)
	DEF_ASM(cl)
	DEF_ASM(dl)
	DEF_ASM(bl)
	DEF_ASM(ah)
	DEF_ASM(ch)
	DEF_ASM(dh)
	DEF_ASM(bh)
	DEF_ASM(ax)
	DEF_ASM(cx)
	DEF_ASM(dx)
	DEF_ASM(bx)
	DEF_ASM(sp)
	DEF_ASM(bp)
	DEF_ASM(si)
	DEF_ASM(di)
	DEF_ASM(eax)
	DEF_ASM(ecx)
	DEF_ASM(edx)
	DEF_ASM(ebx)
	DEF_ASM(esp)
	DEF_ASM(ebp)
	DEF_ASM(esi)
	DEF_ASM(edi)

	DEF_ASM(rax)
	DEF_ASM(rcx)
	DEF_ASM(rdx)
	DEF_ASM(rbx)
	DEF_ASM(rsp)
	DEF_ASM(rbp)
	DEF_ASM(rsi)
	DEF_ASM(rdi)

	DEF_ASM(mm0)
	DEF_ASM(mm1)
	DEF_ASM(mm2)
	DEF_ASM(mm3)
	DEF_ASM(mm4)
	DEF_ASM(mm5)
	DEF_ASM(mm6)
	DEF_ASM(mm7)
	DEF_ASM(xmm0)
	DEF_ASM(xmm1)
	DEF_ASM(xmm2)
	DEF_ASM(xmm3)
	DEF_ASM(xmm4)
	DEF_ASM(xmm5)
	DEF_ASM(xmm6)
	DEF_ASM(xmm7)
	DEF_ASM(cr0)
	DEF_ASM(cr1)
	DEF_ASM(cr2)
	DEF_ASM(cr3)
	DEF_ASM(cr4)
	DEF_ASM(cr5)
	DEF_ASM(cr6)
	DEF_ASM(cr7)
	DEF_ASM(tr0)
	DEF_ASM(tr1)
	DEF_ASM(tr2)
	DEF_ASM(tr3)
	DEF_ASM(tr4)
	DEF_ASM(tr5)
	DEF_ASM(tr6)
	DEF_ASM(tr7)
	DEF_ASM(db0)
	DEF_ASM(db1)
	DEF_ASM(db2)
	DEF_ASM(db3)
	DEF_ASM(db4)
	DEF_ASM(db5)
	DEF_ASM(db6)
	DEF_ASM(db7)
	DEF_ASM(dr0)
	DEF_ASM(dr1)
	DEF_ASM(dr2)
	DEF_ASM(dr3)
	DEF_ASM(dr4)
	DEF_ASM(dr5)
	DEF_ASM(dr6)
	DEF_ASM(dr7)
	DEF_ASM(es)
	DEF_ASM(cs)
	DEF_ASM(ss)
	DEF_ASM(ds)
	DEF_ASM(fs)
	DEF_ASM(gs)
	DEF_ASM(st)
	DEF_ASM(rip)
	/* The four low parts of sp/bp/si/di that exist only on
	    x86-64 (encoding aliased to ah,ch,dh,dh when not using REX). */

	DEF_ASM(spl)
	DEF_ASM(bpl)
	DEF_ASM(sil)
	DEF_ASM(dil)
	/* generic two operands */

	DEF_BWLX(mov)

	DEF_BWLX(add)
	DEF_BWLX(or)
	DEF_BWLX(adc)
	DEF_BWLX(sbb)
	DEF_BWLX(and)
	DEF_BWLX(sub)
	DEF_BWLX(xor)
	DEF_BWLX(cmp)
	/* unary ops */

	DEF_BWLX(inc)
	DEF_BWLX(dec)
	DEF_BWLX(not)
	DEF_BWLX(neg)
	DEF_BWLX(mul)
	DEF_BWLX(imul)
	DEF_BWLX(div)
	DEF_BWLX(idiv)

	DEF_BWLX(xchg)
	DEF_BWLX(test)
	/* shifts */

	DEF_BWLX(rol)
	DEF_BWLX(ror)
	DEF_BWLX(rcl)
	DEF_BWLX(rcr)
	DEF_BWLX(shl)
	DEF_BWLX(shr)
	DEF_BWLX(sar)

	DEF_WLX(shld)
	DEF_WLX(shrd)

	DEF_ASM(pushw)
	DEF_ASM(pushl)

	DEF_ASM(pushq)

	DEF_ASM(push)

	DEF_ASM(popw)
	DEF_ASM(popl)

	DEF_ASM(popq)

	DEF_ASM(pop)

	DEF_BWL(in)
	DEF_BWL(out)

	DEF_WLX(movzb)
	DEF_ASM(movzwl)
	DEF_ASM(movsbw)
	DEF_ASM(movsbl)
	DEF_ASM(movswl)

	DEF_ASM(movsbq)
	DEF_ASM(movswq)
	DEF_ASM(movzwq)
	DEF_ASM(movslq)

	DEF_WLX(lea)

	DEF_ASM(les)
	DEF_ASM(lds)
	DEF_ASM(lss)
	DEF_ASM(lfs)
	DEF_ASM(lgs)

	DEF_ASM(call)
	DEF_ASM(jmp)
	DEF_ASM(lcall)
	DEF_ASM(ljmp)

	DEF_ASMTEST(j,)

	DEF_ASMTEST(set,)
	DEF_ASMTEST(set,b)
	DEF_ASMTEST(cmov,)

	DEF_WLX(bsf)
	DEF_WLX(bsr)
	DEF_WLX(bt)
	DEF_WLX(bts)
	DEF_WLX(btr)
	DEF_WLX(btc)
	DEF_WLX(popcnt)
	DEF_WLX(tzcnt)
	DEF_WLX(lzcnt)

	DEF_WLX(lar)
	DEF_WLX(lsl)
	/* generic FP ops */

	DEF_FP(add)
	DEF_FP(mul)

	DEF_ASM(fcom)
	DEF_ASM(fcom_1)/* non existent op, just to have a regular table */

	DEF_FP1(com)

	DEF_FP(comp)
	DEF_FP(sub)
	DEF_FP(subr)
	DEF_FP(div)
	DEF_FP(divr)

	DEF_BWLX(xadd)
	DEF_BWLX(cmpxchg)
	/* string ops */

	DEF_BWLX(cmps)
	DEF_BWLX(scmp)
	DEF_BWL(ins)
	DEF_BWL(outs)
	DEF_BWLX(lods)
	DEF_BWLX(slod)
	DEF_BWLX(movs)
	DEF_BWLX(smov)
	DEF_BWLX(scas)
	DEF_BWLX(ssca)
	DEF_BWLX(stos)
	DEF_BWLX(ssto)
	/* generic asm ops */

#define ALT(x)
#define DEF_ASM_OP0(name,opcode) DEF_ASM(name)
#define DEF_ASM_OP0L(name,opcode,group,instr_type)
#define DEF_ASM_OP1(name,opcode,group,instr_type,op0)
#define DEF_ASM_OP2(name,opcode,group,instr_type,op0,op1)

	DEF_ASM_OP0(clc, 0xf8)/* must be first OP0 */

	DEF_ASM_OP0(cld, 0xfc)
	DEF_ASM_OP0(cli, 0xfa)
	DEF_ASM_OP0(clts, 0x0f06)
	DEF_ASM_OP0(cmc, 0xf5)
	DEF_ASM_OP0(lahf, 0x9f)
	DEF_ASM_OP0(sahf, 0x9e)
	DEF_ASM_OP0(pushfq, 0x9c)
	DEF_ASM_OP0(popfq, 0x9d)
	DEF_ASM_OP0(pushf, 0x9c)
	DEF_ASM_OP0(popf, 0x9d)
	DEF_ASM_OP0(stc, 0xf9)
	DEF_ASM_OP0(std, 0xfd)
	DEF_ASM_OP0(sti, 0xfb)
	DEF_ASM_OP0(aaa, 0x37)
	DEF_ASM_OP0(aas, 0x3f)
	DEF_ASM_OP0(daa, 0x27)
	DEF_ASM_OP0(das, 0x2f)
	DEF_ASM_OP0(aad, 0xd50a)
	DEF_ASM_OP0(aam, 0xd40a)
	DEF_ASM_OP0(cbw, 0x6698)
	DEF_ASM_OP0(cwd, 0x6699)
	DEF_ASM_OP0(cwde, 0x98)
	DEF_ASM_OP0(cdq, 0x99)
	DEF_ASM_OP0(cbtw, 0x6698)
	DEF_ASM_OP0(cwtl, 0x98)
	DEF_ASM_OP0(cwtd, 0x6699)
	DEF_ASM_OP0(cltd, 0x99)
	DEF_ASM_OP0(cqto, 0x4899)
	DEF_ASM_OP0(int3, 0xcc)
	DEF_ASM_OP0(into, 0xce)
	DEF_ASM_OP0(iret, 0xcf)
	DEF_ASM_OP0(iretw, 0x66cf)
	DEF_ASM_OP0(iretl, 0xcf)
	DEF_ASM_OP0(iretq, 0x48cf)
	DEF_ASM_OP0(rsm, 0x0faa)
	DEF_ASM_OP0(hlt, 0xf4)
	DEF_ASM_OP0(wait, 0x9b)
	DEF_ASM_OP0(nop, 0x90)
	DEF_ASM_OP0(pause, 0xf390)
	DEF_ASM_OP0(xlat, 0xd7)

	DEF_ASM_OP0L(vmcall, 0xc1, 0, OPC_0F01)
	DEF_ASM_OP0L(vmlaunch, 0xc2, 0, OPC_0F01)
	DEF_ASM_OP0L(vmresume, 0xc3, 0, OPC_0F01)
	DEF_ASM_OP0L(vmxoff, 0xc4, 0, OPC_0F01)
	/* strings */

	ALT(DEF_ASM_OP0L(cmpsb, 0xa6, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(scmpb, 0xa6, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(insb, 0x6c, 0, OPC_BWL))
	ALT(DEF_ASM_OP0L(outsb, 0x6e, 0, OPC_BWL))

	ALT(DEF_ASM_OP0L(lodsb, 0xac, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(slodb, 0xac, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(movsb, 0xa4, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(smovb, 0xa4, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(scasb, 0xae, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sscab, 0xae, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(stosb, 0xaa, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sstob, 0xaa, 0, OPC_BWLX))
	/* bits */

	ALT(DEF_ASM_OP2(bsfw, 0x0fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(bsrw, 0x0fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))

	ALT(DEF_ASM_OP2(btw, 0x0fa3, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btw, 0x0fba, 4, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btsw, 0x0fab, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btsw, 0x0fba, 5, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btrw, 0x0fb3, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btrw, 0x0fba, 6, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btcw, 0x0fbb, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btcw, 0x0fba, 7, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(popcntw, 0xf30fb8, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))

	ALT(DEF_ASM_OP2(tzcntw, 0xf30fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(lzcntw, 0xf30fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))
	/* prefixes */

	DEF_ASM_OP0(lock, 0xf0)
	DEF_ASM_OP0(rep, 0xf3)
	DEF_ASM_OP0(repe, 0xf3)
	DEF_ASM_OP0(repz, 0xf3)
	DEF_ASM_OP0(repne, 0xf2)
	DEF_ASM_OP0(repnz, 0xf2)

	DEF_ASM_OP0(invd, 0x0f08)
	DEF_ASM_OP0(wbinvd, 0x0f09)
	DEF_ASM_OP0(cpuid, 0x0fa2)
	DEF_ASM_OP0(wrmsr, 0x0f30)
	DEF_ASM_OP0(rdtsc, 0x0f31)
	DEF_ASM_OP0(rdmsr, 0x0f32)
	DEF_ASM_OP0(rdpmc, 0x0f33)

	DEF_ASM_OP0(syscall, 0x0f05)
	DEF_ASM_OP0(sysret, 0x0f07)
	DEF_ASM_OP0L(sysretq, 0x480f07, 0, 0)
	DEF_ASM_OP0(ud2, 0x0f0b)
	/* NOTE: we took the same order as gas opcode definition order */
	/* Right now we can't express the fact that 0xa1/0xa3 can't use $eax and a
	   32 bit moffset as operands.
	ALT(DEF_ASM_OP2(movb, 0xa0, 0, OPC_BWLX, OPT_ADDR, OPT_EAX))
	ALT(DEF_ASM_OP2(movb, 0xa2, 0, OPC_BWLX, OPT_EAX, OPT_ADDR)) */

	ALT(DEF_ASM_OP2(movb, 0x88, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movb, 0x8a, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	/* The moves are special: the 0xb8 form supports IM64 (the only insn that
	   does) with REG64.  It doesn't support IM32 with REG64, it would use
	   the full movabs form (64bit immediate).  For IM32->REG64 we prefer
	   the 0xc7 opcode.  So disallow all 64bit forms and code the rest by hand. */

	ALT(DEF_ASM_OP2(movb, 0xb0, 0, OPC_REG | OPC_BWLX, OPT_IM, OPT_REG))
	ALT(DEF_ASM_OP2(mov, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movq, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movb, 0xc6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(movw, 0x8c, 0, OPC_MODRM | OPC_WLX, OPT_SEG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movw, 0x8e, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_SEG))

	ALT(DEF_ASM_OP2(movw, 0x0f20, 0, OPC_MODRM | OPC_WLX, OPT_CR, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f21, 0, OPC_MODRM | OPC_WLX, OPT_DB, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f22, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_CR))
	ALT(DEF_ASM_OP2(movw, 0x0f23, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_DB))

	ALT(DEF_ASM_OP2(movsbw, 0x660fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG16))
	ALT(DEF_ASM_OP2(movsbl, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movsbq, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(movswl, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movswq, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movslq, 0x63, 0, OPC_MODRM, OPT_REG32 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movzbw, 0x0fb6, 0, OPC_MODRM | OPC_WLX, OPT_REG8 | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(movzwl, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movzwq, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP1(pushq, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(push, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x666a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(pushw, 0xff, 6, OPC_MODRM | OPC_WLX, OPT_REG64 | OPT_EA))
	ALT(DEF_ASM_OP1(pushw, 0x6668, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(pushw, 0x68, 0, OPC_WLX, OPT_IM32))
	ALT(DEF_ASM_OP1(pushw, 0x06, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(popw, 0x8f, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP1(popw, 0x07, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_REGW, OPT_EAX))
	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_EAX, OPT_REGW))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG,
			OPT_REG))

	ALT(DEF_ASM_OP2(inb, 0xe4, 0, OPC_BWL, OPT_IM8, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xe4, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(inb, 0xec, 0, OPC_BWL, OPT_DX, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xec, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(outb, 0xe6, 0, OPC_BWL, OPT_EAX, OPT_IM8))
	ALT(DEF_ASM_OP1(outb, 0xe6, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(outb, 0xee, 0, OPC_BWL, OPT_EAX, OPT_DX))
	ALT(DEF_ASM_OP1(outb, 0xee, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(leaw, 0x8d, 0, OPC_MODRM | OPC_WLX, OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP2(les, 0xc4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lds, 0xc5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lss, 0x0fb2, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lfs, 0x0fb4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lgs, 0x0fb5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	/* arith */

	ALT(DEF_ASM_OP2(addb, 0x00, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_EA | OPT_REG))/* XXX: use D bit ? */

	ALT(DEF_ASM_OP2(addb, 0x02, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX,
			OPT_EA | OPT_REG, OPT_REG))
	ALT(DEF_ASM_OP2(addb, 0x04, 0, OPC_ARITH | OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(addw, 0x83, 0, OPC_ARITH | OPC_MODRM | OPC_WLX, OPT_IM8S,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(addb, 0x80, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_IM,
			OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG,
			OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0xa8, 0, OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(testb, 0xf6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP1(incb, 0xfe, 0, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(decb, 0xfe, 1, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(notb, 0xf6, 2, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(negb, 0xf6, 3, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(mulb, 0xf6, 4, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(imulb, 0xf6, 5, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(imulw, 0x0faf, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA,
			OPT_REG))
	ALT(DEF_ASM_OP3(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S,
			OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW))
	ALT(DEF_ASM_OP3(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW))

	ALT(DEF_ASM_OP1(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
	ALT(DEF_ASM_OP1(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA,
			OPT_EAX))
	/* shifts */

	ALT(DEF_ASM_OP2(rolb, 0xc0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_IM8,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(rolb, 0xd2, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_CL,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP1(rolb, 0xd0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT,
			OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP3(shldw, 0x0fa4, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fac, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_EA | OPT_REGW))

	ALT(DEF_ASM_OP1(call, 0xff, 2, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(call, 0xe8, 0, 0, OPT_DISP))
	DEF_ASM_OP1(callq, 0xff, 2, OPC_MODRM, OPT_INDIR)
	ALT(DEF_ASM_OP1(callq, 0xe8, 0, 0, OPT_DISP))
	ALT(DEF_ASM_OP1(jmp, 0xff, 4, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(jmp, 0xeb, 0, 0, OPT_DISP8))

	ALT(DEF_ASM_OP1(lcall, 0xff, 3, OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(ljmp, 0xff, 5, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(ljmpw, 0x66ff, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(ljmpl, 0xff, 5, OPC_MODRM, OPT_EA)

	ALT(DEF_ASM_OP1(int, 0xcd, 0, 0, OPT_IM8))
	ALT(DEF_ASM_OP1(seto, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	ALT(DEF_ASM_OP1(setob, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	DEF_ASM_OP2(enter, 0xc8, 0, 0, OPT_IM16, OPT_IM8)
	DEF_ASM_OP0(leave, 0xc9)
	DEF_ASM_OP0(ret, 0xc3)
	DEF_ASM_OP0(retq, 0xc3)
	ALT(DEF_ASM_OP1(retq, 0xc2, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(ret, 0xc2, 0, 0, OPT_IM16))
	DEF_ASM_OP0(lret, 0xcb)
	ALT(DEF_ASM_OP1(lret, 0xca, 0, 0, OPT_IM16))

	ALT(DEF_ASM_OP1(jo, 0x70, 0, OPC_TEST, OPT_DISP8))
	DEF_ASM_OP1(loopne, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopnz, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loope, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopz, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loop, 0xe2, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(jecxz, 0x67e3, 0, 0, OPT_DISP8)
	/* float */
	/* specific fcomp handling */

	ALT(DEF_ASM_OP0L(fcomp, 0xd8d9, 0, 0))

	ALT(DEF_ASM_OP1(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(fadd, 0xdcc0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP2(fmul, 0xdcc8, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(fadd, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(faddp, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(fadds, 0xd8, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiaddl, 0xda, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(faddl, 0xdc, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiadds, 0xde, 0, OPC_FARITH | OPC_MODRM, OPT_EA))

	DEF_ASM_OP0(fucompp, 0xdae9)
	DEF_ASM_OP0(ftst, 0xd9e4)
	DEF_ASM_OP0(fxam, 0xd9e5)
	DEF_ASM_OP0(fld1, 0xd9e8)
	DEF_ASM_OP0(fldl2t, 0xd9e9)
	DEF_ASM_OP0(fldl2e, 0xd9ea)
	DEF_ASM_OP0(fldpi, 0xd9eb)
	DEF_ASM_OP0(fldlg2, 0xd9ec)
	DEF_ASM_OP0(fldln2, 0xd9ed)
	DEF_ASM_OP0(fldz, 0xd9ee)

	DEF_ASM_OP0(f2xm1, 0xd9f0)
	DEF_ASM_OP0(fyl2x, 0xd9f1)
	DEF_ASM_OP0(fptan, 0xd9f2)
	DEF_ASM_OP0(fpatan, 0xd9f3)
	DEF_ASM_OP0(fxtract, 0xd9f4)
	DEF_ASM_OP0(fprem1, 0xd9f5)
	DEF_ASM_OP0(fdecstp, 0xd9f6)
	DEF_ASM_OP0(fincstp, 0xd9f7)
	DEF_ASM_OP0(fprem, 0xd9f8)
	DEF_ASM_OP0(fyl2xp1, 0xd9f9)
	DEF_ASM_OP0(fsqrt, 0xd9fa)
	DEF_ASM_OP0(fsincos, 0xd9fb)
	DEF_ASM_OP0(frndint, 0xd9fc)
	DEF_ASM_OP0(fscale, 0xd9fd)
	DEF_ASM_OP0(fsin, 0xd9fe)
	DEF_ASM_OP0(fcos, 0xd9ff)
	DEF_ASM_OP0(fchs, 0xd9e0)
	DEF_ASM_OP0(fabs, 0xd9e1)
	DEF_ASM_OP0(fninit, 0xdbe3)
	DEF_ASM_OP0(fnclex, 0xdbe2)
	DEF_ASM_OP0(fnop, 0xd9d0)
	DEF_ASM_OP0(fwait, 0x9b)
	/* fp load */

	DEF_ASM_OP1(fld, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fldl, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(flds, 0xd9, 0, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fldl, 0xdd, 0, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fildl, 0xdb, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildq, 0xdf, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildll, 0xdf, 5, OPC_MODRM,OPT_EA)
	DEF_ASM_OP1(fldt, 0xdb, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbld, 0xdf, 4, OPC_MODRM, OPT_EA)
	/* fp store */

	DEF_ASM_OP1(fst, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fstl, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fsts, 0xd9, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstps, 0xd9, 3, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fstl, 0xdd, 2, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fstpl, 0xdd, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fist, 0xdf, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistp, 0xdf, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistl, 0xdb, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpl, 0xdb, 3, OPC_MODRM, OPT_EA)

	DEF_ASM_OP1(fstp, 0xddd8, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fistpq, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpll, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstpt, 0xdb, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbstp, 0xdf, 6, OPC_MODRM, OPT_EA)
	/* exchange */

	DEF_ASM_OP0(fxch, 0xd9c9)
	ALT(DEF_ASM_OP1(fxch, 0xd9c8, 0, OPC_REG, OPT_ST))
	/* misc FPU */

	DEF_ASM_OP1(fucom, 0xdde0, 0, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fucomp, 0xdde8, 0, OPC_REG, OPT_ST )

	DEF_ASM_OP0L(finit, 0xdbe3, 0, OPC_FWAIT)
	DEF_ASM_OP1(fldcw, 0xd9, 5, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnstcw, 0xd9, 7, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstcw, 0xd9, 7, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP0(fnstsw, 0xdfe0)
	ALT(DEF_ASM_OP1(fnstsw, 0xdfe0, 0, 0, OPT_EAX ))
	ALT(DEF_ASM_OP1(fnstsw, 0xdd, 7, OPC_MODRM, OPT_EA ))
	DEF_ASM_OP1(fstsw, 0xdfe0, 0, OPC_FWAIT, OPT_EAX )
	ALT(DEF_ASM_OP0L(fstsw, 0xdfe0, 0, OPC_FWAIT))
	ALT(DEF_ASM_OP1(fstsw, 0xdd, 7, OPC_MODRM | OPC_FWAIT, OPT_EA ))
	DEF_ASM_OP0L(fclex, 0xdbe2, 0, OPC_FWAIT)
	DEF_ASM_OP1(fnstenv, 0xd9, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstenv, 0xd9, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(fldenv, 0xd9, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnsave, 0xdd, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fsave, 0xdd, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(frstor, 0xdd, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(ffree, 0xddc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(ffreep, 0xdfc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fxsave, 0x0fae, 0, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fxrstor, 0x0fae, 1, OPC_MODRM, OPT_EA )
	/* The *q forms of fxrstor/fxsave use a REX prefix.
	       If the operand would use extended registers we would have to modify
	       it instead of generating a second one.  Currently that's no
	       problem with TCC, we don't use extended registers.  */

	DEF_ASM_OP1(fxsaveq, 0x0fae, 0, OPC_MODRM | OPC_48, OPT_EA )
	DEF_ASM_OP1(fxrstorq, 0x0fae, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* segments */

	DEF_ASM_OP2(arpl, 0x63, 0, OPC_MODRM, OPT_REG16, OPT_REG16 | OPT_EA)
	ALT(DEF_ASM_OP2(larw, 0x0f02, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA,
			OPT_REG))
	DEF_ASM_OP1(lgdt, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lgdtq, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidt, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidtq, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lldt, 0x0f00, 2, OPC_MODRM, OPT_EA | OPT_REG)
	DEF_ASM_OP1(lmsw, 0x0f01, 6, OPC_MODRM, OPT_EA | OPT_REG)
	ALT(DEF_ASM_OP2(lslw, 0x0f03, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG,
			OPT_REG))
	DEF_ASM_OP1(ltr, 0x0f00, 3, OPC_MODRM, OPT_EA | OPT_REG16)
	DEF_ASM_OP1(sgdt, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sgdtq, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidt, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidtq, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sldt, 0x0f00, 0, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(smsw, 0x0f01, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM, OPT_REG32 | OPT_EA)
	ALT(DEF_ASM_OP1(str, 0x660f00, 1, OPC_MODRM, OPT_REG16))
	ALT(DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM | OPC_48, OPT_REG64))
	DEF_ASM_OP1(verr, 0x0f00, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(verw, 0x0f00, 5, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP0L(swapgs, 0x0f01, 7, OPC_MODRM)
	/* 486 */
	/* bswap can't be applied to 16bit regs */

	DEF_ASM_OP1(bswap, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapl, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapq, 0x0fc8, 0, OPC_REG | OPC_48, OPT_REG64 )

	ALT(DEF_ASM_OP2(xaddb, 0x0fc0, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_REG | OPT_EA ))
	ALT(DEF_ASM_OP2(cmpxchgb, 0x0fb0, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_REG | OPT_EA ))
	DEF_ASM_OP1(invlpg, 0x0f01, 7, OPC_MODRM, OPT_EA )
	/* pentium */

	DEF_ASM_OP1(cmpxchg8b, 0x0fc7, 1, OPC_MODRM, OPT_EA )
	/* AMD 64 */

	DEF_ASM_OP1(cmpxchg16b, 0x0fc7, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* pentium pro */

	ALT(DEF_ASM_OP2(cmovo, 0x0f40, 0, OPC_MODRM | OPC_TEST | OPC_WLX,
			OPT_REGW | OPT_EA, OPT_REGW))

	DEF_ASM_OP2(fcmovb, 0xdac0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmove, 0xdac8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovbe, 0xdad0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovu, 0xdad8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnb, 0xdbc0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovne, 0xdbc8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnbe, 0xdbd0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnu, 0xdbd8, 0, OPC_REG, OPT_ST, OPT_ST0 )

	DEF_ASM_OP2(fucomi, 0xdbe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomi, 0xdbf0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fucomip, 0xdfe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomip, 0xdff0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	/* mmx */

	DEF_ASM_OP0(emms, 0x0f77)/* must be last OP0 */

	DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_MMXSSE )
	/* movd shouldn't accept REG64, but AMD64 spec uses it for 32 and 64 bit
	       moves, so let's be compatible. */

	ALT(DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6e, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6f, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_MMX ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG32 ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))
	ALT(DEF_ASM_OP2(movq, 0x0f7f, 0, OPC_MODRM, OPT_MMX, OPT_EA | OPT_MMX ))
	ALT(DEF_ASM_OP2(movq, 0x660fd6, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0xf30f7e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))

	DEF_ASM_OP2(packssdw, 0x0f6b, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packsswb, 0x0f63, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packuswb, 0x0f67, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddb, 0x0ffc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddw, 0x0ffd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddd, 0x0ffe, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsb, 0x0fec, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsw, 0x0fed, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusb, 0x0fdc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusw, 0x0fdd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pand, 0x0fdb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pandn, 0x0fdf, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqb, 0x0f74, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqw, 0x0f75, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqd, 0x0f76, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtb, 0x0f64, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtw, 0x0f65, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtd, 0x0f66, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaddwd, 0x0ff5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmulhw, 0x0fe5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmullw, 0x0fd5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(por, 0x0feb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psllw, 0x0ff1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllw, 0x0f71, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(pslld, 0x0ff2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(pslld, 0x0f72, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psllq, 0x0ff3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllq, 0x0f73, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psraw, 0x0fe1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psraw, 0x0f71, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrad, 0x0fe2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrad, 0x0f72, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlw, 0x0fd1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlw, 0x0f71, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrld, 0x0fd2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrld, 0x0f72, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlq, 0x0fd3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlq, 0x0f73, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psubb, 0x0ff8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubw, 0x0ff9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubd, 0x0ffa, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsb, 0x0fe8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsw, 0x0fe9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusb, 0x0fd8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusw, 0x0fd9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhbw, 0x0f68, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhwd, 0x0f69, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhdq, 0x0f6a, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklbw, 0x0f60, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklwd, 0x0f61, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckldq, 0x0f62, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pxor, 0x0fef, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	/* sse */

	DEF_ASM_OP1(ldmxcsr, 0x0fae, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(stmxcsr, 0x0fae, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP2(movups, 0x0f10, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movups, 0x0f11, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movaps, 0x0f28, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movaps, 0x0f29, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movhps, 0x0f16, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movhps, 0x0f17, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(addps, 0x0f58, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(cvtpi2ps, 0x0f2a, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_SSE )
	DEF_ASM_OP2(cvtps2pi, 0x0f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE,
			OPT_REG64 ))
	DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE,
			OPT_REG64 ))
	DEF_ASM_OP2(cvttps2pi, 0x0f2c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(andps, 0x0f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(divps, 0x0f5e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(maxps, 0x0f5f, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(minps, 0x0f5d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(mulps, 0x0f59, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgb, 0x0fe0, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgw, 0x0fe3, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pmaxsw, 0x0fee, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaxub, 0x0fde, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminsw, 0x0fea, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminub, 0x0fda, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(rcpss, 0x0f53, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(rsqrtps, 0x0f52, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtps, 0x0f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtss, 0xf30f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(subps, 0x0f5c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	/* sse2 */

	DEF_ASM_OP2(andpd, 0x660f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	DEF_ASM_OP2(sqrtsd, 0xf20f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	/* movnti should only accept REG32 and REG64, we accept more */

	DEF_ASM_OP2(movnti, 0x0fc3, 0, OPC_MODRM, OPT_REG, OPT_EA)
	DEF_ASM_OP2(movntil, 0x0fc3, 0, OPC_MODRM, OPT_REG32, OPT_EA)
	DEF_ASM_OP2(movntiq, 0x0fc3, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_EA)
	DEF_ASM_OP1(prefetchnta, 0x0f18, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht0, 0x0f18, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht1, 0x0f18, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht2, 0x0f18, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetchw, 0x0f0d, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP0L(lfence, 0x0fae, 5, OPC_MODRM)
	DEF_ASM_OP0L(mfence, 0x0fae, 6, OPC_MODRM)
	DEF_ASM_OP0L(sfence, 0x0fae, 7, OPC_MODRM)
	DEF_ASM_OP1(clflush, 0x0fae, 7, OPC_MODRM, OPT_EA)
	/* Control-Flow Enforcement */

	DEF_ASM_OP0L(endbr64, 0xf30f1e, 7, OPC_MODRM)
#undef ALT
#undef DEF_ASM_OP0
#undef DEF_ASM_OP0L
#undef DEF_ASM_OP1
#undef DEF_ASM_OP2
#undef DEF_ASM_OP3

#define ALT(x)
#define DEF_ASM_OP0(name,opcode)
#define DEF_ASM_OP0L(name,opcode,group,instr_type) DEF_ASM(name)
#define DEF_ASM_OP1(name,opcode,group,instr_type,op0) DEF_ASM(name)
#define DEF_ASM_OP2(name,opcode,group,instr_type,op0,op1) DEF_ASM(name)

	DEF_ASM_OP0(clc, 0xf8)/* must be first OP0 */

	DEF_ASM_OP0(cld, 0xfc)
	DEF_ASM_OP0(cli, 0xfa)
	DEF_ASM_OP0(clts, 0x0f06)
	DEF_ASM_OP0(cmc, 0xf5)
	DEF_ASM_OP0(lahf, 0x9f)
	DEF_ASM_OP0(sahf, 0x9e)
	DEF_ASM_OP0(pushfq, 0x9c)
	DEF_ASM_OP0(popfq, 0x9d)
	DEF_ASM_OP0(pushf, 0x9c)
	DEF_ASM_OP0(popf, 0x9d)
	DEF_ASM_OP0(stc, 0xf9)
	DEF_ASM_OP0(std, 0xfd)
	DEF_ASM_OP0(sti, 0xfb)
	DEF_ASM_OP0(aaa, 0x37)
	DEF_ASM_OP0(aas, 0x3f)
	DEF_ASM_OP0(daa, 0x27)
	DEF_ASM_OP0(das, 0x2f)
	DEF_ASM_OP0(aad, 0xd50a)
	DEF_ASM_OP0(aam, 0xd40a)
	DEF_ASM_OP0(cbw, 0x6698)
	DEF_ASM_OP0(cwd, 0x6699)
	DEF_ASM_OP0(cwde, 0x98)
	DEF_ASM_OP0(cdq, 0x99)
	DEF_ASM_OP0(cbtw, 0x6698)
	DEF_ASM_OP0(cwtl, 0x98)
	DEF_ASM_OP0(cwtd, 0x6699)
	DEF_ASM_OP0(cltd, 0x99)
	DEF_ASM_OP0(cqto, 0x4899)
	DEF_ASM_OP0(int3, 0xcc)
	DEF_ASM_OP0(into, 0xce)
	DEF_ASM_OP0(iret, 0xcf)
	DEF_ASM_OP0(iretw, 0x66cf)
	DEF_ASM_OP0(iretl, 0xcf)
	DEF_ASM_OP0(iretq, 0x48cf)
	DEF_ASM_OP0(rsm, 0x0faa)
	DEF_ASM_OP0(hlt, 0xf4)
	DEF_ASM_OP0(wait, 0x9b)
	DEF_ASM_OP0(nop, 0x90)
	DEF_ASM_OP0(pause, 0xf390)
	DEF_ASM_OP0(xlat, 0xd7)

	DEF_ASM_OP0L(vmcall, 0xc1, 0, OPC_0F01)
	DEF_ASM_OP0L(vmlaunch, 0xc2, 0, OPC_0F01)
	DEF_ASM_OP0L(vmresume, 0xc3, 0, OPC_0F01)
	DEF_ASM_OP0L(vmxoff, 0xc4, 0, OPC_0F01)
	/* strings */

	ALT(DEF_ASM_OP0L(cmpsb, 0xa6, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(scmpb, 0xa6, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(insb, 0x6c, 0, OPC_BWL))
	ALT(DEF_ASM_OP0L(outsb, 0x6e, 0, OPC_BWL))

	ALT(DEF_ASM_OP0L(lodsb, 0xac, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(slodb, 0xac, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(movsb, 0xa4, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(smovb, 0xa4, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(scasb, 0xae, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sscab, 0xae, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(stosb, 0xaa, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sstob, 0xaa, 0, OPC_BWLX))
	/* bits */

	ALT(DEF_ASM_OP2(bsfw, 0x0fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(bsrw, 0x0fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))

	ALT(DEF_ASM_OP2(btw, 0x0fa3, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btw, 0x0fba, 4, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btsw, 0x0fab, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btsw, 0x0fba, 5, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btrw, 0x0fb3, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btrw, 0x0fba, 6, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btcw, 0x0fbb, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btcw, 0x0fba, 7, OPC_MODRM | OPC_WLX, OPT_IM8,
			OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(popcntw, 0xf30fb8, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))

	ALT(DEF_ASM_OP2(tzcntw, 0xf30fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(lzcntw, 0xf30fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA,
			OPT_REGW))
	/* prefixes */

	DEF_ASM_OP0(lock, 0xf0)
	DEF_ASM_OP0(rep, 0xf3)
	DEF_ASM_OP0(repe, 0xf3)
	DEF_ASM_OP0(repz, 0xf3)
	DEF_ASM_OP0(repne, 0xf2)
	DEF_ASM_OP0(repnz, 0xf2)

	DEF_ASM_OP0(invd, 0x0f08)
	DEF_ASM_OP0(wbinvd, 0x0f09)
	DEF_ASM_OP0(cpuid, 0x0fa2)
	DEF_ASM_OP0(wrmsr, 0x0f30)
	DEF_ASM_OP0(rdtsc, 0x0f31)
	DEF_ASM_OP0(rdmsr, 0x0f32)
	DEF_ASM_OP0(rdpmc, 0x0f33)

	DEF_ASM_OP0(syscall, 0x0f05)
	DEF_ASM_OP0(sysret, 0x0f07)
	DEF_ASM_OP0L(sysretq, 0x480f07, 0, 0)
	DEF_ASM_OP0(ud2, 0x0f0b)
	/* NOTE: we took the same order as gas opcode definition order */
	/* Right now we can't express the fact that 0xa1/0xa3 can't use $eax and a
	   32 bit moffset as operands.
	ALT(DEF_ASM_OP2(movb, 0xa0, 0, OPC_BWLX, OPT_ADDR, OPT_EAX))
	ALT(DEF_ASM_OP2(movb, 0xa2, 0, OPC_BWLX, OPT_EAX, OPT_ADDR)) */

	ALT(DEF_ASM_OP2(movb, 0x88, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movb, 0x8a, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	/* The moves are special: the 0xb8 form supports IM64 (the only insn that
	   does) with REG64.  It doesn't support IM32 with REG64, it would use
	   the full movabs form (64bit immediate).  For IM32->REG64 we prefer
	   the 0xc7 opcode.  So disallow all 64bit forms and code the rest by hand. */

	ALT(DEF_ASM_OP2(movb, 0xb0, 0, OPC_REG | OPC_BWLX, OPT_IM, OPT_REG))
	ALT(DEF_ASM_OP2(mov, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movq, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movb, 0xc6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(movw, 0x8c, 0, OPC_MODRM | OPC_WLX, OPT_SEG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movw, 0x8e, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_SEG))

	ALT(DEF_ASM_OP2(movw, 0x0f20, 0, OPC_MODRM | OPC_WLX, OPT_CR, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f21, 0, OPC_MODRM | OPC_WLX, OPT_DB, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f22, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_CR))
	ALT(DEF_ASM_OP2(movw, 0x0f23, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_DB))

	ALT(DEF_ASM_OP2(movsbw, 0x660fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG16))
	ALT(DEF_ASM_OP2(movsbl, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movsbq, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(movswl, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movswq, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movslq, 0x63, 0, OPC_MODRM, OPT_REG32 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movzbw, 0x0fb6, 0, OPC_MODRM | OPC_WLX, OPT_REG8 | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(movzwl, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movzwq, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP1(pushq, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(push, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x666a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(pushw, 0xff, 6, OPC_MODRM | OPC_WLX, OPT_REG64 | OPT_EA))
	ALT(DEF_ASM_OP1(pushw, 0x6668, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(pushw, 0x68, 0, OPC_WLX, OPT_IM32))
	ALT(DEF_ASM_OP1(pushw, 0x06, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(popw, 0x8f, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP1(popw, 0x07, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_REGW, OPT_EAX))
	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_EAX, OPT_REGW))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG,
			OPT_REG))

	ALT(DEF_ASM_OP2(inb, 0xe4, 0, OPC_BWL, OPT_IM8, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xe4, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(inb, 0xec, 0, OPC_BWL, OPT_DX, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xec, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(outb, 0xe6, 0, OPC_BWL, OPT_EAX, OPT_IM8))
	ALT(DEF_ASM_OP1(outb, 0xe6, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(outb, 0xee, 0, OPC_BWL, OPT_EAX, OPT_DX))
	ALT(DEF_ASM_OP1(outb, 0xee, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(leaw, 0x8d, 0, OPC_MODRM | OPC_WLX, OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP2(les, 0xc4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lds, 0xc5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lss, 0x0fb2, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lfs, 0x0fb4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lgs, 0x0fb5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	/* arith */

	ALT(DEF_ASM_OP2(addb, 0x00, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_EA | OPT_REG))/* XXX: use D bit ? */

	ALT(DEF_ASM_OP2(addb, 0x02, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX,
			OPT_EA | OPT_REG, OPT_REG))
	ALT(DEF_ASM_OP2(addb, 0x04, 0, OPC_ARITH | OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(addw, 0x83, 0, OPC_ARITH | OPC_MODRM | OPC_WLX, OPT_IM8S,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(addb, 0x80, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_IM,
			OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG,
			OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0xa8, 0, OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(testb, 0xf6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP1(incb, 0xfe, 0, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(decb, 0xfe, 1, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(notb, 0xf6, 2, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(negb, 0xf6, 3, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(mulb, 0xf6, 4, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(imulb, 0xf6, 5, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(imulw, 0x0faf, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA,
			OPT_REG))
	ALT(DEF_ASM_OP3(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S,
			OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW))
	ALT(DEF_ASM_OP3(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW | OPT_EA,
			OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW))

	ALT(DEF_ASM_OP1(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
	ALT(DEF_ASM_OP1(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA,
			OPT_EAX))
	/* shifts */

	ALT(DEF_ASM_OP2(rolb, 0xc0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_IM8,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(rolb, 0xd2, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_CL,
			OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP1(rolb, 0xd0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT,
			OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP3(shldw, 0x0fa4, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fac, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW,
			OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_REGW,
			OPT_EA | OPT_REGW))

	ALT(DEF_ASM_OP1(call, 0xff, 2, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(call, 0xe8, 0, 0, OPT_DISP))
	DEF_ASM_OP1(callq, 0xff, 2, OPC_MODRM, OPT_INDIR)
	ALT(DEF_ASM_OP1(callq, 0xe8, 0, 0, OPT_DISP))
	ALT(DEF_ASM_OP1(jmp, 0xff, 4, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(jmp, 0xeb, 0, 0, OPT_DISP8))

	ALT(DEF_ASM_OP1(lcall, 0xff, 3, OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(ljmp, 0xff, 5, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(ljmpw, 0x66ff, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(ljmpl, 0xff, 5, OPC_MODRM, OPT_EA)

	ALT(DEF_ASM_OP1(int, 0xcd, 0, 0, OPT_IM8))
	ALT(DEF_ASM_OP1(seto, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	ALT(DEF_ASM_OP1(setob, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	DEF_ASM_OP2(enter, 0xc8, 0, 0, OPT_IM16, OPT_IM8)
	DEF_ASM_OP0(leave, 0xc9)
	DEF_ASM_OP0(ret, 0xc3)
	DEF_ASM_OP0(retq, 0xc3)
	ALT(DEF_ASM_OP1(retq, 0xc2, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(ret, 0xc2, 0, 0, OPT_IM16))
	DEF_ASM_OP0(lret, 0xcb)
	ALT(DEF_ASM_OP1(lret, 0xca, 0, 0, OPT_IM16))

	ALT(DEF_ASM_OP1(jo, 0x70, 0, OPC_TEST, OPT_DISP8))
	DEF_ASM_OP1(loopne, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopnz, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loope, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopz, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loop, 0xe2, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(jecxz, 0x67e3, 0, 0, OPT_DISP8)
	/* float */
	/* specific fcomp handling */

	ALT(DEF_ASM_OP0L(fcomp, 0xd8d9, 0, 0))

	ALT(DEF_ASM_OP1(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(fadd, 0xdcc0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP2(fmul, 0xdcc8, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(fadd, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(faddp, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(fadds, 0xd8, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiaddl, 0xda, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(faddl, 0xdc, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiadds, 0xde, 0, OPC_FARITH | OPC_MODRM, OPT_EA))

	DEF_ASM_OP0(fucompp, 0xdae9)
	DEF_ASM_OP0(ftst, 0xd9e4)
	DEF_ASM_OP0(fxam, 0xd9e5)
	DEF_ASM_OP0(fld1, 0xd9e8)
	DEF_ASM_OP0(fldl2t, 0xd9e9)
	DEF_ASM_OP0(fldl2e, 0xd9ea)
	DEF_ASM_OP0(fldpi, 0xd9eb)
	DEF_ASM_OP0(fldlg2, 0xd9ec)
	DEF_ASM_OP0(fldln2, 0xd9ed)
	DEF_ASM_OP0(fldz, 0xd9ee)

	DEF_ASM_OP0(f2xm1, 0xd9f0)
	DEF_ASM_OP0(fyl2x, 0xd9f1)
	DEF_ASM_OP0(fptan, 0xd9f2)
	DEF_ASM_OP0(fpatan, 0xd9f3)
	DEF_ASM_OP0(fxtract, 0xd9f4)
	DEF_ASM_OP0(fprem1, 0xd9f5)
	DEF_ASM_OP0(fdecstp, 0xd9f6)
	DEF_ASM_OP0(fincstp, 0xd9f7)
	DEF_ASM_OP0(fprem, 0xd9f8)
	DEF_ASM_OP0(fyl2xp1, 0xd9f9)
	DEF_ASM_OP0(fsqrt, 0xd9fa)
	DEF_ASM_OP0(fsincos, 0xd9fb)
	DEF_ASM_OP0(frndint, 0xd9fc)
	DEF_ASM_OP0(fscale, 0xd9fd)
	DEF_ASM_OP0(fsin, 0xd9fe)
	DEF_ASM_OP0(fcos, 0xd9ff)
	DEF_ASM_OP0(fchs, 0xd9e0)
	DEF_ASM_OP0(fabs, 0xd9e1)
	DEF_ASM_OP0(fninit, 0xdbe3)
	DEF_ASM_OP0(fnclex, 0xdbe2)
	DEF_ASM_OP0(fnop, 0xd9d0)
	DEF_ASM_OP0(fwait, 0x9b)
	/* fp load */

	DEF_ASM_OP1(fld, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fldl, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(flds, 0xd9, 0, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fldl, 0xdd, 0, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fildl, 0xdb, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildq, 0xdf, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildll, 0xdf, 5, OPC_MODRM,OPT_EA)
	DEF_ASM_OP1(fldt, 0xdb, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbld, 0xdf, 4, OPC_MODRM, OPT_EA)
	/* fp store */

	DEF_ASM_OP1(fst, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fstl, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fsts, 0xd9, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstps, 0xd9, 3, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fstl, 0xdd, 2, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fstpl, 0xdd, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fist, 0xdf, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistp, 0xdf, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistl, 0xdb, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpl, 0xdb, 3, OPC_MODRM, OPT_EA)

	DEF_ASM_OP1(fstp, 0xddd8, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fistpq, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpll, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstpt, 0xdb, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbstp, 0xdf, 6, OPC_MODRM, OPT_EA)
	/* exchange */

	DEF_ASM_OP0(fxch, 0xd9c9)
	ALT(DEF_ASM_OP1(fxch, 0xd9c8, 0, OPC_REG, OPT_ST))
	/* misc FPU */

	DEF_ASM_OP1(fucom, 0xdde0, 0, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fucomp, 0xdde8, 0, OPC_REG, OPT_ST )

	DEF_ASM_OP0L(finit, 0xdbe3, 0, OPC_FWAIT)
	DEF_ASM_OP1(fldcw, 0xd9, 5, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnstcw, 0xd9, 7, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstcw, 0xd9, 7, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP0(fnstsw, 0xdfe0)
	ALT(DEF_ASM_OP1(fnstsw, 0xdfe0, 0, 0, OPT_EAX ))
	ALT(DEF_ASM_OP1(fnstsw, 0xdd, 7, OPC_MODRM, OPT_EA ))
	DEF_ASM_OP1(fstsw, 0xdfe0, 0, OPC_FWAIT, OPT_EAX )
	ALT(DEF_ASM_OP0L(fstsw, 0xdfe0, 0, OPC_FWAIT))
	ALT(DEF_ASM_OP1(fstsw, 0xdd, 7, OPC_MODRM | OPC_FWAIT, OPT_EA ))
	DEF_ASM_OP0L(fclex, 0xdbe2, 0, OPC_FWAIT)
	DEF_ASM_OP1(fnstenv, 0xd9, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstenv, 0xd9, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(fldenv, 0xd9, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnsave, 0xdd, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fsave, 0xdd, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(frstor, 0xdd, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(ffree, 0xddc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(ffreep, 0xdfc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fxsave, 0x0fae, 0, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fxrstor, 0x0fae, 1, OPC_MODRM, OPT_EA )
	/* The *q forms of fxrstor/fxsave use a REX prefix.
	       If the operand would use extended registers we would have to modify
	       it instead of generating a second one.  Currently that's no
	       problem with TCC, we don't use extended registers.  */

	DEF_ASM_OP1(fxsaveq, 0x0fae, 0, OPC_MODRM | OPC_48, OPT_EA )
	DEF_ASM_OP1(fxrstorq, 0x0fae, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* segments */

	DEF_ASM_OP2(arpl, 0x63, 0, OPC_MODRM, OPT_REG16, OPT_REG16 | OPT_EA)
	ALT(DEF_ASM_OP2(larw, 0x0f02, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA,
			OPT_REG))
	DEF_ASM_OP1(lgdt, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lgdtq, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidt, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidtq, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lldt, 0x0f00, 2, OPC_MODRM, OPT_EA | OPT_REG)
	DEF_ASM_OP1(lmsw, 0x0f01, 6, OPC_MODRM, OPT_EA | OPT_REG)
	ALT(DEF_ASM_OP2(lslw, 0x0f03, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG,
			OPT_REG))
	DEF_ASM_OP1(ltr, 0x0f00, 3, OPC_MODRM, OPT_EA | OPT_REG16)
	DEF_ASM_OP1(sgdt, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sgdtq, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidt, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidtq, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sldt, 0x0f00, 0, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(smsw, 0x0f01, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM, OPT_REG32 | OPT_EA)
	ALT(DEF_ASM_OP1(str, 0x660f00, 1, OPC_MODRM, OPT_REG16))
	ALT(DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM | OPC_48, OPT_REG64))
	DEF_ASM_OP1(verr, 0x0f00, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(verw, 0x0f00, 5, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP0L(swapgs, 0x0f01, 7, OPC_MODRM)
	/* 486 */
	/* bswap can't be applied to 16bit regs */

	DEF_ASM_OP1(bswap, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapl, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapq, 0x0fc8, 0, OPC_REG | OPC_48, OPT_REG64 )

	ALT(DEF_ASM_OP2(xaddb, 0x0fc0, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_REG | OPT_EA ))
	ALT(DEF_ASM_OP2(cmpxchgb, 0x0fb0, 0, OPC_MODRM | OPC_BWLX, OPT_REG,
			OPT_REG | OPT_EA ))
	DEF_ASM_OP1(invlpg, 0x0f01, 7, OPC_MODRM, OPT_EA )
	/* pentium */

	DEF_ASM_OP1(cmpxchg8b, 0x0fc7, 1, OPC_MODRM, OPT_EA )
	/* AMD 64 */

	DEF_ASM_OP1(cmpxchg16b, 0x0fc7, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* pentium pro */

	ALT(DEF_ASM_OP2(cmovo, 0x0f40, 0, OPC_MODRM | OPC_TEST | OPC_WLX,
			OPT_REGW | OPT_EA, OPT_REGW))

	DEF_ASM_OP2(fcmovb, 0xdac0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmove, 0xdac8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovbe, 0xdad0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovu, 0xdad8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnb, 0xdbc0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovne, 0xdbc8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnbe, 0xdbd0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnu, 0xdbd8, 0, OPC_REG, OPT_ST, OPT_ST0 )

	DEF_ASM_OP2(fucomi, 0xdbe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomi, 0xdbf0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fucomip, 0xdfe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomip, 0xdff0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	/* mmx */

	DEF_ASM_OP0(emms, 0x0f77)/* must be last OP0 */

	DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_MMXSSE )
	/* movd shouldn't accept REG64, but AMD64 spec uses it for 32 and 64 bit
	       moves, so let's be compatible. */

	ALT(DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6e, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6f, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_MMX ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG32 ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))
	ALT(DEF_ASM_OP2(movq, 0x0f7f, 0, OPC_MODRM, OPT_MMX, OPT_EA | OPT_MMX ))
	ALT(DEF_ASM_OP2(movq, 0x660fd6, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0xf30f7e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))

	DEF_ASM_OP2(packssdw, 0x0f6b, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packsswb, 0x0f63, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packuswb, 0x0f67, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddb, 0x0ffc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddw, 0x0ffd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddd, 0x0ffe, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsb, 0x0fec, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsw, 0x0fed, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusb, 0x0fdc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusw, 0x0fdd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pand, 0x0fdb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pandn, 0x0fdf, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqb, 0x0f74, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqw, 0x0f75, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqd, 0x0f76, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtb, 0x0f64, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtw, 0x0f65, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtd, 0x0f66, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaddwd, 0x0ff5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmulhw, 0x0fe5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmullw, 0x0fd5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(por, 0x0feb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psllw, 0x0ff1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllw, 0x0f71, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(pslld, 0x0ff2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(pslld, 0x0f72, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psllq, 0x0ff3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllq, 0x0f73, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psraw, 0x0fe1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psraw, 0x0f71, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrad, 0x0fe2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrad, 0x0f72, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlw, 0x0fd1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlw, 0x0f71, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrld, 0x0fd2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrld, 0x0f72, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlq, 0x0fd3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlq, 0x0f73, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psubb, 0x0ff8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubw, 0x0ff9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubd, 0x0ffa, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsb, 0x0fe8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsw, 0x0fe9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusb, 0x0fd8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusw, 0x0fd9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhbw, 0x0f68, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhwd, 0x0f69, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhdq, 0x0f6a, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklbw, 0x0f60, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklwd, 0x0f61, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckldq, 0x0f62, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pxor, 0x0fef, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	/* sse */

	DEF_ASM_OP1(ldmxcsr, 0x0fae, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(stmxcsr, 0x0fae, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP2(movups, 0x0f10, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movups, 0x0f11, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movaps, 0x0f28, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movaps, 0x0f29, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movhps, 0x0f16, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movhps, 0x0f17, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(addps, 0x0f58, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(cvtpi2ps, 0x0f2a, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_SSE )
	DEF_ASM_OP2(cvtps2pi, 0x0f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE,
			OPT_REG64 ))
	DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE,
			OPT_REG64 ))
	DEF_ASM_OP2(cvttps2pi, 0x0f2c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(andps, 0x0f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(divps, 0x0f5e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(maxps, 0x0f5f, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(minps, 0x0f5d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(mulps, 0x0f59, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgb, 0x0fe0, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgw, 0x0fe3, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pmaxsw, 0x0fee, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaxub, 0x0fde, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminsw, 0x0fea, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminub, 0x0fda, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(rcpss, 0x0f53, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(rsqrtps, 0x0f52, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtps, 0x0f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtss, 0xf30f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(subps, 0x0f5c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	/* sse2 */

	DEF_ASM_OP2(andpd, 0x660f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	DEF_ASM_OP2(sqrtsd, 0xf20f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	/* movnti should only accept REG32 and REG64, we accept more */

	DEF_ASM_OP2(movnti, 0x0fc3, 0, OPC_MODRM, OPT_REG, OPT_EA)
	DEF_ASM_OP2(movntil, 0x0fc3, 0, OPC_MODRM, OPT_REG32, OPT_EA)
	DEF_ASM_OP2(movntiq, 0x0fc3, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_EA)
	DEF_ASM_OP1(prefetchnta, 0x0f18, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht0, 0x0f18, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht1, 0x0f18, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht2, 0x0f18, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetchw, 0x0f0d, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP0L(lfence, 0x0fae, 5, OPC_MODRM)
	DEF_ASM_OP0L(mfence, 0x0fae, 6, OPC_MODRM)
	DEF_ASM_OP0L(sfence, 0x0fae, 7, OPC_MODRM)
	DEF_ASM_OP1(clflush, 0x0fae, 7, OPC_MODRM, OPT_EA)
	/* Control-Flow Enforcement */

	DEF_ASM_OP0L(endbr64, 0xf30f1e, 7, OPC_MODRM)
#undef ALT
#undef DEF_ASM_OP0
#undef DEF_ASM_OP0L
#undef DEF_ASM_OP1
#undef DEF_ASM_OP2
#undef DEF_ASM_OP3
#undef DEF
	;
/* WARNING: the content of this string encodes token numbers */

static const unsigned char tok_two_chars[] =
	/* outdated -- gr
	    "<=\236>=\235!=\225&&\240||\241++\244--\242==\224<<\1>>\2+=\253"
	    "-=\255*=\252/=\257%=\245&=\246^=\336|=\374->\313..\250##\266";
	*/

{
	'<','=', TOK_LE,
	'>','=', TOK_GE,
	'!','=', TOK_NE,
	'&','&', TOK_LAND,
	'|','|', TOK_LOR,
	'+','+', TOK_INC,
	'-','-', TOK_DEC,
	'=','=', TOK_EQ,
	'<','<', TOK_SHL,
	'>','>', TOK_SAR,
	'+','=', TOK_A_ADD,
	'-','=', TOK_A_SUB,
	'*','=', TOK_A_MUL,
	'/','=', TOK_A_DIV,
	'%','=', TOK_A_MOD,
	'&','=', TOK_A_AND,
	'^','=', TOK_A_XOR,
	'|','=', TOK_A_OR,
	'-','>', TOK_ARROW,
	'.','.', TOK_TWODOTS,
	'#','#', TOK_TWOSHARPS,
	0
};

ST_FUNC void skip(int c)
{
	if (tok != c) {
		char tmp[40];
		pstrcpy(tmp, sizeof tmp, get_tok_str(c, &tokc));
		tcc_error("'%s' expected (got '%s')", tmp, get_tok_str(tok, &tokc));
	}
	next();
}

ST_FUNC void expect(const char *msg)
{
	tcc_error("%s expected", msg);
}
#define tal_free(al,p) tal_free_impl(al, p)
#define tal_realloc(al,p,size) tal_realloc_impl(al, p, size)
#define TAL_DEBUG_PARAMS
/* allocator for TokenSym in table_ident */
#define TOKSYM_TAL_SIZE (256 * 1024)
/* allocator for TokenString instances */
#define TOKSTR_TAL_SIZE (256 * 1024)

typedef struct TinyAlloc {
	uint8_t *p;
	uint8_t *bufend;
	struct TinyAlloc *next;
	unsigned nb_allocs;
	unsigned size;

	union {
		uint8_t buffer[1];
		size_t _aligner_;
	};
} TinyAlloc;

typedef struct tal_header_t {
	size_t size;/* word align */

} tal_header_t;

#define TAL_ALIGN(size) (((size) + (sizeof (size_t) - 1)) & ~(sizeof (size_t) - 1))
/* ------------------------------------------------------------------------- */

static TinyAlloc *tal_new(TinyAlloc **pal, unsigned size)
{
	TinyAlloc *al = tcc_malloc(sizeof(TinyAlloc) - sizeof (size_t) + size);
	al->p = al->buffer;
	al->bufend = al->buffer + size;
	al->nb_allocs = 0;
	al->next = *pal, *pal = al;
	al->size = al->next ? al->next->size : size;

	return al;
}

static void tal_delete(TinyAlloc **pal)
{
	TinyAlloc *al = *pal, *next;

tail_call:
	next = al->next;
	tcc_free(al);
	al = next;
	if (al)
		goto tail_call;
	*pal = al;
}

static void tal_free_impl(TinyAlloc **pal, void *p TAL_DEBUG_PARAMS)
{
	TinyAlloc *al, **top = pal;
	tal_header_t *header;

	if (!p)
		return;
	header = (tal_header_t *)p - 1;
	al = *pal;
	while ((uint8_t *)p < al->buffer || (uint8_t *)p > al->bufend)
		al = *(pal = &al->next);
	if (0 == --al->nb_allocs) {
		*pal = al->next;
		if ((al->bufend - al->buffer) > al->size) {
//fprintf(stderr, "free big tal: %u\n", header->size);

			tcc_free(al);
		} else {
			/* reset and move to front */

			al->p = al->buffer;
			al->next = *top, *top = al;
		}
	} else if ((uint8_t *)p + header->size == al->p) {
		al->p = (uint8_t *)header;
	}
}

static void *tal_realloc_impl(TinyAlloc **pal, void *p,
			      unsigned size TAL_DEBUG_PARAMS)
{
	tal_header_t *header;
	void *ret;
	unsigned adj_size = TAL_ALIGN(size) + sizeof(tal_header_t);
	TinyAlloc *al = *pal;

	if (p) {
		/* reallpc case */

		while ((uint8_t *)p < al->buffer || (uint8_t *)p > al->bufend)
			al = al->next;
		header = (tal_header_t *)p - 1;
		if ((uint8_t *)p + header->size == al->p)
			al->p = (uint8_t *)header; /* maybe reuse */

		if (al->p + adj_size > al->bufend) {
			ret = tal_realloc(pal, 0, size);
			memcpy(ret, p, header->size);
			tal_free(pal, p);
			return ret;
		} else if (al->p != (uint8_t *)header) {
			memcpy((tal_header_t *)al->p + 1, p, header->size);

		}
	} else {
		/* new alloc case */

		while (al->p + adj_size > al->bufend) {
			al = al->next;
			if (!al) {
				unsigned new_size = (*pal)->size;
				if (adj_size > new_size) {
					new_size = adj_size;
//fprintf(stderr, "%s:%d: alloc big tal: %u\n", file->filename, file->line_num, adj_size - sizeof(tal_header_t));

				}
				al = tal_new(pal, new_size);
				break;
			}
		}
		al->nb_allocs++;
	}
	header = (tal_header_t *)al->p;
	header->size = adj_size - sizeof(tal_header_t);
	al->p += adj_size;
	ret = header + 1;
	return ret;
}
/* USE_TAL */
/* ------------------------------------------------------------------------- */
/* CString handling */

static void cstr_realloc(CString *cstr, int new_size)
{
	int size;

	size = cstr->size_allocated;
	if (size < 8)
		size = 8;/* no need to allocate a too small first string */

	while (size < new_size)
		size = size * 2;
	cstr->data = tcc_realloc(cstr->data, size);
	cstr->size_allocated = size;
}
/* add a byte */

ST_INLN void cstr_ccat(CString *cstr, int ch)
{
	int size;
	size = cstr->size + 1;
	if (size > cstr->size_allocated)
		cstr_realloc(cstr, size);
	cstr->data[size - 1] = ch;
	cstr->size = size;
}

ST_INLN char *unicode_to_utf8 (char *b, uint32_t Uc)
{
	if (Uc<0x80)
		*b++=Uc;
	else if (Uc<0x800)
		*b++=192+Uc/64, *b++=128+Uc%64;
	else if (Uc-0xd800u<0x800)
		goto error;
	else if (Uc<0x10000)
		*b++=224+Uc/4096, *b++=128+Uc/64%64, *b++=128+Uc%64;
	else if (Uc<0x110000)
		*b++=240+Uc/262144, *b++=128+Uc/4096%64, *b++=128+Uc/64%64, *b++=128+Uc%64;
	else
error: tcc_error("0x%x is not a valid universal character", Uc);
	return b;
}
/* add a unicode character expanded into utf8 */

ST_INLN void cstr_u8cat(CString *cstr, int ch)
{
	char buf[4], *e;
	e = unicode_to_utf8(buf, (uint32_t)ch);
	cstr_cat(cstr, buf, e - buf);
}
/* add string of 'len', or of its len/len+1 when 'len' == -1/0 */

ST_FUNC void cstr_cat(CString *cstr, const char *str, int len)
{
	int size;
	if (len <= 0)
		len = strlen(str) + 1 + len;
	size = cstr->size + len;
	if (size > cstr->size_allocated)
		cstr_realloc(cstr, size);
	memmove(cstr->data + cstr->size, str, len);
	cstr->size = size;
}
/* add a wide char */

ST_FUNC void cstr_wccat(CString *cstr, int ch)
{
	int size;
	size = cstr->size + sizeof(nwchar_t);
	if (size > cstr->size_allocated)
		cstr_realloc(cstr, size);
	*(nwchar_t *)(cstr->data + size - sizeof(nwchar_t)) = ch;
	cstr->size = size;
}

ST_FUNC void cstr_new(CString *cstr)
{
	memset(cstr, 0, sizeof(CString));
}
/* free string and reset it to NULL */

ST_FUNC void cstr_free(CString *cstr)
{
	tcc_free(cstr->data);
}
/* reset string to empty */

ST_FUNC void cstr_reset(CString *cstr)
{
	cstr->size = 0;
}

ST_FUNC int cstr_vprintf(CString *cstr, const char *fmt, va_list ap)
{
	va_list v;
	int len, size = 80;
	for (;;) {
		size += cstr->size;
		if (size > cstr->size_allocated)
			cstr_realloc(cstr, size);
		size = cstr->size_allocated - cstr->size;
		va_copy(v, ap);
		len = vsnprintf(cstr->data + cstr->size, size, fmt, v);
		va_end(v);
		if (len >= 0 && len < size)
			break;
		size *= 2;
	}
	cstr->size += len;
	return len;
}

ST_FUNC int cstr_printf(CString *cstr, const char *fmt, ...)
{
	va_list ap;
	int len;
	va_start(ap, fmt);
	len = cstr_vprintf(cstr, fmt, ap);
	va_end(ap);
	return len;
}
/* XXX: unicode ? */

static void add_char(CString *cstr, int c)
{
	if (c == '\'' || c == '\"' || c == '\\') {
		/* XXX: could be more precise if char or string */

		cstr_ccat(cstr, '\\');
	}
	if (c >= 32 && c <= 126) {
		cstr_ccat(cstr, c);
	} else {
		cstr_ccat(cstr, '\\');
		if (c == '\n') {
			cstr_ccat(cstr, 'n');
		} else {
			cstr_ccat(cstr, '0' + ((c >> 6) & 7));
			cstr_ccat(cstr, '0' + ((c >> 3) & 7));
			cstr_ccat(cstr, '0' + (c & 7));
		}
	}
}
/* ------------------------------------------------------------------------- */
/* allocate a new token */

static TokenSym *tok_alloc_new(TokenSym **pts, const char *str, int len)
{
	TokenSym *ts, **ptable;
	int i;

	if (tok_ident >= SYM_FIRST_ANOM)
		tcc_error("memory full (symbols)");
	/* expand token table if needed */

	i = tok_ident - TOK_IDENT;
	if ((i % TOK_ALLOC_INCR) == 0) {
		ptable = tcc_realloc(table_ident, (i + TOK_ALLOC_INCR) * sizeof(TokenSym *));
		table_ident = ptable;
	}

	ts = tal_realloc(&toksym_alloc, 0, sizeof(TokenSym) + len);
	table_ident[i] = ts;
	ts->tok = tok_ident++;
	ts->sym_define = NULL;
	ts->sym_label = NULL;
	ts->sym_struct = NULL;
	ts->sym_identifier = NULL;
	ts->len = len;
	ts->hash_next = NULL;
	memcpy(ts->str, str, len);
	ts->str[len] = '\0';
	*pts = ts;
	return ts;
}

#define TOK_HASH_INIT 1
#define TOK_HASH_FUNC(h,c) ((h) + ((h) << 5) + ((h) >> 27) + (c))
/* find a token and add it if not found */

ST_FUNC TokenSym *tok_alloc(const char *str, int len)
{
	TokenSym *ts, **pts;
	int i;
	unsigned int h;

	h = TOK_HASH_INIT;
	for (i=0; i<len; i++)
		h = TOK_HASH_FUNC(h, ((unsigned char *)str)[i]);
	h &= (TOK_HASH_SIZE - 1);

	pts = &hash_ident[h];
	for (;;) {
		ts = *pts;
		if (!ts)
			break;
		if (ts->len == len && !memcmp(ts->str, str, len))
			return ts;
		pts = &(ts->hash_next);
	}
	return tok_alloc_new(pts, str, len);
}

ST_FUNC int tok_alloc_const(const char *str)
{
	return tok_alloc(str, strlen(str))->tok;
}
/* XXX: buffer overflow */
/* XXX: float tokens */

ST_FUNC const char *get_tok_str(int v, CValue *cv)
{
	char *p;
	int i, len;

	cstr_reset(&cstr_buf);
	p = cstr_buf.data;

	switch (v) {
	case TOK_CINT:
	case TOK_CUINT:
	case TOK_CLONG:
	case TOK_CULONG:
	case TOK_CLLONG:
	case TOK_CULLONG:
		/* XXX: not quite exact, but only useful for testing  */

		sprintf(p, "%llu", (unsigned long long)cv->i);
		break;
	case TOK_LCHAR:
		cstr_ccat(&cstr_buf, 'L');
	case TOK_CCHAR:
		cstr_ccat(&cstr_buf, '\'');
		add_char(&cstr_buf, cv->i);
		cstr_ccat(&cstr_buf, '\'');
		cstr_ccat(&cstr_buf, '\0');
		break;
	case TOK_PPNUM:
	case TOK_PPSTR:
		return (char *)cv->str.data;
	case TOK_LSTR:
		cstr_ccat(&cstr_buf, 'L');
	case TOK_STR:
		cstr_ccat(&cstr_buf, '\"');
		if (v == TOK_STR) {
			len = cv->str.size - 1;
			for (i=0; i<len; i++)
				add_char(&cstr_buf, ((unsigned char *)cv->str.data)[i]);
		} else {
			len = (cv->str.size / sizeof(nwchar_t)) - 1;
			for (i=0; i<len; i++)
				add_char(&cstr_buf, ((nwchar_t *)cv->str.data)[i]);
		}
		cstr_ccat(&cstr_buf, '\"');
		cstr_ccat(&cstr_buf, '\0');
		break;

	case TOK_CFLOAT:
		return strcpy(p, "<float>");
	case TOK_CDOUBLE:
		return strcpy(p, "<double>");
	case TOK_CLDOUBLE:
		return strcpy(p, "<long double>");
	case TOK_LINENUM:
		return strcpy(p, "<linenumber>");
	/* above tokens have value, the ones below don't */

	case TOK_LT:
		v = '<';
		goto addv;
	case TOK_GT:
		v = '>';
		goto addv;
	case TOK_DOTS:
		return strcpy(p, "...");
	case TOK_A_SHL:
		return strcpy(p, "<<=");
	case TOK_A_SAR:
		return strcpy(p, ">>=");
	case TOK_EOF:
		return strcpy(p, "<eof>");
	case 0:/* anonymous nameless symbols */

		return strcpy(p, "<no name>");
	default:
		v &= ~(SYM_FIELD | SYM_STRUCT);
		if (v < TOK_IDENT) {
			/* search in two bytes table */

			const unsigned char *q = tok_two_chars;
			while (*q) {
				if (q[2] == v) {
					*p++ = q[0];
					*p++ = q[1];
					*p = '\0';
					return cstr_buf.data;
				}
				q += 3;
			}
			if (v >= 127 || (v < 32 && !is_space(v) && v != '\n')) {
				sprintf(p, "<\\x%02x>", v);
				break;
			}
addv:
			*p++ = v;
			*p = '\0';
		} else if (v < tok_ident) {
			return table_ident[v - TOK_IDENT]->str;
		} else if (v >= SYM_FIRST_ANOM) {
			/* special name for anonymous symbol */

			sprintf(p, "L.%u", v - SYM_FIRST_ANOM);
		} else {
			/* should never happen */

			return NULL;
		}
		break;
	}
	return cstr_buf.data;
}
/* return the current character, handling end of block if necessary
   (but not stray) */

static int handle_eob(void)
{
	BufferedFile *bf = file;
	int len;
	/* only tries to read if really end of buffer */

	if (bf->buf_ptr >= bf->buf_end) {
		if (bf->fd >= 0) {

			len = IO_BUF_SIZE;

			len = read(bf->fd, bf->buffer, len);
			if (len < 0)
				len = 0;
		} else {
			len = 0;
		}
		total_bytes += len;
		bf->buf_ptr = bf->buffer;
		bf->buf_end = bf->buffer + len;
		*bf->buf_end = CH_EOB;
	}
	if (bf->buf_ptr < bf->buf_end) {
		return bf->buf_ptr[0];
	} else {
		bf->buf_ptr = bf->buf_end;
		return CH_EOF;
	}
}
/* read next char from current input file and handle end of input buffer */

static int next_c(void)
{
	int ch = *++file->buf_ptr;
	/* end of buffer/file handling */

	if (ch == CH_EOB && file->buf_ptr >= file->buf_end)
		ch = handle_eob();
	return ch;
}
/* input with '\[\r]\n' handling. */

static int handle_stray_noerror(int err)
{
	int ch;
	while ((ch = next_c()) == '\\') {
		ch = next_c();
		if (ch == '\n') {
newl:
			file->line_num++;
		} else {
			if (ch == '\r') {
				ch = next_c();
				if (ch == '\n')
					goto newl;
				*--file->buf_ptr = '\r';
			}
			if (err)
				tcc_error("stray '\\' in program");
			/* may take advantage of 'BufferedFile.unget[4}' */

			return *--file->buf_ptr = '\\';
		}
	}
	return ch;
}

#define ninp() handle_stray_noerror(0)
/* handle '\\' in strings, comments and skipped regions */

static int handle_bs(uint8_t **p)
{
	int c;
	file->buf_ptr = *p - 1;
	c = ninp();
	*p = file->buf_ptr;
	return c;
}
/* skip the stray and handle the \\n case. Output an error if
   incorrect char after the stray */

static int handle_stray(uint8_t **p)
{
	int c;
	file->buf_ptr = *p - 1;
	c = handle_stray_noerror(!(parse_flags & PARSE_FLAG_ACCEPT_STRAYS));
	*p = file->buf_ptr;
	return c;
}
/* handle the complicated stray case */

#define PEEKC(c,p) { c = *++p; if (c == '\\') c = handle_stray(&p); }

static int skip_spaces(void)
{
	int ch;
	--file->buf_ptr;
	do {
		ch = ninp();
	} while (isidnum_table[ch - CH_EOF] & IS_SPC);
	return ch;
}
/* single line C++ comments */

static uint8_t *parse_line_comment(uint8_t *p)
{
	int c;
	for (;;) {
		for (;;) {
			c = *++p;
redo:
			if (c == '\n' || c == '\\')
				break;
			c = *++p;
			if (c == '\n' || c == '\\')
				break;
		}
		if (c == '\n')
			break;
		c = handle_bs(&p);
		if (c == CH_EOF)
			break;
		if (c != '\\')
			goto redo;
	}
	return p;
}
/* C comments */

static uint8_t *parse_comment(uint8_t *p)
{
	int c;
	for (;;) {
		/* fast skip loop */

		for (;;) {
			c = *++p;
redo:
			if (c == '\n' || c == '*' || c == '\\')
				break;
			c = *++p;
			if (c == '\n' || c == '*' || c == '\\')
				break;
		}
		/* now we can handle all the cases */

		if (c == '\n') {
			file->line_num++;
		} else if (c == '*') {
			do {
				c = *++p;
			} while (c == '*');
			if (c == '\\')
				c = handle_bs(&p);
			if (c == '/')
				break;
			goto check_eof;
		} else {
			c = handle_bs(&p);
check_eof:
			if (c == CH_EOF)
				tcc_error("unexpected end of file in comment");
			if (c != '\\')
				goto redo;
		}
	}
	return p + 1;
}
/* parse a string without interpreting escapes */

static uint8_t *parse_pp_string(uint8_t *p, int sep, CString *str)
{
	int c;
	for (;;) {
		c = *++p;
redo:
		if (c == sep) {
			break;
		} else if (c == '\\') {
			c = handle_bs(&p);
			if (c == CH_EOF) {
unterminated_string:
				/* XXX: indicate line number of start of string */

				tok_flags &= ~TOK_FLAG_BOL;
				tcc_error("missing terminating %c character", sep);
			} else if (c == '\\') {
				if (str)
					cstr_ccat(str, c);
				c = *++p;
				/* add char after '\\' unconditionally */

				if (c == '\\') {
					c = handle_bs(&p);
					if (c == CH_EOF)
						goto unterminated_string;
				}
				goto add_char;
			} else {
				goto redo;
			}
		} else if (c == '\n') {
add_lf:
			if (ACCEPT_LF_IN_STRINGS) {
				file->line_num++;
				goto add_char;
			} else if (str) {/* not skipping */

				goto unterminated_string;
			} else {
//tcc_warning("missing terminating %c character", sep);

				return p;
			}
		} else if (c == '\r') {
			c = *++p;
			if (c == '\\')
				c = handle_bs(&p);
			if (c == '\n')
				goto add_lf;
			if (c == CH_EOF)
				goto unterminated_string;
			if (str)
				cstr_ccat(str, '\r');
			goto redo;
		} else {
add_char:
			if (str)
				cstr_ccat(str, c);
		}
	}
	p++;
	return p;
}
/* skip block of text until #else, #elif or #endif. skip also pairs of
   #if/#endif */

static void preprocess_skip(void)
{
	int a, start_of_line, c, in_warn_or_error;
	uint8_t *p;

	p = file->buf_ptr;
	a = 0;
redo_start:
	start_of_line = 1;
	in_warn_or_error = 0;
	for (;;) {
		c = *p;
		switch (c) {
		case ' ':
		case '\t':
		case '\f':
		case '\v':
		case '\r':
			p++;
			continue;
		case '\n':
			file->line_num++;
			p++;
			goto redo_start;
		case '\\':
			c = handle_bs(&p);
			if (c == CH_EOF)
				expect("#endif");
			if (c == '\\')
				++p;
			continue;
		/* skip strings */

		case '\"':
		case '\'':
			if (in_warn_or_error)
				goto _default;
			tok_flags &= ~TOK_FLAG_BOL;
			p = parse_pp_string(p, c, NULL);
			break;
		/* skip comments */

		case '/':
			if (in_warn_or_error)
				goto _default;
			++p;
			c = handle_bs(&p);
			if (c == '*') {
				p = parse_comment(p);
			} else if (c == '/') {
				p = parse_line_comment(p);
			}
			continue;
		case '#':
			p++;
			if (start_of_line) {
				file->buf_ptr = p;
				next_nomacro();
				p = file->buf_ptr;
				if (a == 0 &&
				    (tok == TOK_ELSE || tok == TOK_ELIF || tok == TOK_ENDIF))
					goto the_end;
				if (tok == TOK_IF || tok == TOK_IFDEF || tok == TOK_IFNDEF)
					a++;
				else if (tok == TOK_ENDIF)
					a--;
				else if ( tok == TOK_ERROR || tok == TOK_WARNING)
					in_warn_or_error = 1;
				else if (tok == TOK_LINEFEED)
					goto redo_start;
				else if (parse_flags & PARSE_FLAG_ASM_FILE)
					p = parse_line_comment(p - 1);
			}

			else if (parse_flags & PARSE_FLAG_ASM_FILE)
				p = parse_line_comment(p - 1);

			break;
_default:
		default:
			p++;
			break;
		}
		start_of_line = 0;
	}
the_end: ;
	file->buf_ptr = p;
}
/* token string handling */
ST_INLN void tok_str_new(TokenString *s)
{
	s->str = NULL;
	s->len = s->need_spc = 0;
	s->allocated_len = 0;
	s->last_line_num = -1;
}

ST_FUNC TokenString *tok_str_alloc(void)
{
	TokenString *str = tal_realloc(&tokstr_alloc, 0, sizeof *str);
	tok_str_new(str);
	return str;
}

ST_FUNC void tok_str_free_str(int *str)
{
	tal_free(&tokstr_alloc, str);
}

ST_FUNC void tok_str_free(TokenString *str)
{
	tok_str_free_str(str->str);
	tal_free(&tokstr_alloc, str);
}

ST_FUNC int *tok_str_realloc(TokenString *s, int new_size)
{
	int *str, size;

	size = s->allocated_len;
	if (size < 16)
		size = 16;
	while (size < new_size)
		size = size * 2;
	if (size > s->allocated_len) {
		str = tal_realloc(&tokstr_alloc, s->str, size * sizeof(int));
		s->allocated_len = size;
		s->str = str;
	}
	return s->str;
}

ST_FUNC void tok_str_add(TokenString *s, int t)
{
	int len, *str;

	len = s->len;
	str = s->str;
	if (len >= s->allocated_len)
		str = tok_str_realloc(s, len + 1);
	str[len++] = t;
	s->len = len;
}

ST_FUNC void begin_macro(TokenString *str, int alloc)
{
	str->alloc = alloc;
	str->prev = macro_stack;
	str->prev_ptr = macro_ptr;
	str->save_line_num = file->line_num;
	macro_ptr = str->str;
	macro_stack = str;
}

ST_FUNC void end_macro(void)
{
	TokenString *str = macro_stack;
	macro_stack = str->prev;
	macro_ptr = str->prev_ptr;
	file->line_num = str->save_line_num;
	if (str->alloc == 0) {
		/* matters if str not alloced, may be tokstr_buf */

		str->len = str->need_spc = 0;
	} else {
		if (str->alloc == 2)
			str->str = NULL;/* don't free */

		tok_str_free(str);
	}
}

static void tok_str_add2(TokenString *s, int t, CValue *cv)
{
	int len, *str;

	len = s->len;
	str = s->str;
	/* allocate space for worst case */

	if (len + TOK_MAX_SIZE >= s->allocated_len)
		str = tok_str_realloc(s, len + TOK_MAX_SIZE + 1);
	str[len++] = t;
	switch (t) {
	case TOK_CINT:
	case TOK_CUINT:
	case TOK_CCHAR:
	case TOK_LCHAR:
	case TOK_CFLOAT:
	case TOK_LINENUM:
	case TOK_CLONG:
	case TOK_CULONG:

		str[len++] = cv->tab[0];
		break;
	case TOK_PPNUM:
	case TOK_PPSTR:
	case TOK_STR:
	case TOK_LSTR: {
		/* Insert the string into the int array. */

		size_t nb_words =
			1 + (cv->str.size + sizeof(int) - 1) / sizeof(int);
		if (len + nb_words >= s->allocated_len)
			str = tok_str_realloc(s, len + nb_words + 1);
		str[len] = cv->str.size;
		memcpy(&str[len + 1], cv->str.data, cv->str.size);
		len += nb_words;
	}
	break;
	case TOK_CDOUBLE:
	case TOK_CLLONG:
	case TOK_CULLONG:

		str[len++] = cv->tab[0];
		str[len++] = cv->tab[1];
		break;
	case TOK_CLDOUBLE:
		str[len++] = cv->tab[0];
		str[len++] = cv->tab[1];
		if (LDOUBLE_WORDS >= 3)
			str[len++] = cv->tab[2];
		if (LDOUBLE_WORDS >= 4)
			str[len++] = cv->tab[3];
	default:
		break;
	}
	s->len = len;
}
/* add the current parse token in token string 's' */

ST_FUNC void tok_str_add_tok(TokenString *s)
{
	CValue cval;
	/* save line number info */

	if (file->line_num != s->last_line_num) {
		s->last_line_num = file->line_num;
		cval.i = s->last_line_num;
		tok_str_add2(s, TOK_LINENUM, &cval);
	}
	tok_str_add2(s, tok, &tokc);
}
/* like tok_str_add2(), add a space if needed */

static void tok_str_add2_spc(TokenString *s, int t, CValue *cv)
{
	if (s->need_spc == 3)
		tok_str_add(s, ' ');
	s->need_spc = 2;
	tok_str_add2(s, t, cv);
}
/* get a token from an integer array and increment pointer. */

static inline void tok_get(int *t, const int **pp, CValue *cv)
{
	const int *p = *pp;
	int n, *tab;

	tab = cv->tab;
	switch (*t = *p++) {
	case TOK_CLONG:

	case TOK_CINT:
	case TOK_CCHAR:
	case TOK_LCHAR:
	case TOK_LINENUM:
		cv->i = *p++;
		break;
	case TOK_CULONG:

	case TOK_CUINT:
		cv->i = (unsigned)*p++;
		break;
	case TOK_CFLOAT:
		tab[0] = *p++;
		break;
	case TOK_STR:
	case TOK_LSTR:
	case TOK_PPNUM:
	case TOK_PPSTR:
		cv->str.size = *p++;
		cv->str.data = (char *)p;
		p += (cv->str.size + sizeof(int) - 1) / sizeof(int);
		break;
	case TOK_CDOUBLE:
	case TOK_CLLONG:
	case TOK_CULLONG:

		n = 2;
		goto copy;
	case TOK_CLDOUBLE:
		n = LDOUBLE_WORDS;
copy:
		do
			*tab++ = *p++;
		while (--n);
		break;
	default:
		break;
	}
	*pp = p;
}
#define TOK_GET(t,p,c) do { int _t = **(p); if (TOK_HAS_VALUE(_t)) tok_get(t, p, c); else *(t) = _t, ++*(p); } while (0)

static int macro_is_equal(const int *a, const int *b)
{
	CValue cv;
	int t;

	if (!a || !b)
		return 1;

	while (*a && *b) {
		cstr_reset(&tokcstr);
		TOK_GET(&t, &a, &cv);
		cstr_cat(&tokcstr, get_tok_str(t, &cv), 0);
		TOK_GET(&t, &b, &cv);
		if (strcmp(tokcstr.data, get_tok_str(t, &cv)))
			return 0;
	}
	return !(*a || *b);
}
/* defines handling */

ST_INLN void define_push(int v, int macro_type, int *str, Sym *first_arg)
{
	Sym *s, *o;

	o = define_find(v);
	s = sym_push2(&define_stack, v, macro_type, 0);
	s->d = str;
	s->next = first_arg;
	table_ident[v - TOK_IDENT]->sym_define = s;

	if (o && !macro_is_equal(o->d, s->d))
		tcc_warning("%s redefined", get_tok_str(v, NULL));
}
/* undefined a define symbol. Its name is just set to zero */

ST_FUNC void define_undef(Sym *s)
{
	int v = s->v;
	if (v >= TOK_IDENT && v < tok_ident)
		table_ident[v - TOK_IDENT]->sym_define = NULL;
}

ST_INLN Sym *define_find(int v)
{
	v -= TOK_IDENT;
	if ((unsigned)v >= (unsigned)(tok_ident - TOK_IDENT))
		return NULL;
	return table_ident[v]->sym_define;
}
/* free define stack until top reaches 'b' */

ST_FUNC void free_defines(Sym *b)
{
	while (define_stack != b) {
		Sym *top = define_stack;
		define_stack = top->prev;
		tok_str_free_str(top->d);
		define_undef(top);
		sym_free(top);
	}
}
/* fake the nth "#if defined test_..." for tcc -dt -run */

static void maybe_run_test(TCCState *s)
{
	const char *p;
	if (s->include_stack_ptr != s->include_stack)
		return;
	p = get_tok_str(tok, NULL);
	if (0 != memcmp(p, "test_", 5))
		return;
	if (0 != --s->run_test)
		return;
	fprintf(s->ppfp, &"\n[%s]\n"[!(s->dflag & 32)], p), fflush(s->ppfp);
	define_push(tok, MACRO_OBJ, NULL, NULL);
}

ST_FUNC void skip_to_eol(int warn)
{
	if (tok == TOK_LINEFEED)
		return;
	if (warn)
		tcc_warning("extra tokens after directive");
	while (macro_stack)
		end_macro();
	file->buf_ptr = parse_line_comment(file->buf_ptr - 1);
	next_nomacro();
}

static CachedInclude *
search_cached_include(TCCState *s1, const char *filename, int add);

static int parse_include(TCCState *s1, int do_next, int test)
{
	int c, i;
	char name[1024], buf[1024], *p;
	CachedInclude *e;

	c = skip_spaces();
	if (c == '<' || c == '\"') {
		cstr_reset(&tokcstr);
		file->buf_ptr = parse_pp_string(file->buf_ptr, c == '<' ? '>' : c, &tokcstr);
		i = tokcstr.size;
		pstrncpy(name, sizeof name, tokcstr.data, i);
		next_nomacro();
	} else {
		/* computed #include : concatenate tokens until result is one of
		           the two accepted forms.  Don't convert pp-tokens to tokens here. */

		parse_flags = PARSE_FLAG_PREPROCESS
			      | PARSE_FLAG_LINEFEED
			      | (parse_flags & PARSE_FLAG_ASM_FILE);
		name[0] = 0;
		for (;;) {
			next();
			p = name, i = strlen(p) - 1;
			if (i > 0
			    && ((p[0] == '"' && p[i] == '"')
				|| (p[0] == '<' && p[i] == '>')))
				break;
			if (tok == TOK_LINEFEED)
				tcc_error("'#include' expects \"FILENAME\" or <FILENAME>");
			pstrcat(name, sizeof name, get_tok_str(tok, &tokc));
		}
		c = p[0];
		/* remove '<>|""' */

		memmove(p, p + 1, i - 1), p[i - 1] = 0;
	}

	if (!test)
		skip_to_eol(1);

	i = do_next ? file->include_next_index : -1;
	for (;;) {
		++i;
		if (i == 0) {
			/* check absolute include path */

			if (!IS_ABSPATH(name))
				continue;
			buf[0] = '\0';
		} else if (i == 1) {
			/* search in file's dir if "header.h" */

			if (c != '\"')
				continue;
			p = file->true_filename;
			pstrncpy(buf, sizeof buf, p, tcc_basename(p) - p);
		} else {
			int j = i - 2, k = j - s1->nb_include_paths;
			if (k < 0)
				p = s1->include_paths[j];
			else if (k < s1->nb_sysinclude_paths)
				p = s1->sysinclude_paths[k];
			else if (test)
				return 0;
			else
				tcc_error("include file '%s' not found", name);
			pstrcpy(buf, sizeof buf, p);
			pstrcat(buf, sizeof buf, "/");
		}
		pstrcat(buf, sizeof buf, name);
		e = search_cached_include(s1, buf, 0);
		if (e && (define_find(e->ifndef_macro) || e->once)) {
			/* no need to parse the include because the 'ifndef macro'
			               is defined (or had #pragma once) */

			if ((s1->verbose | 1) == 3)/* -vv[v] */

				printf("=> %*s%s\n",
				       (int)(s1->include_stack_ptr - s1->include_stack), "", buf);
			return 1;
		}
		if (tcc_open(s1, buf) >= 0)
			break;
	}

	if (test) {
		tcc_close();
	} else {
		if (s1->include_stack_ptr >= s1->include_stack + INCLUDE_STACK_SIZE)
			tcc_error("#include recursion too deep");
		/* push previous file on stack */

		*s1->include_stack_ptr++ = file->prev;
		file->include_next_index = i;
		/* update target deps */

		if (s1->gen_deps) {
			BufferedFile *bf = file;
			while (i == 1 && (bf = bf->prev))
				i = bf->include_next_index;
			/* skip system include files */

			if (s1->include_sys_deps || i - 2 < s1->nb_include_paths)
				dynarray_add(&s1->target_deps, &s1->nb_target_deps,
					     tcc_strdup(buf));
		}
		/* add include file debug info */

		tcc_debug_bincl(s1);
	}
	return 1;
}
/* eval an expression for #if/#elif */

static int expr_preprocess(TCCState *s1)
{
	int c, t;
	int t0 = tok;
	TokenString *str;

	str = tok_str_alloc();
	pp_expr = 1;
	while (1) {
		next();/* do macro subst */

		t = tok;
		if (tok < TOK_IDENT) {
			if (tok == TOK_LINEFEED || tok == TOK_EOF)
				break;
			if (tok >= TOK_STR && tok <= TOK_CLDOUBLE)
				tcc_error("invalid constant in preprocessor expression");

		} else if (tok == TOK_DEFINED) {
			parse_flags &= ~PARSE_FLAG_PREPROCESS;/* no macro subst */

			next();
			t = tok;
			if (t == '(')
				next();
			parse_flags |= PARSE_FLAG_PREPROCESS;
			if (tok < TOK_IDENT)
				expect("identifier after 'defined'");
			if (s1->run_test)
				maybe_run_test(s1);
			c = 0;
			if (define_find(tok)
			    || tok == TOK___HAS_INCLUDE
			    || tok == TOK___HAS_INCLUDE_NEXT)
				c = 1;
			if (t == '(') {
				next();
				if (tok != ')')
					expect("')'");
			}
			goto c_number;
		} else if (tok == TOK___HAS_INCLUDE ||
			   tok == TOK___HAS_INCLUDE_NEXT) {
			t = tok;
			next();
			if (tok != '(')
				expect("'('");
			c = parse_include(s1, t - TOK___HAS_INCLUDE, 1);
			if (tok != ')')
				expect("')'");
			goto c_number;
		} else {
			/* if undefined macro, replace with zero */

			c = 0;
c_number:
			tok = TOK_CLLONG;/* type intmax_t */

			tokc.i = c;
		}
		tok_str_add_tok(str);
	}
	if (0 == str->len)
		tcc_error("#%s with no expression", get_tok_str(t0, 0));
	tok_str_add(str, TOK_EOF);/* simulate end of file */

	pp_expr = t0;/* redirect pre-processor expression error messages */

	t = tok;
	/* now evaluate C constant expression */

	begin_macro(str, 1);
	next();
	c = expr_const();
	if (tok != TOK_EOF)
		tcc_error("...");
	pp_expr = 0;
	end_macro();
	tok = t;/* restore LF or EOF */

	return c != 0;
}

ST_FUNC void pp_error(CString *cs)
{
	cstr_printf(cs, "bad preprocessor expression: #%s", get_tok_str(pp_expr, 0));
	macro_ptr = macro_stack->str;
	while (next(), tok != TOK_EOF)
		cstr_printf(cs, " %s", get_tok_str(tok, &tokc));
}
/* parse after #define */

ST_FUNC void parse_define(void)
{
	Sym *s, *first, **ps;
	int v, t, varg, is_vaargs, t0;
	int saved_parse_flags = parse_flags;
	TokenString str;

	v = tok;
	if (v < TOK_IDENT || v == TOK_DEFINED)
		tcc_error("invalid macro name '%s'", get_tok_str(tok, &tokc));
	first = NULL;
	t = MACRO_OBJ;
	/* We have to parse the whole define as if not in asm mode, in particular
	       no line comment with '#' must be ignored.  Also for function
	       macros the argument list must be parsed without '.' being an ID
	       character.  */

	parse_flags = ((parse_flags & ~PARSE_FLAG_ASM_FILE) | PARSE_FLAG_SPACES);
	/* '(' must be just after macro definition for MACRO_FUNC */

	next_nomacro();
	parse_flags &= ~PARSE_FLAG_SPACES;
	is_vaargs = 0;
	if (tok == '(') {
		int dotid = set_idnum('.', 0);
		next_nomacro();
		ps = &first;
		if (tok != ')')
			for (;;) {
				varg = tok;
				next_nomacro();
				is_vaargs = 0;
				if (varg == TOK_DOTS) {
					varg = TOK___VA_ARGS__;
					is_vaargs = 1;
				} else if (tok == TOK_DOTS && gnu_ext) {
					is_vaargs = 1;
					next_nomacro();
				}
				if (varg < TOK_IDENT)
bad_list:
					tcc_error("bad macro parameter list");
				s = sym_push2(&define_stack, varg | SYM_FIELD, is_vaargs, 0);
				*ps = s;
				ps = &s->next;
				if (tok == ')')
					break;
				if (tok != ',' || is_vaargs)
					goto bad_list;
				next_nomacro();
			}
		parse_flags |= PARSE_FLAG_SPACES;
		next_nomacro();
		t = MACRO_FUNC;
		set_idnum('.', dotid);
	}
	/* The body of a macro definition should be parsed such that identifiers
	       are parsed like the file mode determines (i.e. with '.' being an
	       ID character in asm mode).  But '#' should be retained instead of
	       regarded as line comment leader, so still don't set ASM_FILE
	       in parse_flags. */

	parse_flags |= PARSE_FLAG_ACCEPT_STRAYS | PARSE_FLAG_SPACES |
		       PARSE_FLAG_LINEFEED;
	tok_str_new(&str);
	t0 = 0;
	while (tok != TOK_LINEFEED && tok != TOK_EOF) {
		if (is_space(tok)) {
			str.need_spc |= 1;
		} else {
			if (TOK_TWOSHARPS == tok) {
				if (0 == t0)
					goto bad_twosharp;
				tok = TOK_PPJOIN;
				t |= MACRO_JOIN;
			}
			tok_str_add2_spc(&str, tok, &tokc);
			t0 = tok;
		}
		next_nomacro();
	}
	parse_flags = saved_parse_flags;
	tok_str_add(&str, 0);
	if (t0 == TOK_PPJOIN)
bad_twosharp:
		tcc_error("'##' cannot appear at either end of macro");
	define_push(v, t, str.str, first);
//tok_print(str.str, "#define (%d) %s %d:", t | is_vaargs * 4, get_tok_str(v, 0));

}

static CachedInclude *search_cached_include(TCCState *s1, const char *filename,
		int add)
{
	const char *s, *basename;
	unsigned int h;
	CachedInclude *e;
	int c, i, len;

	s = basename = tcc_basename(filename);
	h = TOK_HASH_INIT;
	while ((c = (unsigned char)*s) != 0) {

		h = TOK_HASH_FUNC(h, toup(c));

		s++;
	}
	h &= (CACHED_INCLUDES_HASH_SIZE - 1);

	i = s1->cached_includes_hash[h];
	for (;;) {
		if (i == 0)
			break;
		e = s1->cached_includes[i - 1];
		if (0 == PATHCMP(filename, e->filename))
			return e;
		if (e->once
		    && 0 == PATHCMP(basename, tcc_basename(e->filename))
		    && 0 == normalized_PATHCMP(filename, e->filename)
		   )
			return e;
		i = e->hash_next;
	}
	if (!add)
		return NULL;

	e = tcc_malloc(sizeof(CachedInclude) + (len = strlen(filename)));
	memcpy(e->filename, filename, len + 1);
	e->ifndef_macro = e->once = 0;
	dynarray_add(&s1->cached_includes, &s1->nb_cached_includes, e);
	/* add in hash table */

	e->hash_next = s1->cached_includes_hash[h];
	s1->cached_includes_hash[h] = s1->nb_cached_includes;

	return e;
}

static int pragma_parse(TCCState *s1)
{
	next_nomacro();
	if (tok == TOK_push_macro || tok == TOK_pop_macro) {
		int t = tok, v;
		Sym *s;

		if (next(), tok != '(')
			goto pragma_err;
		if (next(), tok != TOK_STR)
			goto pragma_err;
		v = tok_alloc(tokc.str.data, tokc.str.size - 1)->tok;
		if (next(), tok != ')')
			goto pragma_err;
		if (t == TOK_push_macro) {
			while (NULL == (s = define_find(v)))
				define_push(v, 0, NULL, NULL);
			s->type.ref = s;/* set push boundary */

		} else {
			for (s = define_stack; s; s = s->prev)
				if (s->v == v && s->type.ref == s) {
					s->type.ref = NULL;
					break;
				}
		}
		if (s)
			table_ident[v - TOK_IDENT]->sym_define = s->d ? s : NULL;
		else
			tcc_warning("unbalanced #pragma pop_macro");
		pp_debug_tok = t, pp_debug_symv = v;

	} else if (tok == TOK_once) {
		search_cached_include(s1, file->true_filename, 1)->once = 1;

	} else if (s1->output_type == TCC_OUTPUT_PREPROCESS) {
		/* tcc -E: keep pragmas below unchanged */

		unget_tok(' ');
		unget_tok(TOK_PRAGMA);
		unget_tok('#');
		unget_tok(TOK_LINEFEED);
		return 1;

	} else if (tok == TOK_pack) {
		/* This may be:
		           #pragma pack(1) // set
		           #pragma pack() // reset to default
		           #pragma pack(push) // push current
		           #pragma pack(push,1) // push & set
		           #pragma pack(pop) // restore previous */

		next();
		skip('(');
		if (tok == TOK_ASM_pop) {
			next();
			if (s1->pack_stack_ptr <= s1->pack_stack) {
stk_error:
				tcc_error("out of pack stack");
			}
			s1->pack_stack_ptr--;
		} else {
			int val = 0;
			if (tok != ')') {
				if (tok == TOK_ASM_push) {
					next();
					if (s1->pack_stack_ptr >= s1->pack_stack + PACK_STACK_SIZE - 1)
						goto stk_error;
					val = *s1->pack_stack_ptr++;
					if (tok != ',')
						goto pack_set;
					next();
				}
				if (tok != TOK_CINT)
					goto pragma_err;
				val = tokc.i;
				if (val < 1 || val > 16 || (val & (val - 1)) != 0)
					goto pragma_err;
				next();
			}
pack_set:
			*s1->pack_stack_ptr = val;
		}
		if (tok != ')')
			goto pragma_err;

	} else if (tok == TOK_comment) {
		char *p;
		int t;
		next();
		skip('(');
		t = tok;
		next();
		skip(',');
		if (tok != TOK_STR)
			goto pragma_err;
		p = tcc_strdup(tokc.str.data);
		next();
		if (tok != ')')
			goto pragma_err;
		if (t == TOK_lib) {
			dynarray_add(&s1->pragma_libs, &s1->nb_pragma_libs, p);
		} else {
			if (t == TOK_option)
				tcc_set_options(s1, p);
			tcc_free(p);
		}

	} else {
		tcc_warning_c(warn_all)("#pragma %s ignored", get_tok_str(tok, &tokc));
		return 0;
	}
	next();
	return 1;
pragma_err:
	tcc_error("malformed #pragma directive");
}
/* put alternative filename */

ST_FUNC void tccpp_putfile(const char *filename)
{
	char buf[1024];
	buf[0] = 0;
	if (!IS_ABSPATH(filename)) {
		/* prepend directory from real file */

		pstrcpy(buf, sizeof buf, file->true_filename);
		*tcc_basename(buf) = 0;
	}
	pstrcat(buf, sizeof buf, filename);

	normalize_slashes(buf);

	if (0 == strcmp(file->filename, buf))
		return;
//printf("new file '%s'\n", buf);

	if (file->true_filename == file->filename)
		file->true_filename = tcc_strdup(file->filename);
	pstrcpy(file->filename, sizeof file->filename, buf);
	tcc_debug_newfile(tcc_state);
}
/* is_bof is true if first non space token at beginning of file */

ST_FUNC void preprocess(int is_bof)
{
	TCCState *s1 = tcc_state;
	int c, n, saved_parse_flags;
	char buf[1024], *q;
	Sym *s;

	saved_parse_flags = parse_flags;
	parse_flags = PARSE_FLAG_PREPROCESS
		      | PARSE_FLAG_TOK_NUM
		      | PARSE_FLAG_TOK_STR
		      | PARSE_FLAG_LINEFEED
		      | (parse_flags & PARSE_FLAG_ASM_FILE)
		      ;

	next_nomacro();
redo:
	switch (tok) {
	case TOK_DEFINE:
		pp_debug_tok = tok;
		next_nomacro();
		pp_debug_symv = tok;
		parse_define();
		break;
	case TOK_UNDEF:
		pp_debug_tok = tok;
		next_nomacro();
		pp_debug_symv = tok;
		s = define_find(tok);
		/* undefine symbol by putting an invalid name */

		if (s)
			define_undef(s);
		next_nomacro();
		break;
	case TOK_INCLUDE:
	case TOK_INCLUDE_NEXT:
		parse_include(s1, tok - TOK_INCLUDE, 0);
		goto the_end;
	case TOK_IFNDEF:
		c = 1;
		goto do_ifdef;
	case TOK_IF:
		c = expr_preprocess(s1);
		goto do_if;
	case TOK_IFDEF:
		c = 0;
do_ifdef:
		next_nomacro();
		if (tok < TOK_IDENT)
			tcc_error("invalid argument for '#if%sdef'", c ? "n" : "");
		if (is_bof) {
			if (c) {

				file->ifndef_macro = tok;
			}
		}
		if (define_find(tok)
		    || tok == TOK___HAS_INCLUDE
		    || tok == TOK___HAS_INCLUDE_NEXT)
			c ^= 1;
		next_nomacro();
do_if:
		if (s1->ifdef_stack_ptr >= s1->ifdef_stack + IFDEF_STACK_SIZE)
			tcc_error("memory full (ifdef)");
		*s1->ifdef_stack_ptr++ = c;
		goto test_skip;
	case TOK_ELSE:
		next_nomacro();
		if (s1->ifdef_stack_ptr == s1->ifdef_stack)
			tcc_error("#else without matching #if");
		if (s1->ifdef_stack_ptr[-1] & 2)
			tcc_error("#else after #else");
		c = (s1->ifdef_stack_ptr[-1] ^= 3);
		goto test_else;
	case TOK_ELIF:
		if (s1->ifdef_stack_ptr == s1->ifdef_stack)
			tcc_error("#elif without matching #if");
		c = s1->ifdef_stack_ptr[-1];
		if (c > 1)
			tcc_error("#elif after #else");
		/* last #if/#elif expression was true: we skip */

		if (c == 1) {
			skip_to_eol(0);
			c = 0;
		} else {
			c = expr_preprocess(s1);
			s1->ifdef_stack_ptr[-1] = c;
		}
test_else:
		if (s1->ifdef_stack_ptr == file->ifdef_stack_ptr + 1)
			file->ifndef_macro = 0;
test_skip:
		if (!(c & 1)) {
			skip_to_eol(1);
			preprocess_skip();
			is_bof = 0;
			goto redo;
		}
		break;
	case TOK_ENDIF:
		next_nomacro();
		if (s1->ifdef_stack_ptr <= file->ifdef_stack_ptr)
			tcc_error("#endif without matching #if");
		s1->ifdef_stack_ptr--;
		/* '#ifndef macro' was at the start of file. Now we check if
		           an '#endif' is exactly at the end of file */

		if (file->ifndef_macro &&
		    s1->ifdef_stack_ptr == file->ifdef_stack_ptr) {
			file->ifndef_macro_saved = file->ifndef_macro;
			/* need to set to zero to avoid false matches if another
			               #ifndef at middle of file */

			file->ifndef_macro = 0;
			tok_flags |= TOK_FLAG_ENDIF;
		}
		break;

	case TOK_LINE:
		parse_flags &= ~PARSE_FLAG_TOK_NUM;
		next();
		if (tok != TOK_PPNUM) {
_line_err:
			tcc_error("wrong #line format");
		}
		c = 1;
		goto _line_num;
	case TOK_PPNUM:
		if (parse_flags & PARSE_FLAG_ASM_FILE)
			goto ignore;
		c = 0;/* no error with extra tokens */

_line_num:
		for (n = 0, q = tokc.str.data; *q; ++q) {
			if (!isnum(*q))
				goto _line_err;
			n = n * 10 + *q - '0';
		}
		parse_flags &= ~PARSE_FLAG_TOK_STR;/* don't parse escape sequences */

		next();
		if (tok != TOK_LINEFEED) {
			if (tok != TOK_PPSTR || tokc.str.data[0] != '"')
				goto _line_err;
			tokc.str.data[tokc.str.size - 2] = 0;
			tccpp_putfile(tokc.str.data + 1);
			next();
			/* skip optional level number & advance to next line */

			skip_to_eol(c);
		}
		if (file->fd > 0)
			total_lines += file->line_num - n;
		file->line_num = n;
		break;

	case TOK_ERROR:
	case TOK_WARNING: {
		q = buf;
		c = skip_spaces();
		while (c != '\n' && c != CH_EOF) {
			if ((q - buf) < sizeof(buf) - 1)
				*q++ = c;
			c = ninp();
		}
		*q = '\0';
		if (tok == TOK_ERROR)
			tcc_error("#error %s", buf);
		else
			tcc_warning("#warning %s", buf);
		next_nomacro();
		break;
	}
	case TOK_PRAGMA:
		if (!pragma_parse(s1))
			goto ignore;
		break;
	case TOK_LINEFEED:
		goto the_end;
	default:
		/* ignore gas line comment in an 'S' file. */

		if (saved_parse_flags & PARSE_FLAG_ASM_FILE)
			goto ignore;
		if (tok == '!' && is_bof)
			/* '#!' is ignored at beginning to allow C scripts. */

			goto ignore;
		tcc_warning("ignoring unknown preprocessing directive #%s", get_tok_str(tok,
				&tokc));
ignore:
		skip_to_eol(0);
		goto the_end;
	}
	skip_to_eol(1);
the_end:
	parse_flags = saved_parse_flags;
}
/* evaluate escape codes in a string. */

static void parse_escape_string(CString *outstr, const uint8_t *buf,
				int is_long)
{
	int c, n, i;
	const uint8_t *p;

	p = buf;
	for (;;) {
		c = *p;
		if (c == '\0')
			break;
		if (c == '\\') {
			p++;
			/* escape */

			c = *p;
			switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				/* at most three octal digits */

				n = c - '0';
				p++;
				c = *p;
				if (isoct(c)) {
					n = n * 8 + c - '0';
					p++;
					c = *p;
					if (isoct(c)) {
						n = n * 8 + c - '0';
						p++;
					}
				}
				c = n;
				goto add_char_nonext;
			case 'x':
				i = 0;
				goto parse_hex_or_ucn;
			case 'u':
				i = 4;
				goto parse_hex_or_ucn;
			case 'U':
				i = 8;
				goto parse_hex_or_ucn;
parse_hex_or_ucn:
				p++;
				n = 0;
				do {
					c = *p;
					if (c >= 'a' && c <= 'f')
						c = c - 'a' + 10;
					else if (c >= 'A' && c <= 'F')
						c = c - 'A' + 10;
					else if (isnum(c))
						c = c - '0';
					else if (i >= 0)
						expect("more hex digits in universal-character-name");
					else
						goto add_hex_or_ucn;
					n = (unsigned) n * 16 + c;
					p++;
				} while (--i);
				if (is_long) {
add_hex_or_ucn:
					c = n;
					goto add_char_nonext;
				}
				cstr_u8cat(outstr, n);
				continue;
			case 'a':
				c = '\a';
				break;
			case 'b':
				c = '\b';
				break;
			case 'f':
				c = '\f';
				break;
			case 'n':
				c = '\n';
				break;
			case 'r':
				c = '\r';
				break;
			case 't':
				c = '\t';
				break;
			case 'v':
				c = '\v';
				break;
			case 'e':
				if (!gnu_ext)
					goto invalid_escape;
				c = 27;
				break;
			case '\'':
			case '\"':
			case '\\':
			case '?':
				break;
			default:
invalid_escape:
				if (c >= '!' && c <= '~')
					tcc_warning("unknown escape sequence: \'\\%c\'", c);
				else
					tcc_warning("unknown escape sequence: \'\\x%x\'", c);
				break;
			}
		} else if (is_long && c >= 0x80) {
			/* assume we are processing UTF-8 sequence */
			/* reference: The Unicode Standard, Version 10.0, ch3.9 */

			int cont;/* count of continuation bytes */

			int skip;/* how many bytes should skip when error occurred */

			int i;
			/* decode leading byte */

			if (c < 0xC2) {
				skip = 1;
				goto invalid_utf8_sequence;
			} else if (c <= 0xDF) {
				cont = 1;
				n = c & 0x1f;
			} else if (c <= 0xEF) {
				cont = 2;
				n = c & 0xf;
			} else if (c <= 0xF4) {
				cont = 3;
				n = c & 0x7;
			} else {
				skip = 1;
				goto invalid_utf8_sequence;
			}
			/* decode continuation bytes */

			for (i = 1; i <= cont; i++) {
				int l = 0x80, h = 0xBF;
				/* adjust limit for second byte */

				if (i == 1) {
					switch (c) {
					case 0xE0:
						l = 0xA0;
						break;
					case 0xED:
						h = 0x9F;
						break;
					case 0xF0:
						l = 0x90;
						break;
					case 0xF4:
						h = 0x8F;
						break;
					}
				}

				if (p[i] < l || p[i] > h) {
					skip = i;
					goto invalid_utf8_sequence;
				}

				n = (n << 6) | (p[i] & 0x3f);
			}
			/* advance pointer */

			p += 1 + cont;
			c = n;
			goto add_char_nonext;
			/* error handling */

invalid_utf8_sequence:
			tcc_warning("ill-formed UTF-8 subsequence starting with: \'\\x%x\'", c);
			c = 0xFFFD;
			p += skip;
			goto add_char_nonext;

		}
		p++;
add_char_nonext:
		if (!is_long)
			cstr_ccat(outstr, c);
		else {

			/* store as UTF-16 */

			if (c < 0x10000) {
				cstr_wccat(outstr, c);
			} else {
				c -= 0x10000;
				cstr_wccat(outstr, (c >> 10) + 0xD800);
				cstr_wccat(outstr, (c & 0x3FF) + 0xDC00);
			}

		}
	}
	/* add a trailing '\0' */

	if (!is_long)
		cstr_ccat(outstr, '\0');
	else
		cstr_wccat(outstr, '\0');
}

static void parse_string(const char *s, int len)
{
	uint8_t buf[1000], *p = buf;
	int is_long, sep;

	if ((is_long = *s == 'L'))
		++s, --len;
	sep = *s++;
	len -= 2;
	if (len >= sizeof buf)
		p = tcc_malloc(len + 1);
	memcpy(p, s, len);
	p[len] = 0;

	cstr_reset(&tokcstr);
	parse_escape_string(&tokcstr, p, is_long);
	if (p != buf)
		tcc_free(p);

	if (sep == '\'') {
		int char_size, i, n, c;
		/* XXX: make it portable */

		if (!is_long)
			tok = TOK_CCHAR, char_size = 1;
		else
			tok = TOK_LCHAR, char_size = sizeof(nwchar_t);
		n = tokcstr.size / char_size - 1;
		if (n < 1)
			tcc_error("empty character constant");
		if (n > 1)
			tcc_warning_c(warn_all)("multi-character character constant");
		for (c = i = 0; i < n; ++i) {
			if (is_long)
				c = ((nwchar_t *)tokcstr.data)[i];
			else
				c = (c << 8) | ((char *)tokcstr.data)[i];
		}
		tokc.i = c;
	} else {
		tokc.str.size = tokcstr.size;
		tokc.str.data = tokcstr.data;
		if (!is_long)
			tok = TOK_STR;
		else
			tok = TOK_LSTR;
	}
}
/* we use 128 bit (64/112 needed) numbers */

#define BN_SIZE 4
/* bn = (bn << shift) | or_val */

static int bn_lshift(unsigned int *bn, int shift, int or_val)
{
	int i;
	unsigned int v;
	if (bn[BN_SIZE - 1] >> (32 - shift))
		return shift;
	for (i=0; i<BN_SIZE; i++) {
		v = bn[i];
		bn[i] = (v << shift) | or_val;
		or_val = v >> (32 - shift);
	}
	return 0;
}

static void bn_zero(unsigned int *bn)
{
	int i;
	for (i=0; i<BN_SIZE; i++) {
		bn[i] = 0;
	}
}
/* parse number in null terminated string 'p' and return it in the
   current token */

static void parse_number(const char *p)
{
	int b, t, shift, frac_bits, s, exp_val, ch;
	char *q;
	unsigned int bn[BN_SIZE];
	long double d;
	/* number */

	q = token_buf;
	ch = *p++;
	t = ch;
	ch = *p++;
	*q++ = t;
	b = 10;
	if (t == '.') {
		goto float_frac_parse;
	} else if (t == '0') {
		if (ch == 'x' || ch == 'X') {
			q--;
			ch = *p++;
			b = 16;
		} else if (tcc_state->tcc_ext && (ch == 'b' || ch == 'B')) {
			q--;
			ch = *p++;
			b = 2;
		}
	}
	/* parse all digits. cannot check octal numbers at this stage
	       because of floating point constants */

	while (1) {
		if (ch >= 'a' && ch <= 'f')
			t = ch - 'a' + 10;
		else if (ch >= 'A' && ch <= 'F')
			t = ch - 'A' + 10;
		else if (isnum(ch))
			t = ch - '0';
		else
			break;
		if (t >= b)
			break;
		if (q >= token_buf + STRING_MAX_SIZE) {
num_too_long:
			tcc_error("number too long");
		}
		*q++ = ch;
		ch = *p++;
	}
	if (ch == '.' ||
	    ((ch == 'e' || ch == 'E') && b == 10) ||
	    ((ch == 'p' || ch == 'P') && (b == 16 || b == 2))) {
		if (b != 10) {
			/* NOTE: strtox should support that for hexa numbers, but
			               non ISOC99 libcs do not support it, so we prefer to do
			               it by hand */
			/* hexadecimal or binary floats */
			/* XXX: handle overflows */

			frac_bits = 0;
			*q = '\0';
			if (b == 16)
				shift = 4;
			else
				shift = 1;
			bn_zero(bn);
			q = token_buf;
			while (1) {
				t = *q++;
				if (t == '\0') {
					break;
				} else if (t >= 'a') {
					t = t - 'a' + 10;
				} else if (t >= 'A') {
					t = t - 'A' + 10;
				} else {
					t = t - '0';
				}
				frac_bits -= bn_lshift(bn, shift, t);
			}
			if (ch == '.') {
				ch = *p++;
				while (1) {
					t = ch;
					if (t >= 'a' && t <= 'f') {
						t = t - 'a' + 10;
					} else if (t >= 'A' && t <= 'F') {
						t = t - 'A' + 10;
					} else if (t >= '0' && t <= '9') {
						t = t - '0';
					} else {
						break;
					}
					if (t >= b)
						tcc_error("invalid digit");
					frac_bits -= bn_lshift(bn, shift, t);
					frac_bits += shift;
					ch = *p++;
				}
			}
			if (ch != 'p' && ch != 'P')
				expect("exponent");
			ch = *p++;
			s = 1;
			exp_val = 0;
			if (ch == '+') {
				ch = *p++;
			} else if (ch == '-') {
				s = -1;
				ch = *p++;
			}
			if (ch < '0' || ch > '9')
				expect("exponent digits");
			while (ch >= '0' && ch <= '9') {
				/* If exp_val is this large ldexp will return HUGE_VAL */

				if (exp_val < 100000000)
					exp_val = exp_val * 10 + ch - '0';
				ch = *p++;
			}
			exp_val = exp_val * s;
			/* now we can generate the number */
			/* XXX: should patch directly float number */

			d = (long double)bn[3] * 79228162514264337593543950336.0L +
			    (long double)bn[2] * 18446744073709551616.0L +
			    (long double)bn[1] * 4294967296.0L +
			    (long double)bn[0];
			d = ldexpl(d, exp_val - frac_bits);
			t = toup(ch);
			if (t == 'F') {
				ch = *p++;
				tok = TOK_CFLOAT;
				/* float : should handle overflow */

				tokc.f = (float)d;
			} else if (t == 'L') {
				ch = *p++;
				tok = TOK_CLDOUBLE;
				tokc.ld = d;
			} else {
				tok = TOK_CDOUBLE;
				tokc.d = (double)d;
			}
		} else {
			/* decimal floats */

			if (ch == '.') {
				if (q >= token_buf + STRING_MAX_SIZE)
					goto num_too_long;
				*q++ = ch;
				ch = *p++;
float_frac_parse:
				while (ch >= '0' && ch <= '9') {
					if (q >= token_buf + STRING_MAX_SIZE)
						goto num_too_long;
					*q++ = ch;
					ch = *p++;
				}
			}
			if (ch == 'e' || ch == 'E') {
				if (q >= token_buf + STRING_MAX_SIZE)
					goto num_too_long;
				*q++ = ch;
				ch = *p++;
				if (ch == '-' || ch == '+') {
					if (q >= token_buf + STRING_MAX_SIZE)
						goto num_too_long;
					*q++ = ch;
					ch = *p++;
				}
				if (ch < '0' || ch > '9')
					expect("exponent digits");
				while (ch >= '0' && ch <= '9') {
					if (q >= token_buf + STRING_MAX_SIZE)
						goto num_too_long;
					*q++ = ch;
					ch = *p++;
				}
			}
			*q = '\0';
			t = toup(ch);
			errno = 0;
			if (t == 'F') {
				ch = *p++;
				tok = TOK_CFLOAT;
				tokc.f = strtof(token_buf, NULL);
			} else if (t == 'L') {
				ch = *p++;
				tok = TOK_CLDOUBLE;
				tokc.ld = strtold(token_buf, NULL);
			} else {
				tok = TOK_CDOUBLE;
				tokc.d = strtod(token_buf, NULL);
			}
		}
	} else {
		unsigned long long n, n1;
		int lcount, ucount, ov = 0;
		const char *p1;
		/* integer number */

		*q = '\0';
		q = token_buf;
		if (b == 10 && *q == '0') {
			b = 8;
			q++;
		}
		n = 0;
		while (1) {
			t = *q++;
			/* no need for checks except for base 10 / 8 errors */

			if (t == '\0')
				break;
			else if (t >= 'a')
				t = t - 'a' + 10;
			else if (t >= 'A')
				t = t - 'A' + 10;
			else
				t = t - '0';
			if (t >= b)
				tcc_error("invalid digit");
			n1 = n;
			n = n * b + t;
			/* detect overflow */

			if (n1 >= 0x1000000000000000ULL && n / b != n1)
				ov = 1;
		}
		/* Determine the characteristics (unsigned and/or 64bit) the type of
		           the constant must have according to the constant suffix(es) */

		lcount = ucount = 0;
		p1 = p;
		for (;;) {
			t = toup(ch);
			if (t == 'L') {
				if (lcount >= 2)
					tcc_error("three 'l's in integer constant");
				if (lcount && *(p - 1) != ch)
					tcc_error("incorrect integer suffix: %s", p1);
				lcount++;
				ch = *p++;
			} else if (t == 'U') {
				if (ucount >= 1)
					tcc_error("two 'u's in integer constant");
				ucount++;
				ch = *p++;
			} else {
				break;
			}
		}
		/* in #if/#elif expressions, all numbers have type (u)intmax_t anyway */

		if (pp_expr)
			lcount = 2;
		/* Determine if it needs 64 bits and/or unsigned in order to fit */

		if (ucount == 0 && b == 10) {
			if (lcount <= (LONG_SIZE == 4)) {
				if (n >= 0x80000000U)
					lcount = (LONG_SIZE == 4) + 1;
			}
			if (n >= 0x8000000000000000ULL)
				ov = 1, ucount = 1;
		} else {
			if (lcount <= (LONG_SIZE == 4)) {
				if (n >= 0x100000000ULL)
					lcount = (LONG_SIZE == 4) + 1;
				else if (n >= 0x80000000U)
					ucount = 1;
			}
			if (n >= 0x8000000000000000ULL)
				ucount = 1;
		}

		if (ov)
			tcc_warning("integer constant overflow");

		tok = TOK_CINT;
		if (lcount) {
			tok = TOK_CLONG;
			if (lcount == 2)
				tok = TOK_CLLONG;
		}
		if (ucount)
			++tok;/* TOK_CU... */

		tokc.i = n;
	}
	if (ch)
		tcc_error("invalid number");
}
#define PARSE2(c1,tok1,c2,tok2) case c1: PEEKC(c, p); if (c == c2) { p++; tok = tok2; } else { tok = tok1; } break;
/* return next token without macro substitution */

static void next_nomacro(void)
{
	int t, c, is_long, len;
	TokenSym *ts;
	uint8_t *p, *p1;
	unsigned int h;

	p = file->buf_ptr;
redo_no_start:
	c = *p;
	switch (c) {
	case ' ':
	case '\t':
		tok = c;
		p++;
maybe_space:
		if (parse_flags & PARSE_FLAG_SPACES)
			goto keep_tok_flags;
		while (isidnum_table[*p - CH_EOF] & IS_SPC)
			++p;
		goto redo_no_start;
	case '\f':
	case '\v':
	case '\r':
		p++;
		goto redo_no_start;
	case '\\':
		/* first look if it is in fact an end of buffer */

		c = handle_stray(&p);
		if (c == '\\')
			goto parse_simple;
		if (c == CH_EOF) {
			TCCState *s1 = tcc_state;
			if (!(tok_flags & TOK_FLAG_BOL)) {
				/* add implicit newline */

				goto maybe_newline;
			} else if (!(parse_flags & PARSE_FLAG_PREPROCESS)) {
				tok = TOK_EOF;
			} else if (s1->ifdef_stack_ptr != file->ifdef_stack_ptr) {
				tcc_error("missing #endif");
			} else if (s1->include_stack_ptr == s1->include_stack) {
				/* no include left : end of file. */

				tok = TOK_EOF;
			} else {
				/* pop include file */
				/* test if previous '#endif' was after a #ifdef at
				                   start of file */

				if (tok_flags & TOK_FLAG_ENDIF) {

					search_cached_include(s1, file->true_filename, 1)
					->ifndef_macro = file->ifndef_macro_saved;
					tok_flags &= ~TOK_FLAG_ENDIF;
				}
				/* add end of include file debug info */

				tcc_debug_eincl(tcc_state);
				/* pop include stack */

				tcc_close();
				s1->include_stack_ptr--;
				p = file->buf_ptr;
				goto maybe_newline;
			}
		} else {
			goto redo_no_start;
		}
		break;

	case '\n':
		file->line_num++;
		p++;
maybe_newline:
		tok_flags |= TOK_FLAG_BOL;
		if (0 == (parse_flags & PARSE_FLAG_LINEFEED))
			goto redo_no_start;
		tok = TOK_LINEFEED;
		goto keep_tok_flags;

	case '#':
		/* XXX: simplify */

		PEEKC(c, p);
		if ((tok_flags & TOK_FLAG_BOL) &&
		    (parse_flags & PARSE_FLAG_PREPROCESS)) {
			tok_flags &= ~TOK_FLAG_BOL;
			file->buf_ptr = p;
			preprocess(tok_flags & TOK_FLAG_BOF);
			p = file->buf_ptr;
			goto maybe_newline;
		} else {
			if (c == '#') {
				p++;
				tok = TOK_TWOSHARPS;
			} else {

				if (parse_flags & PARSE_FLAG_ASM_FILE) {
					p = parse_line_comment(p - 1);
					goto redo_no_start;
				} else

				{
					tok = '#';
				}
			}
		}
		break;
	/* dollar is allowed to start identifiers when not parsing asm */

	case '$':
		if (!(isidnum_table['$' - CH_EOF] & IS_ID)
		    || (parse_flags & PARSE_FLAG_ASM_FILE))
			goto parse_simple;

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
parse_ident_fast:
		p1 = p;
		h = TOK_HASH_INIT;
		h = TOK_HASH_FUNC(h, c);
		while (c = *++p, isidnum_table[c - CH_EOF] & (IS_ID|IS_NUM))
			h = TOK_HASH_FUNC(h, c);
		len = p - p1;
		if (c != '\\') {
			TokenSym **pts;
			/* fast case : no stray found, so we have the full token
			               and we have already hashed it */

			h &= (TOK_HASH_SIZE - 1);
			pts = &hash_ident[h];
			for (;;) {
				ts = *pts;
				if (!ts)
					break;
				if (ts->len == len && !memcmp(ts->str, p1, len))
					goto token_found;
				pts = &(ts->hash_next);
			}
			ts = tok_alloc_new(pts, (char *) p1, len);
token_found: ;
		} else {
			/* slower case */

			cstr_reset(&tokcstr);
			cstr_cat(&tokcstr, (char *) p1, len);
			p--;
			PEEKC(c, p);
			while (isidnum_table[c - CH_EOF] & (IS_ID|IS_NUM)) {
				cstr_ccat(&tokcstr, c);
				PEEKC(c, p);
			}
			ts = tok_alloc(tokcstr.data, tokcstr.size);
		}
		tok = ts->tok;
		break;
	case 'L':
		t = p[1];
		if (t == '\'' || t == '\"' || t == '\\') {
			PEEKC(c, p);
			if (c == '\'' || c == '\"') {
				is_long = 1;
				goto str_const;
			}
			*--p = c = 'L';
		}
		goto parse_ident_fast;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		t = c;
		PEEKC(c, p);
		/* after the first digit, accept digits, alpha, '.' or sign if
		           prefixed by 'eEpP' */

parse_num:
		cstr_reset(&tokcstr);
		for (;;) {
			cstr_ccat(&tokcstr, t);
			if (!((isidnum_table[c - CH_EOF] & (IS_ID|IS_NUM))
			      || c == '.'
			      || ((c == '+' || c == '-')
				  && (((t == 'e' || t == 'E')
				       && !(parse_flags & PARSE_FLAG_ASM_FILE
					    /* 0xe+1 is 3 tokens in asm */

					    && ((char *)tokcstr.data)[0] == '0'
					    && toup(((char *)tokcstr.data)[1]) == 'X'))
				      || t == 'p' || t == 'P'))))
				break;
			t = c;
			PEEKC(c, p);
		}
		/* We add a trailing '\0' to ease parsing */

		cstr_ccat(&tokcstr, '\0');
		tokc.str.size = tokcstr.size;
		tokc.str.data = tokcstr.data;
		tok = TOK_PPNUM;
		break;

	case '.':
		/* special dot handling because it can also start a number */

		PEEKC(c, p);
		if (isnum(c)) {
			t = '.';
			goto parse_num;
		} else if ((isidnum_table['.' - CH_EOF] & IS_ID)
			   && (isidnum_table[c - CH_EOF] & (IS_ID|IS_NUM))) {
			*--p = c = '.';
			goto parse_ident_fast;
		} else if (c == '.') {
			PEEKC(c, p);
			if (c == '.') {
				p++;
				tok = TOK_DOTS;
			} else {
				*--p = '.';/* may underflow into file->unget[] */

				tok = '.';
			}
		} else {
			tok = '.';
		}
		break;
	case '\'':
	case '\"':
		is_long = 0;
str_const:
		cstr_reset(&tokcstr);
		if (is_long)
			cstr_ccat(&tokcstr, 'L');
		cstr_ccat(&tokcstr, c);
		p = parse_pp_string(p, c, &tokcstr);
		cstr_ccat(&tokcstr, c);
		cstr_ccat(&tokcstr, '\0');
		tokc.str.size = tokcstr.size;
		tokc.str.data = tokcstr.data;
		tok = TOK_PPSTR;
		break;

	case '<':
		PEEKC(c, p);
		if (c == '=') {
			p++;
			tok = TOK_LE;
		} else if (c == '<') {
			PEEKC(c, p);
			if (c == '=') {
				p++;
				tok = TOK_A_SHL;
			} else {
				tok = TOK_SHL;
			}
		} else {
			tok = TOK_LT;
		}
		break;
	case '>':
		PEEKC(c, p);
		if (c == '=') {
			p++;
			tok = TOK_GE;
		} else if (c == '>') {
			PEEKC(c, p);
			if (c == '=') {
				p++;
				tok = TOK_A_SAR;
			} else {
				tok = TOK_SAR;
			}
		} else {
			tok = TOK_GT;
		}
		break;

	case '&':
		PEEKC(c, p);
		if (c == '&') {
			p++;
			tok = TOK_LAND;
		} else if (c == '=') {
			p++;
			tok = TOK_A_AND;
		} else {
			tok = '&';
		}
		break;

	case '|':
		PEEKC(c, p);
		if (c == '|') {
			p++;
			tok = TOK_LOR;
		} else if (c == '=') {
			p++;
			tok = TOK_A_OR;
		} else {
			tok = '|';
		}
		break;

	case '+':
		PEEKC(c, p);
		if (c == '+') {
			p++;
			tok = TOK_INC;
		} else if (c == '=') {
			p++;
			tok = TOK_A_ADD;
		} else {
			tok = '+';
		}
		break;

	case '-':
		PEEKC(c, p);
		if (c == '-') {
			p++;
			tok = TOK_DEC;
		} else if (c == '=') {
			p++;
			tok = TOK_A_SUB;
		} else if (c == '>') {
			p++;
			tok = TOK_ARROW;
		} else {
			tok = '-';
		}
		break;

		PARSE2('!', '!', '=', TOK_NE)
		PARSE2('=', '=', '=', TOK_EQ)
		PARSE2('*', '*', '=', TOK_A_MUL)
		PARSE2('%', '%', '=', TOK_A_MOD)
		PARSE2('^', '^', '=', TOK_A_XOR)
	/* comments or operator */

	case '/':
		PEEKC(c, p);
		if (c == '*') {
			p = parse_comment(p);
			/* comments replaced by a blank */

			tok = ' ';
			goto maybe_space;
		} else if (c == '/') {
			p = parse_line_comment(p);
			tok = ' ';
			goto maybe_space;
		} else if (c == '=') {
			p++;
			tok = TOK_A_DIV;
		} else {
			tok = '/';
		}
		break;
	/* simple tokens */

	case '@':/* only used in assembler */

	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
	case ',':
	case ';':
	case ':':
	case '?':
	case '~':
parse_simple:
		tok = c;
		p++;
		break;
	case 0xEF:/* UTF8 BOM ? */

		if (p[1] == 0xBB && p[2] == 0xBF && p == file->buffer) {
			p += 3;
			goto redo_no_start;
		}
	default:
		if (c >= 0x80 && c <= 0xFF)/* utf8 identifiers */

			goto parse_ident_fast;
		if (parse_flags & PARSE_FLAG_ASM_FILE)
			goto parse_simple;
		tcc_error("unrecognized character \\x%02x", c);
		break;
	}
	tok_flags = 0;
keep_tok_flags:
	file->buf_ptr = p;

}
#define PP_PRINT(x)

static int macro_subst(
	TokenString *tok_str,
	Sym **nested_list,
	const int *macro_str
);
/* substitute arguments in replacement lists in macro_str by the values in
   args (field d) and return allocated string */

static int *macro_arg_subst(Sym **nested_list, const int *macro_str, Sym *args)
{
	int t, t0, t1, t2, n;
	const int *st;
	Sym *s;
	CValue cval;
	TokenString str;
	tok_str_new(&str);
	t0 = t1 = 0;
	while (1) {
		TOK_GET(&t, &macro_str, &cval);
		if (!t)
			break;
		if (t == '#') {
			/* stringize */

			do
				t = *macro_str++;
			while (t == ' ');
			s = sym_find2(args, t);
			if (s) {
				cstr_reset(&tokcstr);
				cstr_ccat(&tokcstr, '\"');
				st = s->d;
				while (*st != TOK_EOF) {
					const char *s;
					TOK_GET(&t, &st, &cval);
					s = get_tok_str(t, &cval);
					while (*s) {
						if (t == TOK_PPSTR && *s != '\'')
							add_char(&tokcstr, *s);
						else
							cstr_ccat(&tokcstr, *s);
						++s;
					}
				}
				cstr_ccat(&tokcstr, '\"');
				cstr_ccat(&tokcstr, '\0');
//printf("\nstringize: <%s>\n", (char *)tokcstr.data);
				/* add string */

				cval.str.size = tokcstr.size;
				cval.str.data = tokcstr.data;
				tok_str_add2(&str, TOK_PPSTR, &cval);

			} else {
				expect("macro parameter after '#'");
			}
		} else if (t >= TOK_IDENT) {
			s = sym_find2(args, t);
			if (s) {
				st = s->d;
				n = 0;
				while ((t2 = macro_str[n]) == ' ')
					++n;
				/* if '##' is present before or after, no arg substitution */

				if (t2 == TOK_PPJOIN || t1 == TOK_PPJOIN) {
					/* special case for var arg macros : ## eats the ','
					                       if empty VA_ARGS variable. */

					if (t1 == TOK_PPJOIN && t0 == ',' && gnu_ext && s->type.t) {
						int c = str.str[str.len - 1];
						while (str.str[--str.len] != ',')
							;
						if (*st == TOK_EOF) {
							/* suppress ',' '##' */

						} else {
							/* suppress '##' and add variable */

							str.len++;
							if (c == ' ')
								str.str[str.len++] = c;
							goto add_var;
						}
					} else {
						if (*st == TOK_EOF)
							tok_str_add(&str, TOK_PLCHLDR);
					}
				} else {
add_var:
					if (!s->e) {
						/* Expand arguments tokens and store them.  In most
									   cases we could also re-expand each argument if
									   used multiple times, but not if the argument
									   contains the __COUNTER__ macro.  */

						TokenString str2;
						tok_str_new(&str2);
						macro_subst(&str2, nested_list, st);
						tok_str_add(&str2, TOK_EOF);
						s->e = str2.str;
					}
					st = s->e;
				}
				while (*st != TOK_EOF) {
					TOK_GET(&t2, &st, &cval);
					tok_str_add2(&str, t2, &cval);
				}
			} else {
				tok_str_add(&str, t);
			}
		} else {
			tok_str_add2(&str, t, &cval);
		}
		if (t != ' ')
			t0 = t1, t1 = t;
	}
	tok_str_add(&str, 0);
	PP_PRINT(("areslt:", 0, str.str));
	return str.str;
}
/* handle the '##' operator. return the resulting string (which must be freed). */

static inline int *macro_twosharps(const int *ptr0)
{
	int t1, t2, n, l;
	CValue cv1, cv2;
	TokenString macro_str1;
	const int *ptr;

	tok_str_new(&macro_str1);
	cstr_reset(&tokcstr);
	for (ptr = ptr0;;) {
		TOK_GET(&t1, &ptr, &cv1);
		if (t1 == 0)
			break;
		for (;;) {
			n = 0;
			while ((t2 = ptr[n]) == ' ')
				++n;
			if (t2 != TOK_PPJOIN)
				break;
			ptr += n;
			while ((t2 = *++ptr) == ' ' || t2 == TOK_PPJOIN)
				;
			TOK_GET(&t2, &ptr, &cv2);
			if (t2 == TOK_PLCHLDR)
				continue;
			if (t1 != TOK_PLCHLDR) {
				cstr_cat(&tokcstr, get_tok_str(t1, &cv1), -1);
				t1 = TOK_PLCHLDR;
			}
			cstr_cat(&tokcstr, get_tok_str(t2, &cv2), -1);
		}
		if (tokcstr.size) {
			cstr_ccat(&tokcstr, 0);
			tcc_open_bf(tcc_state, ":paste:", tokcstr.size);
			memcpy(file->buffer, tokcstr.data, tokcstr.size);
			tok_flags = 0;/* don't interpret '#' */

			for (n = 0;; n = l) {
				next_nomacro();
				tok_str_add2(&macro_str1, tok, &tokc);
				if (*file->buf_ptr == 0)
					break;
				tok_str_add(&macro_str1, ' ');
				l = file->buf_ptr - file->buffer;
				tcc_warning("pasting \"%.*s\" and \"%s\" does not give a valid"
					    " preprocessing token", l - n, file->buffer + n, file->buf_ptr);
			}
			tcc_close();
			cstr_reset(&tokcstr);
		}
		if (t1 != TOK_PLCHLDR)
			tok_str_add2(&macro_str1, t1, &cv1);
	}
	tok_str_add(&macro_str1, 0);
	PP_PRINT(("pasted:", 0, macro_str1.str));
	return macro_str1.str;
}

static int peek_file (TokenString *ws_str)
{
	uint8_t *p = file->buf_ptr - 1;
	int c;
	for (;;) {
		PEEKC(c, p);
		switch (c) {
		case '/':
			PEEKC(c, p);
			if (c == '*')
				p = parse_comment(p);
			else if (c == '/')
				p = parse_line_comment(p);
			else {
				c = *--p = '/';
				goto leave;
			}
			--p, c = ' ';
			break;
		case ' ':
		case '\t':
			break;
		case '\f':
		case '\v':
		case '\r':
			continue;
		case '\n':
			file->line_num++, tok_flags |= TOK_FLAG_BOL;
			break;
		default:
leave:
			file->buf_ptr = p;
			return c;
		}
		if (ws_str)
			tok_str_add(ws_str, c);
	}
}
/* peek or read [ws_str == NULL] next token from function macro call,
   walking up macro levels\ up to the file if necessary */

static int next_argstream(Sym **nested_list, TokenString *ws_str)
{
	int t;
	Sym *sa;

	while (macro_ptr) {
		const int *m = macro_ptr;
		while ((t = *m) != 0) {
			if (ws_str) {
				if (t != ' ')
					return t;
				++m;
			} else {
				TOK_GET(&tok, &macro_ptr, &tokc);
				return tok;
			}
		}
		end_macro();
		/* also, end of scope for nested defined symbol */

		sa = *nested_list;
		if (sa)
			*nested_list = sa->prev, sym_free(sa);
	}
	if (ws_str) {
		return peek_file(ws_str);
	} else {
		next_nomacro();
		if (tok == '\t' || tok == TOK_LINEFEED)
			tok = ' ';
		return tok;
	}
}
/* do macro substitution of current token with macro 's' and add
   result to (tok_str,tok_len). 'nested_list' is the list of all
   macros we got inside to avoid recursing. Return non zero if no
   substitution needs to be done */

static int macro_subst_tok(
	TokenString *tok_str,
	Sym **nested_list,
	Sym *s)
{
	int t;
	int v = s->v;

	PP_PRINT(("#", v, s->d));
	if (s->d) {
		int *mstr = s->d;
		int *jstr;
		Sym *sa;
		int ret;

		if (s->type.t & MACRO_FUNC) {
			int saved_parse_flags = parse_flags;
			TokenString str;
			int parlevel, i;
			Sym *sa1, *args;

			parse_flags |= PARSE_FLAG_SPACES | PARSE_FLAG_LINEFEED
				       | PARSE_FLAG_ACCEPT_STRAYS;

			tok_str_new(&str);
			/* peek next token from argument stream */

			t = next_argstream(nested_list, &str);
			if (t != '(') {
				/* not a macro substitution after all, restore the
				                 * macro token plus all whitespace we've read.
				                 * whitespace is intentionally not merged to preserve
				                 * newlines. */

				parse_flags = saved_parse_flags;
				tok_str_add2_spc(tok_str, v, 0);
				if (parse_flags & PARSE_FLAG_SPACES)
					for (i = 0; i < str.len; i++)
						tok_str_add(tok_str, str.str[i]);
				tok_str_free_str(str.str);
				return 0;
			} else {
				tok_str_free_str(str.str);
			}
			/* argument macro */

			args = NULL;
			sa = s->next;
			/* NOTE: empty args are allowed, except if no args */

			i = 2;/* eat '(' */

			for (;;) {
				do {
					t = next_argstream(nested_list, NULL);
				} while (t == ' ' || --i);

				if (!sa) {
					if (t == ')')/* handle '()' case */

						break;
					tcc_error("macro '%s' used with too many args",
						  get_tok_str(v, 0));
				}
empty_arg:
				tok_str_new(&str);
				parlevel = 0;
				/* NOTE: non zero sa->type.t indicates VA_ARGS */

				while (parlevel > 0
				       || (t != ')' && (t != ',' || sa->type.t))) {
					if (t == TOK_EOF)
						tcc_error("EOF in invocation of macro '%s'",
							  get_tok_str(v, 0));
					if (t == '(')
						parlevel++;
					if (t == ')')
						parlevel--;
					if (t == ' ')
						str.need_spc |= 1;
					else
						tok_str_add2_spc(&str, t, &tokc);
					t = next_argstream(nested_list, NULL);
				}
				tok_str_add(&str, TOK_EOF);
				sa1 = sym_push2(&args, sa->v & ~SYM_FIELD, sa->type.t, 0);
				sa1->d = str.str;
				sa = sa->next;
				if (t == ')') {
					if (!sa)
						break;
					/* special case for gcc var args: add an empty
					                       var arg argument if it is omitted */

					if (sa->type.t && gnu_ext)
						goto empty_arg;
					tcc_error("macro '%s' used with too few args",
						  get_tok_str(v, 0));
				}
				i = 1;
			}
			/* now subst each arg */

			mstr = macro_arg_subst(nested_list, mstr, args);
			/* free memory */

			sa = args;
			while (sa) {
				sa1 = sa->prev;
				tok_str_free_str(sa->d);
				tok_str_free_str(sa->e);
				sym_free(sa);
				sa = sa1;
			}
			parse_flags = saved_parse_flags;
		}
		/* process '##'s (if present) */

		jstr = mstr;
		if (s->type.t & MACRO_JOIN)
			jstr = macro_twosharps(mstr);

		sa = sym_push2(nested_list, v, 0, 0);
		ret = macro_subst(tok_str, nested_list, jstr);
		/* pop nested defined symbol */

		if (sa == *nested_list)
			*nested_list = sa->prev, sym_free(sa);

		if (jstr != mstr)
			tok_str_free_str(jstr);
		if (mstr != s->d)
			tok_str_free_str(mstr);
		return ret;

	} else {
		CValue cval;
		char buf[32], *cstrval = buf;
		/* special macros */

		if (v == TOK___LINE__ || v == TOK___COUNTER__) {
			t = v == TOK___LINE__ ? file->line_num : pp_counter++;
			snprintf(buf, sizeof(buf), "%d", t);
			t = TOK_PPNUM;
			goto add_cstr1;

		} else if (v == TOK___FILE__) {
			cstrval = file->filename;
			goto add_cstr;

		} else if (v == TOK___DATE__ || v == TOK___TIME__) {
			time_t ti;
			struct tm *tm;
			time(&ti);
			tm = localtime(&ti);
			if (v == TOK___DATE__) {
				static char const ab_month_name[12][4] = {
					"Jan", "Feb", "Mar", "Apr", "May", "Jun",
					"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
				};
				snprintf(buf, sizeof(buf), "%s %2d %d",
					 ab_month_name[tm->tm_mon], tm->tm_mday, tm->tm_year + 1900);
			} else {
				snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
					 tm->tm_hour, tm->tm_min, tm->tm_sec);
			}
add_cstr:
			t = TOK_STR;
add_cstr1:
			cval.str.size = strlen(cstrval) + 1;
			cval.str.data = cstrval;
			tok_str_add2_spc(tok_str, t, &cval);
		}
		return 0;
	}
}
/* do macro substitution of macro_str and add result to
   (tok_str,tok_len). 'nested_list' is the list of all macros we got
   inside to avoid recursing. */

static int macro_subst(
	TokenString *tok_str,
	Sym **nested_list,
	const int *macro_str
)
{
	Sym *s;
	int t, nosubst = 0;
	CValue cval;
	TokenString *str;

	while (1) {
		TOK_GET(&t, &macro_str, &cval);
		if (t == 0 || t == TOK_EOF)
			break;
		if (t >= TOK_IDENT) {
			s = define_find(t);
			if (s == NULL || nosubst)
				goto no_subst;
			/* if nested substitution, do nothing */

			if (sym_find2(*nested_list, t)) {
				/* and mark so it doesn't get subst'd again */

				t |= SYM_FIELD;
				goto no_subst;
			}
			str = tok_str_alloc();
			str->str = (int *)macro_str; /* setup stream for possible arguments */

			begin_macro(str, 2);
			nosubst = macro_subst_tok(tok_str, nested_list, s);
			if (macro_stack != str) {
				/* already finished by reading function macro arguments */

				break;
			}
			macro_str = macro_ptr;
			end_macro ();
		} else if (t == ' ') {
			if (parse_flags & PARSE_FLAG_SPACES)
				tok_str->need_spc |= 1;
		} else {
no_subst:
			tok_str_add2_spc(tok_str, t, &cval);
			if (nosubst && t != '(')
				nosubst = 0;
			/* GCC supports 'defined' as result of a macro substitution */

			if (t == TOK_DEFINED && pp_expr)
				nosubst = 1;
		}
	}

	return nosubst;
}
/* return next token with macro substitution */

ST_FUNC void next(void)
{
	int t;
	while (macro_ptr) {
redo:
		t = *macro_ptr;
		if (TOK_HAS_VALUE(t)) {
			tok_get(&tok, &macro_ptr, &tokc);
			if (t == TOK_LINENUM) {
				file->line_num = tokc.i;
				goto redo;
			}
			goto convert;
		} else if (t == 0) {
			/* end of macro or unget token string */

			end_macro();
			continue;
		} else if (t == TOK_EOF) {
			/* do nothing */

		} else {
			++macro_ptr;
			t &= ~SYM_FIELD;/* remove 'nosubst' marker */

			if (t == '\\') {
				if (!(parse_flags & PARSE_FLAG_ACCEPT_STRAYS))
					tcc_error("stray '\\' in program");
			}
		}
		tok = t;
		return;
	}

	next_nomacro();
	t = tok;
	if (t >= TOK_IDENT && (parse_flags & PARSE_FLAG_PREPROCESS)) {
		/* if reading from file, try to substitute macros */

		Sym *s = define_find(t);
		if (s) {
			Sym *nested_list = NULL;
			macro_subst_tok(&tokstr_buf, &nested_list, s);
			tok_str_add(&tokstr_buf, 0);
			begin_macro(&tokstr_buf, 0);
			goto redo;
		}
		return;
	}

convert:
	/* convert preprocessor tokens into C tokens */

	if (t == TOK_PPNUM) {
		if (parse_flags & PARSE_FLAG_TOK_NUM)
			parse_number(tokc.str.data);
	} else if (t == TOK_PPSTR) {
		if (parse_flags & PARSE_FLAG_TOK_STR)
			parse_string(tokc.str.data, tokc.str.size - 1);
	}
}
/* push back current token and set current token to 'last_tok'. Only
   identifier case handled for labels. */

ST_INLN void unget_tok(int last_tok)
{
	TokenString *str = &unget_buf;
	int alloc = 0;
	if (str->len)/* use static buffer except if already in use */

		str = tok_str_alloc(), alloc = 1;
	if (tok != TOK_EOF)
		tok_str_add2(str, tok, &tokc);
	tok_str_add(str, 0);
	begin_macro(str, alloc);
	tok = last_tok;
}
/* ------------------------------------------------------------------------- */
/* init preprocessor */

static const char *const target_os_defs =

	"_WIN32\0"
	"_WIN64\0"

	;

static void putdef(CString *cs, const char *p)
{
	cstr_printf(cs, "#define %s%s\n", p, &" 1"[!!strchr(p, ' ')*2]);
}

static void putdefs(CString *cs, const char *p)
{
	while (*p)
		putdef(cs, p), p = strchr(p, 0) + 1;
}

static void tcc_predefs(TCCState *s1, CString *cs, int is_asm)
{
	cstr_printf(cs, "#define __TINYC__ 9%.2s\n", &TCC_VERSION[4]);
	putdefs(cs, target_machine_defs);
	putdefs(cs, target_os_defs);

	if (is_asm)
		putdef(cs, "__ASSEMBLER__");
	if (s1->output_type == TCC_OUTPUT_PREPROCESS)
		putdef(cs, "__TCC_PP__");
	if (s1->output_type == TCC_OUTPUT_MEMORY)
		putdef(cs, "__TCC_RUN__");

	if (s1->char_is_unsigned)
		putdef(cs, "__CHAR_UNSIGNED__");
	if (s1->optimize > 0)
		putdef(cs, "__OPTIMIZE__");
	if (s1->option_pthread)
		putdef(cs, "_REENTRANT");
	if (s1->leading_underscore)
		putdef(cs, "__leading_underscore");
	cstr_printf(cs, "#define __SIZEOF_POINTER__ %d\n", PTR_SIZE);
	cstr_printf(cs, "#define __SIZEOF_LONG__ %d\n", LONG_SIZE);
	if (!is_asm) {
		putdef(cs, "__STDC__");
		cstr_printf(cs, "#define __STDC_HOSTED__ %d\n", s1->nostdlib ? 0 : 1);
		cstr_printf(cs, "#define __STDC_VERSION__ %dL\n", s1->cversion);
		cstr_cat(cs,
			 /* load more predefs and __builtins */

			 "#include <tccdefs.h>\n"/* load at runtime */

			 , -1);
	}
	cstr_printf(cs, "#define __BASE_FILE__ \"%s\"\n", file->filename);
}

ST_FUNC void preprocess_start(TCCState *s1, int filetype)
{
	int is_asm = !!(filetype & (AFF_TYPE_ASM|AFF_TYPE_ASMPP));

	tccpp_new(s1);

	s1->include_stack_ptr = s1->include_stack;
	s1->ifdef_stack_ptr = s1->ifdef_stack;
	file->ifdef_stack_ptr = s1->ifdef_stack_ptr;
	pp_expr = 0;
	pp_counter = 0;
	pp_debug_tok = pp_debug_symv = 0;
	s1->pack_stack[0] = 0;
	s1->pack_stack_ptr = s1->pack_stack;

	set_idnum('$', s1->dollars_in_identifiers ? IS_ID : 0);
	set_idnum('.', is_asm ? IS_ID : 0);

	if (!(filetype & AFF_TYPE_ASM)) {
		CString cstr;
		cstr_new(&cstr);
		tcc_predefs(s1, &cstr, is_asm);
		if (s1->cmdline_defs.size)
			cstr_cat(&cstr, s1->cmdline_defs.data, s1->cmdline_defs.size);
		if (s1->cmdline_incl.size)
			cstr_cat(&cstr, s1->cmdline_incl.data, s1->cmdline_incl.size);
//printf("%.*s\n", cstr.size, (char*)cstr.data);

		*s1->include_stack_ptr++ = file;
		tcc_open_bf(s1, "<command line>", cstr.size);
		memcpy(file->buffer, cstr.data, cstr.size);
		cstr_free(&cstr);
	}
	parse_flags = is_asm ? PARSE_FLAG_ASM_FILE : 0;
}
/* cleanup from error/setjmp */

ST_FUNC void preprocess_end(TCCState *s1)
{
	while (macro_stack)
		end_macro();
	macro_ptr = NULL;
	while (file)
		tcc_close();
	tccpp_delete(s1);
}

ST_FUNC int set_idnum(int c, int val)
{
	int prev = isidnum_table[c - CH_EOF];
	isidnum_table[c - CH_EOF] = val;
	return prev;
}

ST_FUNC void tccpp_new(TCCState *s)
{
	int i, c;
	const char *p, *r;
	/* init isid table */

	for (i = CH_EOF; i<128; i++)
		set_idnum(i,
			  is_space(i) ? IS_SPC
			  : isid(i) ? IS_ID
			  : isnum(i) ? IS_NUM
			  : 0);

	for (i = 128; i<256; i++)
		set_idnum(i, IS_ID);
	/* init allocators */

	tal_new(&toksym_alloc, TOKSYM_TAL_SIZE);
	tal_new(&tokstr_alloc, TOKSTR_TAL_SIZE);

	memset(hash_ident, 0, TOK_HASH_SIZE * sizeof(TokenSym *));
	memset(s->cached_includes_hash, 0, sizeof s->cached_includes_hash);

	cstr_new(&tokcstr);
	cstr_new(&cstr_buf);
	cstr_realloc(&cstr_buf, STRING_MAX_SIZE);
	tok_str_new(&unget_buf);
	tok_str_realloc(&unget_buf, TOKSTR_MAX_SIZE);
	tok_str_new(&tokstr_buf);
	tok_str_realloc(&tokstr_buf, TOKSTR_MAX_SIZE);

	tok_ident = TOK_IDENT;
	p = tcc_keywords;
	while (*p) {
		r = p;
		for (;;) {
			c = *r++;
			if (c == '\0')
				break;
		}
		tok_alloc(p, r - p - 1);
		p = r;
	}
	/* we add dummy defines for some special macros to speed up tests
	       and to have working defined() */

	define_push(TOK___LINE__, MACRO_OBJ, NULL, NULL);
	define_push(TOK___FILE__, MACRO_OBJ, NULL, NULL);
	define_push(TOK___DATE__, MACRO_OBJ, NULL, NULL);
	define_push(TOK___TIME__, MACRO_OBJ, NULL, NULL);
	define_push(TOK___COUNTER__, MACRO_OBJ, NULL, NULL);
}

ST_FUNC void tccpp_delete(TCCState *s)
{
	int i, n;

	dynarray_reset(&s->cached_includes, &s->nb_cached_includes);
	/* free tokens */

	n = tok_ident - TOK_IDENT;
	if (n > total_idents)
		total_idents = n;
	for (i = n; --i >= 0;)
		tal_free(&toksym_alloc, table_ident[i]);
	tcc_free(table_ident);
	table_ident = NULL;
	/* free static buffers */

	cstr_free(&tokcstr);
	cstr_free(&cstr_buf);
	tok_str_free_str(tokstr_buf.str);
	tok_str_free_str(unget_buf.str);
	/* free allocators */

	tal_delete(&toksym_alloc);
	tal_delete(&tokstr_alloc);
}
/* ------------------------------------------------------------------------- */
/* tcc -E [-P[1]] [-dD} support */

static int pp_need_space(int a, int b);

static void tok_print(const int *str, const char *msg, ...)
{
	FILE *fp = tcc_state->ppfp;
	va_list ap;
	int t, t0, s;
	CValue cval;

	va_start(ap, msg);
	vfprintf(fp, msg, ap);
	va_end(ap);

	s = t0 = 0;
	while (str) {
		TOK_GET(&t, &str, &cval);
		if (t == 0 || t == TOK_EOF)
			break;
		if (pp_need_space(t0, t))
			s = 0;
		fprintf(fp, &" %s"[s], t == TOK_PLCHLDR ? "<>" : get_tok_str(t, &cval));
		s = 1, t0 = t;
	}
	fprintf(fp, "\n");
}

static void pp_line(TCCState *s1, BufferedFile *f, int level)
{
	int d = f->line_num - f->line_ref;

	if (s1->dflag & 4)
		return;

	if (s1->Pflag == LINE_MACRO_OUTPUT_FORMAT_NONE) {
		;
	} else if (level == 0 && f->line_ref && d < 8) {
		while (d > 0)
			fputs("\n", s1->ppfp), --d;
	} else if (s1->Pflag == LINE_MACRO_OUTPUT_FORMAT_STD) {
		fprintf(s1->ppfp, "#line %d \"%s\"\n", f->line_num, f->filename);
	} else {
		fprintf(s1->ppfp, "# %d \"%s\"%s\n", f->line_num, f->filename,
			level > 0 ? " 1" : level < 0 ? " 2" : "");
	}
	f->line_ref = f->line_num;
}

static void define_print(TCCState *s1, int v)
{
	FILE *fp;
	Sym *s;

	s = define_find(v);
	if (NULL == s || NULL == s->d)
		return;

	fp = s1->ppfp;
	fprintf(fp, "#define %s", get_tok_str(v, NULL));
	if (s->type.t & MACRO_FUNC) {
		Sym *a = s->next;
		fprintf(fp,"(");
		if (a)
			for (;;) {
				fprintf(fp,"%s", get_tok_str(a->v, NULL));
				if (!(a = a->next))
					break;
				fprintf(fp,",");
			}
		fprintf(fp,")");
	}
	tok_print(s->d, "");
}

static void pp_debug_defines(TCCState *s1)
{
	int v, t;
	const char *vs;
	FILE *fp;

	t = pp_debug_tok;
	if (t == 0)
		return;

	file->line_num--;
	pp_line(s1, file, 0);
	file->line_ref = ++file->line_num;

	fp = s1->ppfp;
	v = pp_debug_symv;
	vs = get_tok_str(v, NULL);
	if (t == TOK_DEFINE) {
		define_print(s1, v);
	} else if (t == TOK_UNDEF) {
		fprintf(fp, "#undef %s\n", vs);
	} else if (t == TOK_push_macro) {
		fprintf(fp, "#pragma push_macro(\"%s\")\n", vs);
	} else if (t == TOK_pop_macro) {
		fprintf(fp, "#pragma pop_macro(\"%s\")\n", vs);
	}
	pp_debug_tok = 0;
}
/* Add a space between tokens a and b to avoid unwanted textual pasting */

static int pp_need_space(int a, int b)
{
	return 'E' == a ? '+' == b || '-' == b
	       : '+' == a ? TOK_INC == b || '+' == b
	       : '-' == a ? TOK_DEC == b || '-' == b
	       : a >= TOK_IDENT || a == TOK_PPNUM ? b >= TOK_IDENT || b == TOK_PPNUM
	       : 0;
}
/* maybe hex like 0x1e */

static int pp_check_he0xE(int t, const char *p)
{
	if (t == TOK_PPNUM && toup(strchr(p, 0)[-1]) == 'E')
		return 'E';
	return t;
}
/* Preprocess the current file */

ST_FUNC int tcc_preprocess(TCCState *s1)
{
	BufferedFile **iptr;
	int token_seen, spcs, level;
	const char *p;
	char white[400];

	parse_flags = PARSE_FLAG_PREPROCESS
		      | (parse_flags & PARSE_FLAG_ASM_FILE)
		      | PARSE_FLAG_LINEFEED
		      | PARSE_FLAG_SPACES
		      | PARSE_FLAG_ACCEPT_STRAYS
		      ;
	/* Credits to Fabrice Bellard's initial revision to demonstrate its
	       capability to compile and run itself, provided all numbers are
	       given as decimals. tcc -E -P10 will do. */

	if (s1->Pflag == LINE_MACRO_OUTPUT_FORMAT_P10)
		parse_flags |= PARSE_FLAG_TOK_NUM, s1->Pflag = 1;

	if (s1->do_bench) {
		/* for PP benchmarks */

		do
			next();
		while (tok != TOK_EOF);
		return 0;
	}

	token_seen = TOK_LINEFEED, spcs = 0, level = 0;
	if (file->prev)
		pp_line(s1, file->prev, level++);
	pp_line(s1, file, level);

	for (;;) {
		iptr = s1->include_stack_ptr;
		next();
		if (tok == TOK_EOF)
			break;

		level = s1->include_stack_ptr - iptr;
		if (level) {
			if (level > 0)
				pp_line(s1, *iptr, 0);
			pp_line(s1, file, level);
		}
		if (s1->dflag & 7) {
			pp_debug_defines(s1);
			if (s1->dflag & 4)
				continue;
		}

		if (is_space(tok)) {
			if (spcs < sizeof white - 1)
				white[spcs++] = tok;
			continue;
		} else if (tok == TOK_LINEFEED) {
			spcs = 0;
			if (token_seen == TOK_LINEFEED)
				continue;
			++file->line_ref;
		} else if (token_seen == TOK_LINEFEED) {
			pp_line(s1, file, 0);
		} else if (spcs == 0 && pp_need_space(token_seen, tok)) {
			white[spcs++] = ' ';
		}

		white[spcs] = 0, fputs(white, s1->ppfp), spcs = 0;
		fputs(p = get_tok_str(tok, &tokc), s1->ppfp);
		token_seen = pp_check_he0xE(tok, p);
	}
	return 0;
}
/* ------------------------------------------------------------------------- */
/* ==================== tccgen.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) tcc_state->sym
#define TCC_SET_STATE(fn) fn
#undef _tcc_error
/**/
/* global variables */
/* loc : local variable index
   ind : output code index
   rsym: return symbol
   anon_sym: anonymous symbol index
*/
ST_DATA int rsym, anon_sym, ind, loc;

ST_DATA Sym *global_stack;
ST_DATA Sym *local_stack;
ST_DATA Sym *define_stack;
ST_DATA Sym *global_label_stack;
ST_DATA Sym *local_label_stack;

static Sym *sym_free_first;
static void **sym_pools;
static int nb_sym_pools;

static Sym *all_cleanups, *pending_gotos;
static int local_scope;
ST_DATA char debug_modes;

ST_DATA SValue *vtop;
static SValue _vstack[1 + VSTACK_SIZE];
#define vstack (_vstack + 1)

ST_DATA int nocode_wanted;/* no code generation wanted */

/* no static data output wanted either */
#define NODATA_WANTED (nocode_wanted > 0)
/* ON outside of functions and for static initializers */
#define DATA_ONLY_WANTED 0x80000000
/* no code output after unconditional jumps such as with if (0) ... */

#define CODE_OFF_BIT 0x20000000
#define CODE_OFF() if(!nocode_wanted)(nocode_wanted |= CODE_OFF_BIT)
#define CODE_ON() (nocode_wanted &= ~CODE_OFF_BIT)
/* no code output when parsing sizeof()/typeof() etc. (using nocode_wanted++/--) */

#define NOEVAL_MASK 0x0000FFFF
#define NOEVAL_WANTED (nocode_wanted & NOEVAL_MASK)
/* no code output when parsing constant expressions */

#define CONST_WANTED_BIT 0x00010000
#define CONST_WANTED_MASK 0x0FFF0000
#define CONST_WANTED (nocode_wanted & CONST_WANTED_MASK)

ST_DATA int
global_expr;/* true if compound literals must be allocated globally (used during initializers parsing */

ST_DATA CType
func_vt;/* current function return type (used by return instruction) */

ST_DATA int
func_var;/* true if current function is variadic (used by return instruction) */

ST_DATA int func_vc;/* stack address for implicit struct return storage */

ST_DATA int func_ind;/* function start address */

static int func_old;
ST_DATA const char *funcname;
ST_DATA CType int_type, func_old_type, char_type, char_pointer_type;
static CString initstr;
#define VT_SIZE_T (VT_LLONG | VT_UNSIGNED)
#define VT_PTRDIFF_T VT_LLONG

static struct switch_t {
	struct case_t {
		int64_t v1, v2;
		int ind, line;
	} **p;
	int n;/* list of case ranges */

	int def_sym;/* default symbol */

	int nocode_wanted;
	int *bsym;
	struct scope *scope;
	struct switch_t *prev;
	SValue sv;
} *cur_switch;/* current switch */

#define MAX_TEMP_LOCAL_VARIABLE_NUMBER 8
/*list of temporary local variables on the stack in current function. */

static struct temp_local_variable {
	int location;//offset on stack. Svalue.c.i

	short size;
	short align;
} arr_temp_local_vars[MAX_TEMP_LOCAL_VARIABLE_NUMBER];
static int nb_temp_local_vars;

static struct scope {
	struct scope *prev;
	struct {
		int loc, locorig, num;
	} vla;
	struct {
		Sym *s;
		int n;
	} cl;
	int *bsym, *csym;
	Sym *lstk, *llstk;
} *cur_scope, *loop_scope, *root_scope;

typedef struct {
	Section *sec;
	int local_offset;
	Sym *flex_array_ref;
} init_params;
static void init_prec(void);

static void block(int flags);
#define STMT_EXPR 1
#define STMT_COMPOUND 2

static void gen_cast(CType *type);
static void gen_cast_s(int t);
static inline CType *pointed_type(CType *type);
static int is_compatible_types(CType *type1, CType *type2);
static int parse_btype(CType *type, AttributeDef *ad, int ignore_label);
static CType *type_decl(CType *type, AttributeDef *ad, int *v, int td);
static void parse_expr_type(CType *type);
static void init_putv(init_params *p, CType *type, unsigned long c);
static void decl_initializer(init_params *p, CType *type, unsigned long c,
			     int flags);
static void decl_initializer_alloc(CType *type, AttributeDef *ad, int r,
				   int has_init, int v, int scope);
static int decl(int l);
static void expr_eq(void);
static void vpush_type_size(CType *type, int *a);
static int is_compatible_unqualified_types(CType *type1, CType *type2);
static inline int64_t expr_const64(void);
static void vpush64(int ty, unsigned long long v);
static void vpush(CType *type);
static int gvtst(int inv, int t);
static void gen_inline_functions(TCCState *s);
static void free_inline_functions(TCCState *s);
static void skip_or_save_block(TokenString **str);
static void gv_dup(void);
static int get_temp_local_var(int size,int align,int *r2);
static void cast_error(CType *st, CType *dt);
static void end_switch(void);
static void do_Static_assert(void);
/* ------------------------------------------------------------------------- */
/* Automagical code suppression */
/* Clear 'nocode_wanted' at forward label if it was used */

ST_FUNC void gsym(int t)
{
	if (t) {
		gsym_addr(t, ind);
		CODE_ON();
	}
}
/* Clear 'nocode_wanted' if current pc is a label */

static int gind()
{
	int t = ind;
	CODE_ON();
	if (debug_modes)
		tcc_tcov_block_begin(tcc_state);
	return t;
}
/* Set 'nocode_wanted' after unconditional (backwards) jump */

static void gjmp_addr_acs(int t)
{
	gjmp_addr(t);
	CODE_OFF();
}
/* Set 'nocode_wanted' after unconditional (forwards) jump */

static int gjmp_acs(int t)
{
	t = gjmp(t);
	CODE_OFF();
	return t;
}
/* These are #undef'd at the end of this file */

#define gjmp_addr gjmp_addr_acs
#define gjmp gjmp_acs
/* ------------------------------------------------------------------------- */

ST_INLN int is_float(int t)
{
	int bt = t & VT_BTYPE;
	return bt == VT_LDOUBLE
	       || bt == VT_DOUBLE
	       || bt == VT_FLOAT
	       || bt == VT_QFLOAT;
}

static inline int is_integer_btype(int bt)
{
	return bt == VT_BYTE
	       || bt == VT_BOOL
	       || bt == VT_SHORT
	       || bt == VT_INT
	       || bt == VT_LLONG;
}

static int btype_size(int bt)
{
	return bt == VT_BYTE || bt == VT_BOOL ? 1 :
	       bt == VT_SHORT ? 2 :
	       bt == VT_INT ? 4 :
	       bt == VT_LLONG ? 8 :
	       bt == VT_PTR ? PTR_SIZE : 0;
}
/* returns function return register from type */

static int R_RET(int t)
{
	if (!is_float(t))
		return REG_IRET;

	if ((t & VT_BTYPE) == VT_LDOUBLE)
		return TREG_ST0;

	return REG_FRET;
}
/* returns 2nd function return register, if any */

static int R2_RET(int t)
{
	t &= VT_BTYPE;

	if (t == VT_QLONG)
		return REG_IRE2;
	if (t == VT_QFLOAT)
		return REG_FRE2;

	return VT_CONST;
}
/* returns true for two-word types */

#define USING_TWO_WORDS(t) (R2_RET(t) != VT_CONST)
/* put function return registers to stack value */

static void PUT_R_RET(SValue *sv, int t)
{
	sv->r = R_RET(t), sv->r2 = R2_RET(t);
}
/* returns function return register class for type t */

static int RC_RET(int t)
{
	return reg_classes[R_RET(t)] & ~(RC_FLOAT | RC_INT);
}
/* returns generic register class for type t */

static int RC_TYPE(int t)
{
	if (!is_float(t))
		return RC_INT;

	if ((t & VT_BTYPE) == VT_LDOUBLE)
		return RC_ST0;
	if ((t & VT_BTYPE) == VT_QFLOAT)
		return RC_FRET;

	return RC_FLOAT;
}
/* returns 2nd register class corresponding to t and rc */

static int RC2_TYPE(int t, int rc)
{
	if (!USING_TWO_WORDS(t))
		return 0;

	if (rc == RC_IRET)
		return RC_IRE2;

	if (rc == RC_FRET)
		return RC_FRE2;

	if (rc & RC_FLOAT)
		return RC_FLOAT;
	return RC_INT;
}
/* we use our own 'finite' function to avoid potential problems with
   non standard math libs */
/* XXX: endianness dependent */

ST_FUNC int ieee_finite(double d)
{
	int p[4];
	memcpy(p, &d, sizeof(double));
	return ((unsigned)((p[1] | 0x800fffff) + 1)) >> 31;
}

ST_FUNC void test_lvalue(void)
{
	if (!(vtop->r & VT_LVAL))
		expect("lvalue");
}

ST_FUNC void check_vstack(void)
{
	if (vtop != vstack - 1)
		tcc_error("internal compiler error: vstack leak (%d)",
			  (int)(vtop - vstack + 1));
}
/* ------------------------------------------------------------------------- */
/* initialize vstack and types.  This must be done also for tcc -E */
ST_FUNC void tccgen_init(TCCState *s1)
{
	vtop = vstack - 1;
	memset(vtop, 0, sizeof *vtop);
	/* define some often used types */

	int_type.t = VT_INT;

	char_type.t = VT_BYTE;
	if (s1->char_is_unsigned)
		char_type.t |= VT_UNSIGNED;
	char_pointer_type = char_type;
	mk_pointer(&char_pointer_type);

	func_old_type.t = VT_FUNC;
	func_old_type.ref = sym_push(SYM_FIELD, &int_type, 0, 0);
	func_old_type.ref->f.func_call = FUNC_CDECL;
	func_old_type.ref->f.func_type = FUNC_OLD;

	init_prec();

	cstr_new(&initstr);
}

ST_FUNC int tccgen_compile(TCCState *s1)
{
	funcname = "";
	func_ind = -1;
	anon_sym = SYM_FIRST_ANOM;
	nocode_wanted = DATA_ONLY_WANTED;/* no code outside of functions */

	debug_modes = (s1->do_debug ? 1 : 0) | s1->test_coverage << 1;
	global_expr = 0;

	tcc_debug_start(s1);
	tcc_tcov_start (s1);

	parse_flags = PARSE_FLAG_PREPROCESS | PARSE_FLAG_TOK_NUM | PARSE_FLAG_TOK_STR;
	next();
	decl(VT_CONST);
	gen_inline_functions(s1);
	check_vstack();
	/* end of translation unit info */

	tcc_debug_end(s1);
	tcc_tcov_end(s1);
	return 0;
}

ST_FUNC void tccgen_finish(TCCState *s1)
{
	tcc_debug_end(s1);/* just in case of errors: free memory */

	free_inline_functions(s1);
	sym_pop(&global_stack, NULL, 0);
	sym_pop(&local_stack, NULL, 0);
	/* free preprocessor macros */

	free_defines(NULL);
	/* free sym_pools */

	dynarray_reset(&sym_pools, &nb_sym_pools);
	cstr_free(&initstr);
	dynarray_reset(&stk_data, &nb_stk_data);
	while (cur_switch)
		end_switch();
	local_scope = 0;
	loop_scope = NULL;
	all_cleanups = NULL;
	pending_gotos = NULL;
	nb_temp_local_vars = 0;
	global_label_stack = NULL;
	local_label_stack = NULL;
	cur_text_section = NULL;
	sym_free_first = NULL;
}
/* ------------------------------------------------------------------------- */

ST_FUNC ElfSym *elfsym(Sym *s)
{
	if (!s || !s->c)
		return NULL;
	return &((ElfSym *)symtab_section->data)[s->c];
}
/* apply storage attributes to Elf symbol */

ST_FUNC void update_storage(Sym *sym)
{
	ElfSym *esym;
	int sym_bind, old_sym_bind;

	esym = elfsym(sym);
	if (!esym)
		return;

	if (sym->a.visibility)
		esym->st_other = (esym->st_other & ~ELFW(ST_VISIBILITY)(-1))
				 | sym->a.visibility;

	if (sym->type.t & (VT_STATIC | VT_INLINE))
		sym_bind = STB_LOCAL;
	else if (sym->a.weak)
		sym_bind = STB_WEAK;
	else
		sym_bind = STB_GLOBAL;
	old_sym_bind = ELFW(ST_BIND)(esym->st_info);
	if (sym_bind != old_sym_bind) {
		esym->st_info = ELFW(ST_INFO)(sym_bind, ELFW(ST_TYPE)(esym->st_info));
	}

	if (sym->a.dllimport)
		esym->st_other |= ST_PE_IMPORT;
	if (sym->a.dllexport)
		esym->st_other |= ST_PE_EXPORT;
}
/* ------------------------------------------------------------------------- */
/* update sym->c so that it points to an external symbol in section
   'section' with value 'value' */

ST_FUNC void put_extern_sym2(Sym *sym, int sh_num,
			     addr_t value, unsigned long size,
			     int can_add_underscore)
{
	int sym_type, sym_bind, info, other, t;
	ElfSym *esym;
	const char *name;
	char buf1[256];

	if (!sym->c) {
		name = get_tok_str(sym->v, NULL);
		t = sym->type.t;
		if ((t & VT_BTYPE) == VT_FUNC) {
			sym_type = STT_FUNC;
		} else if ((t & VT_BTYPE) == VT_VOID) {
			sym_type = STT_NOTYPE;
			if (IS_ASM_FUNC(t))
				sym_type = STT_FUNC;
		} else if (t & VT_TLS) {
			sym_type = STT_TLS;
		} else {
			sym_type = STT_OBJECT;
		}
		if (t & (VT_STATIC | VT_INLINE))
			sym_bind = STB_LOCAL;
		else
			sym_bind = STB_GLOBAL;
		other = 0;

		if (sym_type == STT_FUNC && sym->type.ref) {
			Sym *ref = sym->type.ref;
			if (ref->a.nodecorate) {
				can_add_underscore = 0;
			}
			if (ref->f.func_call == FUNC_STDCALL && can_add_underscore) {
				sprintf(buf1, "_%s@%d", name, ref->f.func_args * PTR_SIZE);
				name = buf1;
				other |= ST_PE_STDCALL;
				can_add_underscore = 0;
			}
		}
		if (sym->asm_label) {
			name = get_tok_str(sym->asm_label, NULL);
			can_add_underscore = 0;
		}

		if (tcc_state->leading_underscore && can_add_underscore) {
			buf1[0] = '_';
			pstrcpy(buf1 + 1, sizeof(buf1) - 1, name);
			name = buf1;
		}

		info = ELFW(ST_INFO)(sym_bind, sym_type);
		sym->c = put_elf_sym(symtab_section, value, size, info, other, sh_num, name);

		if (debug_modes)
			tcc_debug_extern_sym(tcc_state, sym, sh_num, sym_bind, sym_type);

	} else {
		esym = elfsym(sym);
		esym->st_value = value;
		esym->st_size = size;
		esym->st_shndx = sh_num;
	}
	update_storage(sym);
}

ST_FUNC void put_extern_sym(Sym *sym, Section *s, addr_t value,
			    unsigned long size)
{
	if (nocode_wanted && (NODATA_WANTED || (s && s == cur_text_section)))
		return;
	put_extern_sym2(sym, s ? s->sh_num : SHN_UNDEF, value, size, 1);
}
/* add a new relocation entry to symbol 'sym' in section 's' */

ST_FUNC void greloca(Section *s, Sym *sym, unsigned long offset, int type,
		     addr_t addend)
{
	int c = 0;

	if (nocode_wanted && s == cur_text_section)
		return;

	if (sym) {
		if (0 == sym->c) {
			put_extern_sym(sym, NULL, 0, 0);
			if (sym->sym_scope
			    && (sym->type.t & (VT_STATIC|VT_EXTERN)) == (VT_STATIC|VT_EXTERN)) {
				/* when a local function declaraion redeclares a global static one
				                   then tccelf would not resolve them to the same symbol. */

				Sym *s = sym;
				while (s->prev_tok)
					s = s->prev_tok;
				s->c = sym->c;
			}
		}
		c = sym->c;
	}
	/* now we can add ELF relocation info */

	put_elf_reloca(symtab_section, s, offset, type, c, addend);
}
/* ------------------------------------------------------------------------- */
/* symbol allocator */
static Sym *__sym_malloc(void)
{
	Sym *sym_pool, *sym, *last_sym;
	int i;

	sym_pool = tcc_malloc(SYM_POOL_NB * sizeof(Sym));
	dynarray_add(&sym_pools, &nb_sym_pools, sym_pool);

	last_sym = sym_free_first;
	sym = sym_pool;
	for (i = 0; i < SYM_POOL_NB; i++) {
		sym->next = last_sym;
		last_sym = sym;
		sym++;
	}
	sym_free_first = last_sym;
	return last_sym;
}

static inline Sym *sym_malloc(void)
{
	Sym *sym;

	sym = sym_free_first;
	if (!sym)
		sym = __sym_malloc();
	sym_free_first = sym->next;
	return sym;

}

ST_INLN void sym_free(Sym *sym)
{

	sym->next = sym_free_first;
	sym_free_first = sym;

}
/* push, without hashing */

ST_FUNC Sym *sym_push2(Sym **ps, int v, int t, int c)
{
	Sym *s;

	s = sym_malloc();
	memset(s, 0, sizeof *s);
	s->v = v;
	s->type.t = t;
	s->c = c;
	/* add in stack */

	s->prev = *ps;
	*ps = s;
	return s;
}
/* find a symbol and return its associated structure. 's' is the top
   of the symbol stack */

ST_FUNC Sym *sym_find2(Sym *s, int v)
{
	while (s) {
		if (s->v == v)
			return s;
		s = s->prev;
	}
	return NULL;
}
/* structure lookup */

ST_INLN Sym *struct_find(int v)
{
	v -= TOK_IDENT;
	if ((unsigned)v >= (unsigned)(tok_ident - TOK_IDENT))
		return NULL;
	return table_ident[v]->sym_struct;
}
/* find an identifier */

ST_INLN Sym *sym_find(int v)
{
	v -= TOK_IDENT;
	if ((unsigned)v >= (unsigned)(tok_ident - TOK_IDENT))
		return NULL;
	return table_ident[v]->sym_identifier;
}
/* make sym in-/visible to the parser */

static inline void sym_link(Sym *s, int yes)
{
	TokenSym *ts = table_ident[(s->v & ~SYM_STRUCT) - TOK_IDENT];
	Sym **ps;
	if (s->v & SYM_STRUCT)
		ps = &ts->sym_struct;
	else
		ps = &ts->sym_identifier;
	if (yes) {
		s->prev_tok = *ps, *ps = s;
		s->sym_scope = local_scope;
	} else {
		*ps = s->prev_tok;
	}
}

static inline int sym_scope_ex(Sym *s)
{
	/* enums have 'sym_scope' overwritten by 'enum_val' */

	return IS_ENUM_VAL (s->type.t)
	       ? s->type.ref->sym_scope
	       : s->sym_scope;
}
/* push a given symbol on the symbol stack */

ST_FUNC Sym *sym_push(int v, CType *type, int r, int c)
{
	Sym *s, **ps;
	if (local_stack)
		ps = &local_stack;
	else
		ps = &global_stack;
	s = sym_push2(ps, v, type->t, c);
	s->type.ref = type->ref;
	s->r = r;
	/* don't record fields or anonymous symbols */

	if ((v & ~SYM_STRUCT) < SYM_FIRST_ANOM) {
		/* record symbol in token array */

		sym_link(s, 1);
		if (s->prev_tok && sym_scope_ex(s->prev_tok) == local_scope)
			tcc_error("redeclaration of '%s'", get_tok_str(s->v, NULL));
	}
	return s;
}
/* push a global identifier */

ST_FUNC Sym *global_identifier_push(int v, int t, int c)
{
	Sym *s, **ps;
	s = sym_push2(&global_stack, v, t, c);
	s->r = VT_CONST | VT_SYM;
	/* don't record anonymous symbol */

	if (v < SYM_FIRST_ANOM) {
		ps = &table_ident[v - TOK_IDENT]->sym_identifier;
		/* modify the top most local identifier, so that sym_identifier will
		           point to 's' when popped; happens when called from inline asm */

		while (*ps != NULL && (*ps)->sym_scope)
			ps = &(*ps)->prev_tok;
		s->prev_tok = *ps;
		*ps = s;
	}
	return s;
}
/* pop symbols until top reaches 'b'.  If KEEP is non-zero don't really
   pop them yet from the list, but do remove them from the token array.  */

ST_FUNC void sym_pop(Sym **ptop, Sym *b, int keep)
{
	Sym *s, *ss;
	int v;

	s = *ptop;
	while (s != b) {
		ss = s->prev;
		v = s->v;
		/* remove symbol in token array */

		if ((v & ~SYM_STRUCT) < SYM_FIRST_ANOM)
			sym_link(s, 0);
		if (!keep)
			sym_free(s);
		s = ss;
	}
	if (!keep)
		*ptop = b;
}
/* label lookup */

ST_FUNC Sym *label_find(int v)
{
	v -= TOK_IDENT;
	if ((unsigned)v >= (unsigned)(tok_ident - TOK_IDENT))
		return NULL;
	return table_ident[v]->sym_label;
}

ST_FUNC Sym *label_push(Sym **ptop, int v, int flags)
{
	Sym *s, **ps;
	s = sym_push2(ptop, v, VT_STATIC, 0);
	s->r = flags;
	ps = &table_ident[v - TOK_IDENT]->sym_label;
	if (ptop == &global_label_stack) {
		/* modify the top most local identifier, so that
		           sym_identifier will point to 's' when popped */

		while (*ps != NULL)
			ps = &(*ps)->prev_tok;
	}
	s->prev_tok = *ps;
	*ps = s;
	return s;
}
/* pop labels until element last is reached. Look if any labels are
   undefined. Define symbols if '&&label' was used. */

ST_FUNC void label_pop(Sym **ptop, Sym *slast, int keep)
{
	Sym *s, *s1;
	for (s = *ptop; s != slast; s = s1) {
		s1 = s->prev;
		if (s->r == LABEL_DECLARED) {
			tcc_warning_c(warn_all)("label '%s' declared but not used", get_tok_str(s->v,
					NULL));
		} else if (s->r == LABEL_FORWARD) {
			tcc_error("label '%s' used but not defined",
				  get_tok_str(s->v, NULL));
		} else {
			if (s->c) {
				/* define corresponding symbol. A size of
				                   1 is put. */

				put_extern_sym(s, cur_text_section, s->jind, 1);
			}
		}
		/* remove label */

		if (s->r != LABEL_GONE)
			table_ident[s->v - TOK_IDENT]->sym_label = s->prev_tok;
		if (!keep)
			sym_free(s);
		else
			s->r = LABEL_GONE;
	}
	if (!keep)
		*ptop = slast;
}
/* ------------------------------------------------------------------------- */

static void vcheck_cmp(void)
{
	/* cannot let cpu flags if other instruction are generated. Also
	       avoid leaving VT_JMP anywhere except on the top of the stack
	       because it would complicate the code generator.

	       Don't do this when nocode_wanted.  vtop might come from
	       !nocode_wanted regions (see 88_codeopt.c) and transforming
	       it to a register without actually generating code is wrong
	       as their value might still be used for real.  All values
	       we push under nocode_wanted will eventually be popped
	       again, so that the VT_CMP/VT_JMP value will be in vtop
	       when code is unsuppressed again. */
	/* However if it's just automatic suppression via CODE_OFF/ON()
	       then it seems that we better let things work undisturbed.
	       How can it work at all under nocode_wanted?  Well, gv() will
	       actually clear it at the gsym() in load()/VT_JMP in the
	       generator backends */
	if (vtop->r == VT_CMP && 0 == (nocode_wanted & ~CODE_OFF_BIT))
		gv(RC_INT);
}

static void vsetc(CType *type, int r, CValue *vc)
{
	if (vtop >= vstack + (VSTACK_SIZE - 1))
		tcc_error("memory full (vstack)");
	vcheck_cmp();
	vtop++;
	vtop->type = *type;
	vtop->r = r;
	vtop->r2 = VT_CONST;
	vtop->c = *vc;
	vtop->sym = NULL;
}

ST_FUNC void vswap(void)
{
	SValue tmp;

	vcheck_cmp();
	tmp = vtop[0];
	vtop[0] = vtop[-1];
	vtop[-1] = tmp;
}
/* pop stack value */

ST_FUNC void vpop(void)
{
	int v;
	v = vtop->r & VT_VALMASK;
	/* for x86, we need to pop the FP stack */

	if (v == TREG_ST0) {
		o(0xd8dd);/* fstp %st(0) */

	} else

		if (v == VT_CMP) {
			/* need to put correct jump if && or || without test */

			gsym(vtop->jtrue);
			gsym(vtop->jfalse);
		}
	vtop--;
}
/* push constant of type "type" with useless value */

static void vpush(CType *type)
{
	vset(type, VT_CONST, 0);
}
/* push arbitrary 64bit constant */

static void vpush64(int ty, unsigned long long v)
{
	CValue cval;
	CType ctype;
	ctype.t = ty;
	ctype.ref = NULL;
	cval.i = v;
	vsetc(&ctype, VT_CONST, &cval);
}
/* push integer constant */

ST_FUNC void vpushi(int v)
{
	vpush64(VT_INT, v);
}
/* push a pointer sized constant */

static void vpushs(addr_t v)
{
	vpush64(VT_SIZE_T, v);
}
/* push long long constant */

static inline void vpushll(long long v)
{
	vpush64(VT_LLONG, v);
}

ST_FUNC void vset(CType *type, int r, int v)
{
	CValue cval;
	cval.i = v;
	vsetc(type, r, &cval);
}

static void vseti(int r, int v)
{
	CType type;
	type.t = VT_INT;
	type.ref = NULL;
	vset(&type, r, v);
}

ST_FUNC void vpushv(SValue *v)
{
	if (vtop >= vstack + (VSTACK_SIZE - 1))
		tcc_error("memory full (vstack)");
	vtop++;
	*vtop = *v;
}

static void vdup(void)
{
	vpushv(vtop);
}
/* rotate the stack element at position n-1 to the top */

ST_FUNC void vrotb(int n)
{
	SValue tmp;
	if (--n < 1)
		return;
	vcheck_cmp();
	tmp = vtop[-n];
	memmove(vtop - n, vtop - n + 1, sizeof *vtop * n);
	vtop[0] = tmp;
}
/* rotate the top stack element into position n-1 */

ST_FUNC void vrott(int n)
{
	SValue tmp;
	if (--n < 1)
		return;
	vcheck_cmp();
	tmp = vtop[0];
	memmove(vtop - n + 1, vtop - n, sizeof *vtop * n);
	vtop[-n] = tmp;
}
/* reverse order of the the first n stack elements */

ST_FUNC void vrev(int n)
{
	int i;
	SValue tmp;
	vcheck_cmp();
	for (i = 0, n = -n; i > ++n; --i)
		tmp = vtop[i], vtop[i] = vtop[n], vtop[n] = tmp;
}
/* ------------------------------------------------------------------------- */
/* vtop->r = VT_CMP means CPU-flags have been set from comparison or test. */
/* called from generators to set the result from relational ops  */

ST_FUNC void vset_VT_CMP(int op)
{
	vtop->r = VT_CMP;
	vtop->cmp_op = op;
	vtop->jfalse = 0;
	vtop->jtrue = 0;
}
/* called once before asking generators to load VT_CMP to a register */

static void vset_VT_JMP(void)
{
	int op = vtop->cmp_op;

	if (vtop->jtrue || vtop->jfalse) {
		int origt = vtop->type.t;
		/* we need to jump to 'mov $0,%R' or 'mov $1,%R' */

		int inv = op & (op < 2);/* small optimization */

		vseti(VT_JMP+inv, gvtst(inv, 0));
		vtop->type.t |= origt & (VT_UNSIGNED | VT_DEFSIGN);
	} else {
		/* otherwise convert flags (rsp. 0/1) to register */

		vtop->c.i = op;
		if (op < 2)/* doesn't seem to happen */

			vtop->r = VT_CONST;
	}
}
/* Set CPU Flags, doesn't yet jump */

static void gvtst_set(int inv, int t)
{
	int *p;

	if (vtop->r != VT_CMP) {
		vpushi(0);
		gen_op(TOK_NE);
		if (vtop->r != VT_CMP)/* must be VT_CONST then */

			vset_VT_CMP(vtop->c.i != 0);
	}

	p = inv ? &vtop->jfalse : &vtop->jtrue;
	*p = gjmp_append(*p, t);
}
/* Generate value test
 *
 * Generate a test for any value (jump, comparison and integers) */

static int gvtst(int inv, int t)
{
	int op, x, u;

	gvtst_set(inv, t);
	t = vtop->jtrue, u = vtop->jfalse;
	if (inv)
		x = u, u = t, t = x;
	op = vtop->cmp_op;
	/* jump to the wanted target */

	if (op > 1)
		t = gjmp_cond(op ^ inv, t);
	else if (op != inv)
		t = gjmp(t);
	/* resolve complementary jumps to here */

	gsym(u);

	vtop--;
	return t;
}
/* generate a zero or nozero test */

static void gen_test_zero(int op)
{
	if (vtop->r == VT_CMP) {
		int j;
		if (op == TOK_EQ) {
			j = vtop->jfalse;
			vtop->jfalse = vtop->jtrue;
			vtop->jtrue = j;
			vtop->cmp_op ^= 1;
		}
	} else {
		vpushi(0);
		gen_op(op);
	}
}
/* ------------------------------------------------------------------------- */
/* push a symbol value of TYPE */

ST_FUNC void vpushsym(CType *type, Sym *sym)
{
	CValue cval;
	cval.i = 0;
	vsetc(type, VT_CONST | VT_SYM, &cval);
	vtop->sym = sym;
}
/* Return a static symbol pointing to a section */

ST_FUNC Sym *get_sym_ref(CType *type, Section *sec, unsigned long offset,
			 unsigned long size)
{
	int v;
	Sym *sym;

	v = anon_sym++;
	sym = sym_push(v, type, VT_CONST | VT_SYM, 0);
	sym->type.t |= VT_STATIC;
	put_extern_sym(sym, sec, offset, size);
	return sym;
}
/* push a reference to a section offset by adding a dummy symbol */

static void vpush_ref(CType *type, Section *sec, unsigned long offset,
		      unsigned long size)
{
	vpushsym(type, get_sym_ref(type, sec, offset, size));
}
/* define a new external reference to a symbol 'v' of type 'u' */

ST_FUNC Sym *external_global_sym(int v, CType *type)
{
	Sym *s;

	s = sym_find(v);
	if (!s) {
		/* push forward reference */

		s = global_identifier_push(v, type->t | VT_EXTERN, 0);
		s->type.ref = type->ref;
	} else if (IS_ASM_SYM(s)) {
		s->type.t = type->t | (s->type.t & VT_EXTERN);
		s->type.ref = type->ref;
		update_storage(s);
	}
	return s;
}
/* create an external reference with no specific type similar to asm labels.
   This avoids type conflicts if the symbol is used from C too */

ST_FUNC Sym *external_helper_sym(int v)
{
	CType ct = { VT_ASM_FUNC, NULL };
	return external_global_sym(v, &ct);
}
/* push a reference to an helper function (such as memmove) */

ST_FUNC void vpush_helper_func(int v)
{
	vpushsym(&func_old_type, external_helper_sym(v));
}
/* Merge symbol attributes.  */

static void merge_symattr(struct SymAttr *sa, struct SymAttr *sa1)
{
	if (sa1->aligned && !sa->aligned)
		sa->aligned = sa1->aligned;
	sa->packed |= sa1->packed;
	sa->weak |= sa1->weak;
	sa->nodebug |= sa1->nodebug;
	if (sa1->visibility != STV_DEFAULT) {
		int vis = sa->visibility;
		if (vis == STV_DEFAULT
		    || vis > sa1->visibility)
			vis = sa1->visibility;
		sa->visibility = vis;
	}
	sa->dllexport |= sa1->dllexport;
	sa->nodecorate |= sa1->nodecorate;
	sa->dllimport |= sa1->dllimport;
}
/* Merge function attributes.  */

static void merge_funcattr(struct FuncAttr *fa, struct FuncAttr *fa1)
{
	if (fa1->func_call && !fa->func_call)
		fa->func_call = fa1->func_call;
	if (fa1->func_type && !fa->func_type)
		fa->func_type = fa1->func_type;
	if (fa1->func_args && !fa->func_args)
		fa->func_args = fa1->func_args;
	if (fa1->func_noreturn)
		fa->func_noreturn = 1;
	if (fa1->func_ctor)
		fa->func_ctor = 1;
	if (fa1->func_dtor)
		fa->func_dtor = 1;
}
/* Merge attributes.  */

static void merge_attr(AttributeDef *ad, AttributeDef *ad1)
{
	merge_symattr(&ad->a, &ad1->a);
	merge_funcattr(&ad->f, &ad1->f);

	if (ad1->section)
		ad->section = ad1->section;
	if (ad1->alias_target)
		ad->alias_target = ad1->alias_target;
	if (ad1->asm_label)
		ad->asm_label = ad1->asm_label;
	if (ad1->attr_mode)
		ad->attr_mode = ad1->attr_mode;
}
/* Merge some type attributes.  */

static void patch_type(Sym *sym, CType *type)
{
	if (!(type->t & VT_EXTERN) || IS_ENUM_VAL(sym->type.t)) {
		if (!(sym->type.t & VT_EXTERN))
			tcc_error("redefinition of '%s'", get_tok_str(sym->v, NULL));
		sym->type.t &= ~VT_EXTERN;
	}

	if (IS_ASM_SYM(sym)) {
		/* stay static if both are static */

		sym->type.t = type->t & (sym->type.t | ~VT_STATIC);
		sym->type.ref = type->ref;
		if ((type->t & VT_BTYPE) != VT_FUNC && !(type->t & VT_ARRAY))
			sym->r |= VT_LVAL;
	}

	if (!is_compatible_types(&sym->type, type)) {
		tcc_error("incompatible types for redefinition of '%s'",
			  get_tok_str(sym->v, NULL));

	} else if ((sym->type.t & VT_BTYPE) == VT_FUNC) {
		int static_proto = sym->type.t & VT_STATIC;
		int ft1 = sym->type.ref->f.func_type;
		int ft2 = type->ref->f.func_type;
		/* warn if static follows non-static function declaration */

		if ((type->t & VT_STATIC) && !static_proto
		    /* XXX this test for inline shouldn't be here.  Until we
		                   implement gnu-inline mode again it silences a warning for
		                   mingw caused by our workarounds.  */

		    && !((type->t | sym->type.t) & VT_INLINE))
			tcc_warning("static storage ignored for redefinition of '%s'",
				    get_tok_str(sym->v, NULL));
		/* set 'inline' if both agree or if one has static */

		if ((type->t | sym->type.t) & VT_INLINE) {
			if (!((type->t ^ sym->type.t) & VT_INLINE)
			    || ((type->t | sym->type.t) & VT_STATIC))
				static_proto |= VT_INLINE;
		}

		if (0 == (type->t & VT_EXTERN)) {
			struct FuncAttr f = sym->type.ref->f;
			/* put complete type, use static from prototype */

			sym->type.t = (type->t & ~(VT_STATIC|VT_INLINE)) | static_proto;
			if (ft1 != FUNC_OLD)
				type->ref->f.func_type = ft1;
			sym->type.ref = type->ref;
			merge_funcattr(&sym->type.ref->f, &f);
		} else {
			sym->type.t &= ~VT_INLINE | static_proto;
			if (ft1 == FUNC_OLD && ft2 != FUNC_OLD)
				sym->type.ref = type->ref;
		}

	} else {
		if ((sym->type.t & VT_ARRAY) && type->ref->c >= 0) {
			/* set array size if it was omitted in extern declaration */

			sym->type.ref->c = type->ref->c;
		}
		if ((type->t ^ sym->type.t) & VT_STATIC)
			tcc_warning("storage mismatch for redefinition of '%s'",
				    get_tok_str(sym->v, NULL));
	}
}
/* Merge some storage attributes.  */

static void patch_storage(Sym *sym, AttributeDef *ad, CType *type)
{
	if (type)
		patch_type(sym, type);

	if (sym->a.dllimport != ad->a.dllimport)
		tcc_error("incompatible dll linkage for redefinition of '%s'",
			  get_tok_str(sym->v, NULL));

	merge_symattr(&sym->a, &ad->a);
	if (ad->asm_label)
		sym->asm_label = ad->asm_label;
	update_storage(sym);
}
/* copy sym to other stack */

static Sym *sym_copy(Sym *s0, Sym **ps)
{
	Sym *s;
	s = sym_malloc(), *s = *s0;
	s->prev = *ps, *ps = s;
	if ((s->v & ~SYM_STRUCT) < SYM_FIRST_ANOM)
		sym_link(s, 1);
	return s;
}
/* Symbol 's' was locally declared 'extern' (or as function), and is
   on global_stack.  Now must copy its 'ref' to global_stack too */

static void move_ref_to_global(Sym *s)
{
	Sym *l, **lp;
	int n, bt;

	bt = s->type.t & VT_BTYPE;
	if (!(bt == VT_PTR
	      || bt == VT_FUNC
	      || bt == VT_STRUCT
	      || IS_ENUM(s->type.t)))
		return;

	for (s = s->type.ref, n = 0; s; s = s->next) {
		for (lp = &local_stack; !!(l = *lp); lp = &l->prev) {
			if (l == s) {
				*lp = s->prev;
				s->prev = global_stack, global_stack = s;
				if (n || bt == VT_PTR || bt == VT_FUNC) {
					move_ref_to_global(s);
				} else {
					if ((s->v & ~SYM_STRUCT) < SYM_FIRST_ANOM) {
						/* copy struct/enum tag to local scope */

						s->v |= SYM_FIELD;
						l = sym_copy(s, lp);
						l->v &= ~SYM_FIELD;
					}
				}
				if (bt != VT_PTR)
					n = 1;
				break;
			}
		}
		if (n == 0)/* no next (VT_PTR) or ref not on local_stack */

			break;
	}
}
/* define a new external reference to a symbol 'v' */

static Sym *external_sym(int v, CType *type, int r, AttributeDef *ad)
{
	Sym *s;
	/* look for global symbol */

	s = sym_find(v);
	while (s && s->sym_scope)
		s = s->prev_tok;

	if (!s) {
		/* push forward reference */

		s = global_identifier_push(v, type->t, 0);
		s->r |= r;
		s->a = ad->a;
		s->asm_label = ad->asm_label;
		s->type.ref = type->ref;
	} else {
		patch_storage(s, ad, type);
	}
	if (local_stack) {
		/* make sure that type->ref is on global stack */

		move_ref_to_global(s);
		/* put into local scope */

		sym_copy(s, &local_stack);
	}
	return s;
}
/* save registers up to (vtop - n) stack entry */

ST_FUNC void save_regs(int n)
{
	SValue *p, *p1;
	for (p = vstack, p1 = vtop - n; p <= p1; p++)
		save_reg(p->r);
}
/* save r to the memory stack, and mark it as being free */

ST_FUNC void save_reg(int r)
{
	save_reg_upstack(r, 0);
}
/* save r to the memory stack, and mark it as being free,
   if seen up to (vtop - n) stack entry */

ST_FUNC void save_reg_upstack(int r, int n)
{
	int l, size, align, bt, r2;
	SValue *p, *p1, sv;

	if ((r &= VT_VALMASK) >= VT_CONST)
		return;
	if (nocode_wanted)
		return;
	l = r2 = 0;
	for (p = vstack, p1 = vtop - n; p <= p1; p++) {
		if ((p->r & VT_VALMASK) == r || p->r2 == r) {
			/* must save value on stack if not already done */

			if (!l) {
				bt = p->type.t & VT_BTYPE;
				if (bt == VT_VOID)
					continue;
				if ((p->r & VT_LVAL) || bt == VT_FUNC)
					bt = VT_PTR;
				sv.type.t = bt;
				size = type_size(&sv.type, &align);
				l = get_temp_local_var(size, align, &r2);
				sv.r = VT_LOCAL | VT_LVAL;
				sv.c.i = l;
				sv.sym = NULL;
				store(p->r & VT_VALMASK, &sv);
				/* x86 specific: need to pop fp register ST0 if saved */

				if (r == TREG_ST0) {
					o(0xd8dd);/* fstp %st(0) */

				}
				/* special long long case */

				if (p->r2 < VT_CONST && USING_TWO_WORDS(bt)) {
					sv.c.i += PTR_SIZE;
					store(p->r2, &sv);
				}
			}
			/* mark that stack entry as being saved on the stack */

			if (p->r & VT_LVAL) {
				/* also clear the bounded flag because the
				                   relocation address of the function was stored in
				                   p->c.i */

				p->r = (p->r & ~(VT_VALMASK | VT_BOUNDED)) | VT_LLOCAL;
			} else {
				p->r = VT_LVAL | VT_LOCAL;
				p->type.t &= ~VT_ARRAY;/* cannot combine VT_LVAL with VT_ARRAY */

			}
			p->sym = NULL;
			p->r2 = r2;
			p->c.i = l;
		}
	}
}
/* find a free register of class 'rc'. If none, save one register */
ST_FUNC int get_reg(int rc)
{
	int r;
	SValue *p;
	/* find a free register */

	for (r=0; r<NB_REGS; r++) {
		if (reg_classes[r] & rc) {
			if (nocode_wanted)
				return r;
			for (p=vstack; p<=vtop; p++) {
				if ((p->r & VT_VALMASK) == r ||
				    p->r2 == r)
					goto notfound;
			}
			return r;
		}
notfound: ;
	}
	/* no register left : free the first one on the stack (VERY
	       IMPORTANT to start from the bottom to ensure that we don't
	       spill registers used in gen_opi()) */

	for (p=vstack; p<=vtop; p++) {
		/* look at second register (if long long) */

		r = p->r2;
		if (r < VT_CONST && (reg_classes[r] & rc))
			goto save_found;
		r = p->r & VT_VALMASK;
		if (r < VT_CONST && (reg_classes[r] & rc)) {
save_found:
			save_reg(r);
			return r;
		}
	}
	/* Should never comes here */

	return -1;
}
/* find a free temporary local variable (return the offset on stack) match
   size and align. If none, add new temporary stack variable */

static int get_temp_local_var(int size,int align, int *r2)
{
	int i;
	struct temp_local_variable *temp_var;
	SValue *p;
	int r;
	unsigned used = 0;
	/* mark locations that are still in use */

	for (p = vstack; p <= vtop; p++) {
		r = p->r & VT_VALMASK;
		if (r == VT_LOCAL || r == VT_LLOCAL) {
			r = p->r2 - (VT_CONST + 1);
			if (r >= 0 && r < MAX_TEMP_LOCAL_VARIABLE_NUMBER)
				used |= 1<<r;
		}
	}
	for (i=0; i<nb_temp_local_vars; i++) {
		temp_var=&arr_temp_local_vars[i];
		if (!(used & 1<<i)
		    && temp_var->size>=size
		    && temp_var->align>=align) {
ret_tmp:
			*r2 = (VT_CONST + 1) + i;
			return temp_var->location;
		}
	}
	loc = (loc - size) & -align;
	if (nb_temp_local_vars<MAX_TEMP_LOCAL_VARIABLE_NUMBER) {
		temp_var=&arr_temp_local_vars[i];
		temp_var->location=loc;
		temp_var->size=size;
		temp_var->align=align;
		nb_temp_local_vars++;
		goto ret_tmp;
	}
	*r2 = VT_CONST;
	return loc;
}
/* move register 's' (of type 't') to 'r', and flush previous value of r to memory
   if needed */

static void move_reg(int r, int s, int t)
{
	SValue sv;

	if (r != s) {
		save_reg(r);
		sv.type.t = t;
		sv.type.ref = NULL;
		sv.r = s;
		sv.c.i = 0;
		load(r, &sv);
	}
}
/* get address of vtop (vtop MUST BE an lvalue) */

ST_FUNC void gaddrof(void)
{
	vtop->r &= ~VT_LVAL;
	/* tricky: if saved lvalue, then we can go back to lvalue */

	if ((vtop->r & VT_VALMASK) == VT_LLOCAL)
		vtop->r = (vtop->r & ~VT_VALMASK) | VT_LOCAL | VT_LVAL;
}
/* add debug info for locals or function parameters, optionally
   register bounds */
static void tcc_debug_end_scope(Sym *b, int bounds)
{

	tcc_add_debug_info (tcc_state, local_stack, b);
}
/* increment an lvalue pointer */

static void incr_offset(int offset)
{
	int t = vtop->type.t;
	gaddrof();/* remove VT_LVAL */

	vtop->type.t = VT_PTRDIFF_T;/* set scalar type */

	vpushs(offset);
	gen_op('+');
	vtop->r |= VT_LVAL;
	vtop->type.t = t;
}

static void incr_bf_adr(int o)
{
	vtop->type.t = VT_BYTE | VT_UNSIGNED;
	incr_offset(o);
}
/* single-byte load mode for packed or otherwise unaligned bitfields */

static void load_packed_bf(CType *type, int bit_pos, int bit_size)
{
	int n, o, bits;
	save_reg_upstack(vtop->r, 1);
	vpush64(type->t & VT_BTYPE, 0);// B X

	bits = 0, o = bit_pos >> 3, bit_pos &= 7;
	do {
		vswap();// X B

		incr_bf_adr(o);
		vdup();// X B B

		n = 8 - bit_pos;
		if (n > bit_size)
			n = bit_size;
		if (bit_pos)
			vpushi(bit_pos), gen_op(TOK_SHR), bit_pos = 0;// X B Y

		if (n < 8)
			vpushi((1 << n) - 1), gen_op('&');
		gen_cast(type);
		if (bits)
			vpushi(bits), gen_op(TOK_SHL);
		vrotb(3);// B Y X

		gen_op('|');// B X

		bits += n, bit_size -= n, o = 1;
	} while (bit_size);
	vswap(), vpop();
	if (!(type->t & VT_UNSIGNED)) {
		n = ((type->t & VT_BTYPE) == VT_LLONG ? 64 : 32) - bits;
		vpushi(n), gen_op(TOK_SHL);
		vpushi(n), gen_op(TOK_SAR);
	}
}
/* single-byte store mode for packed or otherwise unaligned bitfields */

static void store_packed_bf(int bit_pos, int bit_size)
{
	int bits, n, o, m, c;
	c = (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
	vswap();// X B

	save_reg_upstack(vtop->r, 1);
	bits = 0, o = bit_pos >> 3, bit_pos &= 7;
	do {
		incr_bf_adr(o);// X B

		vswap();//B X

		c ? vdup() : gv_dup();// B V X

		vrott(3);// X B V

		if (bits)
			vpushi(bits), gen_op(TOK_SHR);
		if (bit_pos)
			vpushi(bit_pos), gen_op(TOK_SHL);
		n = 8 - bit_pos;
		if (n > bit_size)
			n = bit_size;
		if (n < 8) {
			m = ((1 << n) - 1) << bit_pos;
			vpushi(m), gen_op('&');// X B V1

			vpushv(vtop-1);// X B V1 B

			vpushi(m & 0x80 ? ~m & 0x7f : ~m);
			gen_op('&');// X B V1 B1

			gen_op('|');// X B V2

		}
		vdup(), vtop[-1] = vtop[-2];// X B B V2

		vstore(), vpop();// X B

		bits += n, bit_size -= n, bit_pos = 0, o = 1;
	} while (bit_size);
	vpop(), vpop();
}

static int adjust_bf(SValue *sv, int bit_pos, int bit_size)
{
	int t;
	if (0 == sv->type.ref)
		return 0;
	t = sv->type.ref->auxtype;
	if (t != -1 && t != VT_STRUCT) {
		sv->type.t = (sv->type.t & ~(VT_BTYPE | VT_LONG)) | t;
		sv->r |= VT_LVAL;
	}
	return t;
}
/* store vtop a register belonging to class 'rc'. lvalues are
   converted to values. Cannot be used if cannot be converted to
   register value (such as structures). */

ST_FUNC int gv(int rc)
{
	int r, r2, r_ok, r2_ok, rc2, bt;
	int bit_pos, bit_size, size, align;
	/* NOTE: get_reg can modify vstack[] */

	if (vtop->type.t & VT_BITFIELD) {
		CType type;

		bit_pos = BIT_POS(vtop->type.t);
		bit_size = BIT_SIZE(vtop->type.t);
		/* remove bit field info to avoid loops */

		vtop->type.t &= ~VT_STRUCT_MASK;

		type.ref = NULL;
		type.t = vtop->type.t & VT_UNSIGNED;
		if ((vtop->type.t & VT_BTYPE) == VT_BOOL)
			type.t |= VT_UNSIGNED;

		r = adjust_bf(vtop, bit_pos, bit_size);

		if ((vtop->type.t & VT_BTYPE) == VT_LLONG)
			type.t |= VT_LLONG;
		else
			type.t |= VT_INT;

		if (r == VT_STRUCT) {
			load_packed_bf(&type, bit_pos, bit_size);
		} else {
			int bits = (type.t & VT_BTYPE) == VT_LLONG ? 64 : 32;
			/* cast to int to propagate signedness in following ops */

			gen_cast(&type);
			/* generate shifts */

			vpushi(bits - (bit_pos + bit_size));
			gen_op(TOK_SHL);
			vpushi(bits - bit_size);
			/* NOTE: transformed to SHR if unsigned */

			gen_op(TOK_SAR);
		}
		r = gv(rc);
	} else {
		if (is_float(vtop->type.t) &&
		    (vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
			/* CPUs usually cannot use float constants, so we store them
			               generically in data segment */

			init_params p = { rodata_section };
			unsigned long offset;
			size = type_size(&vtop->type, &align);
			if (NODATA_WANTED)
				size = 0, align = 1;
			offset = section_add(p.sec, size, align);
			vpush_ref(&vtop->type, p.sec, offset, size);
			vswap();
			init_putv(&p, &vtop->type, offset);
			vtop->r |= VT_LVAL;
		}

		bt = vtop->type.t & VT_BTYPE;
		if (bt == VT_VOID || bt == VT_STRUCT)/* should not happen */

			return vtop->r;

		rc2 = RC2_TYPE(bt, rc);
		/* need to reload if:
		           - constant
		           - lvalue (need to dereference pointer)
		           - already a register, but not in the right class */

		r = vtop->r & VT_VALMASK;
		r_ok = !(vtop->r & VT_LVAL) && (r < VT_CONST) && (reg_classes[r] & rc);
		r2_ok = !rc2 || ((vtop->r2 < VT_CONST) && (reg_classes[vtop->r2] & rc2));

		if (!r_ok || !r2_ok) {

			if (!r_ok) {
				if (1/* we can 'mov (r),r' in cases */

				    && r < VT_CONST
				    && (reg_classes[r] & rc)
				    && !rc2
				   )
					save_reg_upstack(r, 1);
				else
					r = get_reg(rc);
			}

			if (rc2) {
				int load_type = (bt == VT_QFLOAT) ? VT_DOUBLE : VT_PTRDIFF_T;
				int original_type = vtop->type.t;
				/* two register type load :
				                   expand to two words temporarily */

				if ((vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
					/* load constant */

					unsigned long long ll = vtop->c.i;
					vtop->c.i = ll;/* first word */

					load(r, vtop);
					vtop->r = r;/* save register value */

					vpushi(ll >> 32);/* second word */

				} else if (vtop->r & VT_LVAL) {
					/* We do not want to modifier the long long pointer here.
					                       So we save any other instances down the stack */

					save_reg_upstack(vtop->r, 1);
					/* load from memory */

					vtop->type.t = load_type;
					load(r, vtop);
					vdup();
					vtop[-1].r = r;/* save register value */

					/* increment pointer to get second word */

					incr_offset(PTR_SIZE);
				} else {
					/* move registers */

					if (!r_ok)
						load(r, vtop);
					if (r2_ok && vtop->r2 < VT_CONST)
						goto done;
					vdup();
					vtop[-1].r = r;/* save register value */

					vtop->r = vtop[-1].r2;
				}
				/* Allocate second register. Here we rely on the fact that
				                   get_reg() tries first to free r2 of an SValue. */

				r2 = get_reg(rc2);
				load(r2, vtop);
				vpop();
				/* write second register */

				vtop->r2 = r2;
done:
				vtop->type.t = original_type;
			} else {
				if (vtop->r == VT_CMP)
					vset_VT_JMP();
				/* one register type load */

				load(r, vtop);
			}
		}
		vtop->r = r;

	}
	return r;
}
/* generate vtop[-1] and vtop[0] in resp. classes rc1 and rc2 */

ST_FUNC void gv2(int rc1, int rc2)
{
	/* generate more generic register first. But VT_JMP or VT_CMP
	       values must be generated first in all cases to avoid possible
	       reload errors */

	if (vtop->r != VT_CMP && rc1 <= rc2) {
		vswap();
		gv(rc1);
		vswap();
		gv(rc2);
		/* test if reload is needed for first register */

		if ((vtop[-1].r & VT_VALMASK) >= VT_CONST) {
			vswap();
			gv(rc1);
			vswap();
		}
	} else {
		gv(rc2);
		vswap();
		gv(rc1);
		vswap();
		/* test if reload is needed for first register */

		if ((vtop[0].r & VT_VALMASK) >= VT_CONST) {
			gv(rc2);
		}
	}
}
/* convert stack entry to register and duplicate its value in another
   register */
static void gv_dup(void)
{
	int t, rc, r;

	t = vtop->type.t;
	/* duplicate value */
	rc = RC_TYPE(t);
	gv(rc);
	r = get_reg(rc);
	vdup();
	load(r, vtop);
	vtop->r = r;
}
/* normalize values */
static uint64_t value64(uint64_t l1, int t)
{
	if ((t & VT_BTYPE) == VT_LLONG
	    || (PTR_SIZE == 8 && (t & VT_BTYPE) == VT_PTR))
		return l1;
	else if (t & VT_UNSIGNED)
		return (uint32_t)l1;
	else
		return (uint32_t)l1 | -(l1 & 0x80000000);
}

static uint64_t gen_opic_sdiv(uint64_t a, uint64_t b)
{
	uint64_t x = (a >> 63 ? -a : a) / (b >> 63 ? -b : b);
	return (a ^ b) >> 63 ? -x : x;
}

static int gen_opic_lt(uint64_t a, uint64_t b)
{
	return (a ^ (uint64_t)1 << 63) < (b ^ (uint64_t)1 << 63);
}
/* handle integer constant optimizations and various machine
   independent opt */

static void gen_opic(int op)
{
	SValue *v1 = vtop - 1;
	SValue *v2 = vtop;
	int t1 = v1->type.t & VT_BTYPE;
	int t2 = v2->type.t & VT_BTYPE;
	int c1 = (v1->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
	int c2 = (v2->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
	uint64_t l1 = c1 ? value64(v1->c.i, v1->type.t) : 0;
	uint64_t l2 = c2 ? value64(v2->c.i, v2->type.t) : 0;
	int shm = (t1 == VT_LLONG) ? 63 : 31;
	int r;

	if (c1 && c2) {
		switch (op) {
		case '+':
			l1 += l2;
			break;
		case '-':
			l1 -= l2;
			break;
		case '&':
			l1 &= l2;
			break;
		case '^':
			l1 ^= l2;
			break;
		case '|':
			l1 |= l2;
			break;
		case '*':
			l1 *= l2;
			break;

		case TOK_PDIV:
		case '/':
		case '%':
		case TOK_UDIV:
		case TOK_UMOD:
			/* if division by zero, generate explicit division */

			if (l2 == 0) {
				if (CONST_WANTED && !NOEVAL_WANTED)
					tcc_error("division by zero in constant");
				goto general_case;
			}
			switch (op) {
			default:
				l1 = gen_opic_sdiv(l1, l2);
				break;
			case '%':
				l1 = l1 - l2 * gen_opic_sdiv(l1, l2);
				break;
			case TOK_UDIV:
				l1 = l1 / l2;
				break;
			case TOK_UMOD:
				l1 = l1 % l2;
				break;
			}
			break;
		case TOK_SHL:
			l1 <<= (l2 & shm);
			break;
		case TOK_SHR:
			l1 >>= (l2 & shm);
			break;
		case TOK_SAR:
			l1 = (l1 >> 63) ? ~(~l1 >> (l2 & shm)) : l1 >> (l2 & shm);
			break;
		/* tests */

		case TOK_ULT:
			l1 = l1 < l2;
			break;
		case TOK_UGE:
			l1 = l1 >= l2;
			break;
		case TOK_EQ:
			l1 = l1 == l2;
			break;
		case TOK_NE:
			l1 = l1 != l2;
			break;
		case TOK_ULE:
			l1 = l1 <= l2;
			break;
		case TOK_UGT:
			l1 = l1 > l2;
			break;
		case TOK_LT:
			l1 = gen_opic_lt(l1, l2);
			break;
		case TOK_GE:
			l1 = !gen_opic_lt(l1, l2);
			break;
		case TOK_LE:
			l1 = !gen_opic_lt(l2, l1);
			break;
		case TOK_GT:
			l1 = gen_opic_lt(l2, l1);
			break;
		/* logical */

		case TOK_LAND:
			l1 = l1 && l2;
			break;
		case TOK_LOR:
			l1 = l1 || l2;
			break;
		default:
			goto general_case;
		}
		v1->c.i = value64(l1, v1->type.t);
		v1->r |= v2->r & VT_NONCONST;
		vtop--;
	} else {
		/* if commutative ops, put c2 as constant */

		if (c1 && (op == '+' || op == '&' || op == '^' ||
			   op == '|' || op == '*' || op == TOK_EQ || op == TOK_NE)) {
			vswap();
			c2 = c1;//c = c1, c1 = c2, c2 = c;

			l2 = l1;//l = l1, l1 = l2, l2 = l;

		}
		if (c1 && ((l1 == 0 &&
			    (op == TOK_SHL || op == TOK_SHR || op == TOK_SAR)) ||
			   (l1 == -1 && op == TOK_SAR))) {
			/* treat (0 << x), (0 >> x) and (-1 >> x) as constant */

			vpop();
		} else if (c2 && ((l2 == 0 && (op == '&' || op == '*')) ||
				  (op == '|' &&
				   (l2 == -1 || (l2 == 0xFFFFFFFF && t2 != VT_LLONG))) ||
				  (l2 == 1 && (op == '%' || op == TOK_UMOD)))) {
			/* treat (x & 0), (x * 0), (x | -1) and (x % 1) as constant */

			if (l2 == 1)
				vtop->c.i = 0;
			vswap();
			vtop--;
		} else if (c2 && (((op == '*' || op == '/' || op == TOK_UDIV ||
				    op == TOK_PDIV) &&
				   l2 == 1) ||
				  ((op == '+' || op == '-' || op == '|' || op == '^' ||
				    op == TOK_SHL || op == TOK_SHR || op == TOK_SAR) &&
				   l2 == 0) ||
				  (op == '&' &&
				   (l2 == -1 || (l2 == 0xFFFFFFFF && t2 != VT_LLONG))))) {
			/* filter out NOP operations like x*1, x-0, x&-1... */

			vtop--;
		} else if (c2 && (op == '*' || op == TOK_PDIV || op == TOK_UDIV
				  || op == TOK_UMOD)) {
			/* try to use shifts instead of muls or divs */

			if (l2 > 0 && (l2 & (l2 - 1)) == 0) {
				int n = -1;
				if (op == TOK_UMOD) {
					vtop->c.i = l2 - 1;
					op = '&';
					goto general_case;
				}
				while (l2) {
					l2 >>= 1;
					n++;
				}
				vtop->c.i = n;
				if (op == '*')
					op = TOK_SHL;
				else if (op == TOK_PDIV)
					op = TOK_SAR;
				else
					op = TOK_SHR;
			}
			goto general_case;
		} else if (c2 && (op == '+' || op == '-') &&
			   (r = vtop[-1].r & (VT_VALMASK | VT_LVAL | VT_SYM),
			    r == (VT_CONST | VT_SYM) || r == VT_LOCAL)) {
			/* symbol + constant case */

			if (op == '-')
				l2 = -l2;
			l2 += vtop[-1].c.i;
			/* The backends can't always deal with addends to symbols
				       larger than +-1<<31.  Don't construct such.  */

			if ((int)l2 != l2)
				goto general_case;
			vtop--;
			vtop->c.i = l2;
		} else {
general_case:
			/* call low level op generator */

			if (t1 == VT_LLONG || t2 == VT_LLONG ||
			    (PTR_SIZE == 8 && (t1 == VT_PTR || t2 == VT_PTR)))
				gen_opl(op);
			else
				gen_opi(op);
		}
		if (vtop->r == VT_CONST)
			vtop->r |= VT_NONCONST;/* is const, but only by optimization */

	}
}

#define gen_negf gen_opf
/* generate a floating point operation with constant propagation */
static void gen_opif(int op)
{
	int c1, c2, i, bt;
	SValue *v1, *v2;

	long double f1, f2;

	v1 = vtop - 1;
	v2 = vtop;
	if (op == TOK_NEG)
		v1 = v2;
	bt = v1->type.t & VT_BTYPE;
	/* currently, we cannot do computations with forward symbols */

	c1 = (v1->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
	c2 = (v2->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
	if (c1 && c2) {
		if (bt == VT_FLOAT) {
			f1 = v1->c.f;
			f2 = v2->c.f;
		} else if (bt == VT_DOUBLE) {
			f1 = v1->c.d;
			f2 = v2->c.d;
		} else {
			f1 = v1->c.ld;
			f2 = v2->c.ld;
		}
		/* NOTE: we only do constant propagation if finite number (not
		           NaN or infinity) (ANSI spec) */

		if (!(ieee_finite(f1) || !ieee_finite(f2)) && !CONST_WANTED)
			goto general_case;
		switch (op) {
		case '+':
			f1 += f2;
			break;
		case '-':
			f1 -= f2;
			break;
		case '*':
			f1 *= f2;
			break;
		case '/':
			if (f2 == 0.0) {
				union {
					float f;
					unsigned u;
				} x1, x2, y;
				/* If not in initializer we need to potentially generate
						   FP exceptions at runtime, otherwise we want to fold.  */

				if (!CONST_WANTED)
					goto general_case;
				/* the run-time result of 0.0/0.0 on x87, also of other compilers
				                   when used t\o compile the f1 /= f2 below, would be -nan */

				x1.f = f1, x2.f = f2;
				if (f1 == 0.0)
					y.u = 0x7fc00000;/* nan */

				else
					y.u = 0x7f800000;/* infinity */

				y.u |= (x1.u ^ x2.u) & 0x80000000;/* set sign */

				f1 = y.f;
				break;
			}
			f1 /= f2;
			break;
		case TOK_NEG:
			f1 = -f1;
			goto unary_result;
		case TOK_EQ:
			i = f1 == f2;
make_int:
			vtop -= 2;
			vpushi(i);
			return;
		case TOK_NE:
			i = f1 != f2;
			goto make_int;
		case TOK_LT:
			i = f1 < f2;
			goto make_int;
		case TOK_GE:
			i = f1 >= f2;
			goto make_int;
		case TOK_LE:
			i = f1 <= f2;
			goto make_int;
		case TOK_GT:
			i = f1 > f2;
			goto make_int;
		default:
			goto general_case;
		}
		vtop--;
unary_result:
		/* XXX: overflow test ? */

		if (bt == VT_FLOAT) {
			v1->c.f = f1;
		} else if (bt == VT_DOUBLE) {
			v1->c.d = f1;
		} else {
			v1->c.ld = f1;
		}
	} else {
general_case:
		if (op == TOK_NEG) {
			gen_negf(op);
		} else {
			gen_opf(op);
		}
	}
}
/* print a type. If 'varstr' is not NULL, then the variable is also
   printed in the type */
/* XXX: union */
/* XXX: add array and function pointers */

static void type_to_str(char *buf, int buf_size,
			CType *type, const char *varstr)
{
	int bt, v, t;
	Sym *s, *sa;
	char buf1[256];
	const char *tstr;

	t = type->t;
	bt = t & VT_BTYPE;
	buf[0] = '\0';

	if (t & VT_EXTERN)
		pstrcat(buf, buf_size, "extern ");
	if (t & VT_STATIC)
		pstrcat(buf, buf_size, "static ");
	if (t & VT_TYPEDEF)
		pstrcat(buf, buf_size, "typedef ");
	if (t & VT_INLINE)
		pstrcat(buf, buf_size, "inline ");
	if (bt != VT_PTR) {
		if (t & VT_VOLATILE)
			pstrcat(buf, buf_size, "volatile ");
		if (t & VT_CONSTANT)
			pstrcat(buf, buf_size, "const ");
	}
	if (((t & VT_DEFSIGN) && bt == VT_BYTE)
	    || ((t & VT_UNSIGNED)
		&& (bt == VT_SHORT || bt == VT_INT || bt == VT_LLONG)
		&& !IS_ENUM(t)
	       ))
		pstrcat(buf, buf_size, (t & VT_UNSIGNED) ? "unsigned " : "signed ");

	buf_size -= strlen(buf);
	buf += strlen(buf);

	switch (bt) {
	case VT_VOID:
		tstr = "void";
		goto add_tstr;
	case VT_BOOL:
		tstr = "_Bool";
		goto add_tstr;
	case VT_BYTE:
		tstr = "char";
		goto add_tstr;
	case VT_SHORT:
		tstr = "short";
		goto add_tstr;
	case VT_INT:
		tstr = "int";
		goto maybe_long;
	case VT_LLONG:
		tstr = "long long";
maybe_long:
		if (t & VT_LONG)
			tstr = "long";
		if (!IS_ENUM(t))
			goto add_tstr;
		tstr = "enum ";
		goto tstruct;
	case VT_FLOAT:
		tstr = "float";
		goto add_tstr;
	case VT_DOUBLE:
		tstr = "double";
		if (!(t & VT_LONG))
			goto add_tstr;
	case VT_LDOUBLE:
		tstr = "long double";
add_tstr:
		pstrcat(buf, buf_size, tstr);
		break;
	case VT_STRUCT:
		tstr = "struct ";
		if (IS_UNION(t))
			tstr = "union ";
tstruct:
		pstrcat(buf, buf_size, tstr);
		v = type->ref->v & ~SYM_STRUCT;
		if (v >= SYM_FIRST_ANOM)
			pstrcat(buf, buf_size, "<anonymous>");
		else
			pstrcat(buf, buf_size, get_tok_str(v, NULL));
		break;
	case VT_FUNC:
		s = type->ref;
		buf1[0]=0;
		if (varstr && '*' == *varstr) {
			pstrcat(buf1, sizeof(buf1), "(");
			pstrcat(buf1, sizeof(buf1), varstr);
			pstrcat(buf1, sizeof(buf1), ")");
		}
		pstrcat(buf1, buf_size, "(");
		sa = s->next;
		while (sa != NULL) {
			char buf2[256];
			type_to_str(buf2, sizeof(buf2), &sa->type, NULL);
			pstrcat(buf1, sizeof(buf1), buf2);
			sa = sa->next;
			if (sa)
				pstrcat(buf1, sizeof(buf1), ", ");
		}
		if (s->f.func_type == FUNC_ELLIPSIS)
			pstrcat(buf1, sizeof(buf1), ", ...");
		pstrcat(buf1, sizeof(buf1), ")");
		type_to_str(buf, buf_size, &s->type, buf1);
		goto no_var;
	case VT_PTR:
		s = type->ref;
		if (t & (VT_ARRAY|VT_VLA)) {
			if (varstr && '*' == *varstr)
				snprintf(buf1, sizeof(buf1), "(%s)[%d]", varstr, s->c);
			else
				snprintf(buf1, sizeof(buf1), "%s[%d]", varstr ? varstr : "", s->c);
			type_to_str(buf, buf_size, &s->type, buf1);
			goto no_var;
		}
		pstrcpy(buf1, sizeof(buf1), "*");
		if (t & VT_CONSTANT)
			pstrcat(buf1, buf_size, "const ");
		if (t & VT_VOLATILE)
			pstrcat(buf1, buf_size, "volatile ");
		if (varstr)
			pstrcat(buf1, sizeof(buf1), varstr);
		type_to_str(buf, buf_size, &s->type, buf1);
		goto no_var;
	}
	if (varstr) {
		pstrcat(buf, buf_size, " ");
		pstrcat(buf, buf_size, varstr);
	}
no_var: ;
}

static void type_incompatibility_error(CType* st, CType* dt, const char *fmt)
{
	char buf1[256], buf2[256];
	type_to_str(buf1, sizeof(buf1), st, NULL);
	type_to_str(buf2, sizeof(buf2), dt, NULL);
	tcc_error(fmt, buf1, buf2);
}

static void type_incompatibility_warning(CType* st, CType* dt, const char *fmt)
{
	char buf1[256], buf2[256];
	type_to_str(buf1, sizeof(buf1), st, NULL);
	type_to_str(buf2, sizeof(buf2), dt, NULL);
	tcc_warning(fmt, buf1, buf2);
}

static int pointed_size(CType *type)
{
	int align;
	return type_size(pointed_type(type), &align);
}

static inline int is_null_pointer(SValue *p)
{
	if ((p->r & (VT_VALMASK | VT_LVAL | VT_SYM | VT_NONCONST)) != VT_CONST)
		return 0;
	return ((p->type.t & VT_BTYPE) == VT_INT && (uint32_t)p->c.i == 0) ||
	       ((p->type.t & VT_BTYPE) == VT_LLONG && p->c.i == 0) ||
	       ((p->type.t & VT_BTYPE) == VT_PTR &&
		(PTR_SIZE == 4 ? (uint32_t)p->c.i == 0 : p->c.i == 0) &&
		((pointed_type(&p->type)->t & VT_BTYPE) == VT_VOID) &&
		0 == (pointed_type(&p->type)->t & (VT_CONSTANT | VT_VOLATILE))
	       );
}
/* compare function types. OLD functions match any new functions */

static int is_compatible_func(CType *type1, CType *type2)
{
	Sym *s1, *s2;

	s1 = type1->ref;
	s2 = type2->ref;
	if (s1->f.func_call != s2->f.func_call)
		return 0;
	if (s1->f.func_type != s2->f.func_type
	    && s1->f.func_type != FUNC_OLD
	    && s2->f.func_type != FUNC_OLD)
		return 0;
	for (;;) {
		if (!is_compatible_unqualified_types(&s1->type, &s2->type))
			return 0;
		if (s1->f.func_type == FUNC_OLD || s2->f.func_type == FUNC_OLD )
			return 1;
		s1 = s1->next;
		s2 = s2->next;
		if (!s1)
			return !s2;
		if (!s2)
			return 0;
	}
}
/* return true if type1 and type2 are the same.  If unqualified is
   true, qualifiers on the types are ignored.
 */

static int compare_types(CType *type1, CType *type2, int unqualified)
{
	int bt1, t1, t2;

	if (IS_ENUM(type1->t)) {
		if (IS_ENUM(type2->t))
			return type1->ref == type2->ref;
		type1 = &type1->ref->type;
	} else if (IS_ENUM(type2->t))
		type2 = &type2->ref->type;

	t1 = type1->t & VT_TYPE;
	t2 = type2->t & VT_TYPE;
	if (unqualified) {
		/* strip qualifiers before comparing */

		t1 &= ~(VT_CONSTANT | VT_VOLATILE);
		t2 &= ~(VT_CONSTANT | VT_VOLATILE);
	}
	/* Default Vs explicit signedness only matters for char */

	if ((t1 & VT_BTYPE) != VT_BYTE) {
		t1 &= ~VT_DEFSIGN;
		t2 &= ~VT_DEFSIGN;
	}
	/* XXX: bitfields ? */

	if (t1 != t2)
		return 0;

	if ((t1 & VT_ARRAY)
	    && !(type1->ref->c < 0
		 || type2->ref->c < 0
		 || type1->ref->c == type2->ref->c))
		return 0;
	/* test more complicated cases */

	bt1 = t1 & VT_BTYPE;
	if (bt1 == VT_PTR) {
		type1 = pointed_type(type1);
		type2 = pointed_type(type2);
		return is_compatible_types(type1, type2);
	} else if (bt1 == VT_STRUCT) {
		return (type1->ref == type2->ref);
	} else if (bt1 == VT_FUNC) {
		return is_compatible_func(type1, type2);
	} else {
		return 1;
	}
}

#define CMP_OP 'C'
#define SHIFT_OP 'S'
/* Check if OP1 and OP2 can be "combined" with operation OP, the combined
   type is stored in DEST if non-null (except for pointer plus/minus) . */

static int combine_types(CType *dest, SValue *op1, SValue *op2, int op)
{
	CType *type1, *type2, type;
	int t1, t2, bt1, bt2;
	int ret = 1;
	/* for shifts, 'combine' only left operand */

	if (op == SHIFT_OP)
		op2 = op1;

	type1 = &op1->type, type2 = &op2->type;
	t1 = type1->t, t2 = type2->t;
	bt1 = t1 & VT_BTYPE, bt2 = t2 & VT_BTYPE;

	type.t = VT_VOID;
	type.ref = NULL;

	if (bt1 == VT_VOID || bt2 == VT_VOID) {
		if (op != '?')
			tcc_error("operation on void value");
		/* NOTE: as an extension, we accept void on only one side */

		type.t = VT_VOID;
	} else if (bt1 == VT_PTR || bt2 == VT_PTR) {
		if (op == '+') {
			if (!is_integer_btype(bt1 == VT_PTR ? bt2 : bt1))
				ret = 0;
		}
		/* http://port70.net/~nsz/c/c99/n1256.html#6.5.15p6 */
		/* If one is a null ptr constant the result type is the other.  */

		else if (is_null_pointer (op2))
			type = *type1;
		else if (is_null_pointer (op1))
			type = *type2;
		else if (bt1 != bt2) {
			/* accept comparison or cond-expr between pointer and integer
			               with a warning */

			if ((op == '?' || op == CMP_OP)
			    && (is_integer_btype(bt1) || is_integer_btype(bt2)))
				tcc_warning("pointer/integer mismatch in %s",
					    op == '?' ? "conditional expression" : "comparison");
			else if (op != '-' || !is_integer_btype(bt2))
				ret = 0;
			type = *(bt1 == VT_PTR ? type1 : type2);
		} else {
			CType *pt1 = pointed_type(type1);
			CType *pt2 = pointed_type(type2);
			int pbt1 = pt1->t & VT_BTYPE;
			int pbt2 = pt2->t & VT_BTYPE;
			int newquals, copied = 0;
			if (pbt1 != VT_VOID && pbt2 != VT_VOID
			    && !compare_types(pt1, pt2, 1/*unqualif*/
					     )) {
				if (op != '?' && op != CMP_OP)
					ret = 0;
				else
					type_incompatibility_warning(type1, type2,
								     op == '?'
								     ? "pointer type mismatch in conditional expression ('%s' and '%s')"
								     : "pointer type mismatch in comparison('%s' and '%s')");
			}
			if (op == '?') {
				/* pointers to void get preferred, otherwise the
				                   pointed to types minus qualifs should be compatible */

				type = *((pbt1 == VT_VOID) ? type1 : type2);
				/* combine qualifs */

				newquals = ((pt1->t | pt2->t) & (VT_CONSTANT | VT_VOLATILE));
				if ((~pointed_type(&type)->t & (VT_CONSTANT | VT_VOLATILE))
				    & newquals) {
					/* copy the pointer target symbol */

					type.ref = sym_push(SYM_FIELD, &type.ref->type,
							    0, type.ref->c);
					copied = 1;
					pointed_type(&type)->t |= newquals;
				}
				/* pointers to incomplete arrays get converted to
				                   pointers to completed ones if possible */

				if (pt1->t & VT_ARRAY
				    && pt2->t & VT_ARRAY
				    && pointed_type(&type)->ref->c < 0
				    && (pt1->ref->c > 0 || pt2->ref->c > 0)) {
					if (!copied)
						type.ref = sym_push(SYM_FIELD, &type.ref->type,
								    0, type.ref->c);
					pointed_type(&type)->ref =
						sym_push(SYM_FIELD, &pointed_type(&type)->ref->type,
							 0, pointed_type(&type)->ref->c);
					pointed_type(&type)->ref->c =
						0 < pt1->ref->c ? pt1->ref->c : pt2->ref->c;
				}
			}
		}
		if (op == CMP_OP)
			type.t = VT_SIZE_T;
	} else if (bt1 == VT_STRUCT || bt2 == VT_STRUCT) {
		if (op != '?' || !compare_types(type1, type2, 1))
			ret = 0;
		type = *type1;
	} else if (is_float(bt1) || is_float(bt2)) {
		if (bt1 == VT_LDOUBLE || bt2 == VT_LDOUBLE) {
			type.t = VT_LDOUBLE;
		} else if (bt1 == VT_DOUBLE || bt2 == VT_DOUBLE) {
			type.t = VT_DOUBLE;
		} else {
			type.t = VT_FLOAT;
		}
	} else if (bt1 == VT_LLONG || bt2 == VT_LLONG) {
		/* cast to biggest op */

		type.t = VT_LLONG | VT_LONG;
		if (bt1 == VT_LLONG)
			type.t &= t1;
		if (bt2 == VT_LLONG)
			type.t &= t2;
		/* convert to unsigned if it does not fit in a long long */

		if ((t1 & (VT_BTYPE | VT_UNSIGNED)) == (VT_LLONG | VT_UNSIGNED) ||
		    (t2 & (VT_BTYPE | VT_UNSIGNED)) == (VT_LLONG | VT_UNSIGNED))
			type.t |= VT_UNSIGNED;
	} else {
		/* integer operations */

		type.t = VT_INT | (VT_LONG & (t1 | t2));
		/* convert to unsigned if it does not fit in an integer */

		if (((t1 & (VT_BTYPE | VT_UNSIGNED)) == (VT_INT | VT_UNSIGNED)
		     && (!(t1 & VT_BITFIELD) || BIT_SIZE(t1) == 32))
		    || ((t2 & (VT_BTYPE | VT_UNSIGNED)) == (VT_INT | VT_UNSIGNED)
			&& (!(t2 & VT_BITFIELD) || BIT_SIZE(t2) == 32)))
			type.t |= VT_UNSIGNED;
	}
	if (dest)
		*dest = type;
	return ret;
}
/* generic gen_op: handles types problems */

ST_FUNC void gen_op(int op)
{
	int t1, t2, bt1, bt2, t;
	CType type1, combtype;
	int op_class = op;

	if (op == TOK_SHR || op == TOK_SAR || op == TOK_SHL)
		op_class = SHIFT_OP;
	else if (TOK_ISCOND(op))/* == != > ... */

		op_class = CMP_OP;

redo:
	t1 = vtop[-1].type.t;
	t2 = vtop[0].type.t;
	bt1 = t1 & VT_BTYPE;
	bt2 = t2 & VT_BTYPE;

	if (bt1 == VT_FUNC || bt2 == VT_FUNC) {
		if (bt2 == VT_FUNC) {
			mk_pointer(&vtop->type);
			gaddrof();
		}
		if (bt1 == VT_FUNC) {
			vswap();
			mk_pointer(&vtop->type);
			gaddrof();
			vswap();
		}
		goto redo;
	} else if (!combine_types(&combtype, vtop - 1, vtop, op_class)) {
op_err:
		tcc_error("invalid operand types for binary operation");
	} else if (bt1 == VT_PTR || bt2 == VT_PTR) {
		/* at least one operand is a pointer */
		/* relational op: must be both pointers */

		int align;
		if (op_class == CMP_OP)
			goto std_op;
		/* if both pointers, then it must be the '-' op */

		if (bt1 == VT_PTR && bt2 == VT_PTR) {
			if (op != '-')
				goto op_err;
			vpush_type_size(pointed_type(&vtop[-1].type), &align);
			vtop->type.t &= ~VT_UNSIGNED;
			vrott(3);
			gen_opic(op);
			vtop->type.t = VT_PTRDIFF_T;
			vswap();
			gen_op(TOK_PDIV);
		} else {
			/* exactly one pointer : must be '+' or '-'. */

			if (op != '-' && op != '+')
				goto op_err;
			/* Put pointer as first operand */

			if (bt2 == VT_PTR) {
				vswap();
				t = t1, t1 = t2, t2 = t;
				bt2 = bt1;
			}

			type1 = vtop[-1].type;
			vpush_type_size(pointed_type(&vtop[-1].type), &align);
			vtop->type.t &= ~VT_UNSIGNED;
			gen_op('*');
			{
				gen_opic(op);
			}
			type1.t &= ~(VT_ARRAY|VT_VLA);
			/* put again type if gen_opic() swaped operands */

			vtop->type = type1;
		}
	} else {
		/* floats can only be used for a few operations */

		if (is_float(combtype.t)
		    && op != '+' && op != '-' && op != '*' && op != '/'
		    && op_class != CMP_OP) {
			goto op_err;
		}
std_op:
		t = t2 = combtype.t;
		/* special case for shifts and long long: we keep the shift as
		           an integer */

		if (op_class == SHIFT_OP)
			t2 = VT_INT;
		/* XXX: currently, some unsigned operations are explicit, so
		           we modify them here */

		if (t & VT_UNSIGNED) {
			if (op == TOK_SAR)
				op = TOK_SHR;
			else if (op == '/')
				op = TOK_UDIV;
			else if (op == '%')
				op = TOK_UMOD;
			else if (op == TOK_LT)
				op = TOK_ULT;
			else if (op == TOK_GT)
				op = TOK_UGT;
			else if (op == TOK_LE)
				op = TOK_ULE;
			else if (op == TOK_GE)
				op = TOK_UGE;
		}
		vswap();
		gen_cast_s(t);
		vswap();
		gen_cast_s(t2);
		if (is_float(t))
			gen_opif(op);
		else
			gen_opic(op);
		if (op_class == CMP_OP) {
			/* relational op: the result is an int */

			vtop->type.t = VT_INT;
		} else {
			vtop->type.t = t;
		}
	}
// Make sure that we have converted to an rvalue:

	if (vtop->r & VT_LVAL)
		gv(RC_TYPE(vtop->type.t));
}
/* generic itof for unsigned long long case */

static void gen_cvt_itof1(int t)
{
	if ((vtop->type.t & (VT_BTYPE | VT_UNSIGNED)) ==
	    (VT_LLONG | VT_UNSIGNED)) {

		if (t == VT_FLOAT)
			vpush_helper_func(TOK___floatundisf);

		else if (t == VT_LDOUBLE)
			vpush_helper_func(TOK___floatundixf);

		else
			vpush_helper_func(TOK___floatundidf);
		vrott(2);
		gfunc_call(1);
		vpushi(0);
		PUT_R_RET(vtop, t);
	} else {
		gen_cvt_itof(t);
	}
}
/* generic ftoi for unsigned long long case */

static void gen_cvt_ftoi1(int t)
{
	int st;
	if (t == (VT_LLONG | VT_UNSIGNED)) {
		/* not handled natively */

		st = vtop->type.t & VT_BTYPE;
		if (st == VT_FLOAT)
			vpush_helper_func(TOK___fixunssfdi);

		else if (st == VT_LDOUBLE)
			vpush_helper_func(TOK___fixunsxfdi);

		else
			vpush_helper_func(TOK___fixunsdfdi);
		vrott(2);
		gfunc_call(1);
		vpushi(0);
		PUT_R_RET(vtop, t);
	} else {
		gen_cvt_ftoi(t);
	}
}
/* special delayed cast for char/short */

static void force_charshort_cast(void)
{
	int sbt = BFGET(vtop->r, VT_MUSTCAST) == 2 ? VT_LLONG : VT_INT;
	int dbt = vtop->type.t;
	vtop->r &= ~VT_MUSTCAST;
	vtop->type.t = sbt;
	gen_cast_s(dbt == VT_BOOL ? VT_BYTE|VT_UNSIGNED : dbt);
	vtop->type.t = dbt;
}

static void gen_cast_s(int t)
{
	CType type;
	type.t = t;
	type.ref = NULL;
	gen_cast(&type);
}
/* cast 'vtop' to 'type'. Casting to bitfields is forbidden. */

static void gen_cast(CType *type)
{
	int sbt, dbt, sf, df, c;
	int dbt_bt, sbt_bt, ds, ss, bits, trunc;
	/* special delayed cast for char/short */

	if (vtop->r & VT_MUSTCAST)
		force_charshort_cast();
	/* bitfields first get cast to ints */

	if (vtop->type.t & VT_BITFIELD)
		gv(RC_INT);

	if (IS_ENUM(type->t) && type->ref->c < 0)
		tcc_error("cast to incomplete type");

	dbt = type->t & (VT_BTYPE | VT_UNSIGNED);
	sbt = vtop->type.t & (VT_BTYPE | VT_UNSIGNED);
	if (sbt == VT_FUNC)
		sbt = VT_PTR;

again:
	if (sbt != dbt) {
		sf = is_float(sbt);
		df = is_float(dbt);
		dbt_bt = dbt & VT_BTYPE;
		sbt_bt = sbt & VT_BTYPE;
		if (dbt_bt == VT_VOID) {
			/* do not confuse backends with VT_VOID in registers */

			vpop(), vpushi(0);
			goto done;
		}
		if (sbt_bt == VT_VOID) {
error:
			cast_error(&vtop->type, type);
		}

		c = (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;
		if (c) {
			/* constant case: we can do it now */
			/* XXX: in ISOC, cannot do it if error in convert */

			if (sbt == VT_FLOAT)
				vtop->c.ld = vtop->c.f;
			else if (sbt == VT_DOUBLE)
				vtop->c.ld = vtop->c.d;

			if (df) {
				if (sbt_bt == VT_LLONG) {
					if ((sbt & VT_UNSIGNED) || !(vtop->c.i >> 63))
						vtop->c.ld = vtop->c.i;
					else
						vtop->c.ld = -(long double)-vtop->c.i;
				} else if (!sf) {
					if ((sbt & VT_UNSIGNED) || !(vtop->c.i >> 31))
						vtop->c.ld = (uint32_t)vtop->c.i;
					else
						vtop->c.ld = -(long double)-(uint32_t)vtop->c.i;
				}

				if (dbt == VT_FLOAT)
					vtop->c.f = (float)vtop->c.ld;
				else if (dbt == VT_DOUBLE)
					vtop->c.d = (double)vtop->c.ld;
			} else if (sf && dbt == VT_BOOL) {
				vtop->c.i = (vtop->c.ld != 0);
			} else {
				if (sf) {
					if (dbt & VT_UNSIGNED)
						vtop->c.i = (uint64_t)vtop->c.ld;
					else
						vtop->c.i = (int64_t)vtop->c.ld;
				} else if (sbt_bt == VT_LLONG || (PTR_SIZE == 8 && sbt == VT_PTR))
					;
				else if (sbt & VT_UNSIGNED)
					vtop->c.i = (uint32_t)vtop->c.i;
				else
					vtop->c.i = ((uint32_t)vtop->c.i | -(vtop->c.i & 0x80000000));

				if (dbt_bt == VT_LLONG || (PTR_SIZE == 8 && dbt == VT_PTR))
					;
				else if (dbt == VT_BOOL)
					vtop->c.i = (vtop->c.i != 0);
				else {
					uint32_t m = dbt_bt == VT_BYTE ? 0xff :
						     dbt_bt == VT_SHORT ? 0xffff :
						     0xffffffff;
					vtop->c.i &= m;
					if (!(dbt & VT_UNSIGNED))
						vtop->c.i |= -(vtop->c.i & ((m >> 1) + 1));
				}
			}
			goto done;

		} else if (dbt == VT_BOOL
			   && (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM))
			   == (VT_CONST | VT_SYM)) {
			/* addresses are considered non-zero (see tcctest.c:sinit23) */

			vtop->r = VT_CONST;
			vtop->c.i = 1;
			goto done;
		}
		/* cannot generate code for global or static initializers */

		if (nocode_wanted & DATA_ONLY_WANTED) {
			if (df)
				vtop->r = get_reg(RC_FLOAT);/* don't confuse backends */

			goto done;
		}
		/* non constant case: generate code */

		if (dbt == VT_BOOL) {
			gen_test_zero(TOK_NE);
			goto done;
		}

		if (sf || df) {
			if (sf && df) {
				/* convert from fp to fp */

				gen_cvt_ftof(dbt);
			} else if (df) {
				/* convert int to fp */

				gen_cvt_itof1(dbt);
			} else {
				/* convert fp to int */

				sbt = dbt;
				if (dbt_bt != VT_LLONG && dbt_bt != VT_INT)
					sbt = VT_INT;
				gen_cvt_ftoi1(sbt);
				goto again;/* may need char/short cast */

			}
			goto done;
		}

		ds = btype_size(dbt_bt);
		ss = btype_size(sbt_bt);
		if (ds == 0 || ss == 0)
			goto error;
		/* same size and no sign conversion needed */

		if (ds == ss && ds >= 4)
			goto done;
		if (dbt_bt == VT_PTR || sbt_bt == VT_PTR) {
			tcc_warning("cast between pointer and integer of different size");
			if (sbt_bt == VT_PTR) {
				/* put integer type to allow logical operations below */

				vtop->type.t = (PTR_SIZE == 8 ? VT_LLONG : VT_INT);
			}
		}
		/* processor allows { int a = 0, b = *(char*)&a; }
		           That means that if we cast to less width, we can just
		           change the type and read it still later. */

#define ALLOW_SUBTYPE_ACCESS 1

		if (ALLOW_SUBTYPE_ACCESS && (vtop->r & VT_LVAL)) {
			/* value still in memory */

			if (ds <= ss)
				goto done;
			/* ss <= 4 here */

			if (ds <= 4 && !(dbt == (VT_SHORT | VT_UNSIGNED) && sbt == VT_BYTE)) {
				gv(RC_INT);
				goto done;/* no 64bit envolved */

			}
		}
		gv(RC_INT);

		trunc = 0;
		if (ds == 8) {
			/* need to convert from 32bit to 64bit */

			if (sbt & VT_UNSIGNED) {

				goto done;

			} else {
				gen_cvt_sxtw();
				goto done;
			}
			ss = ds, ds = 4, dbt = sbt;
		} else if (ss == 8) {
			/* RISC-V keeps 32bit vals in registers sign-extended.
			               So here we need a sign-extension for signed types and
			               zero-extension. for unsigned types. */

			trunc = 32;/* zero upper 32 bits for non RISC-V targets */

		} else {
			ss = 4;
		}

		if (ds >= ss)
			goto done;

		if (ss == 4) {
			gen_cvt_csti(dbt);
			goto done;
		}

		bits = (ss - ds) * 8;
		/* for unsigned, gen_op will convert SAR to SHR */

		vtop->type.t = (ss == 8 ? VT_LLONG : VT_INT) | (dbt & VT_UNSIGNED);
		vpushi(bits);
		gen_op(TOK_SHL);
		vpushi(bits - trunc);
		gen_op(TOK_SAR);
		vpushi(trunc);
		gen_op(TOK_SHR);
	}
done:
	vtop->type = *type;
	vtop->type.t &= ~ ( VT_CONSTANT | VT_VOLATILE | VT_ARRAY | VT_TLS );
}
/* return type size as known at compile time. Put alignment at 'a' */

ST_FUNC int type_size(CType *type, int *a)
{
	Sym *s;
	int bt;

	bt = type->t & VT_BTYPE;
	if (bt == VT_STRUCT) {
		/* struct/union */

		s = type->ref;
		*a = s->r;
		return s->c;
	} else if (bt == VT_PTR) {
		if (type->t & VT_ARRAY) {
			int ts;
			s = type->ref;
			ts = type_size(&s->type, a);
			if (s->c < 0)
				return s->c;
			return ts * s->c;
		} else {
			*a = PTR_SIZE;
			return PTR_SIZE;
		}
	} else if (IS_ENUM(type->t) && type->ref->c < 0) {
		*a = 0;
		return -1;/* incomplete enum */

	} else if (bt == VT_LDOUBLE) {
		*a = LDOUBLE_ALIGN;
		return LDOUBLE_SIZE;
	} else if (bt == VT_DOUBLE || bt == VT_LLONG) {

		*a = 8;

		return 8;
	} else if (bt == VT_INT || bt == VT_FLOAT) {
		*a = 4;
		return 4;
	} else if (bt == VT_SHORT) {
		*a = 2;
		return 2;
	} else if (bt == VT_QLONG || bt == VT_QFLOAT) {
		*a = 8;
		return 16;
	} else {
		/* char, void, function, _Bool */

		*a = 1;
		return 1;
	}
}
/* push type size as known at runtime time on top of value stack. Put
   alignment at 'a' */

static void vpush_type_size(CType *type, int *a)
{
	if (type->t & VT_VLA) {
		type_size(&type->ref->type, a);
		vset(&int_type, VT_LOCAL|VT_LVAL, type->ref->c);
	} else {
		int size = type_size(type, a);
		if (size < 0)
			tcc_error("unknown type size");
		vpushs(size);
	}
}
/* return the pointed type of t */

static inline CType *pointed_type(CType *type)
{
	return &type->ref->type;
}
/* modify type so that its it is a pointer to type. */

ST_FUNC void mk_pointer(CType *type)
{
	Sym *s;
	s = sym_push(SYM_FIELD, type, 0, -1);
	type->t = VT_PTR | (type->t & VT_STORAGE);
	type->ref = s;
}
/* return true if type1 and type2 are exactly the same (including
   qualifiers).
*/

static int is_compatible_types(CType *type1, CType *type2)
{
	return compare_types(type1,type2,0);
}
/* return true if type1 and type2 are the same (ignoring qualifiers).
*/

static int is_compatible_unqualified_types(CType *type1, CType *type2)
{
	return compare_types(type1,type2,1);
}

static void cast_error(CType *st, CType *dt)
{
	type_incompatibility_error(st, dt, "cannot convert '%s' to '%s'");
}
/* verify type compatibility to store vtop in 'dt' type */

static void verify_assign_cast(CType *dt)
{
	CType *st, *type1, *type2;
	int dbt, sbt, qualwarn, lvl;

	st = &vtop->type;/* source type */

	dbt = dt->t & VT_BTYPE;
	sbt = st->t & VT_BTYPE;
	if (dt->t & VT_CONSTANT)
		tcc_warning("assignment of read-only location");
	switch (dbt) {
	case VT_VOID:
		if (sbt != dbt)
			tcc_error("assignment to void expression");
		break;
	case VT_PTR:
		/* special cases for pointers */
		/* '0' can also be a pointer */

		if (is_null_pointer(vtop))
			break;
		/* accept implicit pointer to integer cast with warning */

		if (is_integer_btype(sbt)) {
			tcc_warning("assignment makes pointer from integer without a cast");
			break;
		}
		type1 = pointed_type(dt);
		if (sbt == VT_PTR)
			type2 = pointed_type(st);
		else if (sbt == VT_FUNC)
			type2 = st;/* a function is implicitly a function pointer */

		else
			goto error;
		if (is_compatible_types(type1, type2))
			break;
		for (qualwarn = lvl = 0;; ++lvl) {
			if (((type2->t & VT_CONSTANT) && !(type1->t & VT_CONSTANT)) ||
			    ((type2->t & VT_VOLATILE) && !(type1->t & VT_VOLATILE)))
				qualwarn = 1;
			dbt = type1->t & (VT_BTYPE|VT_LONG);
			sbt = type2->t & (VT_BTYPE|VT_LONG);
			if (dbt != VT_PTR || sbt != VT_PTR)
				break;
			type1 = pointed_type(type1);
			type2 = pointed_type(type2);
		}
		if (!is_compatible_unqualified_types(type1, type2)) {
			if ((dbt == VT_VOID || sbt == VT_VOID) && lvl == 0) {
				/* void * can match anything */

			} else if (dbt == sbt
				   && is_integer_btype(sbt & VT_BTYPE)
				   && IS_ENUM(type1->t) + IS_ENUM(type2->t)
				   + !!((type1->t ^ type2->t) & VT_UNSIGNED) < 2) {
				/* Like GCC don't warn by default for merely changes
						   in pointer target signedness.  Do warn for different
						   base types, though, in particular for unsigned enums
						   and signed int targets.  */

			} else {
				tcc_warning("assignment from incompatible pointer type");
				break;
			}
		}
		if (qualwarn)
			tcc_warning_c(warn_discarded_qualifiers)
			("assignment discards qualifiers from pointer target type");
		break;
	case VT_BYTE:
	case VT_SHORT:
	case VT_INT:
	case VT_LLONG:
		if (sbt == VT_PTR || sbt == VT_FUNC) {
			tcc_warning("assignment makes integer from pointer without a cast");
		} else if (sbt == VT_STRUCT) {
			goto case_VT_STRUCT;
		}
		/* XXX: more tests */

		break;
	case VT_STRUCT:
case_VT_STRUCT:
		if (!is_compatible_unqualified_types(dt, st)) {
error:
			cast_error(st, dt);
		}
		break;
	}
}

static void gen_assign_cast(CType *dt)
{
	verify_assign_cast(dt);
	gen_cast(dt);
}
/* store vtop in lvalue pushed on stack */

ST_FUNC void vstore(void)
{
	int sbt, dbt, ft, r, size, align, bit_size, bit_pos, delayed_cast;

	ft = vtop[-1].type.t;
	sbt = vtop->type.t & VT_BTYPE;
	dbt = ft & VT_BTYPE;
	verify_assign_cast(&vtop[-1].type);

	if (sbt == VT_STRUCT) {
		/* if structure, only generate pointer */
		/* structure assignment : generate memcpy */

		size = type_size(&vtop->type, &align);
		/* destination, keep on stack() as result */

		vpushv(vtop - 1);

		vtop->type.t = VT_PTR;
		gaddrof();
		/* source */

		vswap();

		vtop->type.t = VT_PTR;
		gaddrof();

		if (1

		   ) {
			gen_struct_copy(size);
		} else

		{
			/* type size */

			vpushi(size);
			/* Use memmove, rather than memcpy, as dest and src may be same: */
			vpush_helper_func(TOK_memmove);
			vrott(4);
			gfunc_call(3);
		}

	} else if (ft & VT_BITFIELD) {
		/* bitfield store handling */
		/* save lvalue as expression result (example: s.b = s.a = n;) */

		vdup(), vtop[-1] = vtop[-2];

		bit_pos = BIT_POS(ft);
		bit_size = BIT_SIZE(ft);
		/* remove bit field info to avoid loops */

		vtop[-1].type.t = ft & ~VT_STRUCT_MASK;

		if (dbt == VT_BOOL) {
			gen_cast(&vtop[-1].type);
			vtop[-1].type.t = (vtop[-1].type.t & ~VT_BTYPE) | (VT_BYTE | VT_UNSIGNED);
		}
		r = adjust_bf(vtop - 1, bit_pos, bit_size);
		if (dbt != VT_BOOL) {
			gen_cast(&vtop[-1].type);
			dbt = vtop[-1].type.t & VT_BTYPE;
		}
		if (r == VT_STRUCT) {
			store_packed_bf(bit_pos, bit_size);
		} else {
			unsigned long long mask = (1ULL << bit_size) - 1;
			if (dbt != VT_BOOL) {
				/* mask source */

				if (dbt == VT_LLONG)
					vpushll(mask);
				else
					vpushi((unsigned)mask);
				gen_op('&');
			}
			/* shift source */

			vpushi(bit_pos);
			gen_op(TOK_SHL);
			vswap();
			/* duplicate destination */

			vdup();
			vrott(3);
			/* load destination, mask and or with source */

			if (dbt == VT_LLONG)
				vpushll(~(mask << bit_pos));
			else
				vpushi(~((unsigned)mask << bit_pos));
			gen_op('&');
			gen_op('|');
			/* store result */

			vstore();
			/* ... and discard */

			vpop();
		}
	} else if (dbt == VT_VOID) {
		--vtop;
	} else {
		/* optimize char/short casts */

		delayed_cast = 0;
		if ((dbt == VT_BYTE || dbt == VT_SHORT)
		    && is_integer_btype(sbt)
		   ) {
			if ((vtop->r & VT_MUSTCAST)
			    && btype_size(dbt) > btype_size(sbt)
			   )
				force_charshort_cast();
			delayed_cast = 1;
		} else {
			gen_cast(&vtop[-1].type);
		}
		gv(RC_TYPE(dbt));/* generate value */

		if (delayed_cast) {
			vtop->r |= BFVAL(VT_MUSTCAST, (sbt == VT_LLONG) + 1);
//tcc_warning("deley cast %x -> %x", sbt, dbt);

			vtop->type.t = ft & VT_TYPE;
		}
		/* if lvalue was saved on stack, must read it */

		if ((vtop[-1].r & VT_VALMASK) == VT_LLOCAL) {
			SValue sv;
			r = get_reg(RC_INT);
			sv.type.t = VT_PTRDIFF_T;
			sv.r = VT_LOCAL | VT_LVAL;
			sv.c.i = vtop[-1].c.i;
			sv.sym = NULL;
			load(r, &sv);
			vtop[-1].r = r | VT_LVAL;
		}

		r = vtop->r & VT_VALMASK;
		/* two word case handling :
		               store second register at word + 4 (or +8 for x86-64)  */

		if (USING_TWO_WORDS(dbt)) {
			int load_type = (dbt == VT_QFLOAT) ? VT_DOUBLE : VT_PTRDIFF_T;
			vtop[-1].type.t = load_type;
			store(r, vtop - 1);
			vswap();
			incr_offset(PTR_SIZE);
			vswap();
			/* XXX: it works because r2 is spilled last ! */

			store(vtop->r2, vtop - 1);
		} else {
			/* single word */

			store(r, vtop - 1);
		}
		vswap();
		vtop--;/* NOT vpop() because on x86 it would flush the fp stack */

	}
}
/* post defines POST/PRE add. c is the token ++ or -- */

ST_FUNC void inc(int post, int c)
{
	test_lvalue();
	vdup();/* save lvalue */

	if (post) {
		gv_dup();/* duplicate value */

		vrotb(3);
		vrotb(3);
	}
	/* add constant */

	vpushi(c - TOK_MID);
	gen_op('+');
	vstore();/* store value */

	if (post)
		vpop();/* if post op, return saved value */

}

ST_FUNC CString *parse_mult_str (const char *msg)
{
	/* read the string */

	if (tok != TOK_STR)
		expect(msg);
	cstr_reset(&initstr);
	while (tok == TOK_STR) {
		/* XXX: add \0 handling too ? */

		cstr_cat(&initstr, tokc.str.data, -1);
		next();
	}
	cstr_ccat(&initstr, '\0');
	return &initstr;
}
/* If I is >= 1 and a power of two, returns log2(i)+1.
   If I is 0 returns 0.  */

ST_FUNC int exact_log2p1(int i)
{
	int ret;
	if (!i)
		return 0;
	for (ret = 1; i >= 1 << 8; ret += 8)
		i >>= 8;
	if (i >= 1 << 4)
		ret += 4, i >>= 4;
	if (i >= 1 << 2)
		ret += 2, i >>= 2;
	if (i >= 1 << 1)
		ret++;
	return ret;
}
/* Parse __attribute__((...)) GNUC extension. */

static void parse_attribute(AttributeDef *ad)
{
	int t, n;
	char *astr;
	AttributeDef ad_tmp;

redo:
	if (tok != TOK_ATTRIBUTE1 && tok != TOK_ATTRIBUTE2)
		return;
	if (NULL == ad)/* skip over / ignore attributes */

		ad = &ad_tmp;

	next();
	skip('(');
	skip('(');
	while (tok != ')') {
		if (tok < TOK_IDENT)
			expect("attribute name");
		t = tok;
		next();
		switch (t) {
		case TOK_CLEANUP1:
		case TOK_CLEANUP2: {
			Sym *s;

			skip('(');
			s = sym_find(tok);
			if (!s) {
				tcc_warning_c(warn_implicit_function_declaration)(
					"implicit declaration of function '%s'", get_tok_str(tok, &tokc));
				s = external_global_sym(tok, &func_old_type);
			} else if ((s->type.t & VT_BTYPE) != VT_FUNC)
				tcc_error("'%s' is not declared as function", get_tok_str(tok, &tokc));
			ad->cleanup_func = s;
			next();
			skip(')');
			break;
		}
		case TOK_CONSTRUCTOR1:
		case TOK_CONSTRUCTOR2:
			ad->f.func_ctor = 1;
			break;
		case TOK_DESTRUCTOR1:
		case TOK_DESTRUCTOR2:
			ad->f.func_dtor = 1;
			break;
		case TOK_ALWAYS_INLINE1:
		case TOK_ALWAYS_INLINE2:
			ad->f.func_alwinl = 1;
			break;
		case TOK_SECTION1:
		case TOK_SECTION2:
			skip('(');
			astr = parse_mult_str("section name")->data;
			n = tcc_state->nb_sections;
			ad->section = find_section(tcc_state, astr);
			ad->new_section = n < tcc_state->nb_sections;
			skip(')');
			break;
		case TOK_ALIAS1:
		case TOK_ALIAS2:
			skip('(');
			astr = parse_mult_str("alias(\"target\")")->data;
			/* save string as token, for later */

			ad->alias_target = tok_alloc_const(astr);
			skip(')');
			break;
		case TOK_VISIBILITY1:
		case TOK_VISIBILITY2:
			skip('(');
			astr = parse_mult_str("visibility(\"default|hidden|internal|protected\")")->data;
			if (!strcmp (astr, "default"))
				ad->a.visibility = STV_DEFAULT;
			else if (!strcmp (astr, "hidden"))
				ad->a.visibility = STV_HIDDEN;
			else if (!strcmp (astr, "internal"))
				ad->a.visibility = STV_INTERNAL;
			else if (!strcmp (astr, "protected"))
				ad->a.visibility = STV_PROTECTED;
			else
				expect("visibility(\"default|hidden|internal|protected\")");
			skip(')');
			break;
		case TOK_ALIGNED1:
		case TOK_ALIGNED2:
			if (tok == '(') {
				next();
				n = expr_const();
				if (n <= 0 || (n & (n - 1)) != 0)
					tcc_error("alignment must be a positive power of two");
				skip(')');
			} else {
				n = MAX_ALIGN;
			}
			ad->a.aligned = exact_log2p1(n);
			if (n != 1 << (ad->a.aligned - 1))
				tcc_error("alignment of %d is larger than implemented", n);
			break;
		case TOK_PACKED1:
		case TOK_PACKED2:
			ad->a.packed = 1;
			break;
		case TOK_WEAK1:
		case TOK_WEAK2:
			ad->a.weak = 1;
			break;
		case TOK_NODEBUG1:
		case TOK_NODEBUG2:
			ad->a.nodebug = 1;
			break;
		case TOK_USED1:
		case TOK_USED2:
		case TOK_UNUSED1:
		case TOK_UNUSED2:
			/* currently, no need to handle it because tcc does not
			               track used/unused objects */

			break;
		case TOK_CONST1:
		case TOK_CONST2:
		case TOK_CONST3:
		case TOK_PURE1:
		case TOK_PURE2:
			/* ignored */

			break;
		case TOK_NOINLINE:
			/* ignored */

			break;
		case TOK_FORMAT1:
		case TOK_FORMAT2:
			/* ignored */

			goto skip_param;
		case TOK_NORETURN1:
		case TOK_NORETURN2:
			ad->f.func_noreturn = 1;
			break;
		case TOK_CDECL1:
		case TOK_CDECL2:
		case TOK_CDECL3:
			ad->f.func_call = FUNC_CDECL;
			break;
		case TOK_STDCALL1:
		case TOK_STDCALL2:
		case TOK_STDCALL3:
			ad->f.func_call = FUNC_STDCALL;
			break;
		case TOK_MODE:
			skip('(');
			switch (tok) {
			case TOK_MODE_DI:
				ad->attr_mode = VT_LLONG + 1;
				break;
			case TOK_MODE_QI:
				ad->attr_mode = VT_BYTE + 1;
				break;
			case TOK_MODE_HI:
				ad->attr_mode = VT_SHORT + 1;
				break;
			case TOK_MODE_SI:
			case TOK_MODE_word:
				ad->attr_mode = VT_INT + 1;
				break;
			default:
				tcc_warning("__mode__(%s) not supported\n", get_tok_str(tok, NULL));
				break;
			}
			next();
			skip(')');
			break;
		case TOK_DLLEXPORT:
			ad->a.dllexport = 1;
			break;
		case TOK_NODECORATE:
			ad->a.nodecorate = 1;
			break;
		case TOK_DLLIMPORT:
			ad->a.dllimport = 1;
			break;
		default:
			tcc_warning_c(warn_unsupported)("'%s' attribute ignored", get_tok_str(t, NULL));
			/* skip parameters */

skip_param:
			if (tok == '(') {
				int parenthesis = 0;
				do {
					if (tok == '(')
						parenthesis++;
					else if (tok == ')')
						parenthesis--;
					next();
				} while (parenthesis && tok != -1);
			}
			break;
		}
		if (tok != ',')
			break;
		next();
	}
	skip(')');
	skip(')');
	goto redo;
}

static Sym *find_field (CType *type, int v, int *cumofs)
{
	Sym *s = type->ref;
	int v1 = v | SYM_FIELD;
	if (!(v & SYM_FIELD)) {/* top-level call */

		if ((type->t & VT_BTYPE) != VT_STRUCT)
			expect("struct or union");
		if (v < TOK_UIDENT)
			expect("field name");
		if (s->c < 0)
			tcc_error("dereferencing incomplete type '%s'",
				  get_tok_str(s->v & ~SYM_STRUCT, 0));
	}
	while ((s = s->next) != NULL) {
		if (s->v == v1) {
			*cumofs = s->c;
			return s;
		}
		if ((s->type.t & VT_BTYPE) == VT_STRUCT
		    && s->v >= (SYM_FIRST_ANOM | SYM_FIELD)) {
			/* try to find field in anonymous sub-struct/union */

			Sym *ret = find_field (&s->type, v1, cumofs);
			if (ret) {
				*cumofs += s->c;
				return ret;
			}
		}
	}
	if (!(v & SYM_FIELD))
		tcc_error("field not found: %s", get_tok_str(v, NULL));
	return s;
}

static void check_fields (CType *type, int check)
{
	Sym *s = type->ref;

	while ((s = s->next) != NULL) {
		int v = s->v & ~SYM_FIELD;
		if (v < SYM_FIRST_ANOM) {
			TokenSym *ts = table_ident[v - TOK_IDENT];
			if (check && (ts->tok & SYM_FIELD))
				tcc_error("duplicate member '%s'", get_tok_str(v, NULL));
			ts->tok ^= SYM_FIELD;
		} else if ((s->type.t & VT_BTYPE) == VT_STRUCT)
			check_fields (&s->type, check);
	}
}

static void struct_layout(CType *type, AttributeDef *ad)
{
	int size, align, maxalign, offset, c, bit_pos, bit_size;
	int packed, a, bt, prevbt, prev_bit_size;
	int pcc = !tcc_state->ms_bitfields;
	int pragma_pack = *tcc_state->pack_stack_ptr;
	Sym *f;

	maxalign = 1;
	offset = 0;
	c = 0;
	bit_pos = 0;
	prevbt = VT_STRUCT;/* make it never match */

	prev_bit_size = 0;
//#define BF_DEBUG

	for (f = type->ref->next; f; f = f->next) {
		if (f->type.t & VT_BITFIELD)
			bit_size = BIT_SIZE(f->type.t);
		else
			bit_size = -1;
		size = type_size(&f->type, &align);
		a = f->a.aligned ? 1 << (f->a.aligned - 1) : 0;
		packed = 0;

		if (pcc && bit_size == 0) {
			/* in pcc mode, packing does not affect zero-width bitfields */

		} else {
			/* in pcc mode, attribute packed overrides if set. */

			if (pcc && (f->a.packed || ad->a.packed))
				align = packed = 1;
			/* pragma pack overrides align if lesser and packs bitfields always */

			if (pragma_pack) {
				packed = 1;
				if (pragma_pack < align)
					align = pragma_pack;
				/* in pcc mode pragma pack also overrides individual align */

				if (pcc && pragma_pack < a)
					a = 0;
			}
		}
		/* some individual align was specified */

		if (a)
			align = a;

		if (type->ref->type.t == VT_UNION) {
			if (pcc && bit_size >= 0)
				size = (bit_size + 7) >> 3;
			offset = 0;
			if (size > c)
				c = size;

		} else if (bit_size < 0) {
			if (pcc)
				c += (bit_pos + 7) >> 3;
			c = (c + align - 1) & -align;
			offset = c;
			if (size > 0)
				c += size;
			bit_pos = 0;
			prevbt = VT_STRUCT;
			prev_bit_size = 0;

		} else {
			/* A bit-field.  Layout is more complicated.  There are two
				       options: PCC (GCC) compatible and MS compatible */

			if (pcc) {
				/* In PCC layout a bit-field is placed adjacent to the
				                   preceding bit-fields, except if:
				                   - it has zero-width
				                   - an individual alignment was given
				                   - it would overflow its base type container and
				                     there is no packing */

				if (bit_size == 0) {
new_field:
					c = (c + ((bit_pos + 7) >> 3) + align - 1) & -align;
					bit_pos = 0;
				} else if (f->a.aligned) {
					goto new_field;
				} else if (!packed) {
					int a8 = align * 8;
					int ofs = ((c * 8 + bit_pos) % a8 + bit_size + a8 - 1) / a8;
					if (ofs > size / align)
						goto new_field;
				}
				/* in pcc mode, long long bitfields have type int if they fit */

				if (size == 8 && bit_size <= 32)
					f->type.t = (f->type.t & ~VT_BTYPE) | VT_INT, size = 4;

				while (bit_pos >= align * 8)
					c += align, bit_pos -= align * 8;
				offset = c;
				/* In PCC layout named bit-fields influence the alignment
						   of the containing struct using the base types alignment,
						   except for packed fields (which here have correct align).  */

				if (f->v & SYM_FIRST_ANOM
// && bit_size // ??? gcc on ARM/rpi does that

				   )
					align = 1;

			} else {
				bt = f->type.t & VT_BTYPE;
				if ((bit_pos + bit_size > size * 8)
				    || (bit_size > 0) == (bt != prevbt)
				   ) {
					c = (c + align - 1) & -align;
					offset = c;
					bit_pos = 0;
					/* In MS bitfield mode a bit-field run always uses
							       at least as many bits as the underlying type.
							       To start a new run it's also required that this
							       or the last bit-field had non-zero width.  */

					if (bit_size || prev_bit_size)
						c += size;
				}
				/* In MS layout the records alignment is normally
						   influenced by the field, except for a zero-width
						   field at the start of a run (but by further zero-width
						   fields it is again).  */

				if (bit_size == 0 && prevbt != bt)
					align = 1;
				prevbt = bt;
				prev_bit_size = bit_size;
			}

			f->type.t = (f->type.t & ~(0x3f << VT_STRUCT_SHIFT))
				    | (bit_pos << VT_STRUCT_SHIFT);
			bit_pos += bit_size;
		}
		if (align > maxalign)
			maxalign = align;
		f->c = offset;
		f->r = 0;
	}

	if (pcc)
		c += (bit_pos + 7) >> 3;
	/* store size and alignment */

	a = bt = ad->a.aligned ? 1 << (ad->a.aligned - 1) : 1;
	if (a < maxalign)
		a = maxalign;
	type->ref->r = a;
	if (pragma_pack && pragma_pack < maxalign && 0 == pcc) {
		/* can happen if individual align for some member was given.  In
		           this case MSVC ignores maxalign when aligning the size */

		a = pragma_pack;
		if (a < bt)
			a = bt;
	}
	c = (c + a - 1) & -a;
	type->ref->c = c;
	/* check whether we can access bitfields by their type */

	for (f = type->ref->next; f; f = f->next) {
		int s, px, cx, c0;
		CType t;

		if (0 == (f->type.t & VT_BITFIELD))
			continue;
		f->type.ref = f;
		f->auxtype = -1;
		bit_size = BIT_SIZE(f->type.t);
		if (bit_size == 0)
			continue;
		bit_pos = BIT_POS(f->type.t);
		size = type_size(&f->type, &align);

		if (bit_pos + bit_size <= size * 8 && f->c + size <= c

		   )
			continue;
		/* try to access the field using a different type */

		c0 = -1, s = align = 1;
		t.t = VT_BYTE;
		for (;;) {
			px = f->c * 8 + bit_pos;
			cx = (px >> 3) & -align;
			px = px - (cx << 3);
			if (c0 == cx)
				break;
			s = (px + bit_size + 7) >> 3;
			if (s > 4) {
				t.t = VT_LLONG;
			} else if (s > 2) {
				t.t = VT_INT;
			} else if (s > 1) {
				t.t = VT_SHORT;
			} else {
				t.t = VT_BYTE;
			}
			s = type_size(&t, &align);
			c0 = cx;
		}

		if (px + bit_size <= s * 8 && cx + s <= c

		   ) {
			/* update offset and bit position */

			f->c = cx;
			bit_pos = px;
			f->type.t = (f->type.t & ~(0x3f << VT_STRUCT_SHIFT))
				    | (bit_pos << VT_STRUCT_SHIFT);
			if (s != size)
				f->auxtype = t.t;

		} else {
			/* fall back to load/store single-byte wise */

			f->auxtype = VT_STRUCT;

		}
	}
}
/* Does 'n' fit into integer type 't' ? */

static int in_range(long long n, int t)
{
	unsigned long long m = (1ULL << (btype_size(t & VT_BTYPE) * 8 - 1)) - 1;
	if (t & VT_UNSIGNED)
		return n <= (m << 1) + 1;
	return n >= -(long long)m - 1 && n <= (long long)m;
}
/* enum/struct/union declaration. u is VT_ENUM/VT_STRUCT/VT_UNION */

static void struct_decl(CType *type, int u)
{
	int v, c, size, align, flexible;
	int bit_size, bsize, bt, ut;
	Sym *s, *ss, **ps;
	AttributeDef ad, ad1;
	CType type1, btype;

	memset(&ad, 0, sizeof ad);
	next();
	parse_attribute(&ad);

	v = 0;
	if (tok >= TOK_IDENT)/* struct/enum tag */

		v = tok, next();

	bt = ut = 0;
	if (u == VT_ENUM) {
		ut = VT_INT;
		if (tok == ':') {/* C2x enum : <type> ... */

			next();
			if (!parse_btype(&btype, &ad1, 0)
			    || !is_integer_btype(btype.t & VT_BTYPE))
				expect("enum type");
			bt = ut = btype.t & (VT_BTYPE|VT_LONG|VT_UNSIGNED|VT_DEFSIGN);
		}
	}

	if (v) {
		/* struct already defined ? return it */

		s = struct_find(v);
		if (s && (s->sym_scope == local_scope || (tok != '{' && tok != ';'))) {
			if (u == s->type.t)
				goto do_decl;
			if (u == VT_ENUM && IS_ENUM(s->type.t))/* XXX: check integral types */

				goto do_decl;
			tcc_error("redeclaration of '%s'", get_tok_str(v, NULL));
		}
	} else {
		if (tok != '{')
			expect("struct/union/enum name");
		v = anon_sym++;
	}
	/* Record the original enum/struct/union token.  */

	type1.t = u | ut;
	type1.ref = NULL;
	/* we put an undefined size for struct/union */

	s = sym_push(v | SYM_STRUCT, &type1, 0, bt ? 0 : -1);
	s->r = 0;/* default alignment is zero as gcc */

do_decl:
	type->t = s->type.t;
	type->ref = s;

	if (tok == '{') {
		next();
		if (s->c != -1
		    && !(u == VT_ENUM && s->c == 0))/* not yet defined typed enum */

			tcc_error("struct/union/enum already defined");
		s->c = -2;
		/* cannot be empty */
		/* non empty enums are not allowed */

		ps = &s->next;
		if (u == VT_ENUM) {
			long long ll = 0, pl = 0, nl = 0;
			CType t;
			t.ref = s;
			s->sym_scope = local_scope;/* anonymous symbol won't have set */

			/* enum symbols have static storage */

			t.t = VT_INT|VT_STATIC|VT_ENUM_VAL;
			if (bt)
				t.t = bt|VT_STATIC|VT_ENUM_VAL;
			for (;;) {
				v = tok;
				if (v < TOK_UIDENT)
					expect("identifier");
				next();
				if (tok == '=') {
					next();
					ll = expr_const64();
				}
				if (bt && !in_range(ll, t.t))
					tcc_error("enumerator '%s' out of range of its type",
						  get_tok_str(v, NULL));
				ss = sym_push(v, &t, VT_CONST, 0);
				ss->enum_val = ll;
				*ps = ss, ps = &ss->next;
				if (ll < nl)
					nl = ll;
				if (ll > pl)
					pl = ll;
				if (tok != ',')
					break;
				next();
				ll++;
				/* NOTE: we accept a trailing comma */

				if (tok == '}')
					break;
			}
			skip('}');

			if (bt) {
				t.t = bt;
				s->c = 2;
				goto enum_done;
			}
			/* set integral type of the enum */

			t.t = VT_INT;
			if (nl >= 0) {
				if (pl != (unsigned)pl)
					t.t = (LONG_SIZE==8 ? VT_LLONG|VT_LONG : VT_LLONG);
				t.t |= VT_UNSIGNED;
			} else if (pl != (int)pl || nl != (int)nl)
				t.t = (LONG_SIZE==8 ? VT_LLONG|VT_LONG : VT_LLONG);
			/* set type for enum members */

			for (ss = s->next; ss; ss = ss->next) {
				ll = ss->enum_val;
				if (ll == (int)ll)/* default is int if it fits */

					continue;
				if (t.t & VT_UNSIGNED) {
					ss->type.t |= VT_UNSIGNED;
					if (ll == (unsigned)ll)
						continue;
				}
				ss->type.t = (ss->type.t & ~VT_BTYPE)
					     | (LONG_SIZE==8 ? VT_LLONG|VT_LONG : VT_LLONG);
			}
			s->c = 1;
enum_done:
			s->type.t = type->t = t.t | VT_ENUM;

		} else {
			c = 0;
			flexible = 0;
			while (tok != '}') {
				if (!parse_btype(&btype, &ad1, 0)) {
					if (tok == TOK_STATIC_ASSERT) {
						do_Static_assert();
						continue;
					}
					skip(';');
					continue;
				}
				while (1) {
					if (flexible)
						tcc_error("flexible array member '%s' not at the end of struct",
							  get_tok_str(v, NULL));
					bit_size = -1;
					v = 0;
					type1 = btype;
					if (tok != ':') {
						if (tok != ';')
							type_decl(&type1, &ad1, &v, TYPE_DIRECT);
						if (v == 0) {
							if ((type1.t & VT_BTYPE) != VT_STRUCT)
								expect("identifier");
							else {
								int v = btype.ref->v;
								if ((v & ~SYM_STRUCT) < SYM_FIRST_ANOM) {
									if (tcc_state->ms_extensions == 0)
										expect("identifier");
								}
							}
						}
						if (type_size(&type1, &align) < 0) {
							if ((u == VT_STRUCT) && (type1.t & VT_ARRAY) && c)
								flexible = 1;
							else
								tcc_error("field '%s' has incomplete type",
									  get_tok_str(v, NULL));
						}
						if ((type1.t & VT_BTYPE) == VT_FUNC ||
						    (type1.t & VT_BTYPE) == VT_VOID ||
						    (type1.t & VT_STORAGE))
							tcc_error("invalid type for '%s'",
								  get_tok_str(v, NULL));
					}
					if (tok == ':') {
						next();
						bit_size = expr_const();
						/* XXX: handle v = 0 case for messages */

						if (bit_size < 0)
							tcc_error("negative width in bit-field '%s'",
								  get_tok_str(v, NULL));
						if (v && bit_size == 0)
							tcc_error("zero width for bit-field '%s'",
								  get_tok_str(v, NULL));
						parse_attribute(&ad1);
					}
					size = type_size(&type1, &align);
					if (bit_size >= 0) {
						bt = type1.t & VT_BTYPE;
						if (bt != VT_INT &&
						    bt != VT_BYTE &&
						    bt != VT_SHORT &&
						    bt != VT_BOOL &&
						    bt != VT_LLONG)
							tcc_error("bitfields must have scalar type");
						bsize = size * 8;
						if (bit_size > bsize) {
							tcc_error("width of '%s' exceeds its type",
								  get_tok_str(v, NULL));
						} else if (bit_size == bsize
							   && !*tcc_state->pack_stack_ptr
							   && !ad.a.packed && !ad1.a.packed) {
							/* no need for bit fields */

							;
						} else if (bit_size == 64) {
							tcc_error("field width 64 not implemented");
						} else {
							type1.t = (type1.t & ~VT_STRUCT_MASK)
								  | VT_BITFIELD
								  | ((unsigned)bit_size << (VT_STRUCT_SHIFT + 6));
						}
					}
					if (v != 0 || (type1.t & VT_BTYPE) == VT_STRUCT) {
						/* Remember we've seen a real field to check
									   for placement of flexible array member. */

						c = 1;
					}
					/* If member is a struct or bit-field, enforce
							       placing into the struct (as anonymous).  */

					if (v == 0 &&
					    ((type1.t & VT_BTYPE) == VT_STRUCT ||
					     bit_size >= 0)) {
						v = anon_sym++;
					}
					if (v) {
						ss = sym_push(v | SYM_FIELD, &type1, 0, 0);
						ss->a = ad1.a;
						*ps = ss;
						ps = &ss->next;
					}
					if (tok == ';' || tok == TOK_EOF)
						break;
					skip(',');
				}
				skip(';');
			}
			skip('}');
			parse_attribute(&ad);
			if (ad.cleanup_func) {
				tcc_warning("attribute '__cleanup__' ignored on type");
			}
			check_fields(type, 1);
			check_fields(type, 0);
			struct_layout(type, &ad);
		}
		if (debug_modes)
			tcc_debug_fix_forw(tcc_state, type);
	}
}

static void sym_to_attr(AttributeDef *ad, Sym *s)
{
	merge_symattr(&ad->a, &s->a);
	merge_funcattr(&ad->f, &s->f);
}
/* Add type qualifiers to a type. If the type is an array then the qualifiers
   are added to the element type, copied because it could be a typedef. */

static void parse_btype_qualify(CType *type, int qualifiers)
{
	while (type->t & VT_ARRAY) {
		type->ref = sym_push(SYM_FIELD, &type->ref->type, 0, type->ref->c);
		type = &type->ref->type;
	}
	type->t |= qualifiers;
}
/* return 0 if no type declaration. otherwise, return the basic type
   and skip it.
 */

static int parse_btype(CType *type, AttributeDef *ad, int ignore_label)
{
	int t, u, bt, st, type_found, typespec_found, g, n;
	Sym *s;
	CType type1;

	memset(ad, 0, sizeof(AttributeDef));
	type_found = 0;
	typespec_found = 0;
	t = VT_INT;
	bt = st = -1;
	type->ref = NULL;

	while (1) {
		switch (tok) {
		case TOK_EXTENSION:
			/* currently, we really ignore extension */

			next();
			continue;
		/* basic types */

		case TOK_CHAR:
			u = VT_BYTE;
basic_type:
			next();
basic_type1:
			if (u == VT_SHORT || u == VT_LONG) {
				if (st != -1 || (bt != -1 && bt != VT_INT))
tmbt: tcc_error("too many basic types");
				st = u;
			} else {
				if (bt != -1 || (st != -1 && u != VT_INT))
					goto tmbt;
				if ((t & VT_DEFSIGN) && (u == VT_VOID || u > VT_LLONG))
					goto tmbt;
				bt = u;
			}
			if (u != VT_INT)
				t = (t & ~(VT_BTYPE|VT_LONG)) | u;
			typespec_found = 1;
			break;
		case TOK_VOID:
			u = VT_VOID;
			goto basic_type;
		case TOK_SHORT:
			u = VT_SHORT;
			goto basic_type;
		case TOK_INT:
			u = VT_INT;
			goto basic_type;
		case TOK_ALIGNAS: {
			int n;
			AttributeDef ad1;
			next();
			skip('(');
			memset(&ad1, 0, sizeof(AttributeDef));
			if (parse_btype(&type1, &ad1, 0)) {
				type_decl(&type1, &ad1, &n, TYPE_ABSTRACT);
				if (ad1.a.aligned)
					n = 1 << (ad1.a.aligned - 1);
				else
					type_size(&type1, &n);
			} else {
				n = expr_const();
				if (n < 0 || (n & (n - 1)) != 0)
					tcc_error("alignment must be a positive power of two");
			}
			skip(')');
			ad->a.aligned = exact_log2p1(n);
		}
		continue;
		case TOK_LONG:
			if ((t & VT_BTYPE) == VT_DOUBLE) {
				t = (t & ~(VT_BTYPE|VT_LONG)) | VT_LDOUBLE;
			} else if ((t & (VT_BTYPE|VT_LONG)) == VT_LONG) {
				t = (t & ~(VT_BTYPE|VT_LONG)) | VT_LLONG;
			} else {
				u = VT_LONG;
				goto basic_type;
			}
			next();
			break;
		case TOK_BOOL:
			u = VT_BOOL;
			goto basic_type;
		case TOK_COMPLEX:
			tcc_error("_Complex is not yet supported");
		case TOK_FLOAT:
			u = VT_FLOAT;
			goto basic_type;
		case TOK_DOUBLE:
			if ((t & (VT_BTYPE|VT_LONG)) == VT_LONG) {
				t = (t & ~(VT_BTYPE|VT_LONG)) | VT_LDOUBLE;
			} else {
				u = VT_DOUBLE;
				goto basic_type;
			}
			next();
			break;
		case TOK_ENUM:
			struct_decl(&type1, VT_ENUM);
basic_type2:
			u = type1.t;
			type->ref = type1.ref;
			goto basic_type1;
		case TOK_STRUCT:
			struct_decl(&type1, VT_STRUCT);
			goto basic_type2;
		case TOK_UNION:
			struct_decl(&type1, VT_UNION);
			goto basic_type2;
		/* type modifiers */

		case TOK__Atomic:
			next();
			type->t = t;
			parse_btype_qualify(type, VT_ATOMIC);
			t = type->t;
			if (tok == '(') {
				parse_expr_type(&type1);
				/* remove all storage modifiers except typedef */

				type1.t &= ~(VT_STORAGE&~VT_TYPEDEF);
				if (type1.ref)
					sym_to_attr(ad, type1.ref);
				goto basic_type2;
			}
			break;
		case TOK_CONST1:
		case TOK_CONST2:
		case TOK_CONST3:
			type->t = t;
			parse_btype_qualify(type, VT_CONSTANT);
			t = type->t;
			next();
			break;
		case TOK_VOLATILE1:
		case TOK_VOLATILE2:
		case TOK_VOLATILE3:
			type->t = t;
			parse_btype_qualify(type, VT_VOLATILE);
			t = type->t;
			next();
			break;
		case TOK_SIGNED1:
		case TOK_SIGNED2:
		case TOK_SIGNED3:
			if ((t & (VT_DEFSIGN|VT_UNSIGNED)) == (VT_DEFSIGN|VT_UNSIGNED))
				tcc_error("signed and unsigned modifier");
			t |= VT_DEFSIGN;
			next();
			typespec_found = 1;
			break;
		case TOK_REGISTER:
		case TOK_AUTO:
		case TOK_RESTRICT1:
		case TOK_RESTRICT2:
		case TOK_RESTRICT3:
			next();
			break;
		case TOK_UNSIGNED:
			if ((t & (VT_DEFSIGN|VT_UNSIGNED)) == VT_DEFSIGN)
				tcc_error("signed and unsigned modifier");
			t |= VT_DEFSIGN | VT_UNSIGNED;
			next();
			typespec_found = 1;
			break;
		/* storage */

		case TOK_EXTERN:
			g = VT_EXTERN;
			goto storage;
		case TOK_STATIC:
			g = VT_STATIC;
			goto storage;
		case TOK_TYPEDEF:
			g = VT_TYPEDEF;
			goto storage;
storage:
			if (t & (VT_EXTERN|VT_STATIC|VT_TYPEDEF) & ~g)
				tcc_error("multiple storage classes");
			t |= g;
			next();
			break;
		case TOK_INLINE1:
		case TOK_INLINE2:
		case TOK_INLINE3:
			t |= VT_INLINE;
			next();
			break;
		case TOK_NORETURN3:
			next();
			ad->f.func_noreturn = 1;
			break;
		/* GNUC attribute */

		case TOK_ATTRIBUTE1:
		case TOK_ATTRIBUTE2:
			parse_attribute(ad);
			if (ad->attr_mode) {
				u = ad->attr_mode -1;
				t = (t & ~(VT_BTYPE|VT_LONG)) | u;
			}
			continue;
		/* GNUC typeof */

		case TOK_TYPEOF1:
		case TOK_TYPEOF2:
		case TOK_TYPEOF3:
			next();
			parse_expr_type(&type1);
			/* remove all storage modifiers except typedef */

			type1.t &= ~(VT_STORAGE&~VT_TYPEDEF);
			if (type1.ref) {
				sym_to_attr(ad, type1.ref);
				if (type1.t & VT_ARRAY)
					type1.t |= VT_BT_ARRAY;
			}
			goto basic_type2;
		case TOK_THREAD_LOCAL:
		case TOK___thread:
			if (t & VT_TLS)
				tcc_error("multiple thread-local storage specifiers");
			t |= VT_TLS;
			next();
			break;
		default:
			if (typespec_found)
				goto the_end;
			s = sym_find(tok);
			if (!s || !(s->type.t & VT_TYPEDEF))
				goto the_end;

			n = tok, next();
			if (tok == ':' && ignore_label) {
				/* ignore if it's a label */

				unget_tok(n);
				goto the_end;
			}

			t &= ~(VT_BTYPE|VT_LONG);
			u = t & ~(VT_CONSTANT | VT_VOLATILE), t ^= u;
			type->t = (s->type.t & ~VT_TYPEDEF) | u;
			type->ref = s->type.ref;
			if (t)
				parse_btype_qualify(type, t);
			t = type->t;
			if (t & VT_ARRAY)
				t |= VT_BT_ARRAY;
			/* get attributes from typedef */

			sym_to_attr(ad, s);
			typespec_found = 1;
			st = bt = -2;
			break;
		}
		type_found = 1;
	}
the_end:
	if (tcc_state->char_is_unsigned) {
		if ((t & (VT_DEFSIGN|VT_BTYPE)) == VT_BYTE)
			t |= VT_UNSIGNED;
	}
	/* VT_LONG is used just as a modifier for VT_INT / VT_LLONG */

	bt = t & (VT_BTYPE|VT_LONG);
	if (bt == VT_LONG)
		t |= LONG_SIZE == 8 ? VT_LLONG : VT_INT;

	if (bt == VT_LDOUBLE)
		t = (t & ~(VT_BTYPE|VT_LONG)) | (VT_DOUBLE|VT_LONG);

	type->t = t;
	return type_found;
}
/* convert a function parameter type (array to pointer and function to
   function pointer) */

static inline void convert_parameter_type(CType *pt)
{
	/* remove const and volatile qualifiers (XXX: const could be used
	       to indicate a const function parameter */

	pt->t &= ~(VT_CONSTANT | VT_VOLATILE);
	/* array must be transformed to pointer according to ANSI C */

	pt->t &= ~(VT_ARRAY | VT_VLA);
	if ((pt->t & VT_BTYPE) == VT_FUNC) {
		mk_pointer(pt);
	}
}

ST_FUNC CString *parse_asm_str(void)
{
	skip('(');
	return parse_mult_str("string constant");
}
/* Parse an asm label and return the token */

static int asm_label_instr(void)
{
	int v;
	char *astr;

	next();
	astr = parse_asm_str()->data;
	skip(')');

	v = tok_alloc_const(astr);
	return v;
}

static int post_type(CType *type, AttributeDef *ad, int storage, int td)
{
	int n, l, t1, arg_size, align;
	Sym **plast, *s, *first, **ps, *sr;
	AttributeDef ad1;
	CType pt;
	TokenString *vla_array_tok = NULL;
	int *vla_array_str = NULL;

	if (tok == '(') {
		/* function type, or recursive declarator (return if so) */

		next();
		if (TYPE_DIRECT == (td & (TYPE_DIRECT|TYPE_ABSTRACT)))
			return 0;
		/* we push a anonymous symbol which will contain the function prototype */
		/* it also serves as a boundary for the function parameter scope */

		ps = local_stack ? &local_stack : &global_stack;
		++local_scope;
		sr = sym_push2(ps, SYM_FIELD, 0, 0);

		if (tok == ')')
			l = 0;
		else if (parse_btype(&pt, &ad1, 0))
			l = FUNC_NEW;
		else if (td & (TYPE_DIRECT|TYPE_ABSTRACT)) {
			sym_pop(ps, sr->prev, 0);
			--local_scope;
			merge_attr (ad, &ad1);
			return 0;
		} else
			l = FUNC_OLD;

		first = NULL;
		plast = &first;
		arg_size = 0;
		if (l) {
			for (;;) {
				/* read param name and compute offset */

				if (l != FUNC_OLD) {
					if ((pt.t & VT_BTYPE) == VT_VOID && tok == ')')
						break;
					type_decl(&pt, &ad1, &n, TYPE_DIRECT | TYPE_ABSTRACT | TYPE_PARAM);
					if ((pt.t & VT_BTYPE) == VT_VOID)
						tcc_error("parameter declared as void");
					if (n == 0 || (td & TYPE_PARAM))
						n |= SYM_FIELD;
				} else {
					n = tok;
					pt.t = VT_INT | VT_EXTERN;/* default type */

					pt.ref = NULL;
					next();
				}
				if (n < TOK_UIDENT)
					expect("identifier");
				convert_parameter_type(&pt);
				arg_size += (type_size(&pt, &align) + PTR_SIZE - 1) / PTR_SIZE;
				/* these symbols may be evaluated for VLArrays (see below, under
				                   nocode_wanted) Example: int func(int a, int b[++a]); */

				s = sym_push(n, &pt, VT_LOCAL|VT_LVAL, 0);
				*plast = s;
				plast = &s->next;
				if (tok == ')')
					break;
				skip(',');
				if (l == FUNC_NEW && tok == TOK_DOTS) {
					l = FUNC_ELLIPSIS;
					next();
					break;
				}
				if (l == FUNC_NEW && !parse_btype(&pt, &ad1, 0))
					tcc_error("invalid type");
			}
		} else
			/* if no parameters, then old type prototype */

			l = FUNC_OLD;
		skip(')');
		/* NOTE: const is ignored in returned type as it has a special
		           meaning in gcc / C++ */

		type->t &= ~VT_CONSTANT;
		/* some ancient pre-K&R C allows a function to return an array
		           and the array brackets to be put after the arguments, such
		           that "int c()[]" means something like "int[] c()" */

		if (tok == '[') {
			next();
			skip(']');/* only handle simple "[]" */

			mk_pointer(type);
		}
		ad->f.func_args = arg_size;
		ad->f.func_type = l;
		sr->type = *type, s = sr;
		s->a = ad->a;
		s->f = ad->f;
		s->next = first;
		type->t = VT_FUNC;
		type->ref = s;
		/* unlink parameter symbols from the token table, keep on stack */

		sym_pop(ps, sr, 1);
		--local_scope;

	} else if (tok == '[') {
		int saved_nocode_wanted = nocode_wanted;
		/* array definition */

		next();
		n = -1;
		t1 = 0;
		if (td & TYPE_PARAM)
			while (1) {
				/* XXX The optional type-quals and static should only be accepted
					       in parameter decls.  The '*' as well, and then even only
					       in prototypes (not function defs).  */

				switch (tok) {
				case TOK_RESTRICT1:
				case TOK_RESTRICT2:
				case TOK_RESTRICT3:
				case TOK_CONST1:
				case TOK_VOLATILE1:
				case TOK_STATIC:
				case '*':
					next();
					continue;
				default:
					break;
				}
				if (tok != ']') {
					/* Code generation is not done now but has to be done
							   at start of function. Save code here for later use. */

					nocode_wanted = 1;
					skip_or_save_block(&vla_array_tok);
					unget_tok(0);
					vla_array_str = vla_array_tok->str;
					begin_macro(vla_array_tok, 2);
					next();
					gexpr();
					end_macro();
					next();
					goto check;
				}
				break;

			} else if (tok != ']') {
			if (!local_stack || (storage & VT_STATIC))
				vpushi(expr_const());
			else {
				/* VLAs (which can only happen with local_stack && !VT_STATIC)
						   length must always be evaluated, even under nocode_wanted,
						   so that its size slot is initialized (e.g. under sizeof
						   or typeof).  */

				nocode_wanted = 0;
				gexpr();
			}
check:
			if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST) {
				n = vtop->c.i;
				if (n < 0)
					tcc_error("invalid array size");
			} else {
				if (!is_integer_btype(vtop->type.t & VT_BTYPE))
					tcc_error("size of variable length array should be an integer");
				n = 0;
				t1 = VT_VLA;
			}
		}
		skip(']');
		/* parse next post type */

		post_type(type, ad, storage, (td & ~(TYPE_DIRECT|TYPE_ABSTRACT)) | TYPE_NEST);

		if ((type->t & VT_BTYPE) == VT_FUNC)
			tcc_error("declaration of an array of functions");
		if ((type->t & VT_BTYPE) == VT_VOID
		    || type_size(type, &align) < 0)
			tcc_error("declaration of an array of incomplete type elements");

		t1 |= type->t & VT_VLA;

		if (t1 & VT_VLA) {
			if (n < 0) {
				if (td & TYPE_NEST)
					tcc_error("need explicit inner array size in VLAs");
			} else {
				loc -= type_size(&int_type, &align);
				loc &= -align;
				n = loc;

				vpush_type_size(type, &align);
				gen_op('*');
				vset(&int_type, VT_LOCAL|VT_LVAL, n);
				vswap();
				vstore();
			}
		}
		if (n != -1)
			vpop();
		nocode_wanted = saved_nocode_wanted;
		/* we push an anonymous symbol which will contain the array
		           element type */

		s = sym_push(SYM_FIELD, type, 0, n);
		type->t = (t1 ? VT_VLA : VT_ARRAY) | VT_PTR;
		type->ref = s;

		if (vla_array_str) {
			/* for function args, the top dimension is converted to pointer */

			if ((t1 & VT_VLA) && (td & TYPE_NEST))
				s->vla_array_str = vla_array_str;
			else
				tok_str_free_str(vla_array_str);
		}
	}
	return 1;
}
/* Parse a type declarator (except basic type), and return the type
   in 'type'. 'td' is a bitmask indicating which kind of type decl is
   expected. 'type' should contain the basic type. 'ad' is the
   attribute definition of the basic type. It can be modified by
   type_decl().  If this (possibly abstract) declarator is a pointer chain
   it returns the innermost pointed to type (equals *type, but is a different
   pointer), otherwise returns type itself, that's used for recursive calls.  */
static CType *type_decl(CType *type, AttributeDef *ad, int *v, int td)
{
	CType *post, *ret;
	int qualifiers, storage;
	/* recursive type, remove storage bits first, apply them later again */

	storage = type->t & VT_STORAGE;
	type->t &= ~VT_STORAGE;
	post = ret = type;

	while (tok == '*') {
		qualifiers = 0;
redo:
		next();
		switch (tok) {
		case TOK__Atomic:
			qualifiers |= VT_ATOMIC;
			goto redo;
		case TOK_CONST1:
		case TOK_CONST2:
		case TOK_CONST3:
			qualifiers |= VT_CONSTANT;
			goto redo;
		case TOK_VOLATILE1:
		case TOK_VOLATILE2:
		case TOK_VOLATILE3:
			qualifiers |= VT_VOLATILE;
			goto redo;
		case TOK_RESTRICT1:
		case TOK_RESTRICT2:
		case TOK_RESTRICT3:
			goto redo;
		/* XXX: clarify attribute handling */

		case TOK_ATTRIBUTE1:
		case TOK_ATTRIBUTE2:
			parse_attribute(ad);
			break;
		}
		mk_pointer(type);
		type->t |= qualifiers;
		if (ret == type)
			/* innermost pointed to type is the one for the first derivation */

			ret = pointed_type(type);
	}

	if (tok == '(') {
		/* This is possibly a parameter type list for abstract declarators
			   ('int ()'), use post_type for testing this.  */

		if (!post_type(type, ad, 0, td)) {
			/* It's not, so it's a nested declarator, and the post operations
				       apply to the innermost pointed to type (if any).  */
			/* XXX: this is not correct to modify 'ad' at this point, but
				       the syntax is not clear */

			parse_attribute(ad);
			post = type_decl(type, ad, v, td);
			skip(')');
		} else
			goto abstract;
	} else if (tok >= TOK_IDENT && (td & TYPE_DIRECT)) {
		/* type identifier */

		*v = tok;
		next();
	} else {
abstract:
		if (!(td & TYPE_ABSTRACT))
			expect("identifier");
		*v = 0;
	}
	post_type(post, ad, post != ret ? 0 : storage,
		  td & ~(TYPE_DIRECT|TYPE_ABSTRACT));
	parse_attribute(ad);
	type->t |= storage;
	return ret;
}
/* indirection with full error checking and bound check */

ST_FUNC void indir(void)
{
	if ((vtop->type.t & VT_BTYPE) != VT_PTR) {
		if ((vtop->type.t & VT_BTYPE) == VT_FUNC)
			return;
		expect("pointer");
	}
	if (vtop->r & VT_LVAL)
		gv(RC_INT);
	vtop->type = *pointed_type(&vtop->type);
	/* Arrays and functions are never lvalues */

	if (!(vtop->type.t & (VT_ARRAY | VT_VLA))
	    && (vtop->type.t & VT_BTYPE) != VT_FUNC) {
		vtop->r |= VT_LVAL;
		/* if bound checking, the referenced pointer must be checked */

	}
}
/* pass a parameter to a function and do type checking and casting */

static void gfunc_param_typed(Sym *func, Sym *arg)
{
	int func_type;
	CType type;

	func_type = func->f.func_type;
	if (func_type == FUNC_OLD ||
	    (func_type == FUNC_ELLIPSIS && arg == NULL)) {
		/* default casting : only need to convert float to double */

		if ((vtop->type.t & VT_BTYPE) == VT_FLOAT) {
			gen_cast_s(VT_DOUBLE);
		} else if (vtop->type.t & VT_BITFIELD) {
			type.t = vtop->type.t & (VT_BTYPE | VT_UNSIGNED);
			type.ref = vtop->type.ref;
			gen_cast(&type);
		} else if (vtop->r & VT_MUSTCAST) {
			force_charshort_cast();
		}
	} else if (arg == NULL) {
		tcc_error("too many arguments to function");
	} else {
		type = arg->type;
		type.t &= ~VT_CONSTANT;/* need to do that to avoid false warning */

		gen_assign_cast(&type);
	}
}
/* parse an expression and return its type without any side effect. */

static void expr_type(CType *type, void (*expr_fn)(void))
{
	nocode_wanted++;
	expr_fn();
	*type = vtop->type;
	vpop();
	nocode_wanted--;
}
/* parse an expression of the form '(type)' or '(expr)' and return its
   type */

static void parse_expr_type(CType *type)
{
	int n;
	AttributeDef ad;

	skip('(');
	if (parse_btype(type, &ad, 0)) {
		type_decl(type, &ad, &n, TYPE_ABSTRACT);
	} else {
		expr_type(type, gexpr);
	}
	skip(')');
}

static void parse_type(CType *type)
{
	AttributeDef ad;
	int n;

	if (!parse_btype(type, &ad, 0)) {
		expect("type");
	}
	type_decl(type, &ad, &n, TYPE_ABSTRACT);
}

static void parse_builtin_params(int nc, const char *args)
{
	char c, sep = '(';
	CType type;
	if (nc)
		nocode_wanted++;
	next();
	if (*args == 0)
		skip(sep);
	while ((c = *args++)) {
		skip(sep);
		sep = ',';
		if (c == 't') {
			parse_type(&type);
			vpush(&type);
			continue;
		}
		expr_eq();
		type.ref = NULL;
		type.t = 0;
		switch (c) {
		case 'e':
			continue;
		case 'V':
			type.t = VT_CONSTANT;
		case 'v':
			type.t |= VT_VOID;
			mk_pointer (&type);
			break;
		case 'S':
			type.t = VT_CONSTANT;
		case 's':
			type.t |= char_type.t;
			mk_pointer (&type);
			break;
		case 'i':
			type.t = VT_INT;
			break;
		case 'l':
			type.t = VT_SIZE_T;
			break;
		default:
			break;
		}
		gen_assign_cast(&type);
	}
	skip(')');
	if (nc)
		nocode_wanted--;
}

static void parse_atomic(int atok)
{
	int size, align, arg, t, save = 0;
	CType *atom, *atom_ptr, ct = {0};
	SValue store;
	char buf[40];
	static const char *const templates[] = {
		/*
		         * Each entry consists of callback and function template.
		         * The template represents argument types and return type.
		         *
		         * ? void (return-only)
		         * b bool
		         * a atomic
		         * A read-only atomic
		         * p pointer to memory
		         * v value
		         * l load pointer
		         * s save pointer
		         * m memory model
		         */
		/* keep in order of appearance in tcctok.h: */
		/* __atomic_store */
		"alm.?",
		/* __atomic_load */
		"Asm.v",
		/* __atomic_exchange */
		"alsm.v",
		/* __atomic_compare_exchange */
		"aplbmm.b",
		/* __atomic_fetch_add */
		"avm.v",
		/* __atomic_fetch_sub */
		"avm.v",
		/* __atomic_fetch_or */
		"avm.v",
		/* __atomic_fetch_xor */
		"avm.v",
		/* __atomic_fetch_and */
		"avm.v",
		/* __atomic_fetch_nand */
		"avm.v",
		/* __atomic_and_fetch */
		"avm.v",
		/* __atomic_sub_fetch */
		"avm.v",
		/* __atomic_or_fetch */
		"avm.v",
		/* __atomic_xor_fetch */
		"avm.v",
		/* __atomic_and_fetch */
		"avm.v",
		/* __atomic_nand_fetch */
		"avm.v"
	};
	const char *template = templates[(atok - TOK___atomic_store)];

	atom = atom_ptr = NULL;
	size = 0;/* pacify compiler */

	next();
	skip('(');
	for (arg = 0;;) {
		expr_eq();
		switch (template[arg]) {
		case 'a':
		case 'A':
			atom_ptr = &vtop->type;
			if ((atom_ptr->t & VT_BTYPE) != VT_PTR)
				expect("pointer");
			atom = pointed_type(atom_ptr);
			size = type_size(atom, &align);
			if (size > 8
			    || (size & (size - 1))
			    || (atok > TOK___atomic_compare_exchange
				&& (0 == btype_size(atom->t & VT_BTYPE)
				    || (atom->t & VT_BTYPE) == VT_PTR)))
				expect("integral or integer-sized pointer target type");
			/* GCC does not care either: */
			/* if (!(atom->t & VT_ATOMIC))
			                tcc_warning("pointer target declaration is missing '_Atomic'"); */

			break;

		case 'p':
			if ((vtop->type.t & VT_BTYPE) != VT_PTR
			    || type_size(pointed_type(&vtop->type), &align) != size)
				tcc_error("pointer target type mismatch in argument %d", arg + 1);
			gen_assign_cast(atom_ptr);
			break;
		case 'v':
			gen_assign_cast(atom);
			break;
		case 'l':
			indir();
			gen_assign_cast(atom);
			break;
		case 's':
			save = 1;
			indir();
			store = *vtop;
			vpop();
			break;
		case 'm':
			gen_assign_cast(&int_type);
			break;
		case 'b':
			ct.t = VT_BOOL;
			gen_assign_cast(&ct);
			break;
		}
		if ('.' == template[++arg])
			break;
		skip(',');
	}
	skip(')');

	ct.t = VT_VOID;
	switch (template[arg + 1]) {
	case 'b':
		ct.t = VT_BOOL;
		break;
	case 'v':
		ct = *atom;
		break;
	}

	sprintf(buf, "%s_%d", get_tok_str(atok, 0), size);
	vpush_helper_func(tok_alloc_const(buf));
	vrott(arg - save + 1);
	gfunc_call(arg - save);

	vpush(&ct);
	PUT_R_RET(vtop, ct.t);
	t = ct.t & VT_BTYPE;
	if (t == VT_BYTE || t == VT_SHORT || t == VT_BOOL) {

		vtop->r |= BFVAL(VT_MUSTCAST, 1);

	}
	gen_cast(&ct);
	if (save) {
		vpush(&ct);
		*vtop = store;
		vswap();
		vstore();
	}
}

ST_FUNC void unary(void)
{
	int n, t, align, size, r;
	CType type;
	Sym *s;
	AttributeDef ad;
	/* generate line number info */

	if (debug_modes)
		tcc_debug_line(tcc_state), tcc_tcov_check_line (tcc_state, 1);

	type.ref = NULL;
	/* XXX: GCC 2.95.3 does not generate a table although it should be
	       better here */

tok_next:
	switch (tok) {
	case TOK_EXTENSION:
		next();
		goto tok_next;
	case TOK_LCHAR:

		t = VT_SHORT|VT_UNSIGNED;
		goto push_tokc;

	case TOK_CINT:
	case TOK_CCHAR:
		t = VT_INT;
push_tokc:
		type.t = t;
		vsetc(&type, VT_CONST, &tokc);
		next();
		break;
	case TOK_CUINT:
		t = VT_INT | VT_UNSIGNED;
		goto push_tokc;
	case TOK_CLLONG:
		t = VT_LLONG;
		goto push_tokc;
	case TOK_CULLONG:
		t = VT_LLONG | VT_UNSIGNED;
		goto push_tokc;
	case TOK_CFLOAT:
		t = VT_FLOAT;
		goto push_tokc;
	case TOK_CDOUBLE:
		t = VT_DOUBLE;
		goto push_tokc;
	case TOK_CLDOUBLE:

		t = VT_DOUBLE | VT_LONG;
		tokc.d = tokc.ld;

		goto push_tokc;
	case TOK_CLONG:
		t = (LONG_SIZE == 8 ? VT_LLONG : VT_INT) | VT_LONG;
		goto push_tokc;
	case TOK_CULONG:
		t = (LONG_SIZE == 8 ? VT_LLONG : VT_INT) | VT_LONG | VT_UNSIGNED;
		goto push_tokc;
	case TOK___FUNCTION__:
		if (!gnu_ext)
			goto tok_identifier;
	/* fall thru */

	case TOK___FUNC__:
		tok = TOK_STR;
		cstr_reset(&tokcstr);
		cstr_cat(&tokcstr, funcname, 0);
		tokc.str.size = tokcstr.size;
		tokc.str.data = tokcstr.data;
		goto case_TOK_STR;
	case TOK_LSTR:

		t = VT_SHORT | VT_UNSIGNED;

		goto str_init;
	case TOK_STR:
case_TOK_STR:
		/* string parsing */

		t = char_type.t;
str_init:
		if (tcc_state->warn_write_strings & WARN_ON)
			t |= VT_CONSTANT;
		type.t = t;
		mk_pointer(&type);
		type.t |= VT_ARRAY;
		memset(&ad, 0, sizeof(AttributeDef));
		ad.section = rodata_section;
		decl_initializer_alloc(&type, &ad, VT_CONST, 2, 0, 0);
		break;
	case TOK_SOTYPE:
	case '(':
		t = tok;
		next();
		/* cast ? */

		if (parse_btype(&type, &ad, 0)) {
			type_decl(&type, &ad, &n, TYPE_ABSTRACT);
			skip(')');
			/* check ISOC99 compound literal */

			if (tok == '{') {
				/* data is allocated locally by default */

				if (global_expr)
					r = VT_CONST;
				else
					r = VT_LOCAL;
				/* all except arrays are lvalues */

				if (!(type.t & VT_ARRAY))
					r |= VT_LVAL;
				memset(&ad, 0, sizeof(AttributeDef));
				decl_initializer_alloc(&type, &ad, r, 1, 0, 0);
			} else if (t == TOK_SOTYPE) {/* from sizeof/alignof (...) */

				vpush(&type);
				return;
			} else {
				unary();
				gen_cast(&type);
			}
		} else if (tok == '{') {
			int saved_nocode_wanted = nocode_wanted;
			if (CONST_WANTED && !NOEVAL_WANTED)
				expect("constant");
			if (0 == local_scope)
				tcc_error("statement expression outside of function");
			/* save all registers */

			save_regs(0);
			/* statement expression : we do not accept break/continue
			               inside as GCC does.  We do retain the nocode_wanted state,
				       as statement expressions can't ever be entered from the
				       outside, so any reactivation of code emission (from labels
				       or loop heads) can be disabled again after the end of it. */
			/* default return value is (void) */

			vpushi(0), vtop->type.t = VT_VOID;
			block(STMT_EXPR);
			/* If the statement expr can be entered, then we retain the current
			               nocode_wanted state (from e.g. a 'return 0;' in the stmt-expr).
			               If it can't be entered then the state is that from before the
			               statement expression.  */

			if (saved_nocode_wanted)
				nocode_wanted = saved_nocode_wanted;
			skip(')');
		} else {
			gexpr();
			skip(')');
		}
		break;
	case '*':
		next();
		unary();
		indir();
		break;
	case '&':
		next();
		unary();
		/* functions names must be treated as function pointers,
		           except for unary '&' and sizeof. Since we consider that
		           functions are not lvalues, we only have to handle it
		           there and in function calls. */
		/* arrays can also be used although they are not lvalues */

		if ((vtop->type.t & VT_BTYPE) != VT_FUNC &&
		    !(vtop->type.t & (VT_ARRAY | VT_VLA)))
			test_lvalue();
		if (vtop->sym)
			vtop->sym->a.addrtaken = 1;
		mk_pointer(&vtop->type);
		gaddrof();
		break;
	case '!':
		next();
		unary();
		gen_test_zero(TOK_EQ);
		break;
	case '~':
		next();
		unary();
		vpushi(-1);
		gen_op('^');
		break;
	case '+':
		next();
		unary();
		if ((vtop->type.t & VT_BTYPE) == VT_PTR)
			tcc_error("pointer not accepted for unary plus");
		/* In order to force cast, we add zero, except for floating point
			   where we really need an noop (otherwise -0.0 will be transformed
			   into +0.0).  */

		if (!is_float(vtop->type.t)) {
			vpushi(0);
			gen_op('+');
		}
		break;
	case TOK_SIZEOF:
	case TOK_ALIGNOF1:
	case TOK_ALIGNOF2:
	case TOK_ALIGNOF3:
		t = tok;
		next();
		if (tok == '(')
			tok = TOK_SOTYPE;
		expr_type(&type, unary);
		if (t == TOK_SIZEOF) {
			vpush_type_size(&type, &align);
			gen_cast_s(VT_SIZE_T);
		} else {
			type_size(&type, &align);
			s = NULL;
			if (vtop[1].r & VT_SYM)
				s = vtop[1].sym;/* hack: accessing previous vtop */

			if (s && s->a.aligned)
				align = 1 << (s->a.aligned - 1);
			vpushs(align);
		}
		break;

	case TOK_builtin_expect:
		/* __builtin_expect is a no-op for now */

		parse_builtin_params(0, "ee");
		vpop();
		break;
	case TOK_builtin_types_compatible_p:
		parse_builtin_params(0, "tt");
		vtop[-1].type.t &= ~(VT_CONSTANT | VT_VOLATILE);
		vtop[0].type.t &= ~(VT_CONSTANT | VT_VOLATILE);
		n = is_compatible_types(&vtop[-1].type, &vtop[0].type);
		vtop -= 2;
		vpushi(n);
		break;
	case TOK_builtin_choose_expr: {
		int64_t c;
		next();
		skip('(');
		c = expr_const64();
		skip(',');
		if (!c) {
			nocode_wanted++;
		}
		expr_eq();
		if (!c) {
			vpop();
			nocode_wanted--;
		}
		skip(',');
		if (c) {
			nocode_wanted++;
		}
		expr_eq();
		if (c) {
			vpop();
			nocode_wanted--;
		}
		skip(')');
	}
	break;
	case TOK_builtin_constant_p:
		parse_builtin_params(1, "e");
		n = 1;
		if ((vtop->r & (VT_VALMASK | VT_LVAL)) != VT_CONST
		    || ((vtop->r & VT_SYM) && vtop->sym->a.addrtaken)
		   )
			n = 0;
		vtop--;
		vpushi(n);
		break;
	case TOK_builtin_unreachable:
		parse_builtin_params(0, "");/* just skip '()' */

		type.t = VT_VOID;
		vpush(&type);
		CODE_OFF();
		break;
	case TOK_builtin_frame_address:
	case TOK_builtin_return_address: {
		int tok1 = tok;
		int level;
		next();
		skip('(');
		level = expr_const();
		if (level < 0)
			tcc_error("%s only takes positive integers", get_tok_str(tok1, 0));
		skip(')');
		type.t = VT_VOID;
		mk_pointer(&type);
		vset(&type, VT_LOCAL, 0);/* local frame */

		while (level--) {

			mk_pointer(&vtop->type);
			indir();/* -> parent frame */

		}
		if (tok1 == TOK_builtin_return_address) {
// assume return address is just above frame pointer on stack
			vpushi(PTR_SIZE);
			gen_op('+');

			mk_pointer(&vtop->type);
			indir();
		}
	}
	break;

	case TOK_builtin_va_start:
		parse_builtin_params(0, "ee");
		r = vtop->r & VT_VALMASK;
		if (r == VT_LLOCAL)
			r = VT_LOCAL;
		if (r != VT_LOCAL)
			tcc_error("__builtin_va_start expects a local variable");
		vtop->r = r;
		vtop->type = char_pointer_type;
		vtop->c.i += 8;
		vstore();
		break;
	/* atomic operations */
	case TOK___atomic_store:
	case TOK___atomic_load:
	case TOK___atomic_exchange:
	case TOK___atomic_compare_exchange:
	case TOK___atomic_fetch_add:
	case TOK___atomic_fetch_sub:
	case TOK___atomic_fetch_or:
	case TOK___atomic_fetch_xor:
	case TOK___atomic_fetch_and:
	case TOK___atomic_fetch_nand:
	case TOK___atomic_add_fetch:
	case TOK___atomic_sub_fetch:
	case TOK___atomic_or_fetch:
	case TOK___atomic_xor_fetch:
	case TOK___atomic_and_fetch:
	case TOK___atomic_nand_fetch:
		parse_atomic(tok);
		break;
	/* pre operations */

	case TOK_INC:
	case TOK_DEC:
		t = tok;
		next();
		unary();
		inc(0, t);
		break;
	case '-':
		next();
		unary();
		if (is_float(vtop->type.t)) {
			gen_opif(TOK_NEG);
		} else {
			vpushi(0);
			vswap();
			gen_op('-');
		}
		break;
	case TOK_LAND:
		if (!gnu_ext)
			goto tok_identifier;
		next();
		/* allow to take the address of a label */

		if (tok < TOK_UIDENT)
			expect("label identifier");
		s = label_find(tok);
		if (!s) {
			s = label_push(&global_label_stack, tok, LABEL_FORWARD);
		} else {
			if (s->r == LABEL_DECLARED)
				s->r = LABEL_FORWARD;
		}
		if ((s->type.t & VT_BTYPE) != VT_PTR) {
			s->type.t = VT_VOID;
			mk_pointer(&s->type);
			s->type.t |= VT_STATIC;
		}
		vpushsym(&s->type, s);
		next();
		break;

	case TOK_GENERIC: {
		CType controlling_type;
		int has_default = 0;
		int has_match = 0;
		int learn = 0;
		TokenString *str = NULL;
		int saved_nocode_wanted = nocode_wanted;
		nocode_wanted &= ~CONST_WANTED_MASK;

		next();
		skip('(');
		expr_type(&controlling_type, expr_eq);
		convert_parameter_type (&controlling_type);

		nocode_wanted = saved_nocode_wanted;

		for (;;) {
			learn = 0;
			skip(',');
			if (tok == TOK_DEFAULT) {
				if (has_default)
					tcc_error("too many 'default'");
				has_default = 1;
				if (!has_match)
					learn = 1;
				next();
			} else {
				int v;
				parse_btype(&type, &ad, 0);
				type_decl(&type, &ad, &v, TYPE_ABSTRACT);
				if (compare_types(&controlling_type, &type, 0)) {
					if (has_match) {
						tcc_error("type match twice");
					}
					has_match = 1;
					learn = 1;
				}
			}
			skip(':');
			if (learn) {
				if (str)
					tok_str_free(str);
				skip_or_save_block(&str);
			} else {
				skip_or_save_block(NULL);
			}
			if (tok == ')')
				break;
		}
		if (!str) {
			char buf[60];
			type_to_str(buf, sizeof buf, &controlling_type, NULL);
			tcc_error("type '%s' does not match any association", buf);
		}
		begin_macro(str, 1);
		next();
		expr_eq();
		if (tok != TOK_EOF)
			expect(",");
		end_macro();
		next();
		break;
	}
// special qnan , snan and infinity values

	case TOK___NAN__:
		n = 0x7fc00000;
special_math_val:
		vpushi(n);
		vtop->type.t = VT_FLOAT;
		next();
		break;
	case TOK___SNAN__:
		n = 0x7f800001;
		goto special_math_val;
	case TOK___INF__:
		n = 0x7f800000;
		goto special_math_val;

	default:
tok_identifier:
		if (tok < TOK_UIDENT)
			tcc_error("expression expected before '%s'", get_tok_str(tok, &tokc));
		t = tok;
		next();
		s = sym_find(t);
		if (!s || IS_ASM_SYM(s)) {
			const char *name = get_tok_str(t, NULL);
			if (tok != '(')
				tcc_error("'%s' undeclared", name);
			/* for simple function calls, we tolerate undeclared
			               external reference to int() function */

			if (!func_old)
				tcc_warning_c(warn_implicit_function_declaration)(
					"implicit declaration of function '%s'", name);
			s = external_global_sym(t, &func_old_type);
		}

		r = s->r;
		/* A symbol that has a register is a local register variable,
		           which starts out as VT_LOCAL value.  */

		if ((r & VT_VALMASK) < VT_CONST)
			r = (r & ~VT_VALMASK) | VT_LOCAL;

		vset(&s->type, r, s->c);
		/* Point to s as backpointer (even without r&VT_SYM).
			   Will be used by at least the x86 inline asm parser for
			   regvars.  */

		vtop->sym = s;

		if (r & VT_SYM) {
			vtop->c.i = 0;

			if (s->a.dllimport) {
				mk_pointer(&vtop->type);
				vtop->r |= VT_LVAL;
				indir();
			}

		} else if (r == VT_CONST && IS_ENUM_VAL(s->type.t)) {
			vtop->c.i = s->enum_val;
		}
		break;
	}
	/* post operations */

	while (1) {
		if (tok == TOK_INC || tok == TOK_DEC) {
			inc(1, tok);
			next();
		} else if (tok == '.' || tok == TOK_ARROW) {
			int qualifiers, cumofs;
			/* field */

			if (tok == TOK_ARROW)
				indir();
			qualifiers = vtop->type.t & (VT_CONSTANT | VT_VOLATILE);
			test_lvalue();
			/* expect pointer on structure */

			next();
			s = find_field(&vtop->type, tok, &cumofs);
			/* add field offset to pointer */

			gaddrof();
			vtop->type = char_pointer_type;/* change type to 'char *' */

			vpushi(cumofs);
			gen_op('+');
			/* change type to field type, and set to lvalue */

			vtop->type = s->type;
			if (qualifiers)
				parse_btype_qualify(&vtop->type, qualifiers);
			/* an array is never an lvalue */

			if (!(vtop->type.t & VT_ARRAY)) {
				vtop->r |= VT_LVAL;

			}
			next();
		} else if (tok == '[') {
			next();
			gexpr();
			gen_op('+');
			indir();
			skip(']');
		} else if (tok == '(') {
			SValue ret;
			Sym *sa;
			int nb_args, ret_nregs, ret_align, regsize, variadic;
			TokenString *p, *p2;
			/* function call  */

			if ((vtop->type.t & VT_BTYPE) != VT_FUNC) {
				/* pointer test (no array accepted) */

				if ((vtop->type.t & (VT_BTYPE | VT_ARRAY)) == VT_PTR) {
					vtop->type = *pointed_type(&vtop->type);
					if ((vtop->type.t & VT_BTYPE) != VT_FUNC)
						goto error_func;
				} else {
error_func:
					expect("function pointer");
				}
			} else {
				vtop->r &= ~VT_LVAL;/* no lvalue */

			}
			/* get return type */

			s = vtop->type.ref;
			next();
			sa = s->next;/* first parameter */

			nb_args = regsize = 0;
			ret.r2 = VT_CONST;
			/* compute first implicit argument if a structure is returned */

			if ((s->type.t & VT_BTYPE) == VT_STRUCT) {
				variadic = (s->f.func_type == FUNC_ELLIPSIS);
				ret_nregs = gfunc_sret(&s->type, variadic, &ret.type,
						       &ret_align, &regsize);
				if (ret_nregs <= 0) {
					/* get some space for the returned structure */

					size = type_size(&s->type, &align);
					loc = (loc - size) & -align;
					ret.type = s->type;
					ret.r = VT_LOCAL | VT_LVAL;
					/* pass it as 'int' to avoid structure arg passing
					                       problems */

					vseti(VT_LOCAL, loc);

					ret.c = vtop->c;
					if (ret_nregs < 0)
						vtop--;
					else
						nb_args++;
				}
			} else {
				ret_nregs = 1;
				ret.type = s->type;
			}

			if (ret_nregs > 0) {
				/* return in register */

				ret.c.i = 0;
				PUT_R_RET(&ret, ret.type.t);
			}

			p = NULL;
			if (tok != ')') {
				r = tcc_state->reverse_funcargs;
				for (;;) {
					if (r) {
						skip_or_save_block(&p2);
						p2->prev = p, p = p2;
					} else {
						expr_eq();
						gfunc_param_typed(s, sa);
					}
					nb_args++;
					if (sa)
						sa = sa->next;
					if (tok == ')')
						break;
					skip(',');
				}
			}
			if (sa)
				tcc_error("too few arguments to function");

			if (p) {/* with reverse_funcargs */

				for (n = 0; p; p = p2, ++n) {
					p2 = p, sa = s;
					do {
						sa = sa->next, p2 = p2->prev;
					} while (p2 && sa);
					p2 = p->prev;
					begin_macro(p, 1), next();
					expr_eq();
					gfunc_param_typed(s, sa);
					end_macro();
				}
				vrev(n);
			}

			next();
			vcheck_cmp();/* the generators don't like VT_CMP on vtop */

			gfunc_call(nb_args);

			if (ret_nregs < 0) {
				vsetc(&ret.type, ret.r, &ret.c);

			} else {
				/* return value */

				n = ret_nregs;
				while (n > 1) {
					int rc = reg_classes[ret.r] & ~(RC_INT | RC_FLOAT);
					/* We assume that when a structure is returned in multiple
					                       registers, their classes are consecutive values of the
					                       suite s(n) = 2^n */

					rc <<= --n;
					for (r = 0; r < NB_REGS; ++r)
						if (reg_classes[r] & rc)
							break;
					vsetc(&ret.type, r, &ret.c);
				}
				vsetc(&ret.type, ret.r, &ret.c);
				vtop->r2 = ret.r2;
				/* handle packed struct return */

				if (((s->type.t & VT_BTYPE) == VT_STRUCT) && ret_nregs) {
					int addr, offset;

					size = type_size(&s->type, &align);
					/* We're writing whole regs often, make sure there's enough
					                       space.  Assume register size is power of 2.  */

					size = (size + regsize - 1) & -regsize;
					if (ret_align > align)
						align = ret_align;
					loc = (loc - size) & -align;
					addr = loc;
					offset = 0;
					for (;;) {
						vset(&ret.type, VT_LOCAL | VT_LVAL, addr + offset);
						vswap();
						vstore();
						vtop--;
						if (--ret_nregs == 0)
							break;
						offset += regsize;
					}
					vset(&s->type, VT_LOCAL | VT_LVAL, addr);
				}
				/* Promote char/short return values. This is matters only
				                   for calling function that were not compiled by TCC and
				                   only on some architectures.  For those where it doesn't
				                   matter we expect things to be already promoted to int,
				                   but not larger.  */

				t = s->type.t & VT_BTYPE;
				if (t == VT_BYTE || t == VT_SHORT || t == VT_BOOL) {

					vtop->r |= BFVAL(VT_MUSTCAST, 1);

				}
			}
			if (s->f.func_noreturn) {
				if (debug_modes)
					tcc_tcov_block_end(tcc_state, -1);
				CODE_OFF();
			}
		} else {
			break;
		}
	}
}
/* defined precedence_parser */
#define expr_landor_next(op) unary(), expr_infix(precedence(op) + 1)
#define expr_lor() unary(), expr_infix(1)

static int precedence(int tok)
{
	switch (tok) {
	case TOK_LOR:
		return 1;
	case TOK_LAND:
		return 2;
	case '|':
		return 3;
	case '^':
		return 4;
	case '&':
		return 5;
	case TOK_EQ:
	case TOK_NE:
		return 6;
relat:
	case TOK_ULT:
	case TOK_UGE:
		return 7;
	case TOK_SHL:
	case TOK_SAR:
		return 8;
	case '+':
	case '-':
		return 9;
	case '*':
	case '/':
	case '%':
		return 10;
	default:
		if (tok >= TOK_ULE && tok <= TOK_GT)
			goto relat;
		return 0;
	}
}
static unsigned char prec[256];
static void init_prec(void)
{
	int i;
	for (i = 0; i < 256; i++)
		prec[i] = precedence(i);
}
#define precedence(i) ((unsigned)i < 256 ? prec[i] : 0)

static void expr_landor(int op);

static void expr_infix(int p)
{
	int t = tok, p2;
	while ((p2 = precedence(t)) >= p) {
		if (t == TOK_LOR || t == TOK_LAND) {
			expr_landor(t);
		} else {
			next();
			unary();
			if (precedence(tok) > p2)
				expr_infix(p2 + 1);
			gen_op(t);
		}
		t = tok;
	}
}
/* Assuming vtop is a value used in a conditional context
   (i.e. compared with zero) return 0 if it's false, 1 if
   true and -1 if it can't be statically determined.  */

static int condition_3way(void)
{
	int c = -1;
	if ((vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST &&
	    (!(vtop->r & VT_SYM) || !vtop->sym->a.weak)) {
		vdup();
		gen_cast_s(VT_BOOL);
		c = vtop->c.i;
		vpop();
	}
	return c;
}

static void expr_landor(int op)
{
	int t = 0, cc = 1, f = 0, i = op == TOK_LAND, c;
	for (;;) {
		c = f ? i : condition_3way();
		if (c < 0)
			save_regs(1), cc = 0;
		else if (c != i)
			nocode_wanted++, f = 1;
		if (tok != op)
			break;
		if (c < 0)
			t = gvtst(i, t);
		else
			vpop();
		next();
		expr_landor_next(op);
	}
	if (cc || f) {
		vpop();
		vpushi(i ^ f);
		gsym(t);
		nocode_wanted -= f;
	} else {
		gvtst_set(i, t);
	}
}

static int is_cond_bool(SValue *sv)
{
	if ((sv->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST
	    && (sv->type.t & VT_BTYPE) == VT_INT)
		return (unsigned)sv->c.i < 2;
	if (sv->r == VT_CMP)
		return 1;
	return 0;
}

static void expr_cond(void)
{
	int tt, u, r1, r2, rc, t1, t2, islv, c, g;
	SValue sv;
	CType type;

	expr_lor();
	if (tok == '?') {
		next();
		c = condition_3way();
		g = (tok == ':' && gnu_ext);
		tt = 0;
		if (!g) {
			if (c < 0) {
				save_regs(1);
				tt = gvtst(1, 0);
			} else {
				vpop();
			}
		} else if (c < 0) {
			/* needed to avoid having different registers saved in
			               each branch */

			save_regs(1);
			gv_dup();
			tt = gvtst(0, 0);
		}

		if (c == 0)
			nocode_wanted++;
		if (!g)
			gexpr();

		if ((vtop->type.t & VT_BTYPE) == VT_FUNC)
			mk_pointer(&vtop->type);
		sv = *vtop;/* save value to handle it later */

		vtop--;/* no vpop so that FP stack is not flushed */

		if (g) {
			u = tt;
		} else if (c < 0) {
			u = gjmp(0);
			gsym(tt);
		} else
			u = 0;

		if (c == 0)
			nocode_wanted--;
		if (c == 1)
			nocode_wanted++;
		skip(':');
		expr_cond();

		if ((vtop->type.t & VT_BTYPE) == VT_FUNC)
			mk_pointer(&vtop->type);
		/* cast operands to correct type according to ISOC rules */

		if (!combine_types(&type, &sv, vtop, '?'))
			type_incompatibility_error(&sv.type, &vtop->type,
						   "type mismatch in conditional expression (have '%s' and '%s')");

		if (c < 0 && is_cond_bool(vtop) && is_cond_bool(&sv)) {
			/* optimize "if (f ? a > b : c || d) ..." for example, where normally
			               "a < b" and "c || d" would be forced to "(int)0/1" first, whereas
			               this code jumps directly to the if's then/else branches. */

			t1 = gvtst(0, 0);
			t2 = gjmp(0);
			gsym(u);
			vpushv(&sv);
			/* combine jump targets of 2nd op with VT_CMP of 1st op */

			gvtst_set(0, t1);
			gvtst_set(1, t2);
			gen_cast(&type);
// tcc_warning("two conditions expr_cond");

			return;
		}
		/* keep structs lvalue by transforming `(expr ? a : b)` to `*(expr ? &a : &b)` so
		           that `(expr ? a : b).mem` does not error  with "lvalue expected" */

		islv = VT_STRUCT == (type.t & VT_BTYPE);
		/* now we convert second operand */

		if (c != 1) {
			gen_cast(&type);
			if (islv) {
				mk_pointer(&vtop->type);
				gaddrof();
			}
		}

		rc = RC_TYPE(type.t);
		/* for long longs, we use fixed registers to avoid having
		           to handle a complicated move */

		if (USING_TWO_WORDS(type.t))
			rc = RC_RET(type.t);

		tt = r2 = 0;
		if (c < 0) {
			if (type.t != VT_VOID)
				r2 = gv(rc);
			tt = gjmp(0);
		}
		gsym(u);
		if (c == 1)
			nocode_wanted--;
		/* this is horrible, but we mu\st also convert first
		           operand */

		if (c != 0) {
			*vtop = sv;
			gen_cast(&type);
			if (islv) {
				mk_pointer(&vtop->type);
				gaddrof();
			}
		}

		if (c < 0) {
			if (type.t != VT_VOID) {
				r1 = gv(rc);
				move_reg(r2, r1, islv ? VT_PTR : type.t);
				vtop->r = r2;
			}
			gsym(tt);
		}

		if (islv)
			indir();
	}
}

static void expr_eq(void)
{
	int t;

	expr_cond();
	if ((t = tok) == '=' || TOK_ASSIGN(t)) {
		test_lvalue();
		next();
		if (t == '=') {
			expr_eq();
		} else {
			vdup();
			expr_eq();
			gen_op(TOK_ASSIGN_OP(t));
		}
		vstore();
	}
}

ST_FUNC void gexpr(void)
{
	expr_eq();
	if (tok == ',') {
		do {
			vpop();
			next();
			expr_eq();
		} while (tok == ',');
		/* convert array & function to pointer */

		convert_parameter_type(&vtop->type);
		/* make builtin_constant_p((1,2)) return 0 (like on gcc) */

		if ((vtop->r & VT_VALMASK) == VT_CONST && nocode_wanted && !CONST_WANTED)
			if (vtop->type.t != VT_VOID && (vtop->type.t & VT_BTYPE) != VT_STRUCT)
				gv(RC_TYPE(vtop->type.t));
	}
}
/* parse a constant expression and return value in vtop.  */

static void expr_const1(void)
{
	nocode_wanted += CONST_WANTED_BIT;
	expr_cond();
	nocode_wanted -= CONST_WANTED_BIT;
}
/* parse an integer constant and return its value. */

static inline int64_t expr_const64(void)
{
	int64_t c;
	expr_const1();
	if ((vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM | VT_NONCONST)) != VT_CONST)
		expect("constant expression");
	c = vtop->c.i;
	vpop();
	return c;
}
/* parse an integer constant and return its value.
   Complain if it doesn't fit 32bit (signed or unsigned).  */

ST_FUNC int expr_const(void)
{
	int c;
	int64_t wc = expr_const64();
	c = wc;
	if (c != wc && (unsigned)c != wc)
		tcc_error("constant exceeds 32 bit");
	return c;
}
/* ------------------------------------------------------------------------- */
/* return from function */

static void gfunc_return(CType *func_type)
{
	if ((func_type->t & VT_BTYPE) == VT_STRUCT) {
		CType type, ret_type;
		int ret_align, ret_nregs, regsize;
		ret_nregs = gfunc_sret(func_type, func_var, &ret_type,
				       &ret_align, &regsize);
		if (ret_nregs < 0) {

		} else if (0 == ret_nregs) {
			/* if returning structure, must copy it to implicit
			               first pointer arg location */

			type = *func_type;
			mk_pointer(&type);
			vset(&type, VT_LOCAL | VT_LVAL, func_vc);
			indir();
			vswap();
			/* copy structure value to pointer */

			vstore();
		} else {
			/* returning structure packed into registers */

			int size, addr, align, rc, n;
			size = type_size(func_type,&align);
			if (ret_nregs * regsize > size ||
			    ((align & (ret_align - 1))
			     && ((vtop->r & VT_VALMASK) < VT_CONST/* pointer to struct */

				 || (vtop->c.i & (ret_align - 1))
				))) {
				if (ret_nregs * regsize > size)
					size = ret_nregs * regsize;
				if (ret_align > align)
					align = ret_align;
				loc = (loc - size) & -align;
				addr = loc;
				type = *func_type;
				vset(&type, VT_LOCAL | VT_LVAL, addr);
				vswap();
				vstore();
				vpop();
				vset(&ret_type, VT_LOCAL | VT_LVAL, addr);
			}
			vtop->type = ret_type;
			rc = RC_RET(ret_type.t);
//printf("struct return: n:%d t:%02x rc:%02x\n", ret_nregs, ret_type.t, rc);

			for (n = ret_nregs; --n > 0;) {
				vdup();
				gv(rc);
				vswap();
				incr_offset(regsize);
				/* We assume that when a structure is returned in multiple
				                   registers, their classes are consecutive values of the
				                   suite s(n) = 2^n */

				rc <<= 1;
			}
			gv(rc);
			vtop -= ret_nregs - 1;
		}
	} else {
		gv(RC_RET(func_type->t));
	}
	vtop--;/* NOT vpop() because on x86 it would flush the fp stack */

}

static void check_func_return(void)
{
	if ((func_vt.t & VT_BTYPE) == VT_VOID)
		return;
	if ((!strcmp(funcname, "main") || func_old)
	    && (func_vt.t & VT_BTYPE) == VT_INT) {
		/* main returns 0 by default */

		vpushi(0);
		gfunc_return(&func_vt);
	} else {
		tcc_warning("function might return no value: '%s'", funcname);
	}
}
/* ------------------------------------------------------------------------- */
/* switch/case */

static int case_cmp(uint64_t a, uint64_t b)
{
	if (cur_switch->sv.type.t & VT_UNSIGNED)
		return a < b ? -1 : a > b;
	else
		return (int64_t)a < (int64_t)b ? -1 : (int64_t)a > (int64_t)b;
}

static int case_cmp_qs(const void *pa, const void *pb)
{
	return case_cmp((*(struct case_t **)pa)->v1, (*(struct case_t **)pb)->v1);
}

static void case_sort(struct switch_t *sw)
{
	struct case_t **p;
	if (sw->n < 2)
		return;
	qsort(sw->p, sw->n, sizeof *sw->p, case_cmp_qs);
	p = sw->p;
	while (p < sw->p + sw->n - 1) {
		if (case_cmp(p[0]->v2, p[1]->v1) >= 0) {
			int l1 = p[0]->line, l2 = p[1]->line;
			/* using special format "%i:..." to show specific line */

			tcc_error("%i:duplicate case value", l1 > l2 ? l1 : l2);
		} else if (p[0]->v2 + 1 == p[1]->v1 && p[0]->ind == p[1]->ind) {
			/* treat "case 1: case 2: case 3:" like "case 1 ... 3: */

			p[1]->v1 = p[0]->v1;
			tcc_free(p[0]);
			memmove(p, p + 1, (--sw->n - (p - sw->p)) * sizeof *p);
		} else
			++p;
	}
}

static int gcase(struct case_t **base, int len, int dsym)
{
	struct case_t *p;
	int t, l2, e;

	t = vtop->type.t & VT_BTYPE;
	if (t != VT_LLONG)
		t = VT_INT;
	while (len) {
		/* binary search while len > 8, else linear */

		l2 = len > 8 ? len/2 : 0;
		p = base[l2];
		vdup(), vpush64(t, p->v2);
		if (l2 == 0 && p->v1 == p->v2) {
			gen_op(TOK_EQ);/* jmp to case when equal */

			gsym_addr(gvtst(0, 0), p->ind);
		} else {
			/* case v1 ... v2 */

			gen_op(TOK_GT);/* jmp over when > V2 */

			if (len == 1)/* last case test jumps to default when false */

				dsym = gvtst(0, dsym), e = 0;
			else
				e = gvtst(0, 0);
			vdup(), vpush64(t, p->v1);
			gen_op(TOK_GE);/* jmp to case when >= V1 */

			gsym_addr(gvtst(0, 0), p->ind);
			dsym = gcase(base, l2, dsym);
			gsym(e);
		}
		++l2, base += l2, len -= l2;
	}
	/* jump automagically will suppress more jumps */

	return gjmp(dsym);
}

static void end_switch(void)
{
	struct switch_t *sw = cur_switch;
	dynarray_reset(&sw->p, &sw->n);
	cur_switch = sw->prev;
	tcc_free(sw);
}
/* ------------------------------------------------------------------------- */
/* __attribute__((cleanup(fn))) */
/* protect symbol lvalues from further modification  */

static void save_lvalues(void)
{
	SValue *sv = vtop;
	while (sv >= vstack) {
		if (sv->sym && (sv->r & VT_LVAL)) {
			int align, size = type_size(&sv->type, &align);
			int r2, l = get_temp_local_var(size, align, &r2);
			vset(&sv->type, VT_LOCAL | VT_LVAL, l), vtop->r2 = r2;
			vpushv(sv), *sv = vtop[-1], vstore(), --vtop;
		}
		--sv;
	}
}

static void try_call_scope_cleanup(Sym *stop)
{
	Sym *cls = cur_scope->cl.s;
	for (; cls != stop; cls = cls->next) {
		Sym *fs = cls->cleanup_func;
		Sym *vs = cls->cleanup_sym;
		save_lvalues();
		vpushsym(&fs->type, fs);
		vset(&vs->type, vs->r, vs->c);
		vtop->sym = vs;
		mk_pointer(&vtop->type);
		gaddrof();
		gfunc_call(1);
	}
}

static void try_call_cleanup_goto(Sym *cleanupstate)
{
	Sym *oc, *cc;
	int ocd, ccd;

	if (!cur_scope->cl.s)
		return;
	/* search NCA of both cleanup chains given parents and initial depth */

	ocd = cleanupstate ? cleanupstate->v & ~SYM_FIELD : 0;
	for (ccd = cur_scope->cl.n, oc = cleanupstate; ocd > ccd; --ocd, oc = oc->next)
		;
	for (cc = cur_scope->cl.s; ccd > ocd; --ccd, cc = cc->next)
		;
	for (; cc != oc; cc = cc->next, oc = oc->next, --ccd)
		;

	try_call_scope_cleanup(cc);
}
/* call 'func' for each __attribute__((cleanup(func))) */

static void block_cleanup(struct scope *o)
{
	int jmp = 0;
	Sym *g, **pg;
	for (pg = &pending_gotos; (g = *pg) && g->c > o->cl.n;) {
		if (g->cleanup_label->r & LABEL_FORWARD) {
			Sym *pcl = g->next;
			if (!jmp)
				jmp = gjmp(0);
			gsym(pcl->jnext);
			try_call_scope_cleanup(o->cl.s);
			pcl->jnext = gjmp(0);
			if (!o->cl.n)
				goto remove_pending;
			g->c = o->cl.n;
			pg = &g->prev;
		} else {
remove_pending:
			*pg = g->prev;
			sym_free(g);
		}
	}
	gsym(jmp);
	try_call_scope_cleanup(o->cl.s);
}
/* ------------------------------------------------------------------------- */
/* VLA */

static void vla_restore(int loc)
{
	if (loc)
		gen_vla_sp_restore(loc);
}

static void vla_leave(struct scope *o)
{
	struct scope *c = cur_scope, *v = NULL;
	for (; c != o && c; c = c->prev)
		if (c->vla.num)
			v = c;
	if (v)
		vla_restore(v->vla.locorig);
}
/* ------------------------------------------------------------------------- */
/* local scopes */

static void new_scope(struct scope *o)
{
	/* copy and link previous scope */

	*o = *cur_scope;
	o->prev = cur_scope;
	cur_scope = o;
	cur_scope->vla.num = 0;
	/* record local declaration stack position */

	o->lstk = local_stack;
	o->llstk = local_label_stack;
	++local_scope;
}

static void prev_scope(struct scope *o, int is_expr)
{
	vla_leave(o->prev);

	if (o->cl.s != o->prev->cl.s)
		block_cleanup(o->prev);

	if (debug_modes)
		tcc_debug_end_scope(o->lstk, !is_expr);
	/* pop locally defined labels */

	label_pop(&local_label_stack, o->llstk, is_expr);
	/* In the is_expr case (a statement expression is finished here),
	       vtop might refer to symbols on the local_stack.  Either via the
	       type or via vtop->sym.  We can't pop those nor any that in turn
	       might be referred to.  To make it easier we don't roll back
	       any symbols in that case; some upper level call to block() will
	       do that.  We do have to remove such symbols from the lookup
	       tables, though.  sym_pop will do that.  */
	/* pop locally defined symbols */
	sym_pop(&local_stack, o->lstk, is_expr);
	cur_scope = o->prev;
	--local_scope;
}
/* leave a scope via break/continue(/goto) */

static void leave_scope(struct scope *o)
{
	if (!o)
		return;
	try_call_scope_cleanup(o->cl.s);
	vla_leave(o);
}
/* short versiona for scopes with 'if/do/while/switch' which can
   declare only types (of struct/union/enum) */

static void new_scope_s(struct scope *o)
{
	o->lstk = local_stack;
	++local_scope;
}

static void prev_scope_s(struct scope *o)
{
	sym_pop(&local_stack, o->lstk, 0);
	--local_scope;
}
/* ------------------------------------------------------------------------- */
/* call block from 'for do while' loops */

static void lblock(int *bsym, int *csym)
{
	struct scope *lo = loop_scope, *co = cur_scope;
	int *b = co->bsym, *c = co->csym;
	if (csym) {
		co->csym = csym;
		loop_scope = co;
	}
	co->bsym = bsym;
	block(0);
	co->bsym = b;
	if (csym) {
		co->csym = c;
		loop_scope = lo;
	}
}
/* c2y if/switch declaration */

static void gexpr_decl(void)
{
	int v = decl(VT_JMP);
	if (v > 1 && tok != ';') {
		Sym *s = sym_find(v);
		vset(&s->type, s->r, (s->r & VT_SYM) ? 0 : s->c);
		vtop->sym = s;
	} else {
		if (v)
			skip(';');
		gexpr();
	}
}

static void block(int flags)
{
	int a, b, c, d, e, t;
	struct scope o;
	Sym *s;

again:
	t = tok;
	/* If the token carries a value, next() might destroy it. Only with
	       invalid code such as f(){"123"4;} */

	if (TOK_HAS_VALUE(t))
		goto expr;
	next();

	if (debug_modes)
		tcc_tcov_check_line (tcc_state, 0), tcc_tcov_block_begin (tcc_state);

	if (t == TOK_IF) {
		new_scope_s(&o);
		skip('(');
		gexpr_decl();
		a = gvtst(1, 0);
		skip(')');
		block(0);
		if (tok == TOK_ELSE) {
			d = gjmp(0);
			gsym(a);
			next();
			block(0);
			gsym(d);/* patch else jmp */

		} else {
			gsym(a);
		}
		prev_scope_s(&o);

	} else if (t == TOK_WHILE) {
		new_scope_s(&o);
		d = gind();
		skip('(');
		gexpr();
		a = gvtst(1, 0);
		skip(')');
		b = 0;
		lblock(&a, &b);
		gjmp_addr(d);
		gsym_addr(b, d);
		gsym(a);
		prev_scope_s(&o);

	} else if (t == '{') {
		if (debug_modes)
			tcc_debug_stabn(tcc_state, N_LBRAC, ind - func_ind);
		new_scope(&o);
		/* handle local labels declarations */

		while (tok == TOK_LABEL) {
			do {
				next();
				if (tok < TOK_UIDENT)
					expect("label identifier");
				label_push(&local_label_stack, tok, LABEL_DECLARED);
				next();
			} while (tok == ',');
			skip(';');
		}

		while (tok != '}') {
			decl(VT_LOCAL);
			if (tok != '}') {
				block(flags | STMT_COMPOUND);
			}
		}

		prev_scope(&o, flags & STMT_EXPR);
		if (debug_modes)
			tcc_debug_stabn(tcc_state, N_RBRAC, ind - func_ind);
		if (local_scope)
			next();
		else if (!nocode_wanted)
			check_func_return();

	} else if (t == TOK_RETURN) {
		b = (func_vt.t & VT_BTYPE) != VT_VOID;
		if (tok != ';') {
			gexpr();
			if (b) {
				gen_assign_cast(&func_vt);
			} else {
				if (vtop->type.t != VT_VOID)
					tcc_warning("void function returns a value");
				vtop--;
			}
		} else if (b && func_old && (func_vt.t & VT_BTYPE) == VT_INT) {
			vpushi(0);
		} else if (b) {
			tcc_warning("'return' with no value");
			b = 0;
		}
		leave_scope(root_scope);
		if (b)
			gfunc_return(&func_vt);
		skip(';');
		/* jump unless last stmt in top-level block */

		if (tok != '}' || local_scope != 1)
			rsym = gjmp(rsym);
		if (debug_modes)
			tcc_tcov_block_end (tcc_state, -1);
		CODE_OFF();

	} else if (t == TOK_BREAK) {
		/* compute jump */

		if (!cur_scope->bsym)
			tcc_error("cannot break");
		if (cur_switch && cur_scope->bsym == cur_switch->bsym)
			leave_scope(cur_switch->scope);
		else
			leave_scope(loop_scope);
		*cur_scope->bsym = gjmp(*cur_scope->bsym);
		skip(';');

	} else if (t == TOK_CONTINUE) {
		/* compute jump */

		if (!cur_scope->csym)
			tcc_error("cannot continue");
		leave_scope(loop_scope);
		*cur_scope->csym = gjmp(*cur_scope->csym);
		skip(';');

	} else if (t == TOK_FOR) {
		new_scope(&o);

		skip('(');
		if (tok != ';') {
			/* c99 for-loop init decl? */

			if (!decl(VT_JMP)) {
				/* no, regular for-loop init expr */

				gexpr();
				vpop();
			}
		}
		skip(';');
		a = b = 0;
		c = d = gind();
		if (tok != ';') {
			gexpr();
			a = gvtst(1, 0);
		}
		skip(';');
		if (tok != ')') {
			e = gjmp(0);
			d = gind();
			gexpr();
			vpop();
			gjmp_addr(c);
			gsym(e);
		}
		skip(')');
		lblock(&a, &b);
		gjmp_addr(d);
		gsym_addr(b, d);
		gsym(a);
		prev_scope(&o, 0);

	} else if (t == TOK_DO) {
		new_scope_s(&o);
		a = b = 0;
		d = gind();
		lblock(&a, &b);
		gsym(b);
		skip(TOK_WHILE);
		skip('(');
		gexpr();
		c = gvtst(0, 0);
		skip(')');
		skip(';');
		gsym_addr(c, d);
		gsym(a);
		prev_scope_s(&o);

	} else if (t == TOK_SWITCH) {
		struct switch_t *sw;

		sw = tcc_mallocz(sizeof *sw);
		sw->bsym = &a;
		sw->scope = cur_scope;
		sw->prev = cur_switch;
		sw->nocode_wanted = nocode_wanted;
		cur_switch = sw;

		new_scope_s(&o);
		skip('(');
		gexpr_decl();
		if (!is_integer_btype(vtop->type.t & VT_BTYPE))
			tcc_error("switch value not an integer");
		skip(')');
		sw->sv = *vtop--;/* save switch value */

		a = 0;
		b = gjmp(0);/* jump to first case */

		lblock(&a, NULL);
		a = gjmp(a);/* add implicit break */

		/* case lookup */

		gsym(b);
		prev_scope_s(&o);
		if (sw->nocode_wanted)
			goto skip_switch;
		case_sort(sw);
		sw->bsym = NULL;/* marker for 32bit:gen_opl() */

		vpushv(&sw->sv);
		gv(RC_INT);
		d = gcase(sw->p, sw->n, 0);
		vpop();
		if (sw->def_sym)
			gsym_addr(d, sw->def_sym);
		else
			gsym(d);
skip_switch:
		/* break label */

		gsym(a);
		end_switch();

	} else if (t == TOK_CASE) {
		struct case_t *cr;
		if (!cur_switch)
			expect("switch");
		cr = tcc_malloc(sizeof(struct case_t));
		dynarray_add(&cur_switch->p, &cur_switch->n, cr);
		t = cur_switch->sv.type.t;
		cr->v1 = cr->v2 = value64(expr_const64(), t);
		if (tok == TOK_DOTS && gnu_ext) {
			next();
			cr->v2 = value64(expr_const64(), t);
			if (case_cmp(cr->v2, cr->v1) < 0)
				tcc_warning("empty case range");
		}
		/* case and default are unreachable from a switch under nocode_wanted */

		if (!cur_switch->nocode_wanted)
			cr->ind = gind();
		cr->line = file->line_num;
		skip(':');
		goto block_after_label;

	} else if (t == TOK_DEFAULT) {
		if (!cur_switch)
			expect("switch");
		if (cur_switch->def_sym)
			tcc_error("too many 'default'");
		cur_switch->def_sym = cur_switch->nocode_wanted ? -1 : gind();
		skip(':');
		goto block_after_label;

	} else if (t == TOK_GOTO) {
		vla_restore(cur_scope->vla.locorig);
		if (tok == '*' && gnu_ext) {
			/* computed goto */

			next();
			gexpr();
			if ((vtop->type.t & VT_BTYPE) != VT_PTR)
				expect("pointer");
			ggoto();

		} else if (tok >= TOK_UIDENT) {
			s = label_find(tok);
			/* put forward definition if needed */

			if (!s)
				s = label_push(&global_label_stack, tok, LABEL_FORWARD);
			else if (s->r == LABEL_DECLARED)
				s->r = LABEL_FORWARD;

			if (s->r & LABEL_FORWARD) {
				/* start new goto chain for cleanups, linked via label->next */

				if (cur_scope->cl.s && !nocode_wanted) {
					sym_push2(&pending_gotos, SYM_FIELD, 0, cur_scope->cl.n);
					pending_gotos->cleanup_label = s;
					s = sym_push2(&s->next, SYM_FIELD, 0, 0);
					pending_gotos->next = s;
				}
				s->jnext = gjmp(s->jnext);
			} else {
				try_call_cleanup_goto(s->cleanupstate);
				gjmp_addr(s->jind);
			}
			next();

		} else {
			expect("label identifier");
		}
		skip(';');

	} else if (t == TOK_ASM1 || t == TOK_ASM2 || t == TOK_ASM3) {
		asm_instr();

	} else {
		if (tok == ':' && t >= TOK_UIDENT) {
			/* label case */

			next();
			s = label_find(t);
			if (s) {
				if (s->r == LABEL_DEFINED)
					tcc_error("duplicate label '%s'", get_tok_str(s->v, NULL));
				s->r = LABEL_DEFINED;
				if (s->next) {
					Sym *pcl;/* pending cleanup goto */

					for (pcl = s->next; pcl; pcl = pcl->prev)
						gsym(pcl->jnext);
					sym_pop(&s->next, NULL, 0);
				} else
					gsym(s->jnext);
			} else {
				s = label_push(&global_label_stack, t, LABEL_DEFINED);
			}
			s->jind = gind();
			s->cleanupstate = cur_scope->cl.s;

block_after_label:
			/* Accept attributes after labels (e.g. 'unused') */

			parse_attribute(NULL);

			if (debug_modes)
				tcc_tcov_reset_ind(tcc_state);
			vla_restore(cur_scope->vla.loc);

			if (tok != '}') {
				if (0 == (flags & STMT_COMPOUND))
					goto again;
				/* C23: insert implicit null-statement whithin compound statement */

			} else {
				/* we accept this, but it is a mistake */

				tcc_warning_c(warn_all)("deprecated use of label at end of compound statement");
			}
		} else {
			/* expression case */

			if (t != ';') {
				unget_tok(t);
expr:
				if (flags & STMT_EXPR) {
					vpop();
					gexpr();
				} else {
					gexpr();
					vpop();
				}
				skip(';');
			}
		}
	}

	if (debug_modes)
		tcc_tcov_check_line (tcc_state, 0), tcc_tcov_block_end (tcc_state, 0);
}
/* This skips over a stream of tokens containing balanced {} and ()
   pairs, stopping at outer ',' ';' and '}' (or matching '}' if we started
   with a '{').  If STR then allocates and stores the skipped tokens
   in *STR.  This doesn't check if () and {} are nested correctly,
   i.e. "({)}" is accepted.  */

static void skip_or_save_block(TokenString **str)
{
	int braces = tok == '{';
	int level = 0;
	if (str)
		*str = tok_str_alloc();

	while (1) {
		int t = tok;
		if (level == 0
		    && (t == ','
			|| t == ';'
			|| t == '}'
			|| t == ')'
			|| t == ']'))
			break;
		if (t == TOK_EOF) {
			if (str || level > 0)
				tcc_error("unexpected end of file");
			else
				break;
		}
		if (str)
			tok_str_add_tok(*str);
		next();
		if (t == '{' || t == '(' || t == '[') {
			level++;
		} else if (t == '}' || t == ')' || t == ']') {
			level--;
			if (level == 0 && braces && t == '}')
				break;
		}
	}
	if (str)
		tok_str_add(*str, TOK_EOF);
}

#define EXPR_CONST 1
#define EXPR_ANY 2

static void parse_init_elem(int expr_type)
{
	int saved_global_expr;
	switch (expr_type) {
	case EXPR_CONST:
		/* compound literals must be allocated globally in this case */

		saved_global_expr = global_expr;
		global_expr = 1;
		expr_const1();
		global_expr = saved_global_expr;
		/* NOTE: symbols are accepted, as well as lvalue for anon symbols
			   (compound literals).  */

		if (((vtop->r & (VT_VALMASK | VT_LVAL)) != VT_CONST
		     && ((vtop->r & (VT_SYM|VT_LVAL)) != (VT_SYM|VT_LVAL)
			 || vtop->sym->v < SYM_FIRST_ANOM))

		    || ((vtop->r & VT_SYM) && vtop->sym->a.dllimport)

		   )
			tcc_error("initializer element is not constant");
		break;
	case EXPR_ANY:
		expr_eq();
		break;
	}
}

static void init_assert(init_params *p, int offset)
{
	if (p->sec ? !NODATA_WANTED && offset > p->sec->data_offset
	    : !nocode_wanted && offset > p->local_offset)
		tcc_internal_error("initializer overflow");
}
/* put zeros for variable based init */

static void init_putz(init_params *p, unsigned long c, int size)
{
	init_assert(p, c + size);
	if (p->sec) {
		/* nothing to do because globals are already set to zero */

	} else {
		vpush_helper_func(TOK_memset);
		vseti(VT_LOCAL, c);
		vpushi(0);
		vpushs(size);

		gfunc_call(3);
	}
}

#define DIF_FIRST 1
#define DIF_SIZE_ONLY 2
#define DIF_HAVE_ELEM 4
#define DIF_CLEAR 8
/* delete relocations for specified range c ... c + size. Unfortunatly
   in very special cases, relocations may occur unordered */

static void decl_design_delrels(Section *sec, int c, int size)
{
	ElfW_Rel *rel, *rel2, *rel_end;
	if (!sec || !sec->reloc)
		return;
	rel = rel2 = (ElfW_Rel *)sec->reloc->data;
	rel_end = (ElfW_Rel *)(sec->reloc->data + sec->reloc->data_offset);
	while (rel < rel_end) {
		if (rel->r_offset >= c && rel->r_offset < c + size) {
			sec->reloc->data_offset -= sizeof *rel;
		} else {
			if (rel2 != rel)
				memcpy(rel2, rel, sizeof *rel);
			++rel2;
		}
		++rel;
	}
}

static void decl_design_flex(init_params *p, Sym *ref, int index)
{
	if (ref == p->flex_array_ref) {
		if (index >= ref->c)
			ref->c = index + 1;
	} else if (ref->c < 0 && index >= 0)
		tcc_error("flexible array has zero size in this context");
}
/* t is the array or struct type. c is the array or struct
   address. cur_field is the pointer to the current
   field, for arrays the 'c' member contains the current start
   index.  'flags' is as in decl_initializer.
   'al' contains the already initialized length of the
   current container (starting at c).  This returns the new length of that.  */

static int decl_designator(init_params *p, CType *type, unsigned long c,
			   Sym **cur_field, int flags, int al)
{
	Sym *s, *f;
	int index, index_last, align, l, nb_elems, elem_size;
	unsigned long corig = c;

	elem_size = 0;
	nb_elems = 1;

	if (flags & DIF_HAVE_ELEM)
		goto no_designator;

	if (gnu_ext && tok >= TOK_UIDENT) {
		l = tok, next();
		if (tok == ':')
			goto struct_field;
		unget_tok(l);
	}
	/* NOTE: we only support ranges for last designator */

	while (nb_elems == 1 && (tok == '[' || tok == '.')) {
		if (tok == '[') {
			if (!(type->t & VT_ARRAY))
				expect("array type");
			next();
			index = index_last = expr_const();
			if (tok == TOK_DOTS && gnu_ext) {
				next();
				index_last = expr_const();
			}
			skip(']');
			s = type->ref;
			decl_design_flex(p, s, index_last);
			if (index < 0 || index_last >= s->c || index_last < index)
				tcc_error("index exceeds array bounds or range is empty");
			if (cur_field)
				(*cur_field)->c = index_last;
			type = pointed_type(type);
			elem_size = type_size(type, &align);
			c += index * elem_size;
			nb_elems = index_last - index + 1;
		} else {
			int cumofs;
			next();
			l = tok;
struct_field:
			next();
			f = find_field(type, l, &cumofs);
			if (cur_field)
				*cur_field = f;
			type = &f->type;
			c += cumofs;
		}
		cur_field = NULL;
	}
	if (!cur_field) {
		if (tok == '=') {
			next();
		} else if (!gnu_ext) {
			expect("=");
		}
	} else {
no_designator:
		if (type->t & VT_ARRAY) {
			index = (*cur_field)->c;
			s = type->ref;
			decl_design_flex(p, s, index);
			if (index >= s->c)
				tcc_error("too many initializers");
			type = pointed_type(type);
			elem_size = type_size(type, &align);
			c += index * elem_size;
		} else {
			f = *cur_field;
			/* Skip bitfield padding. Also with size 32 and 64. */

			while (f && (f->v & SYM_FIRST_ANOM) &&
			       is_integer_btype(f->type.t & VT_BTYPE))
				*cur_field = f = f->next;
			if (!f)
				tcc_error("too many initializers");
			type = &f->type;
			c += f->c;
		}
	}

	if (!elem_size)/* for structs */

		elem_size = type_size(type, &align);
	/* Using designators the same element can be initialized more
	       than once.  In that case we need to delete possibly already
	       existing relocations. */

	if (!(flags & DIF_SIZE_ONLY) && c - corig < al) {
		decl_design_delrels(p->sec, c, elem_size * nb_elems);
		flags &= ~DIF_CLEAR;/* mark stack dirty too */

	}

	decl_initializer(p, type, c, flags & ~DIF_FIRST);

	if (!(flags & DIF_SIZE_ONLY) && nb_elems > 1) {
		Sym aref = {0};
		CType t1;
		int i;
		if (p->sec || (type->t & VT_ARRAY)) {
			/* make init_putv/vstore believe it were a struct */

			aref.c = elem_size;
			t1.t = VT_STRUCT, t1.ref = &aref;
			type = &t1;
		}
		if (p->sec)
			vpush_ref(type, p->sec, c, elem_size);
		else
			vset(type, VT_LOCAL|VT_LVAL, c);
		for (i = 1; i < nb_elems; i++) {
			vdup();
			init_putv(p, type, c + elem_size * i);
		}
		vpop();
	}

	c += nb_elems * elem_size;
	if (c - corig > al)
		al = c - corig;
	return al;
}

static void write_ldouble(unsigned char *d, void *s)
{
//printf("long double %Lf\n", *(long double*)s);

	if (sizeof (long double) == 8 && LDOUBLE_SIZE >= 10) {
		/* our 'long double' is a double really (_WIN32, __APPLE__) */

		uint64_t m = *(uint64_t *)s;
		int e = m >> 48;
		int f = e >> 4 & 0x7FF;
		m <<= 11;
		if (0 == f) {
			if (0 == m)
				goto set;
			for (f = 1; !(m & 1ULL<<63); --f)
				m <<= 1;
		}
		if (f == 0x7ff)
			f = 0x43FF;
		e = (e & 0x8000) | (f + 0x3C00);
		m |= 1ULL<<63;
set:
		/* double -> extended */

		write64le(d, m);
		write16le(d+8, e);

		;
	} else {
		/* extended -> extended */

		memcpy(d, s, 10);
	}
}
/* store a value or an expression directly in global data or in local array */

static void init_putv(init_params *p, CType *type, unsigned long c)
{
	int bt;
	void *ptr;
	CType dtype;
	int size, align;
	Section *sec = p->sec;
	uint64_t val;

	dtype = *type;
	dtype.t &= ~VT_CONSTANT;/* need to do that to avoid false warning */

	size = type_size(type, &align);
	if (type->t & VT_BITFIELD)
		size = (BIT_POS(type->t) + BIT_SIZE(type->t) + 7) / 8;
	init_assert(p, c + size);

	if (sec) {
		/* XXX: not portable */
		/* XXX: generate error if incorrect relocation */

		gen_assign_cast(&dtype);
		bt = type->t & VT_BTYPE;

		if ((vtop->r & VT_SYM)
		    && bt != VT_PTR
		    && (bt != (PTR_SIZE == 8 ? VT_LLONG : VT_INT)
			|| (type->t & VT_BITFIELD))
		    && !((vtop->r & VT_CONST) && vtop->sym->v >= SYM_FIRST_ANOM)
		   )
			tcc_error("initializer element is not computable at load time");

		if (NODATA_WANTED) {
			vtop--;
			return;
		}

		ptr = sec->data + c;
		val = vtop->c.i;

		if ((vtop->r & (VT_SYM|VT_CONST)) == (VT_SYM|VT_CONST)
		    && vtop->sym->v >= SYM_FIRST_ANOM
		    && ((vtop->r & VT_LVAL)/* compound literal */

			|| bt == VT_STRUCT/* designator */

		       )) {
			/* memcpy stuff over.  */

			Section *ssec;
			ElfSym *esym;
			ElfW_Rel *rel;
			esym = elfsym(vtop->sym);
			ssec = tcc_state->sections[esym->st_shndx];
			memmove (ptr, ssec->data + esym->st_value + (int)vtop->c.i, size);
			if (ssec->reloc) {
				/* We need to copy over all memory contents, and that
						   includes relocations.  Use the fact that relocs are
						   created it order, so look from the end of relocs
						   until we hit one before the copied region.  */

				unsigned long relofs = ssec->reloc->data_offset;
				while (relofs >= sizeof(*rel)) {
					relofs -= sizeof(*rel);
					rel = (ElfW_Rel *)(ssec->reloc->data + relofs);
					if (rel->r_offset >= esym->st_value + size)
						continue;
					if (rel->r_offset < esym->st_value)
						break;
					put_elf_reloca(symtab_section, sec,
						       c + rel->r_offset - esym->st_value,
						       ELFW(R_TYPE)(rel->r_info),
						       ELFW(R_SYM)(rel->r_info),

						       rel->r_addend

						      );
				}
			}
		} else {
			if (type->t & VT_BITFIELD) {
				int bit_pos, bit_size, bits, n;
				unsigned char *p, v, m;
				bit_pos = BIT_POS(vtop->type.t);
				bit_size = BIT_SIZE(vtop->type.t);
				p = (unsigned char *)ptr + (bit_pos >> 3);
				bit_pos &= 7, bits = 0;
				while (bit_size) {
					n = 8 - bit_pos;
					if (n > bit_size)
						n = bit_size;
					v = val >> bits << bit_pos;
					m = ((1 << n) - 1) << bit_pos;
					*p = (*p & ~m) | (v & m);
					bits += n, bit_size -= n, bit_pos = 0, ++p;
				}
			} else
				switch (bt) {
				case VT_BOOL:
					*(char *)ptr = val != 0;
					break;
				case VT_BYTE:
					*(char *)ptr = val;
					break;
				case VT_SHORT:
					write16le(ptr, val);
					break;
				case VT_FLOAT:
					write32le(ptr, val);
					break;
				case VT_DOUBLE:
					write64le(ptr, val);
					break;
				case VT_LDOUBLE:
					write_ldouble(ptr, &vtop->c.ld);
					break;
				/* intptr_t may need a reloc too, see tcctest.c:relocation_test() */

				case VT_LLONG:
				case VT_PTR:
					if (vtop->r & VT_SYM)
						greloca(sec, vtop->sym, c, R_DATA_PTR, val);
					else
						write64le(ptr, val);
					break;
				case VT_INT:
					write32le(ptr, val);
					break;
				default:
//tcc_internal_error("unexpected type");

					break;
				}
		}
		vtop--;
	} else {
		vset(&dtype, VT_LOCAL|VT_LVAL, c);
		vswap();
		vstore();
		vpop();
	}
}
/* 't' contains the type and storage info. 'c' is the offset of the
   object in section 'sec'. If 'sec' is NULL, it means stack based
   allocation. 'flags & DIF_FIRST' is true if array '{' must be read (multi
   dimension implicit array init handling). 'flags & DIF_SIZE_ONLY' is true if
   size only evaluation is wanted (only for arrays). */

static void decl_initializer(init_params *p, CType *type, unsigned long c,
			     int flags)
{
	int len, n, no_oblock, i;
	int size1, align1;
	Sym *s, *f;
	Sym indexsym;
	CType *t1;
	/* generate line number info */

	if (debug_modes && !(flags & DIF_SIZE_ONLY) && !p->sec)
		tcc_debug_line(tcc_state), tcc_tcov_check_line (tcc_state, 1);

	if (!(flags & DIF_HAVE_ELEM) && tok != '{' &&
	    /* In case of strings we have special handling for arrays, so
	    	   don't consume them as initializer value (which would commit them
	    	   to some anonymous symbol).  */

	    tok != TOK_LSTR && tok != TOK_STR &&
	    (!(flags & DIF_SIZE_ONLY)
	     /* a struct may be initialized from a struct of same type, as in
	                         struct {int x,y;} a = {1,2}, b = {3,4}, c[] = {a,b};
	                    In that case we need to parse the element in order to check
	                    it for compatibility below */

	     || (type->t & VT_BTYPE) == VT_STRUCT)
	   ) {
		int ncw_prev = nocode_wanted;
		if ((flags & DIF_SIZE_ONLY) && !p->sec)
			++nocode_wanted;
		parse_init_elem(!p->sec ? EXPR_ANY : EXPR_CONST);
		nocode_wanted = ncw_prev;
		flags |= DIF_HAVE_ELEM;
	}

	if (type->t & VT_ARRAY) {
		no_oblock = 1;
		if (((flags & DIF_FIRST) && tok != TOK_LSTR && tok != TOK_STR) ||
		    tok == '{') {
			skip('{');
			no_oblock = 0;
		}

		s = type->ref;
		n = s->c;
		t1 = pointed_type(type);
		size1 = type_size(t1, &align1);
		/* only parse strings here if correct type (otherwise: handle
		           them as ((w)char *) expressions */

		if ((tok == TOK_LSTR &&

		     (t1->t & VT_BTYPE) == VT_SHORT && (t1->t & VT_UNSIGNED)

		    ) || (tok == TOK_STR && (t1->t & VT_BTYPE) == VT_BYTE)) {
			len = 0;
			cstr_reset(&initstr);
			if (size1 != (tok == TOK_STR ? 1 : sizeof(nwchar_t)))
				tcc_error("unhandled string literal merging");
			while (tok == TOK_STR || tok == TOK_LSTR) {
				if (initstr.size)
					initstr.size -= size1;
				if (tok == TOK_STR)
					len += tokc.str.size;
				else
					len += tokc.str.size / sizeof(nwchar_t);
				len--;
				cstr_cat(&initstr, tokc.str.data, tokc.str.size);
				next();
			}
			if (tok != ')' && tok != '}' && tok != ',' && tok != ';'
			    && tok != TOK_EOF) {
				/* Not a lone literal but part of a bigger expression.  */

				unget_tok(size1 == 1 ? TOK_STR : TOK_LSTR);
				tokc.str.size = initstr.size;
				tokc.str.data = initstr.data;
				goto do_init_array;
			}

			decl_design_flex(p, s, len);
			if (!(flags & DIF_SIZE_ONLY)) {
				int nb = n, ch;
				if (len < nb)
					nb = len;
				if (len > nb)
					tcc_warning("initializer-string for array is too long");
				/* in order to go faster for common case (char
				                   string in global variable, we handle it
				                   specifically */

				if (p->sec && size1 == 1) {
					init_assert(p, c + nb);
					if (!NODATA_WANTED)
						memcpy(p->sec->data + c, initstr.data, nb);
				} else {
					for (i=0; i<n; i++) {
						if (i >= nb) {
							/* only add trailing zero if enough storage (no
							                             warning in this case since it is standard) */

							if (flags & DIF_CLEAR)
								break;
							if (n - i >= 4) {
								init_putz(p, c + i * size1, (n - i) * size1);
								break;
							}
							ch = 0;
						} else if (size1 == 1)
							ch = ((unsigned char *)initstr.data)[i];
						else
							ch = ((nwchar_t *)initstr.data)[i];
						vpushi(ch);
						init_putv(p, t1, c + i * size1);
					}
				}
			}
			if (tok == ',' && !no_oblock)/* static const char s[] = { "123", }; */

				next();
		} else {

do_init_array:
			indexsym.c = 0;
			f = &indexsym;

do_init_list:
			/* zero memory once in advance */

			if (!(flags & (DIF_CLEAR | DIF_SIZE_ONLY))) {
				init_putz(p, c, n*size1);
				flags |= DIF_CLEAR;
			}

			len = 0;
			/* GNU extension: if the initializer is empty for a flex array,
			               it's size is zero.  We won't enter the loop, so set the size
			               now.  */

			decl_design_flex(p, s, len - 1);
			while (tok != '}' || (flags & DIF_HAVE_ELEM)) {
				len = decl_designator(p, type, c, &f, flags, len);
				flags &= ~DIF_HAVE_ELEM;
				if (type->t & VT_ARRAY) {
					++indexsym.c;
					/* special test for multi dimensional arrays (may not
							       be strictly correct if designators are used at the
							       same time) */

					if (no_oblock && len >= n*size1)
						break;
				} else {
					if (s->type.t == VT_UNION)
						f = NULL;
					else
						f = f->next;
					if (no_oblock && f == NULL)
						break;
				}

				if (tok == '}')
					break;
				skip(',');
			}
		}
		if (!no_oblock)
			skip('}');

	} else if ((flags & DIF_HAVE_ELEM)
		   /* Use i_c_parameter_t, to strip toplevel qualifiers.
		              The source type might have VT_CONSTANT set, which is
		              of course assignable to non-const elements.  */

		   && is_compatible_unqualified_types(type, &vtop->type)) {
		goto one_elem;

	} else if ((type->t & VT_BTYPE) == VT_STRUCT) {
		no_oblock = 1;
		if ((flags & DIF_FIRST) || tok == '{') {
			skip('{');
			no_oblock = 0;
		}
		s = type->ref;
		f = s->next;
		n = s->c;
		size1 = 1;
		goto do_init_list;

	} else if (tok == '{') {
		if (flags & DIF_HAVE_ELEM)
			skip(';');
		next();
		decl_initializer(p, type, c, flags & ~DIF_HAVE_ELEM);
		skip('}');

	} else
one_elem:
		if ((flags & DIF_SIZE_ONLY)) {
			/* If we supported only ISO C we wouldn't have to accept calling
				   this on anything than an array if DIF_SIZE_ONLY (and even then
				   only on the outermost level, so no recursion would be needed),
				   because initializing a flex array member isn't supported.
				   But GNU C supports it, so we need to recurse even into
				   subfields of structs and arrays when DIF_SIZE_ONLY is set.  */
			/* just skip expression */

			if (flags & DIF_HAVE_ELEM)
				vpop();
			else
				skip_or_save_block(NULL);

		} else {
			if (!(flags & DIF_HAVE_ELEM)) {
				/* This should happen only when we haven't parsed
					       the init element above for fear of committing a
					       string constant to memory too early.  */

				if (tok != TOK_STR && tok != TOK_LSTR)
					expect("string constant");
				parse_init_elem(!p->sec ? EXPR_ANY : EXPR_CONST);
			}
			if (!p->sec && (flags & DIF_CLEAR)/* container was already zero'd */

			    && (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST
			    && vtop->c.i == 0
			    && btype_size(type->t & VT_BTYPE)/* not for fp constants */

			   )
				vpop();
			else
				init_putv(p, type, c);
		}
}
/* parse an initializer for type 't' if 'has_init' is non zero, and
   allocate space in local or global data space ('r' is either
   VT_LOCAL or VT_CONST). If 'v' is non zero, then an associated
   variable 'v' of scope 'scope' is declared before initializers
   are parsed. If 'v' is zero, then a reference to the new object
   is put in the value stack. If 'has_init' is 2, a special parsing
   is done to handle string constants. */
static void decl_initializer_alloc(CType *type, AttributeDef *ad, int r,
				   int has_init, int v, int scope)
{
	int size, align, addr;
	TokenString *init_str = NULL;

	Section *sec;
	Sym *flexible_array;
	Sym *sym = NULL;
	int saved_nocode_wanted = nocode_wanted;

	init_params p = {0};

	if (scope == VT_CONST) {
		/* see if a global symbol was already defined */

		sym = sym_find(v);
		if (sym) {
			patch_storage(sym, ad, type);
			/* we accept several definitions of the same global variable. */

			if (!has_init && sym->c && elfsym(sym)->st_shndx != SHN_UNDEF)
				return;
			type = &sym->type;
		}
	}
	/* Always allocate static or global variables */

	if (v && (r & VT_VALMASK) == VT_CONST)
		nocode_wanted |= DATA_ONLY_WANTED;

	flexible_array = NULL;
	size = type_size(type, &align);
	/* exactly one flexible array may be initialized, either the
	       toplevel array or the last member of the toplevel struct */

	if (size < 0) {
// error out except for top-level incomplete arrays
// (arrays of incomplete types are handled in array parsing)

		if (!(type->t & VT_ARRAY))
			tcc_error("initialization of incomplete type");
		/* If the base type itself was an array type of unspecified
		           size (like in 'typedef int arr[]; arr x = {1};') then
		           we will overwrite the unknown size by the real one for
		           this decl.  We need to unshare the ref symbol holding
		           that size.  */

		if (IS_BT_ARRAY(type->t))
			type->ref = sym_push(SYM_FIELD, &type->ref->type, 0, type->ref->c);
		p.flex_array_ref = type->ref;

	} else if (has_init && (type->t & VT_BTYPE) == VT_STRUCT) {
		Sym *field = type->ref->next;
		if (field) {
			while (field->next)
				field = field->next;
			if (field->type.t & VT_ARRAY && field->type.ref->c < 0) {
				flexible_array = field;
				p.flex_array_ref = field->type.ref;
				size = -1;
			}
		}
	}

	if (size < 0) {
		/* If unknown size, do a dry-run 1st pass */

		if (!has_init)
			goto err_size;
		if (has_init == 2) {
			/* only get strings */

			init_str = tok_str_alloc();
			while (tok == TOK_STR || tok == TOK_LSTR) {
				tok_str_add_tok(init_str);
				next();
			}
			tok_str_add(init_str, TOK_EOF);
		} else
			skip_or_save_block(&init_str);
		unget_tok(0);
		/* compute size */

		begin_macro(init_str, 1);
		next();
		decl_initializer(&p, type, 0, DIF_FIRST | DIF_SIZE_ONLY);
		/* prepare second initializer parsing */

		macro_ptr = init_str->str;
		next();
		/* if still unknown size, error */

		size = type_size(type, &align);
		if (size < 0)
err_size:
			tcc_error("unknown type size");
		/* If there's a flex member and it was used in the initializer
		           adjust size.  */

		if (flexible_array && flexible_array->type.ref->c > 0)
			size += flexible_array->type.ref->c
				* pointed_size(&flexible_array->type);
	}
	/* take into account specified alignment if bigger */

	if (ad->a.aligned) {
		int speca = 1 << (ad->a.aligned - 1);
		if (speca > align)
			align = speca;
	} else if (ad->a.packed) {
		align = 1;
	}

	if (!v && NODATA_WANTED)
		size = 0, align = 1;

	if ((r & VT_VALMASK) == VT_LOCAL) {
		sec = NULL;

		loc = (loc - size) & -align;
		addr = loc;
		p.local_offset = addr + size;

		if (v) {
			/* local variable */

			if (ad->asm_label) {
				int reg = asm_parse_regvar(ad->asm_label);
				if (reg >= 0)
					r = (r & ~VT_VALMASK) | reg;
			}

			sym = sym_push(v, type, r, addr);
			if (ad->cleanup_func) {
				Sym *cls = sym_push2(&all_cleanups,
						     SYM_FIELD | ++cur_scope->cl.n, 0, 0);
				cls->cleanup_sym = sym;
				cls->cleanup_func = ad->cleanup_func;
				cls->next = cur_scope->cl.s;
				cur_scope->cl.s = cls;
			}

			sym->a = ad->a;
		} else {
			/* push local reference */

			vset(type, r, addr);
		}
	} else {
		CType *tp = type;
		int is_const;

		while ((tp->t & (VT_BTYPE|VT_ARRAY)) == (VT_PTR|VT_ARRAY))
			tp = &tp->ref->type;
		is_const = tp->t & VT_CONSTANT;
		/* allocate symbol in corresponding section */

		sec = ad->section;
		if (!sec) {
			if (type->t & VT_TLS) {
				sec = find_section(tcc_state, has_init ? ".tdata" : ".tbss");
				sec->sh_flags = SHF_ALLOC | SHF_WRITE | SHF_TLS;
				sec->sh_type = has_init ? SHT_PROGBITS : SHT_NOBITS;
			} else if (is_const) {
				sec = rodata_section;
			} else if (has_init) {
				sec = data_section;
				/*if (g_debug & 4)
				                    tcc_warning("rw data: %s", get_tok_str(v, 0));*/

			} else if (tcc_state->nocommon)
				sec = bss_section;
		} else if (ad->new_section) {
			/* XXX: DWIM logic: set section flags according to first usage */

			if (!is_const) {
				sec->sh_flags |= SHF_WRITE;
				if (!has_init)
					sec->sh_type = SHT_NOBITS;
			}
		}

		if (sec) {
			addr = section_add(sec, size, align);
		} else {
			addr = align;/* SHN_COMMON is special, symbol value is align */

			sec = common_section;
		}

		if (v) {
			if (!sym) {
				sym = sym_push(v, type, r | VT_SYM, 0);
				patch_storage(sym, ad, NULL);
			}
			/* update symbol definition */

			put_extern_sym(sym, sec, addr, size);
		} else {
			/* push global reference */

			vpush_ref(type, sec, addr, size);
			sym = vtop->sym;
			vtop->r |= r;
		}
	}

	if (type->t & VT_VLA) {
		int a;

		if (has_init)
			tcc_error("variable length array cannot be initialized");

		if (NODATA_WANTED)
			goto no_alloc;
		/* save before-VLA stack pointer if needed */

		if (cur_scope->vla.num == 0) {
			if (cur_scope->prev && cur_scope->prev->vla.num) {
				cur_scope->vla.locorig = cur_scope->prev->vla.loc;
			} else {
				gen_vla_sp_save(loc -= PTR_SIZE);
				cur_scope->vla.locorig = loc;
			}
		}

		vpush_type_size(type, &a);
		gen_vla_alloc(type, a);
		/* on _WIN64, because of the function args scratch area, the
		   result of alloca differs from RSP and is returned in RAX.  */

		gen_vla_result(addr), addr = (loc -= PTR_SIZE);

		gen_vla_sp_save(addr);
		cur_scope->vla.loc = addr;
		cur_scope->vla.num++;
	} else if (has_init) {
		p.sec = sec;
		decl_initializer(&p, type, addr, DIF_FIRST);
		/* patch flexible array member size back to -1, */
		/* for possible subsequent similar declarations */

		if (flexible_array)
			flexible_array->type.ref->c = -1;
	}

no_alloc:
	/* restore parse state if needed */

	if (init_str) {
		end_macro();
		next();
	}

	nocode_wanted = saved_nocode_wanted;
}
/* generate vla code saved in post_type() */

static void func_vla_arg_code(Sym *arg)
{
	int align;
	TokenString *vla_array_tok = NULL;

	if (arg->type.ref)
		func_vla_arg_code(arg->type.ref);

	if ((arg->type.t & VT_VLA) && arg->type.ref->vla_array_str) {
		loc -= type_size(&int_type, &align);
		loc &= -align;
		arg->type.ref->c = loc;

		unget_tok(0);
		vla_array_tok = tok_str_alloc();
		vla_array_tok->str = arg->type.ref->vla_array_str;
		begin_macro(vla_array_tok, 1);
		next();
		gexpr();
		end_macro();
		next();
		vpush_type_size(&arg->type.ref->type, &align);
		gen_op('*');
		vset(&int_type, VT_LOCAL|VT_LVAL, arg->type.ref->c);
		vswap();
		vstore();
		vpop();
	}
}

static void func_vla_arg(Sym *sym)
{
	Sym *arg;

	for (arg = sym->type.ref->next; arg; arg = arg->next)
		if ((arg->type.t & VT_BTYPE) == VT_PTR && (arg->type.ref->type.t & VT_VLA))
			func_vla_arg_code(arg->type.ref);
}
/* set the local stack address for function parameter from gfunc_prolog() */

ST_FUNC Sym *gfunc_set_param(Sym *s, int c, int byref)
{
	s = sym_find(s->v);
	if (!s)/* unnamed parameters, not enabled */

		return NULL;
	s->c = c;
	if (byref)
		s->r = VT_LLOCAL | VT_LVAL;/* otherwise VT_LOCAL */

	return s;
}
/* push parameters (and their types), last first */

static void sym_push_params(Sym *ref)
{
	Sym *s = ref;
	while (s->next)
		s = s->next;
	while (s != ref) {
		if ((s->v & ~SYM_STRUCT) < SYM_FIRST_ANOM)
			sym_copy(s, &local_stack);
		s = s->prev;
	}
}
/* parse a function defined by symbol 'sym' and generate its code in
   'cur_text_section' */

static void gen_function(Sym *sym)
{
	struct scope f = { 0 };

	cur_scope = root_scope = &f;
	nocode_wanted = 0;

	ind = cur_text_section->data_offset;
	if (sym->a.aligned) {
		size_t newoff = section_add(cur_text_section, 0,
					    1 << (sym->a.aligned - 1));
		gen_fill_nops(newoff - ind);
	}

	funcname = get_tok_str(sym->v, NULL);
	func_ind = ind;
	func_vt = sym->type.ref->type;
	func_var = sym->type.ref->f.func_type == FUNC_ELLIPSIS;
	func_old = sym->type.ref->f.func_type == FUNC_OLD;
	/* NOTE: we patch the symbol size later */

	put_extern_sym(sym, cur_text_section, ind, 0);

	if (sym->type.ref->f.func_ctor)
		add_array (tcc_state, ".init_array", sym->c);
	if (sym->type.ref->f.func_dtor)
		add_array (tcc_state, ".fini_array", sym->c);
	/* put debug symbol */

	tcc_debug_funcstart(tcc_state, sym);
	/* push a dummy symbol to enable local sym storage */

	sym_push2(&local_stack, SYM_FIELD, 0, 0);
	/* push parameters */

	local_scope = 1;
	sym_push_params(sym->type.ref);

	local_scope = 0;
	rsym = 0;
	nb_temp_local_vars = 0;

	gfunc_prolog(sym);
	tcc_debug_prolog_epilog(tcc_state, 0);
	func_vla_arg(sym);
	block(0);
	gsym(rsym);
	nocode_wanted = 0;
	tcc_debug_end_scope(NULL, !func_var);
	tcc_debug_prolog_epilog(tcc_state, 1);
	gfunc_epilog();
	/* end of function */

	tcc_debug_funcend(tcc_state, ind - func_ind);
	/* patch symbol size */

	elfsym(sym)->st_size = ind - func_ind;
	cur_text_section->data_offset = ind;

	sym_pop(&local_stack, NULL, 0);
	label_pop(&global_label_stack, NULL, 0);
	sym_pop(&all_cleanups, NULL, 0);
	local_scope = 0;
	/* It's bette\r to crash than to generate wrong code */

	cur_text_section = NULL;
	funcname = "";/* for safety */

	func_vt.t = VT_VOID;/* for safety */

	func_var = 0;/* for safety */

	ind = 0;/* for safety */

	func_ind = -1;
	nocode_wanted = DATA_ONLY_WANTED;
	check_vstack();
	/* do this after funcend debug info */

	next();
}

static void gen_inline_functions(TCCState *s)
{
	Sym *sym;
	int inline_generated, i;
	struct InlineFunc *fn;

	tcc_open_bf(s, ":inline:", 0);
	/* iterate while inline function are referenced */

	do {
		inline_generated = 0;
		for (i = 0; i < s->nb_inline_fns; ++i) {
			fn = s->inline_fns[i];
			sym = fn->sym;
			if (sym && (sym->c || !(sym->type.t & VT_INLINE))) {
				/* the function was used or forced (and then not internal):
				                   generate its code and convert it to a normal function */

				fn->sym = NULL;
				tccpp_putfile(fn->filename);
				begin_macro(fn->func_str, 1);
				next();
				cur_text_section = text_section;
				gen_function(sym);
				end_macro();

				inline_generated = 1;
			}
		}
	} while (inline_generated);
	tcc_close();
}

static void free_inline_functions(TCCState *s)
{
	int i;
	/* free tokens of unused inline functions */

	for (i = 0; i < s->nb_inline_fns; ++i) {
		struct InlineFunc *fn = s->inline_fns[i];
		if (fn->sym)
			tok_str_free(fn->func_str);
	}
	dynarray_reset(&s->inline_fns, &s->nb_inline_fns);
}

static void do_Static_assert(void)
{
	int c;
	const char *msg;

	next();
	skip('(');
	c = expr_const();
	msg = "_Static_assert fail";
	if (tok == ',') {
		next();
		msg = parse_mult_str("string constant")->data;
	}
	skip(')');
	if (c == 0)
		tcc_error("%s", msg);
	skip(';');
}

static void pe_check_linkage(CType *type, AttributeDef *ad)
{
	if (!ad->a.dllimport && !ad->a.dllexport)
		return;
	if (type->t & VT_STATIC)
		tcc_error("cannot have dll linkage with static");
	if (type->t & VT_TYPEDEF) {
		const char *m = ad->a.dllimport ? "im" : "ex";
		tcc_warning("'dll%sport' attribute ignored for typedef", m);
		ad->a.dllimport = 0;
		ad->a.dllexport = 0;
	} else if (ad->a.dllimport) {
		if ((type->t & VT_BTYPE) == VT_FUNC)
			ad->a.dllimport = 0;
		else
			type->t |= VT_EXTERN;
	}
}
/* 'l' is VT_LOCAL or VT_CONST to define default storage type
   or VT_CMP if parsing old style parameter list
   or VT_JMP if parsing c99 for decl: for (int i = 0, ...) */
static int decl(int l)
{
	int v, has_init, r, oldint;
	CType type, btype;
	Sym *sym, *sa;
	AttributeDef ad, adbase;
	ElfSym *esym;

	while (1) {

		oldint = 0;
		if (!parse_btype(&btype, &adbase, l == VT_LOCAL)) {
			if (l == VT_JMP)
				return 0;
			/* skip redundant ';' if not in old parameter decl scope */

			if (tok == ';' && l != VT_CMP) {
				next();
				continue;
			}
			if (tok == TOK_STATIC_ASSERT) {
				do_Static_assert();
				continue;
			}
			if (l != VT_CONST)
				break;
			if (tok == TOK_ASM1 || tok == TOK_ASM2 || tok == TOK_ASM3) {
				/* global asm block */

				asm_global_instr();
				continue;
			}
			if (tok >= TOK_UIDENT) {
				/* special test for old K&R protos without explicit int
				                  type. Only accepted when defining global data */

				btype.t = VT_INT;
				oldint = 1;
			} else {
				if (tok != TOK_EOF)
					expect("declaration");
				break;
			}
		}

		if (tok == ';') {
			if ((btype.t & VT_BTYPE) == VT_STRUCT
			    && (btype.ref->v & ~SYM_STRUCT) < SYM_FIRST_ANOM)
				;/* struct decl with named tag */

			else if (IS_ENUM(btype.t))
				;/* enum decl */

			else
				tcc_warning("useless type defines no instances");
			if (l == VT_JMP)
				return 1;
			next();
			continue;
		}

		while (1) {/* iterate thru each declaration */

			type = btype;
			ad = adbase;
			type_decl(&type, &ad, &v, l == VT_CMP ? TYPE_DIRECT | TYPE_PARAM : TYPE_DIRECT);
			/*ptype("decl", &type, v);*/

			if ((type.t & VT_BTYPE) == VT_FUNC) {
				if ((type.t & VT_STATIC) && (l != VT_CONST))
					tcc_error("function without file scope cannot be static");
				/* if old style function prototype, we accept a
				                   declaration list */

				sym = type.ref;
				if (sym->f.func_type == FUNC_OLD && l == VT_CONST) {
					func_vt = type;
					++local_scope;
					decl(VT_CMP);
					--local_scope;
				}
				if ((type.t & (VT_EXTERN|VT_INLINE)) == (VT_EXTERN|VT_INLINE)) {
					/* always_inline functions must be handled as if they
					                       don't generate multiple global defs, even if extern
					                       inline, i.e. GNU inline semantics for those.  Rewrite
					                       them into static inline.  */

					if (tcc_state->gnu89_inline || sym->f.func_alwinl)
						type.t = (type.t & ~VT_EXTERN) | VT_STATIC;
					else
						type.t &= ~VT_INLINE;/* always compile otherwise */

				}

			} else if (oldint) {
				tcc_warning("type defaults to int");
			}

			if (gnu_ext && (tok == TOK_ASM1 || tok == TOK_ASM2 || tok == TOK_ASM3)) {
				ad.asm_label = asm_label_instr();
				/* parse one last attribute list, after asm label */

				parse_attribute(&ad);

			}

			pe_check_linkage(&type, &ad);

			if (tok == '{') {
				if (l != VT_CONST)
					tcc_error("cannot use local functions");
				if ((type.t & VT_BTYPE) != VT_FUNC)
					expect("function definition");
				/* apply post-declaraton attributes */

				merge_funcattr(&type.ref->f, &ad.f);
				/* put function symbol */

				type.t &= ~VT_EXTERN;
				sym = external_sym(v, &type, 0, &ad);
				/* reject abstract declarators in function definition
				                   make old-style float params double */

				for (sa = sym->type.ref; (sa = sa->next) != NULL;) {
					if (!(sa->v & ~SYM_FIELD))
						expect("identifier");
					if (sa->type.t == VT_FLOAT
					    && sym->type.ref->f.func_type == FUNC_OLD) {
						sa->type.t = VT_DOUBLE;
					}
				}
				/* static inline functions are just recorded as a kind
				                   of macro. Their code will be emitted at the end of
				                   the compilation unit only if they are used */

				if (sym->type.t & VT_INLINE) {
					struct InlineFunc *fn;
					fn = tcc_malloc(sizeof *fn + strlen(file->filename));
					strcpy(fn->filename, file->filename);
					fn->sym = sym;
					dynarray_add(&tcc_state->inline_fns,
						     &tcc_state->nb_inline_fns, fn);
					skip_or_save_block(&fn->func_str);
				} else {
					/* compute text section */

					cur_text_section = text_section;
					if (ad.section) {
						cur_text_section = ad.section;
						if (ad.new_section)
							ad.section->sh_flags = text_section->sh_flags;
					}
					gen_function(sym);
				}
				break;
			} else {
				has_init = 0;
				if (l == VT_CMP) {
					/* find parameter in function parameter list */

					for (sym = func_vt.ref->next; sym; sym = sym->next)
						if ((sym->v & ~SYM_FIELD) == v)
							goto found;
					tcc_error("declaration for parameter '%s' but no such parameter",
						  get_tok_str(v, NULL));
found:
					if (type.t & VT_STORAGE)/* 'register' is okay */

						tcc_error("storage class specified for '%s'",
							  get_tok_str(v, NULL));
					if (!(sym->type.t & VT_EXTERN))
						tcc_error("redefinition of parameter '%s'",
							  get_tok_str(v, NULL));
					convert_parameter_type(&type);
					sym->type = type;
				} else if (type.t & VT_TYPEDEF) {
					/* save typedefed type  */
					/* XXX: test storage specifiers ? */

					sym = sym_find(v);
					if (sym && sym->sym_scope == local_scope) {
						if (!is_compatible_types(&sym->type, &type)
						    || !(sym->type.t & VT_TYPEDEF))
							tcc_error("incompatible redefinition of '%s'",
								  get_tok_str(v, NULL));
						sym->type = type;
					} else {
						sym = sym_push(v, &type, 0, 0);
					}
					sym->a = ad.a;
					if ((type.t & VT_BTYPE) == VT_FUNC)
						merge_funcattr(&sym->type.ref->f, &ad.f);
					if (debug_modes)
						tcc_debug_typedef (tcc_state, sym);
				} else if ((type.t & VT_BTYPE) == VT_VOID
					   && !(type.t & VT_EXTERN)) {
					tcc_error("declaration of void object");
				} else {
					r = 0;
					if ((type.t & VT_BTYPE) == VT_FUNC) {
						/* external function definition */
						/* specific case for func_call attribute */

						merge_funcattr(&type.ref->f, &ad.f);
					} else if (!(type.t & VT_ARRAY)) {
						/* not lvalue if array */

						r |= VT_LVAL;
					}

					if (tok == '=')
						has_init = 1;

					if (((type.t & VT_EXTERN) && (!has_init || l != VT_CONST))
					    || (type.t & VT_BTYPE) == VT_FUNC
					    /* as with GCC, uninitialized global arrays with no size
					                               are considered extern: */

					    || ((type.t & VT_ARRAY) && !has_init
						&& l == VT_CONST && type.ref->c < 0)
					   ) {
						/* external variable or function */

						type.t |= VT_EXTERN;
						external_sym(v, &type, r, &ad);
					} else {
						if (l == VT_CONST || (type.t & VT_STATIC))
							r |= VT_CONST;
						else
							r |= VT_LOCAL;
						type.t &= ~VT_EXTERN;
						if (has_init)
							next();
						else if (l == VT_CONST)
							/* uninitialized global variables may be overridden */

							type.t |= VT_EXTERN;
						decl_initializer_alloc(&type, &ad, r, has_init, v, l);
					}

					if (ad.alias_target && l == VT_CONST) {
						/* Aliases need to be emitted when their target symbol
						                           is emitted, even if perhaps unreferenced.
						                           We only support the case where the base is already
						                           defined, otherwise we would need deferring to emit
						                           the aliases until the end of the compile unit.  */

						esym = elfsym(sym_find(ad.alias_target));
						if (!esym)
							tcc_error("unsupported forward __alias__ attribute");
						put_extern_sym2(sym_find(v), esym->st_shndx,
								esym->st_value, esym->st_size, 1);
					}
				}
				if (tok != ',') {
					if (l == VT_JMP)
						return has_init ? v : 1;
					skip(';');
					break;
				}
				next();
			}
		}
	}
	return 0;
}
/* ------------------------------------------------------------------------- */

#undef gjmp_addr
#undef gjmp
/* ------------------------------------------------------------------------- */
/* ==================== tccdbg.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) s1->sym
/* stab debug support */

static const struct {
	int type;
	int size;
	int encoding;
	const char *name;
} default_debug[] = {
	{ VT_INT, 4, DW_ATE_signed, "int:t1=r1;-2147483648;2147483647;" },
	{ VT_BYTE, 1, DW_ATE_signed_char, "char:t2=r2;0;127;" },
	{ VT_LONG | VT_INT, 4, DW_ATE_signed, "long int:t3=r3;-2147483648;2147483647;" },

	{ VT_INT | VT_UNSIGNED, 4, DW_ATE_unsigned, "unsigned int:t4=r4;0;037777777777;" },
	{ VT_LONG | VT_INT | VT_UNSIGNED, 4, DW_ATE_unsigned, "long unsigned int:t5=r5;0;037777777777;" },

	{ VT_QLONG, 16, DW_ATE_signed, "__int128:t6=r6;0;-1;" },
	{ VT_QLONG | VT_UNSIGNED, 16, DW_ATE_unsigned, "__int128 unsigned:t7=r7;0;-1;" },
	{ VT_LLONG, 8, DW_ATE_signed, "long long int:t8=r8;-9223372036854775808;9223372036854775807;" },
	{ VT_LLONG | VT_UNSIGNED, 8, DW_ATE_unsigned, "long long unsigned int:t9=r9;0;01777777777777777777777;" },
	{ VT_SHORT, 2, DW_ATE_signed, "short int:t10=r10;-32768;32767;" },
	{ VT_SHORT | VT_UNSIGNED, 2, DW_ATE_unsigned, "short unsigned int:t11=r11;0;65535;" },
	{ VT_BYTE | VT_DEFSIGN, 1, DW_ATE_signed_char, "signed char:t12=r12;-128;127;" },
	{ VT_BYTE | VT_DEFSIGN | VT_UNSIGNED, 1, DW_ATE_unsigned_char, "unsigned char:t13=r13;0;255;" },
	{ VT_FLOAT, 4, DW_ATE_float, "float:t14=r1;4;0;" },
	{ VT_DOUBLE, 8, DW_ATE_float, "double:t15=r1;8;0;" },

	{ VT_DOUBLE | VT_LONG, 8, DW_ATE_float, "long double:t16=r1;8;0;" },

	{ -1, -1, -1, "_Float32:t17=r1;4;0;" },
	{ -1, -1, -1, "_Float64:t18=r1;8;0;" },
	{ -1, -1, -1, "_Float128:t19=r1;16;0;" },
	{ -1, -1, -1, "_Float32x:t20=r1;8;0;" },
	{ -1, -1, -1, "_Float64x:t21=r1;16;0;" },
	{ -1, -1, -1, "_Decimal32:t22=r1;4;0;" },
	{ -1, -1, -1, "_Decimal64:t23=r1;8;0;" },
	{ -1, -1, -1, "_Decimal128:t24=r1;16;0;" },
	/* if default char is unsigned */

	{ VT_BYTE | VT_UNSIGNED, 1, DW_ATE_unsigned_char, "unsigned char:t25=r25;0;255;" },
	/* boolean type */

	{ VT_BOOL, 1, DW_ATE_boolean, "bool:t26=r26;0;255;" },
	{ VT_VOID, 1, DW_ATE_unsigned_char, "void:t27=27" },

};

#define N_DEFAULT_DEBUG (sizeof (default_debug) / sizeof (default_debug[0]))
/* dwarf debug */

#define DWARF_LINE_BASE -5
#define DWARF_LINE_RANGE 14
#define DWARF_OPCODE_BASE 13

#define DWARF_MIN_INSTR_LEN 1

#define DWARF_ABBREV_COMPILE_UNIT 1
#define DWARF_ABBREV_BASE_TYPE 2
#define DWARF_ABBREV_VARIABLE_EXTERNAL 3
#define DWARF_ABBREV_VARIABLE_STATIC 4
#define DWARF_ABBREV_VARIABLE_LOCAL 5
#define DWARF_ABBREV_FORMAL_PARAMETER 6
#define DWARF_ABBREV_POINTER 7
#define DWARF_ABBREV_ARRAY_TYPE 8
#define DWARF_ABBREV_SUBRANGE_TYPE 9
#define DWARF_ABBREV_TYPEDEF 10
#define DWARF_ABBREV_ENUMERATOR_SIGNED 11
#define DWARF_ABBREV_ENUMERATOR_UNSIGNED 12
#define DWARF_ABBREV_ENUMERATION_TYPE 13
#define DWARF_ABBREV_MEMBER 14
#define DWARF_ABBREV_MEMBER_BF 15
#define DWARF_ABBREV_STRUCTURE_TYPE 16
#define DWARF_ABBREV_STRUCTURE_EMPTY_TYPE 17
#define DWARF_ABBREV_UNION_TYPE 18
#define DWARF_ABBREV_UNION_EMPTY_TYPE 19
#define DWARF_ABBREV_SUBPROGRAM_EXTERNAL 20
#define DWARF_ABBREV_SUBPROGRAM_STATIC 21
#define DWARF_ABBREV_LEXICAL_BLOCK 22
#define DWARF_ABBREV_LEXICAL_EMPTY_BLOCK 23
#define DWARF_ABBREV_SUBROUTINE_TYPE 24
#define DWARF_ABBREV_SUBROUTINE_EMPTY_TYPE 25
#define DWARF_ABBREV_FORMAL_PARAMETER2 26
/* all entries should have been generated with dwarf_uleb128 except
   has_children. All values are currently below 128 so this currently
   works.  */

static const unsigned char dwarf_abbrev_init[] = {
	DWARF_ABBREV_COMPILE_UNIT, DW_TAG_compile_unit, 1,
	DW_AT_producer, DW_FORM_strp,
	DW_AT_language, DW_FORM_data1,
	DW_AT_name, DW_FORM_line_strp,
	DW_AT_comp_dir, DW_FORM_line_strp,
	DW_AT_low_pc, DW_FORM_addr,

	DW_AT_high_pc, DW_FORM_data8,

	DW_AT_stmt_list, DW_FORM_sec_offset,
	0, 0,
	DWARF_ABBREV_BASE_TYPE, DW_TAG_base_type, 0,
	DW_AT_byte_size, DW_FORM_udata,
	DW_AT_encoding, DW_FORM_data1,
	DW_AT_name, DW_FORM_strp,
	0, 0,
	DWARF_ABBREV_VARIABLE_EXTERNAL, DW_TAG_variable, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_external, DW_FORM_flag,
	DW_AT_location, DW_FORM_exprloc,
	0, 0,
	DWARF_ABBREV_VARIABLE_STATIC, DW_TAG_variable, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_location, DW_FORM_exprloc,
	0, 0,
	DWARF_ABBREV_VARIABLE_LOCAL, DW_TAG_variable, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_location, DW_FORM_exprloc,
	0, 0,
	DWARF_ABBREV_FORMAL_PARAMETER, DW_TAG_formal_parameter, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_location, DW_FORM_exprloc,
	0, 0,
	DWARF_ABBREV_POINTER, DW_TAG_pointer_type, 0,
	DW_AT_byte_size, DW_FORM_data1,
	DW_AT_type, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_ARRAY_TYPE, DW_TAG_array_type, 1,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_sibling, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_SUBRANGE_TYPE, DW_TAG_subrange_type, 0,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_upper_bound, DW_FORM_udata,
	0, 0,
	DWARF_ABBREV_TYPEDEF, DW_TAG_typedef, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_type, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_ENUMERATOR_SIGNED, DW_TAG_enumerator, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_const_value, DW_FORM_sdata,
	0, 0,
	DWARF_ABBREV_ENUMERATOR_UNSIGNED, DW_TAG_enumerator, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_const_value, DW_FORM_udata,
	0, 0,
	DWARF_ABBREV_ENUMERATION_TYPE, DW_TAG_enumeration_type, 1,
	DW_AT_name, DW_FORM_strp,
	DW_AT_encoding, DW_FORM_data1,
	DW_AT_byte_size, DW_FORM_data1,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_sibling, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_MEMBER, DW_TAG_member, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_data_member_location, DW_FORM_udata,
	0, 0,
	DWARF_ABBREV_MEMBER_BF, DW_TAG_member, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_bit_size, DW_FORM_udata,
	DW_AT_data_bit_offset, DW_FORM_udata,
	0, 0,
	DWARF_ABBREV_STRUCTURE_TYPE, DW_TAG_structure_type, 1,
	DW_AT_name, DW_FORM_strp,
	DW_AT_byte_size, DW_FORM_udata,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_sibling, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_STRUCTURE_EMPTY_TYPE, DW_TAG_structure_type, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_byte_size, DW_FORM_udata,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	0, 0,
	DWARF_ABBREV_UNION_TYPE, DW_TAG_union_type, 1,
	DW_AT_name, DW_FORM_strp,
	DW_AT_byte_size, DW_FORM_udata,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_sibling, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_UNION_EMPTY_TYPE, DW_TAG_union_type, 0,
	DW_AT_name, DW_FORM_strp,
	DW_AT_byte_size, DW_FORM_udata,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	0, 0,
	DWARF_ABBREV_SUBPROGRAM_EXTERNAL, DW_TAG_subprogram, 1,
	DW_AT_external, DW_FORM_flag,
	DW_AT_name, DW_FORM_strp,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_low_pc, DW_FORM_addr,

	DW_AT_high_pc, DW_FORM_data8,

	DW_AT_sibling, DW_FORM_ref4,
	DW_AT_frame_base, DW_FORM_exprloc,
	0, 0,
	DWARF_ABBREV_SUBPROGRAM_STATIC, DW_TAG_subprogram, 1,
	DW_AT_name, DW_FORM_strp,
	DW_AT_decl_file, DW_FORM_udata,
	DW_AT_decl_line, DW_FORM_udata,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_low_pc, DW_FORM_addr,

	DW_AT_high_pc, DW_FORM_data8,

	DW_AT_sibling, DW_FORM_ref4,
	DW_AT_frame_base, DW_FORM_exprloc,
	0, 0,
	DWARF_ABBREV_LEXICAL_BLOCK, DW_TAG_lexical_block, 1,
	DW_AT_low_pc, DW_FORM_addr,

	DW_AT_high_pc, DW_FORM_data8,

	0, 0,
	DWARF_ABBREV_LEXICAL_EMPTY_BLOCK, DW_TAG_lexical_block, 0,
	DW_AT_low_pc, DW_FORM_addr,

	DW_AT_high_pc, DW_FORM_data8,

	0, 0,
	DWARF_ABBREV_SUBROUTINE_TYPE, DW_TAG_subroutine_type, 1,
	DW_AT_type, DW_FORM_ref4,
	DW_AT_sibling, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_SUBROUTINE_EMPTY_TYPE, DW_TAG_subroutine_type, 0,
	DW_AT_type, DW_FORM_ref4,
	0, 0,
	DWARF_ABBREV_FORMAL_PARAMETER2, DW_TAG_formal_parameter, 0,
	DW_AT_type, DW_FORM_ref4,
	0, 0,
	0
};

static const unsigned char dwarf_line_opcodes[] = {
	0,1,1,1,1,0,0,0,1,0,0,1
};
/* ------------------------------------------------------------------------- */
/* debug state */

#define N_STR_HASH (251)

struct dwarf_str_hash {
	int len;
	unsigned long data_offset;
	struct dwarf_str_hash *next;
};

struct _tccdbg {

	int last_line_num, new_file;
	int section_sym;

	int debug_next_type;

	struct _debug_hash {
		int debug_type;
		Sym *type;
	} *debug_hash_global, *debug_hash_local;
	/* store forward structure/unions types */

	struct _debug_forw_hash {
		Sym *type;
		int n_debug_type;
		int *debug_type;
	} *debug_forw_hash_global, *debug_forw_hash_local;

	int n_debug_hash_global;
	int n_debug_hash_local;
	int n_debug_forw_hash_global;
	int n_debug_forw_hash_local;

	struct _debug_info {
		int start;
		int end;
		int last_debug_hash;
		int last_debug_forw_hash;
		int n_sym;
		struct debug_sym {
			int type;
			unsigned long value;
			char *str;
			Section *sec;
			int sym_index;
			int info;
			int file;
			int line;
		} *sym;
		struct _debug_info *child, *next, *last, *parent;
	} *debug_info, *debug_info_root;

	struct {
		int info;
		int abbrev;
		int line;
		int str;
		int line_str;
	} dwarf_sym;

	struct {
		int start;
		int dir_size;
		char **dir_table;
		int filename_size;
		struct dwarf_filename_struct {
			int dir_entry;
			char *name;
		} *filename_table;
		int line_size;
		int line_max_size;
		unsigned char *line_data;
		int cur_file;
		int last_file;
		int last_pc;
		int last_line;
	} dwarf_line;

	struct {
		int start;
		Sym *func;
		int line;
		int base_type_used[N_DEFAULT_DEBUG];
	} dwarf_info;

	struct dwarf_str_hash *dwarf_str[N_STR_HASH];
	struct dwarf_str_hash *dwarf_line_str[N_STR_HASH];
	/* test coverage */

	struct {
		unsigned long offset;
		unsigned long last_file_name;
		unsigned long last_func_name;
		int ind;
		int line;
	} tcov_data;
};

#define last_line_num s1->dState->last_line_num
#define new_file s1->dState->new_file
#define section_sym s1->dState->section_sym
#define debug_next_type s1->dState->debug_next_type
#define debug_hash_global s1->dState->debug_hash_global
#define debug_hash_local s1->dState->debug_hash_local
#define debug_forw_hash_global s1->dState->debug_forw_hash_global
#define debug_forw_hash_local s1->dState->debug_forw_hash_local
#define n_debug_hash_global s1->dState->n_debug_hash_global
#define n_debug_hash_local s1->dState->n_debug_hash_local
#define n_debug_forw_hash_global s1->dState->n_debug_forw_hash_global
#define n_debug_forw_hash_local s1->dState->n_debug_forw_hash_local
#define debug_info s1->dState->debug_info
#define debug_info_root s1->dState->debug_info_root
#define dwarf_sym s1->dState->dwarf_sym
#define dwarf_line s1->dState->dwarf_line
#define dwarf_info s1->dState->dwarf_info
#define dwarf_str s1->dState->dwarf_str
#define dwarf_line_str s1->dState->dwarf_line_str
#define tcov_data s1->dState->tcov_data
/* ------------------------------------------------------------------------- */

static int put_stabs(TCCState *s1, const char *str, int type, int other,
		     int desc, unsigned long value);

ST_FUNC void tcc_debug_new(TCCState *s1)
{
	int shf = 0;
	if (!s1->dState)
		s1->dState = tcc_mallocz(sizeof *s1->dState);

	if (s1->dwarf) {
		int i;
		/* The sections below are just to make reloctions with
			   R_DATA_32DW work correctly. See tccelf.c */

		static const char *const debug[] = {
			".debug_macro",
			".debug_loc",
			".debug_ranges",
			".debug_loclists",
			".debug_rnglists",
			".debug_str_offsets",
			".debug_addr"
		};

		s1->dwlo = s1->nb_sections;
		dwarf_info_section =
			new_section(s1, ".debug_info", SHT_PROGBITS, shf);
		dwarf_abbrev_section =
			new_section(s1, ".debug_abbrev", SHT_PROGBITS, shf);
		dwarf_line_section =
			new_section(s1, ".debug_line", SHT_PROGBITS, shf);
		dwarf_aranges_section =
			new_section(s1, ".debug_aranges", SHT_PROGBITS, shf);
		for (i = 0; i < sizeof(debug)/sizeof(debug[0]); i++)
			new_section(s1, debug[i], SHT_PROGBITS, 0)->sh_addralign = 1;
		shf |= SHF_MERGE | SHF_STRINGS;
		dwarf_str_section =
			new_section(s1, ".debug_str", SHT_PROGBITS, shf);
		dwarf_str_section->sh_entsize = 1;
		dwarf_info_section->sh_addralign =
			dwarf_abbrev_section->sh_addralign =
				dwarf_line_section->sh_addralign =
					dwarf_aranges_section->sh_addralign =
						dwarf_str_section->sh_addralign = 1;
		if (s1->dwarf >= 5) {
			dwarf_line_str_section =
				new_section(s1, ".debug_line_str", SHT_PROGBITS, shf);
			dwarf_line_str_section->sh_entsize = 1;
			dwarf_line_str_section->sh_addralign = 1;
		}
		s1->dwhi = s1->nb_sections;
	} else {
		stab_section = new_section(s1, ".stab", SHT_PROGBITS, shf);
		stab_section->sh_entsize = sizeof(Stab_Sym);
		stab_section->sh_addralign = sizeof ((Stab_Sym *)0)->n_value;
		stab_section->link = new_section(s1, ".stabstr", SHT_STRTAB, shf);
		/* put first entry */

		put_stabs(s1, "", 0, 0, 0, 0);
	}
}
/* put stab debug information */

static int put_stabs(TCCState *s1, const char *str, int type, int other,
		     int desc,
		     unsigned long value)
{
	Stab_Sym *sym;

	unsigned offset;
	if (type == N_SLINE
	    && (offset = stab_section->data_offset)
	    && (sym = (Stab_Sym *)(stab_section->data + offset) - 1)
	    && sym->n_type == type
	    && sym->n_value == value) {
		/* just update line_number in previous entry */

		sym->n_desc = desc;
		return 0;
	}

	sym = section_ptr_add(stab_section, sizeof(Stab_Sym));
	if (str) {
		sym->n_strx = put_elf_str(stab_section->link, str);
	} else {
		sym->n_strx = 0;
	}
	sym->n_type = type;
	sym->n_other = other;
	sym->n_desc = desc;
	sym->n_value = value;
	return 1;
}

static void put_stabs_r(TCCState *s1, const char *str, int type, int other,
			int desc,
			unsigned long value, Section *sec, int sym_index)
{
	if (put_stabs(s1, str, type, other, desc, value))
		put_elf_reloc(symtab_section, stab_section,
			      stab_section->data_offset - 4,
			      sizeof ((Stab_Sym *)0)->n_value == PTR_SIZE ? R_DATA_PTR : R_DATA_32,
			      sym_index);
}

static void put_stabn(TCCState *s1, int type, int other, int desc, int value)
{
	put_stabs(s1, NULL, type, other, desc, value);
}
/* ------------------------------------------------------------------------- */

#define dwarf_data1(s,data) (*(uint8_t*)section_ptr_add((s), 1) = (data))

#define dwarf_data2(s,data) write16le(section_ptr_add((s), 2), (data))

#define dwarf_data4(s,data) write32le(section_ptr_add((s), 4), (data))

#define dwarf_data8(s,data) write64le(section_ptr_add((s), 8), (data))

static int dwarf_get_section_sym(Section *s)
{
	TCCState *s1 = s->s1;
	return put_elf_sym(symtab_section, 0, 0,
			   ELFW(ST_INFO)(STB_LOCAL, STT_SECTION), 0,
			   s->sh_num, NULL);
}

static void dwarf_reloc(Section *s, int sym, int rel)
{
	TCCState *s1 = s->s1;
	put_elf_reloca(symtab_section, s, s->data_offset, rel, sym, 0);
}

static void free_str(struct dwarf_str_hash **str)
{
	int i;

	for (i = 0; i < N_STR_HASH; i++) {
		while (str[i]) {
			struct dwarf_str_hash *next = str[i]->next;

			tcc_free(str[i]);
			str[i] = next;
		}
	}
}

static unsigned str_hash(const char *s)
{
	unsigned h = 5381;

	while (*s)
		h += (*s++ & 0xffu) + h * 31;
	return h;
}

static void dwarf_string(Section *s, Section *dw, int sym, const char *str)
{
	TCCState *s1 = s->s1;
	int len, hash = str_hash(str) % N_STR_HASH;
	char *ptr;
	struct dwarf_str_hash *new_hash;
	struct dwarf_str_hash **dw_hash =
			dw == dwarf_str_section ? dwarf_str : dwarf_line_str;

	len = strlen(str) + 1;
	new_hash = dw_hash[hash];
	while (new_hash) {
		if (new_hash->len == len &&
		    !memcmp(str, dw->data + new_hash->data_offset, len))
			break;
		new_hash = new_hash->next;
	}
	if (new_hash == NULL) {
		unsigned long offset = dw->data_offset;

		new_hash = dw_hash[hash];
		while (new_hash) {
			unsigned long n = new_hash->data_offset + new_hash->len - len;

			if (new_hash->len > len &&
			    !memcmp(str, dw->data + n, len)) {
				offset = n;
				break;
			}
			new_hash = new_hash->next;
		}
		if (new_hash == NULL) {
			ptr = section_ptr_add(dw, len);
			memmove(ptr, str, len);
		}
		new_hash = (struct dwarf_str_hash *)
			   tcc_malloc(sizeof(struct dwarf_str_hash));
		new_hash->len = len;
		new_hash->data_offset = offset;
		new_hash->next = dw_hash[hash];
		dw_hash[hash] = new_hash;
	}
	put_elf_reloca(symtab_section, s, s->data_offset, R_DATA_32DW, sym,
		       PTR_SIZE == 4 ? 0 : new_hash->data_offset);
	dwarf_data4(s, PTR_SIZE == 4 ? new_hash->data_offset : 0);
}

static void dwarf_strp(Section *s, const char *str)
{
	TCCState *s1 = s->s1;
	dwarf_string(s, dwarf_str_section, dwarf_sym.str, str);
}

static void dwarf_line_strp(Section *s, const char *str)
{
	TCCState *s1 = s->s1;
	dwarf_string(s, dwarf_line_str_section, dwarf_sym.line_str, str);
}

static void dwarf_line_op(TCCState *s1, unsigned char op)
{
	if (dwarf_line.line_size >= dwarf_line.line_max_size) {
		dwarf_line.line_max_size += 1024;
		dwarf_line.line_data =
			(unsigned char *)tcc_realloc(dwarf_line.line_data,
						     dwarf_line.line_max_size);
	}
	dwarf_line.line_data[dwarf_line.line_size++] = op;
}

static void dwarf_file(TCCState *s1)
{
	int i, j;
	char *filename;
	int index_offset = s1->dwarf < 5;

	if (!strcmp(file->filename, "<command line>")) {
		dwarf_line.cur_file = 1;
		return;
	}
	filename = strrchr(file->filename, '/');
	if (filename == NULL) {
		for (i = 1; i < dwarf_line.filename_size; i++)
			if (dwarf_line.filename_table[i].dir_entry == 0 &&
			    strcmp(dwarf_line.filename_table[i].name,
				   file->filename) == 0) {
				dwarf_line.cur_file = i + index_offset;
				return;
			}
		i = -index_offset;
		filename = file->filename;
	} else {
		char *undo = filename;
		char *dir = file->filename;

		*filename++ = '\0';
		for (i = 0; i < dwarf_line.dir_size; i++)
			if (strcmp(dwarf_line.dir_table[i], dir) == 0) {
				for (j = 1; j < dwarf_line.filename_size; j++)
					if (dwarf_line.filename_table[j].dir_entry - index_offset
					    == i &&
					    strcmp(dwarf_line.filename_table[j].name,
						   filename) == 0) {
						*undo = '/';
						dwarf_line.cur_file = j + index_offset;
						return;
					}
				break;
			}
		if (i == dwarf_line.dir_size) {
			dwarf_line.dir_size++;
			dwarf_line.dir_table =
				(char **) tcc_realloc(dwarf_line.dir_table,
						      dwarf_line.dir_size *
						      sizeof (char *));
			dwarf_line.dir_table[i] = tcc_strdup(dir);
		}
		*undo = '/';
	}
	dwarf_line.filename_table =
		(struct dwarf_filename_struct *)
		tcc_realloc(dwarf_line.filename_table,
			    (dwarf_line.filename_size + 1) *
			    sizeof (struct dwarf_filename_struct));
	dwarf_line.filename_table[dwarf_line.filename_size].dir_entry =
		i + index_offset;
	dwarf_line.filename_table[dwarf_line.filename_size].name =
		tcc_strdup(filename);
	dwarf_line.cur_file = dwarf_line.filename_size++ + index_offset;
	return;
}
static int dwarf_sleb128_size (long long value)
{
	int size = 0;
	long long end = value >> 63;
	unsigned char last = end & 0x40;
	unsigned char byte;

	do {
		byte = value & 0x7f;
		value >>= 7;
		size++;
	} while (value != end || (byte & 0x40) != last);
	return size;
}

static void dwarf_uleb128 (Section *s, unsigned long long value)
{
	do {
		unsigned char byte = value & 0x7f;

		value >>= 7;
		dwarf_data1(s, byte | (value ? 0x80 : 0));
	} while (value != 0);
}

static void dwarf_sleb128 (Section *s, long long value)
{
	int more;
	long long end = value >> 63;
	unsigned char last = end & 0x40;

	do {
		unsigned char byte = value & 0x7f;

		value >>= 7;
		more = value != end || (byte & 0x40) != last;
		dwarf_data1(s, byte | (0x80 * more));
	} while (more);
}

static void dwarf_uleb128_op (TCCState *s1, unsigned long long value)
{
	do {
		unsigned char byte = value & 0x7f;

		value >>= 7;
		dwarf_line_op(s1, byte | (value ? 0x80 : 0));
	} while (value != 0);
}

static void dwarf_sleb128_op (TCCState *s1, long long value)
{
	int more;
	long long end = value >> 63;
	unsigned char last = end & 0x40;

	do {
		unsigned char byte = value & 0x7f;

		value >>= 7;
		more = value != end || (byte & 0x40) != last;
		dwarf_line_op(s1, byte | (0x80 * more));
	} while (more);
}
/* start of translation unit info */

ST_FUNC void tcc_debug_start(TCCState *s1)
{
	int i;
	char buf[512];
	char *filename;
	/* we might currently #include the <command-line> */

	filename = file->prev ? file->prev->filename : file->filename;
	/* an elf symbol of type STT_FILE must be put so that STB_LOCAL
	       symbols can be safely used */

	put_elf_sym(symtab_section, 0, 0,
		    ELFW(ST_INFO)(STB_LOCAL, STT_FILE), 0,
		    SHN_ABS, filename);

	if (s1->do_debug) {
		/* put a "mapping symbol" '$a' for llvm-objdump etc. tools needed
		           to make them disassemble again when crt1.o had a '$d' before */

		put_elf_sym(symtab_section, text_section->data_offset, 0,
			    ELFW(ST_INFO)(STB_LOCAL, STT_NOTYPE), 0,
			    text_section->sh_num, "$a");

		new_file = last_line_num = 0;
		debug_next_type = N_DEFAULT_DEBUG;
		debug_hash_global = NULL;
		debug_hash_local = NULL;
		debug_forw_hash_global = NULL;
		debug_forw_hash_local = NULL;
		n_debug_hash_global = 0;
		n_debug_hash_local = 0;
		n_debug_forw_hash_global = 0;
		n_debug_forw_hash_local = 0;

		getcwd(buf, sizeof(buf));

		normalize_slashes(buf);

		if (s1->dwarf) {
			int start_abbrev;
			unsigned char *ptr;
			char *undo;
			/* dwarf_abbrev */

			start_abbrev = dwarf_abbrev_section->data_offset;
			ptr = section_ptr_add(dwarf_abbrev_section, sizeof(dwarf_abbrev_init));
			memcpy(ptr, dwarf_abbrev_init, sizeof(dwarf_abbrev_init));

			if (s1->dwarf < 5) {
				while (*ptr) {
					ptr += 3;
					while (*ptr) {
						if (ptr[1] == DW_FORM_line_strp)
							ptr[1] = DW_FORM_strp;
						if (s1->dwarf < 4) {
							/* These are compatable for DW_TAG_compile_unit
										       DW_AT_stmt_list. */

							if (ptr[1] == DW_FORM_sec_offset)
								ptr[1] = DW_FORM_data4;
							/* This code uses only size < 0x80 so these are
										       compatible. */

							if (ptr[1] == DW_FORM_exprloc)
								ptr[1] = DW_FORM_block1;
						}
						ptr += 2;
					}
					ptr += 2;
				}
			}

			dwarf_sym.info = dwarf_get_section_sym(dwarf_info_section);
			dwarf_sym.abbrev = dwarf_get_section_sym(dwarf_abbrev_section);
			dwarf_sym.line = dwarf_get_section_sym(dwarf_line_section);
			dwarf_sym.str = dwarf_get_section_sym(dwarf_str_section);
			if (tcc_state->dwarf >= 5)
				dwarf_sym.line_str = dwarf_get_section_sym(dwarf_line_str_section);
			else {
				dwarf_line_str_section = dwarf_str_section;
				dwarf_sym.line_str = dwarf_sym.str;
			}
			section_sym = dwarf_get_section_sym(text_section);
			/* dwarf_info */

			dwarf_info.start = dwarf_info_section->data_offset;
			dwarf_data4(dwarf_info_section, 0);// size

			dwarf_data2(dwarf_info_section, s1->dwarf);// version

			if (s1->dwarf >= 5) {
				dwarf_data1(dwarf_info_section, DW_UT_compile);// unit type

				dwarf_data1(dwarf_info_section, PTR_SIZE);
				dwarf_reloc(dwarf_info_section, dwarf_sym.abbrev, R_DATA_32DW);
				dwarf_data4(dwarf_info_section, start_abbrev);
			} else {
				dwarf_reloc(dwarf_info_section, dwarf_sym.abbrev, R_DATA_32DW);
				dwarf_data4(dwarf_info_section, start_abbrev);
				dwarf_data1(dwarf_info_section, PTR_SIZE);
			}

			dwarf_data1(dwarf_info_section, DWARF_ABBREV_COMPILE_UNIT);
			dwarf_strp(dwarf_info_section, "tcc " TCC_VERSION);
			dwarf_data1(dwarf_info_section,
				    s1->cversion == 201112 ? DW_LANG_C11 : DW_LANG_C99);
			dwarf_line_strp(dwarf_info_section, filename);
			dwarf_line_strp(dwarf_info_section, buf);
			dwarf_reloc(dwarf_info_section, section_sym, R_DATA_PTR);

			dwarf_data8(dwarf_info_section, ind);// low pc

			dwarf_data8(dwarf_info_section, 0);// high pc

			dwarf_reloc(dwarf_info_section, dwarf_sym.line, R_DATA_32DW);
			dwarf_data4(dwarf_info_section, dwarf_line_section->data_offset);// stmt_list

			/* dwarf_line */

			dwarf_line.start = dwarf_line_section->data_offset;
			dwarf_data4(dwarf_line_section, 0);// length

			dwarf_data2(dwarf_line_section, s1->dwarf);// version

			if (s1->dwarf >= 5) {
				dwarf_data1(dwarf_line_section, PTR_SIZE);// address size

				dwarf_data1(dwarf_line_section, 0);// segment selector

			}
			dwarf_data4(dwarf_line_section, 0);// prologue Length

			dwarf_data1(dwarf_line_section, DWARF_MIN_INSTR_LEN);
			if (s1->dwarf >= 4)
				dwarf_data1(dwarf_line_section, 1);// maximum ops per instruction

			dwarf_data1(dwarf_line_section, 1);// Initial value of 'is_stmt'

			dwarf_data1(dwarf_line_section, DWARF_LINE_BASE);
			dwarf_data1(dwarf_line_section, DWARF_LINE_RANGE);
			dwarf_data1(dwarf_line_section, DWARF_OPCODE_BASE);
			ptr = section_ptr_add(dwarf_line_section, sizeof(dwarf_line_opcodes));
			memcpy(ptr, dwarf_line_opcodes, sizeof(dwarf_line_opcodes));
			undo = strrchr(filename, '/');
			if (undo)
				*undo = 0;
			dwarf_line.dir_size = 1 + (undo != NULL);
			dwarf_line.dir_table = (char **) tcc_malloc(sizeof (char *) *
				dwarf_line.dir_size);
			dwarf_line.dir_table[0] = tcc_strdup(buf);
			if (undo)
				dwarf_line.dir_table[1] = tcc_strdup(filename);
			dwarf_line.filename_size = 2;
			dwarf_line.filename_table =
				(struct dwarf_filename_struct *)
				tcc_malloc(2*sizeof (struct dwarf_filename_struct));
			dwarf_line.filename_table[0].dir_entry = 0;
			if (undo) {
				dwarf_line.filename_table[0].name = tcc_strdup(undo + 1);
				dwarf_line.filename_table[1].dir_entry = 1;
				dwarf_line.filename_table[1].name = tcc_strdup(undo + 1);
				*undo = '/';
			} else {
				dwarf_line.filename_table[0].name = tcc_strdup(filename);
				dwarf_line.filename_table[1].dir_entry = 0;
				dwarf_line.filename_table[1].name = tcc_strdup(filename);
			}
			dwarf_line.line_size = dwarf_line.line_max_size = 0;
			dwarf_line.line_data = NULL;
			dwarf_line.cur_file = 1;
			dwarf_line.last_file = 0;
			dwarf_line.last_pc = 0;
			dwarf_line.last_line = 1;
			dwarf_line_op(s1, 0);// extended

			dwarf_uleb128_op(s1, 1 + PTR_SIZE);// extended size

			dwarf_line_op(s1, DW_LNE_set_address);
			for (i = 0; i < PTR_SIZE; i++)
				dwarf_line_op(s1, 0);
			memset(&dwarf_info.base_type_used, 0, sizeof(dwarf_info.base_type_used));
		} else {
			/* file info: full path + filename */

			pstrcat(buf, sizeof(buf), "/");
			section_sym = put_elf_sym(symtab_section, 0, 0,
						  ELFW(ST_INFO)(STB_LOCAL, STT_SECTION), 0,
						  text_section->sh_num, NULL);
			put_stabs_r(s1, buf, N_SO, 0, 0,
				    text_section->data_offset, text_section, section_sym);
			put_stabs_r(s1, filename, N_SO, 0, 0,
				    text_section->data_offset, text_section, section_sym);
			for (i = 0; i < N_DEFAULT_DEBUG; i++)
				put_stabs(s1, default_debug[i].name, N_LSYM, 0, 0, 0);
		}
		/* we're currently 'including' the <command line> */

		tcc_debug_bincl(s1);
	}
}

static void fix_debug_forw_hash(TCCState *s1, int global, int start);
/* put end of translation unit info */

ST_FUNC void tcc_debug_end(TCCState *s1)
{

	if (!s1->do_debug || debug_next_type == 0)
		return;

	if (debug_info_root)
		tcc_debug_funcend(s1, 0);/* free stuff in case of errors */

	if (s1->dwarf) {
		int i;
		int start_aranges;
		unsigned char *ptr;
		int text_size = text_section->data_offset;
		/* dwarf_info */

		fix_debug_forw_hash(s1, 0, 0);
		fix_debug_forw_hash(s1, 1, 0);
		dwarf_data1(dwarf_info_section, 0);
		ptr = dwarf_info_section->data + dwarf_info.start;
		write32le(ptr, dwarf_info_section->data_offset - dwarf_info.start - 4);
		write32le(ptr + 25 + (s1->dwarf >= 5) + PTR_SIZE, text_size);
		/* dwarf_aranges */

		start_aranges = dwarf_aranges_section->data_offset;
		dwarf_data4(dwarf_aranges_section, 0);// size

		dwarf_data2(dwarf_aranges_section, 2);// version

		dwarf_reloc(dwarf_aranges_section, dwarf_sym.info, R_DATA_32DW);
		dwarf_data4(dwarf_aranges_section, 0);// dwarf_info

		dwarf_data1(dwarf_aranges_section, 8);// address size

		dwarf_data1(dwarf_aranges_section, 0);// segment selector size

		dwarf_data4(dwarf_aranges_section, 0);// padding

		dwarf_reloc(dwarf_aranges_section, section_sym, R_DATA_PTR);

		dwarf_data8(dwarf_aranges_section, 0);// Begin

		dwarf_data8(dwarf_aranges_section, text_size);// End

		dwarf_data8(dwarf_aranges_section, 0);// End list

		dwarf_data8(dwarf_aranges_section, 0);// End list

		ptr = dwarf_aranges_section->data + start_aranges;
		write32le(ptr, dwarf_aranges_section->data_offset - start_aranges - 4);
		/* dwarf_line */

		if (s1->dwarf >= 5) {
			dwarf_data1(dwarf_line_section, 1);/* col */

			dwarf_uleb128(dwarf_line_section, DW_LNCT_path);
			dwarf_uleb128(dwarf_line_section, DW_FORM_line_strp);
			dwarf_uleb128(dwarf_line_section, dwarf_line.dir_size);
			for (i = 0; i < dwarf_line.dir_size; i++)
				dwarf_line_strp(dwarf_line_section, dwarf_line.dir_table[i]);
			dwarf_data1(dwarf_line_section, 2);/* col */

			dwarf_uleb128(dwarf_line_section, DW_LNCT_path);
			dwarf_uleb128(dwarf_line_section, DW_FORM_line_strp);
			dwarf_uleb128(dwarf_line_section, DW_LNCT_directory_index);
			dwarf_uleb128(dwarf_line_section, DW_FORM_udata);
			dwarf_uleb128(dwarf_line_section, dwarf_line.filename_size);
			for (i = 0; i < dwarf_line.filename_size; i++) {
				dwarf_line_strp(dwarf_line_section,
						dwarf_line.filename_table[i].name);
				dwarf_uleb128(dwarf_line_section,
					      dwarf_line.filename_table[i].dir_entry);
			}
		} else {
			int len;

			for (i = 0; i < dwarf_line.dir_size; i++) {
				len = strlen(dwarf_line.dir_table[i]) + 1;
				ptr = section_ptr_add(dwarf_line_section, len);
				memmove(ptr, dwarf_line.dir_table[i], len);
			}
			dwarf_data1(dwarf_line_section, 0);/* end dir */

			for (i = 0; i < dwarf_line.filename_size; i++) {
				len = strlen(dwarf_line.filename_table[i].name) + 1;
				ptr = section_ptr_add(dwarf_line_section, len);
				memmove(ptr, dwarf_line.filename_table[i].name, len);
				dwarf_uleb128(dwarf_line_section,
					      dwarf_line.filename_table[i].dir_entry);
				dwarf_uleb128(dwarf_line_section, 0);/* time */

				dwarf_uleb128(dwarf_line_section, 0);/* size */

			}
			dwarf_data1(dwarf_line_section, 0);/* end file */

		}
		for (i = 0; i < dwarf_line.dir_size; i++)
			tcc_free(dwarf_line.dir_table[i]);
		tcc_free(dwarf_line.dir_table);
		for (i = 0; i < dwarf_line.filename_size; i++)
			tcc_free(dwarf_line.filename_table[i].name);
		tcc_free(dwarf_line.filename_table);

		dwarf_line_op(s1, 0);// extended

		dwarf_uleb128_op(s1, 1);// extended size

		dwarf_line_op(s1, DW_LNE_end_sequence);
		i = (s1->dwarf >= 5) * 2;
		write32le(&dwarf_line_section->data[dwarf_line.start + 6 + i],
			  dwarf_line_section->data_offset - dwarf_line.start - (10 + i));
		section_ptr_add(dwarf_line_section, 3);
		dwarf_reloc(dwarf_line_section, section_sym, R_DATA_PTR);
		ptr = section_ptr_add(dwarf_line_section, dwarf_line.line_size - 3);
		memmove(ptr - 3, dwarf_line.line_data, dwarf_line.line_size);
		tcc_free(dwarf_line.line_data);
		write32le(dwarf_line_section->data + dwarf_line.start,
			  dwarf_line_section->data_offset - dwarf_line.start - 4);
	} else {
		put_stabs_r(s1, NULL, N_SO, 0, 0,
			    text_section->data_offset, text_section, section_sym);
	}
	free_str (dwarf_str);
	free_str (dwarf_line_str);
	tcc_free (debug_forw_hash_global);
	tcc_free (debug_forw_hash_local);
	tcc_free(debug_hash_global);
	tcc_free(debug_hash_local);
	debug_next_type = 0;
}

static BufferedFile *put_new_file(TCCState *s1)
{
	BufferedFile *f = file;
	/* use upper file if from inline ":asm:" */

	if (f->filename[0] == ':')
		f = f->prev;
	if (f && new_file) {
		new_file = last_line_num = 0;
		if (s1->dwarf)
			dwarf_file(s1);
		else
			put_stabs_r(s1, f->filename, N_SOL, 0, 0, ind, text_section, section_sym);
	}
	return f;
}
/* put alternative filename */

ST_FUNC void tcc_debug_newfile(TCCState *s1)
{
	if (!s1->do_debug)
		return;
	if (s1->dwarf)
		dwarf_file(s1);
	new_file = 1;
}
/* begin of #include */

ST_FUNC void tcc_debug_bincl(TCCState *s1)
{
	if (!s1->do_debug)
		return;
	if (s1->dwarf)
		dwarf_file(s1);
	else
		put_stabs(s1, file->filename, N_BINCL, 0, 0, 0);
	new_file = 1;
}
/* end of #include */

ST_FUNC void tcc_debug_eincl(TCCState *s1)
{
	if (!s1->do_debug)
		return;
	if (s1->dwarf)
		dwarf_file(s1);
	else
		put_stabn(s1, N_EINCL, 0, 0, 0);
	new_file = 1;
}
/* generate line number info */

ST_FUNC void tcc_debug_line(TCCState *s1)
{
	BufferedFile *f;

	if (!s1->do_debug)
		return;
	if (cur_text_section != text_section || nocode_wanted)
		return;
	f = put_new_file(s1);
	if (!f)
		return;
	if (last_line_num == f->line_num)
		return;
	last_line_num = f->line_num;

	if (s1->dwarf) {
		int len_pc = (ind - dwarf_line.last_pc) / DWARF_MIN_INSTR_LEN;
		int len_line = f->line_num - dwarf_line.last_line;
		int n = len_pc * DWARF_LINE_RANGE + len_line + DWARF_OPCODE_BASE -
			DWARF_LINE_BASE;

		if (dwarf_line.cur_file != dwarf_line.last_file) {
			dwarf_line.last_file = dwarf_line.cur_file;
			dwarf_line_op(s1, DW_LNS_set_file);
			dwarf_uleb128_op(s1, dwarf_line.cur_file);
		}
		if (len_pc &&
		    len_line >= DWARF_LINE_BASE
		    && len_line <= (DWARF_OPCODE_BASE + DWARF_LINE_BASE) &&
		    n >= DWARF_OPCODE_BASE && n <= 255)
			dwarf_line_op(s1, n);
		else {
			if (len_pc) {
				n = len_pc * DWARF_LINE_RANGE + 0 + DWARF_OPCODE_BASE - DWARF_LINE_BASE;
				if (n >= DWARF_OPCODE_BASE && n <= 255)
					dwarf_line_op(s1, n);
				else {
					dwarf_line_op(s1, DW_LNS_advance_pc);
					dwarf_uleb128_op(s1, len_pc);
				}
			}
			if (len_line) {
				n = 0 * DWARF_LINE_RANGE + len_line + DWARF_OPCODE_BASE - DWARF_LINE_BASE;
				if (len_line >= DWARF_LINE_BASE
				    && len_line <= (DWARF_OPCODE_BASE + DWARF_LINE_BASE) &&
				    n >= DWARF_OPCODE_BASE && n <= 255)
					dwarf_line_op(s1, n);
				else {
					dwarf_line_op(s1, DW_LNS_advance_line);
					dwarf_sleb128_op(s1, len_line);
					/* advance by nothing */

					dwarf_line_op(s1, DWARF_OPCODE_BASE - DWARF_LINE_BASE);
				}
			}
		}
		dwarf_line.last_pc = ind;
		dwarf_line.last_line = f->line_num;
	} else {
		if (func_ind != -1) {
			put_stabn(s1, N_SLINE, 0, f->line_num, ind - func_ind);
		} else {
			/* from tcc_assemble */

			put_stabs_r(s1, NULL, N_SLINE, 0, f->line_num, ind, text_section, section_sym);
		}
	}
}

static void tcc_debug_stabs (TCCState *s1, const char *str, int type,
			     unsigned long value,
			     Section *sec, int sym_index, int info)
{
	struct debug_sym *s;

	if (debug_info) {
		debug_info->sym =
			(struct debug_sym *)tcc_realloc (debug_info->sym,
				sizeof(struct debug_sym) *
				(debug_info->n_sym + 1));
		s = debug_info->sym + debug_info->n_sym++;
		s->type = type;
		s->value = value;
		s->str = tcc_strdup(str);
		s->sec = sec;
		s->sym_index = sym_index;
		s->info = info;
		s->file = dwarf_line.cur_file;
		s->line = file->line_num;
	} else if (sec)
		put_stabs_r (s1, str, type, 0, 0, value, sec, sym_index);
	else
		put_stabs (s1, str, type, 0, 0, value);
}

static void fix_debug_forw_hash(TCCState *s1, int global, int start)
{
	if (s1->dwarf) {
		int i, j, n_hash = global
				   ? n_debug_forw_hash_global : n_debug_forw_hash_local;
		struct _debug_forw_hash *hash = global
							? debug_forw_hash_global : debug_forw_hash_local;

		for (i = start; i < n_hash; i++) {
			Sym *t = hash[i].type;
			int pos = dwarf_info_section->data_offset;

			dwarf_data1(dwarf_info_section,
				    IS_UNION (t->type.t) ? DWARF_ABBREV_UNION_EMPTY_TYPE
				    : DWARF_ABBREV_STRUCTURE_EMPTY_TYPE);
			dwarf_strp(dwarf_info_section,
				   (t->v & ~SYM_STRUCT) >= SYM_FIRST_ANOM
				   ? "" : get_tok_str(t->v, NULL));
			dwarf_uleb128(dwarf_info_section, 0);
			dwarf_uleb128(dwarf_info_section, dwarf_line.cur_file);
			dwarf_uleb128(dwarf_info_section, file->line_num);
			for (j = 0; j < hash[i].n_debug_type; j++)
				write32le(dwarf_info_section->data +
					  hash[i].debug_type[j],
					  pos - dwarf_info.start);
			tcc_free (hash[i].debug_type);
		}
	}
}

ST_FUNC void tcc_debug_stabn(TCCState *s1, int type, int value)
{
	if (!s1->do_debug)
		return;
	if (type == N_LBRAC) {
		struct _debug_info *info =
			(struct _debug_info *) tcc_mallocz(sizeof (*info));

		info->start = value;
		info->last_debug_hash = n_debug_hash_local;
		info->last_debug_forw_hash = n_debug_forw_hash_local;
		info->parent = debug_info;
		if (debug_info) {
			if (debug_info->child) {
				if (debug_info->child->last)
					debug_info->child->last->next = info;
				else
					debug_info->child->next = info;
				debug_info->child->last = info;
			} else
				debug_info->child = info;
		} else
			debug_info_root = info;
		debug_info = info;
	} else {
		fix_debug_forw_hash(s1, 0, debug_info->last_debug_forw_hash);
		n_debug_hash_local = debug_info->last_debug_hash;
		n_debug_forw_hash_local = debug_info->last_debug_forw_hash;
		debug_info->end = value;
		debug_info = debug_info->parent;
	}
}

static int check_global(Sym *t)
{
	Sym *g = local_stack;

	while (g) {
		if (t == g)
			return 0;
		g = g->prev;
	}
	return 1;
}

static int tcc_debug_find(TCCState *s1, Sym *t, int dwarf)
{
	int i, g = check_global(t);
	int n_hash, *n_forw_hash;
	struct _debug_hash *hash;
	struct _debug_forw_hash **forw_hash;

	if ((t->type.t & VT_BTYPE) == VT_STRUCT && t->c == -1) {
		forw_hash = g ? &debug_forw_hash_global : &debug_forw_hash_local;
		n_forw_hash = g ? &n_debug_forw_hash_global : &n_debug_forw_hash_local;
		for (i = 0; i < *n_forw_hash; i++)
			if (t == (*forw_hash)[i].type)
				return 0;
		*forw_hash = (struct _debug_forw_hash *)
			     tcc_realloc (*forw_hash,
					  (*n_forw_hash + 1) * sizeof(**forw_hash));
		(*forw_hash)[*n_forw_hash].n_debug_type = 0;
		(*forw_hash)[*n_forw_hash].debug_type = NULL;
		(*forw_hash)[(*n_forw_hash)++].type = t;
		return 0;
	}
	hash = g ? debug_hash_global : debug_hash_local;
	n_hash = g ? n_debug_hash_global : n_debug_hash_local;
	for (i = 0; i < n_hash; i++)
		if (t == hash[i].type)
			return hash[i].debug_type;
	return -1;
}

static int tcc_get_dwarf_info(TCCState *s1, Sym *s);

static void tcc_debug_check_forw(TCCState *s1, Sym *t, int debug_type)
{
	if ((t->type.t & VT_BTYPE) == VT_STRUCT && t->type.ref->c == -1) {
		int i, j, g = check_global(t);
		int n_forw_hash;
		struct _debug_forw_hash **forw_hash;

		for (i = g; i <= 1; i++) {
			forw_hash = i ? &debug_forw_hash_global : &debug_forw_hash_local;
			n_forw_hash = i ? n_debug_forw_hash_global : n_debug_forw_hash_local;
			for (j = 0; j < n_forw_hash; j++)
				if (t->type.ref == (*forw_hash)[j].type) {
					(*forw_hash)[j].debug_type =
						tcc_realloc((*forw_hash)[j].debug_type,
							    ((*forw_hash)[j].n_debug_type + 1) * sizeof(int));
					(*forw_hash)[j].debug_type[(*forw_hash)[j].n_debug_type++] =
						debug_type;
					return;
				}
		}
	}
}

static void stabs_struct_complete(TCCState *s1, CType *t);

ST_FUNC void tcc_debug_fix_forw(TCCState *s1, CType *t)
{
	if (!(s1->do_debug & 2))
		return;
	if (0 == s1->dwarf) {
		stabs_struct_complete(s1, t);
		return;
	}
	if ((t->t & VT_BTYPE) == VT_STRUCT && t->ref->c != -1) {
		int i, j, debug_type, g = check_global(t->ref);
		int *n_forw_hash;
		struct _debug_forw_hash **forw_hash;
		forw_hash = g ? &debug_forw_hash_global : &debug_forw_hash_local;
		n_forw_hash = g ? &n_debug_forw_hash_global : &n_debug_forw_hash_local;
		for (i = 0; i < *n_forw_hash; i++)
			if (t->ref == (*forw_hash)[i].type) {
				if (s1->dwarf) {
					Sym sym = {0};
					sym .type = *t ;

					debug_type = tcc_get_dwarf_info(s1, &sym);
					for (j = 0; j < (*forw_hash)[i].n_debug_type; j++)
						write32le(dwarf_info_section->data +
							  (*forw_hash)[i].debug_type[j],
							  debug_type - dwarf_info.start);
					tcc_free((*forw_hash)[i].debug_type);
				}
				(*n_forw_hash)--;
				for (; i < *n_forw_hash; i++)
					(*forw_hash)[i] = (*forw_hash)[i + 1];
			}
	}
}

static int tcc_debug_add(TCCState *s1, Sym *t, int dwarf)
{
	int offset = dwarf ? dwarf_info_section->data_offset : ++debug_next_type;
	int *n_hash, g = check_global(t);
	struct _debug_hash **hash;

	hash = g ? &debug_hash_global : &debug_hash_local;
	n_hash = g ? &n_debug_hash_global : &n_debug_hash_local;
	*hash = (struct _debug_hash *)
		tcc_realloc (*hash,
			     (*n_hash + 1) * sizeof(**hash));
	(*hash)[*n_hash].debug_type = offset;
	(*hash)[(*n_hash)++].type = t;
	return offset;
}

static int STRUCT_NODEBUG(Sym *s)
{
	return
		(s->a.nodebug ||
		 ((s->v & ~SYM_FIELD) >= SYM_FIRST_ANOM &&
		  ((s->type.t & VT_BTYPE) == VT_BYTE ||
		   (s->type.t & VT_BTYPE) == VT_BOOL ||
		   (s->type.t & VT_BTYPE) == VT_SHORT ||
		   (s->type.t & VT_BTYPE) == VT_INT ||
		   (s->type.t & VT_BTYPE) == VT_LLONG)));
}

static int stabs_struct_find(TCCState *s1, Sym *t, int *p_id)
{
	/* A struct/enum has a ref to its type but that type has no ref.
	       So we can (ab)use it for some info.  Here:
	         s->c : stabs type id
	         s->r : already defined in stabs */

	Sym *s = t->type.ref;
	/*
	    if (s && s->v != (SYM_FIELD|0x00DEBBED)) {
	        tcc_error_noabort("tccdbg: internal error: %s", get_tok_str(t->v, 0));
	        if (p_id)
	            *p_id = 0;
	        return 0;
	    }
	*/
	if (NULL == p_id)
		return s && !s->r && t->c >= 0;
	if (NULL == s) {
		/* just use global_stack always */

		s = sym_push2(&global_stack, SYM_FIELD|0x00DEBBED, 0, ++debug_next_type);
		t->type.ref = s;
	}
	*p_id = s->c;
	if (s->r || t->c < 0)/* already defined or still incomplete */

		return 0;
	s->r = 1;
	return 1;
}

static int remove_type_info(int type)
{
	type &= ~(VT_STORAGE | VT_CONSTANT | VT_VOLATILE | VT_VLA);
	if ((type & VT_BTYPE) != VT_BYTE)
		type &= ~VT_DEFSIGN;
	if (!(type & VT_BITFIELD) && (type & VT_STRUCT_MASK) > VT_ENUM)
		type &= ~VT_STRUCT_MASK;
	return type;
}

static void tcc_get_debug_info(TCCState *s1, Sym *s, CString *result)
{
	int type;
	int n = 0;
	int debug_type = -1;
	Sym *t = s;
	CString str;

	for (;;) {
		type = remove_type_info (t->type.t);
		if (type == VT_PTR || type == (VT_PTR | VT_ARRAY))
			n++, t = t->type.ref;
		else
			break;
	}
	if ((type & VT_BTYPE) == VT_STRUCT) {
		t = t->type.ref;
		if (stabs_struct_find(s1, t, &debug_type)) {
			cstr_new (&str);
			cstr_printf (&str, "%s:T%d=%c%d",
				     (t->v & ~SYM_STRUCT) >= SYM_FIRST_ANOM
				     ? "" : get_tok_str(t->v, NULL),
				     debug_type,
				     IS_UNION (t->type.t) ? 'u' : 's',
				     t->c);

			while (t->next) {
				int pos, size, align;
				t = t->next;
				if (STRUCT_NODEBUG(t))
					continue;
				cstr_printf (&str, "%s:",
					     (t->v & ~SYM_FIELD) >= SYM_FIRST_ANOM
					     ? "" : get_tok_str(t->v, NULL)
					    );
				tcc_get_debug_info (s1, t, &str);
				if (t->type.t & VT_BITFIELD) {
					pos = t->c * 8 + BIT_POS(t->type.t);
					size = BIT_SIZE(t->type.t);
				} else {
					pos = t->c * 8;
					size = type_size(&t->type, &align) * 8;
				}
				cstr_printf (&str, ",%d,%d;", pos, size);
			}
			cstr_printf (&str, ";");
			tcc_debug_stabs(s1, str.data, N_LSYM, 0, NULL, 0, 0);
			cstr_free (&str);
		}
	} else if (IS_ENUM(type)) {
		Sym *e = t = t->type.ref;
		if (stabs_struct_find(s1, t, &debug_type)) {
			cstr_new (&str);
			cstr_printf (&str, "%s:T%d=e",
				     (t->v & ~SYM_STRUCT) >= SYM_FIRST_ANOM
				     ? "" : get_tok_str(t->v, NULL),
				     debug_type);
			while (t->next) {
				t = t->next;
				cstr_printf (&str, "%s:",
					     (t->v & ~SYM_FIELD) >= SYM_FIRST_ANOM
					     ? "" : get_tok_str(t->v, NULL));
				cstr_printf (&str, e->type.t & VT_UNSIGNED ? "%u," : "%d,",
					     (int)t->enum_val);
			}
			cstr_printf (&str, ";");
			tcc_debug_stabs(s1, str.data, N_LSYM, 0, NULL, 0, 0);
			cstr_free (&str);
		}
	} else if ((type & VT_BTYPE) != VT_FUNC) {
		type &= ~VT_STRUCT_MASK;
		for (debug_type = 1; debug_type <= N_DEFAULT_DEBUG; debug_type++)
			if (default_debug[debug_type - 1].type == type)
				break;
		if (debug_type > N_DEFAULT_DEBUG)
			return;
	}

	if (NULL == result)/* from stabs_struct_complete() */

		return;

	if (n > 0)
		cstr_printf (result, "%d=", ++debug_next_type);
	t = s;
	for (;;) {
		type = remove_type_info (t->type.t);
		if (type == VT_PTR)
			cstr_printf (result, "%d=*", ++debug_next_type);
		else if (type == (VT_PTR | VT_ARRAY))
			cstr_printf (result, "%d=ar1;0;%d;",
				     ++debug_next_type, t->type.ref->c - 1);
		else if (type == VT_FUNC) {
			cstr_printf (result, "%d=f", ++debug_next_type);
			tcc_get_debug_info (s1, t->type.ref, result);
			return;
		} else
			break;
		t = t->type.ref;
	}
	cstr_printf (result, "%d", debug_type);
}

static void stabs_struct_complete(TCCState *s1, CType *t)
{
	if (stabs_struct_find(s1, t->ref, NULL)) {
		Sym s = {0};
		s.type = *t;
		tcc_get_debug_info(s1, &s, NULL);
	}
}

static int tcc_get_dwarf_info(TCCState *s1, Sym *s)
{
	int type;
	int debug_type = -1;
	Sym *e, *t = s;
	int i;
	int last_pos = -1;
	int retval;

	if (new_file)
		put_new_file(s1);
	for (;;) {
		type = remove_type_info (t->type.t);
		if (type == VT_PTR || type == (VT_PTR | VT_ARRAY))
			t = t->type.ref;
		else
			break;
	}
	if ((type & VT_BTYPE) == VT_STRUCT) {
		t = t->type.ref;
		debug_type = tcc_debug_find(s1, t, 1);
		if (debug_type == -1) {
			int pos_sib = 0, i, *pos_type;

			debug_type = tcc_debug_add(s1, t, 1);
			e = t;
			i = 0;
			while (e->next) {
				e = e->next;
				if (STRUCT_NODEBUG(e))
					continue;
				i++;
			}
			pos_type = (int *) tcc_malloc(i * sizeof(int));
			dwarf_data1(dwarf_info_section,
				    IS_UNION (t->type.t)
				    ? t->next ? DWARF_ABBREV_UNION_TYPE
				    : DWARF_ABBREV_UNION_EMPTY_TYPE
				    : t->next ? DWARF_ABBREV_STRUCTURE_TYPE
				    : DWARF_ABBREV_STRUCTURE_EMPTY_TYPE);
			dwarf_strp(dwarf_info_section,
				   (t->v & ~SYM_STRUCT) >= SYM_FIRST_ANOM
				   ? "" : get_tok_str(t->v, NULL));
			dwarf_uleb128(dwarf_info_section, t->c);
			dwarf_uleb128(dwarf_info_section, dwarf_line.cur_file);
			dwarf_uleb128(dwarf_info_section, file->line_num);
			if (t->next) {
				pos_sib = dwarf_info_section->data_offset;
				dwarf_data4(dwarf_info_section, 0);
			}
			e = t;
			i = 0;
			while (e->next) {
				e = e->next;
				if (STRUCT_NODEBUG(e))
					continue;
				dwarf_data1(dwarf_info_section,
					    e->type.t & VT_BITFIELD ? DWARF_ABBREV_MEMBER_BF
					    : DWARF_ABBREV_MEMBER);
				dwarf_strp(dwarf_info_section,
					   get_tok_str(e->v, NULL));
				dwarf_uleb128(dwarf_info_section, dwarf_line.cur_file);
				dwarf_uleb128(dwarf_info_section, file->line_num);
				pos_type[i++] = dwarf_info_section->data_offset;
				dwarf_data4(dwarf_info_section, 0);
				if (e->type.t & VT_BITFIELD) {
					int pos = e->c * 8 + BIT_POS(e->type.t);
					int size = BIT_SIZE(e->type.t);

					dwarf_uleb128(dwarf_info_section, size);
					dwarf_uleb128(dwarf_info_section, pos);
				} else
					dwarf_uleb128(dwarf_info_section, e->c);
			}
			if (t->next) {
				dwarf_data1(dwarf_info_section, 0);
				write32le(dwarf_info_section->data + pos_sib,
					  dwarf_info_section->data_offset - dwarf_info.start);
			}
			e = t;
			i = 0;
			while (e->next) {
				e = e->next;
				if (STRUCT_NODEBUG(e))
					continue;
				type = tcc_get_dwarf_info(s1, e);
				tcc_debug_check_forw(s1, e, pos_type[i]);
				write32le(dwarf_info_section->data + pos_type[i++],
					  type - dwarf_info.start);
			}
			tcc_free(pos_type);
		}
	} else if (IS_ENUM(type)) {
		t = t->type.ref;
		debug_type = tcc_debug_find(s1, t, 1);
		if (debug_type == -1) {
			int pos_sib, pos_type;
			Sym sym = {0};
			sym.type.t = VT_INT | (type & VT_UNSIGNED);

			pos_type = tcc_get_dwarf_info(s1, &sym);
			debug_type = tcc_debug_add(s1, t, 1);
			dwarf_data1(dwarf_info_section, DWARF_ABBREV_ENUMERATION_TYPE);
			dwarf_strp(dwarf_info_section,
				   (t->v & ~SYM_STRUCT) >= SYM_FIRST_ANOM
				   ? "" : get_tok_str(t->v, NULL));
			dwarf_data1(dwarf_info_section,
				    type & VT_UNSIGNED ? DW_ATE_unsigned : DW_ATE_signed );
			dwarf_data1(dwarf_info_section, 4);
			dwarf_data4(dwarf_info_section, pos_type - dwarf_info.start);
			dwarf_uleb128(dwarf_info_section, dwarf_line.cur_file);
			dwarf_uleb128(dwarf_info_section, file->line_num);
			pos_sib = dwarf_info_section->data_offset;
			dwarf_data4(dwarf_info_section, 0);
			e = t;
			while (e->next) {
				e = e->next;
				dwarf_data1(dwarf_info_section,
					    type & VT_UNSIGNED ? DWARF_ABBREV_ENUMERATOR_UNSIGNED
					    : DWARF_ABBREV_ENUMERATOR_SIGNED);
				dwarf_strp(dwarf_info_section,
					   (e->v & ~SYM_FIELD) >= SYM_FIRST_ANOM
					   ? "" : get_tok_str(e->v, NULL));
				if (type & VT_UNSIGNED)
					dwarf_uleb128(dwarf_info_section, e->enum_val);
				else
					dwarf_sleb128(dwarf_info_section, e->enum_val);
			}
			dwarf_data1(dwarf_info_section, 0);
			write32le(dwarf_info_section->data + pos_sib,
				  dwarf_info_section->data_offset - dwarf_info.start);
		}
	} else if ((type & VT_BTYPE) != VT_FUNC) {
		type &= ~VT_STRUCT_MASK;
		for (i = 1; i <= N_DEFAULT_DEBUG; i++)
			if (default_debug[i - 1].type == type)
				break;
		if (i > N_DEFAULT_DEBUG)
			return 0;
		debug_type = dwarf_info.base_type_used[i - 1];
		if (debug_type == 0) {
			char name[100];

			debug_type = dwarf_info_section->data_offset;
			dwarf_data1(dwarf_info_section, DWARF_ABBREV_BASE_TYPE);
			dwarf_uleb128(dwarf_info_section, default_debug[i - 1].size);
			dwarf_data1(dwarf_info_section, default_debug[i - 1].encoding);
			pstrcpy(name, sizeof name, default_debug[i - 1].name);
			*strchr(name, ':') = 0;
			dwarf_strp(dwarf_info_section, name);
			dwarf_info.base_type_used[i - 1] = debug_type;
		}
	}
	retval = debug_type;
	e = NULL;
	t = s;
	for (;;) {
		type = remove_type_info (t->type.t);
		if (type == VT_PTR) {
			i = dwarf_info_section->data_offset;
			if (retval == debug_type)
				retval = i;
			dwarf_data1(dwarf_info_section, DWARF_ABBREV_POINTER);
			dwarf_data1(dwarf_info_section, PTR_SIZE);
			if (last_pos != -1) {
				tcc_debug_check_forw(s1, e, last_pos);
				write32le(dwarf_info_section->data + last_pos,
					  i - dwarf_info.start);
			}
			last_pos = dwarf_info_section->data_offset;
			e = t->type.ref;
			dwarf_data4(dwarf_info_section, 0);
		} else if (type == (VT_PTR | VT_ARRAY)) {
			int sib_pos, sub_type;
			Sym sym = {0};
			sym.type.t = VT_LONG | VT_INT | VT_UNSIGNED;

			sub_type = tcc_get_dwarf_info(s1, &sym);
			i = dwarf_info_section->data_offset;
			if (retval == debug_type)
				retval = i;
			dwarf_data1(dwarf_info_section, DWARF_ABBREV_ARRAY_TYPE);
			if (last_pos != -1) {
				tcc_debug_check_forw(s1, e, last_pos);
				write32le(dwarf_info_section->data + last_pos,
					  i - dwarf_info.start);
			}
			last_pos = dwarf_info_section->data_offset;
			e = t->type.ref;
			dwarf_data4(dwarf_info_section, 0);
			sib_pos = dwarf_info_section->data_offset;
			dwarf_data4(dwarf_info_section, 0);
			for (;;) {
				dwarf_data1(dwarf_info_section, DWARF_ABBREV_SUBRANGE_TYPE);
				dwarf_data4(dwarf_info_section, sub_type - dwarf_info.start);
				dwarf_uleb128(dwarf_info_section, t->type.ref->c - 1);
				s = t->type.ref;
				type = s->type.t & ~(VT_STORAGE | VT_CONSTANT | VT_VOLATILE);
				if (type != (VT_PTR | VT_ARRAY))
					break;
				t = s;
			}
			dwarf_data1(dwarf_info_section, 0);
			write32le(dwarf_info_section->data + sib_pos,
				  dwarf_info_section->data_offset - dwarf_info.start);
		} else if (type == VT_FUNC) {
			int sib_pos = 0, *pos_type;
			Sym *f;

			i = dwarf_info_section->data_offset;
			debug_type = tcc_get_dwarf_info(s1, t->type.ref);
			if (retval == debug_type)
				retval = i;
			dwarf_data1(dwarf_info_section,
				    t->type.ref->next ? DWARF_ABBREV_SUBROUTINE_TYPE
				    : DWARF_ABBREV_SUBROUTINE_EMPTY_TYPE);
			if (last_pos != -1) {
				tcc_debug_check_forw(s1, e, last_pos);
				write32le(dwarf_info_section->data + last_pos,
					  i - dwarf_info.start);
			}
			last_pos = dwarf_info_section->data_offset;
			e = t->type.ref;
			dwarf_data4(dwarf_info_section, 0);
			if (t->type.ref->next) {
				sib_pos = dwarf_info_section->data_offset;
				dwarf_data4(dwarf_info_section, 0);
			}
			f = t->type.ref;
			i = 0;
			while (f->next) {
				f = f->next;
				i++;
			}
			pos_type = (int *) tcc_malloc(i * sizeof(int));
			f = t->type.ref;
			i = 0;
			while (f->next) {
				f = f->next;
				dwarf_data1(dwarf_info_section, DWARF_ABBREV_FORMAL_PARAMETER2);
				pos_type[i++] = dwarf_info_section->data_offset;
				dwarf_data4(dwarf_info_section, 0);
			}
			if (t->type.ref->next) {
				dwarf_data1(dwarf_info_section, 0);
				write32le(dwarf_info_section->data + sib_pos,
					  dwarf_info_section->data_offset - dwarf_info.start);
			}
			f = t->type.ref;
			i = 0;
			while (f->next) {
				f = f->next;
				type = tcc_get_dwarf_info(s1, f);
				tcc_debug_check_forw(s1, f, pos_type[i]);
				write32le(dwarf_info_section->data + pos_type[i++],
					  type - dwarf_info.start);
			}
			tcc_free(pos_type);
		} else {
			if (last_pos != -1) {
				tcc_debug_check_forw(s1, e, last_pos);
				write32le(dwarf_info_section->data + last_pos,
					  debug_type - dwarf_info.start);
			}
			break;
		}
		t = t->type.ref;
	}
	return retval;
}

static void tcc_debug_finish (TCCState *s1, struct _debug_info *cur)
{
	while (cur) {
		struct _debug_info *next = cur->next;
		int i;

		if (s1->dwarf) {

			for (i = cur->n_sym - 1; i >= 0; i--) {
				struct debug_sym *s = &cur->sym[i];

				dwarf_data1(dwarf_info_section,
					    s->type == N_PSYM
					    ? DWARF_ABBREV_FORMAL_PARAMETER
					    : s->type == N_GSYM
					    ? DWARF_ABBREV_VARIABLE_EXTERNAL
					    : s->type == N_STSYM
					    ? DWARF_ABBREV_VARIABLE_STATIC
					    : DWARF_ABBREV_VARIABLE_LOCAL);
				dwarf_strp(dwarf_info_section, s->str);
				if (s->type == N_GSYM || s->type == N_STSYM) {
					dwarf_uleb128(dwarf_info_section, s->file);
					dwarf_uleb128(dwarf_info_section, s->line);
				}
				dwarf_data4(dwarf_info_section, s->info - dwarf_info.start);
				if (s->type == N_GSYM || s->type == N_STSYM) {
					/* global/static */

					if (s->type == N_GSYM)
						dwarf_data1(dwarf_info_section, 1);
					dwarf_data1(dwarf_info_section, PTR_SIZE + 1);
					dwarf_data1(dwarf_info_section, DW_OP_addr);
					if (s->type == N_STSYM)
						dwarf_reloc(dwarf_info_section, section_sym, R_DATA_PTR);

					dwarf_data8(dwarf_info_section, s->value);

				} else {
					/* param/local */

					dwarf_data1(dwarf_info_section, dwarf_sleb128_size((long)s->value) + 1);
					dwarf_data1(dwarf_info_section, DW_OP_fbreg);
					dwarf_sleb128(dwarf_info_section, (long)s->value);
				}
				tcc_free (s->str);
			}
			tcc_free (cur->sym);
			dwarf_data1(dwarf_info_section,
				    cur->child ? DWARF_ABBREV_LEXICAL_BLOCK
				    : DWARF_ABBREV_LEXICAL_EMPTY_BLOCK);
			dwarf_reloc(dwarf_info_section, section_sym, R_DATA_PTR);

			dwarf_data8(dwarf_info_section, func_ind + cur->start);
			dwarf_data8(dwarf_info_section, cur->end - cur->start);

			tcc_debug_finish (s1, cur->child);
			if (cur->child)
				dwarf_data1(dwarf_info_section, 0);
		} else {
			for (i = 0; i < cur->n_sym; i++) {
				struct debug_sym *s = &cur->sym[i];

				if (s->sec)
					put_stabs_r(s1, s->str, s->type, 0, 0, s->value,
						    s->sec, s->sym_index);
				else
					put_stabs(s1, s->str, s->type, 0, 0, s->value);
				tcc_free (s->str);
			}
			tcc_free (cur->sym);
			put_stabn(s1, N_LBRAC, 0, 0, cur->start);
			tcc_debug_finish (s1, cur->child);
			put_stabn(s1, N_RBRAC, 0, 0, cur->end);
		}
		tcc_free (cur);
		cur = next;
	}
}

ST_FUNC void tcc_add_debug_info(TCCState *s1, Sym *s, Sym *e)
{
	CString debug_str;
	int param;

	if (!(s1->do_debug & 2))
		return;

	cstr_new (&debug_str);
	param = !e;
	for (; s != e; s = s->prev) {
		if (!s->v || (s->r & VT_VALMASK) != VT_LOCAL)
			continue;
		if (s1->dwarf) {
			tcc_debug_stabs(s1, get_tok_str(s->v, NULL),
					param ? N_PSYM : N_LSYM, s->c, NULL, 0,
					tcc_get_dwarf_info(s1, s));
		} else {
			cstr_reset (&debug_str);
			cstr_printf (&debug_str, "%s:%s", get_tok_str(s->v, NULL),
				     param ? "p" : "");
			tcc_get_debug_info(s1, s, &debug_str);
			tcc_debug_stabs(s1, debug_str.data, param ? N_PSYM : N_LSYM,
					s->c, NULL, 0, 0);
		}
	}
	cstr_free (&debug_str);
}
/* put function symbol */

ST_FUNC void tcc_debug_funcstart(TCCState *s1, Sym *sym)
{
	CString debug_str;
	BufferedFile *f;

	if (!s1->do_debug)
		return;
	debug_info_root = NULL;
	debug_info = NULL;
	tcc_debug_stabn(s1, N_LBRAC, ind - func_ind);
	f = put_new_file(s1);
	if (!f)
		return;

	if (s1->dwarf) {
		tcc_debug_line(s1);
		dwarf_info.func = sym;
		dwarf_info.line = file->line_num;
		if (s1->do_backtrace) {
			int i, len;

			dwarf_line_op(s1, 0);// extended

			dwarf_uleb128_op(s1, strlen(funcname) + 2);
			dwarf_line_op(s1, DW_LNE_hi_user - 1);
			len = strlen(funcname) + 1;
			for (i = 0; i < len; i++)
				dwarf_line_op(s1, funcname[i]);
		}
	} else {
		cstr_new (&debug_str);
		cstr_printf(&debug_str, "%s:%c", funcname, sym->type.t & VT_STATIC ? 'f' : 'F');
		tcc_get_debug_info(s1, sym->type.ref, &debug_str);
		put_stabs_r(s1, debug_str.data, N_FUN, 0, f->line_num, 0, cur_text_section,
			    sym->c);
		cstr_free (&debug_str);
		tcc_debug_line(s1);
	}
}

ST_FUNC void tcc_debug_prolog_epilog(TCCState *s1, int value)
{
	if (!s1->do_debug)
		return;
	if (s1->dwarf) {
		dwarf_line_op(s1, value == 0 ? DW_LNS_set_prologue_end
			      : DW_LNS_set_epilogue_begin);
	}
}
/* put function size */

ST_FUNC void tcc_debug_funcend(TCCState *s1, int size)
{
	/* lldb does not like function end and next function start at same pc */

	int min_instr_len;

	if (!s1->do_debug)
		return;
	min_instr_len = dwarf_line.last_pc == ind ? 0 : DWARF_MIN_INSTR_LEN;
	ind -= min_instr_len;
	tcc_debug_line(s1);
	ind += min_instr_len;
	tcc_debug_stabn(s1, N_RBRAC, size);
	if (s1->dwarf) {
		int func_sib = 0;
		Sym *sym = dwarf_info.func;
		int n_debug_info = tcc_get_dwarf_info(s1, sym->type.ref);

		dwarf_data1(dwarf_info_section,
			    sym->type.t & VT_STATIC ? DWARF_ABBREV_SUBPROGRAM_STATIC
			    : DWARF_ABBREV_SUBPROGRAM_EXTERNAL);
		if ((sym->type.t & VT_STATIC) == 0)
			dwarf_data1(dwarf_info_section, 1);
		dwarf_strp(dwarf_info_section, funcname);
		dwarf_uleb128(dwarf_info_section, dwarf_line.cur_file);
		dwarf_uleb128(dwarf_info_section, dwarf_info.line);
		tcc_debug_check_forw(s1, sym->type.ref, dwarf_info_section->data_offset);
		dwarf_data4(dwarf_info_section, n_debug_info - dwarf_info.start);
		dwarf_reloc(dwarf_info_section, section_sym, R_DATA_PTR);

		dwarf_data8(dwarf_info_section, func_ind);// low_pc

		dwarf_data8(dwarf_info_section, size);// high_pc

		func_sib = dwarf_info_section->data_offset;
		dwarf_data4(dwarf_info_section, 0);// sibling

		dwarf_data1(dwarf_info_section, 1);

		dwarf_data1(dwarf_info_section, DW_OP_reg6);// rbp

		tcc_debug_finish (s1, debug_info_root);
		dwarf_data1(dwarf_info_section, 0);
		write32le(dwarf_info_section->data + func_sib,
			  dwarf_info_section->data_offset - dwarf_info.start);
	} else {
		tcc_debug_finish (s1, debug_info_root);
	}
	debug_info_root = 0;
}

ST_FUNC void tcc_debug_extern_sym(TCCState *s1, Sym *sym, int sh_num,
				  int sym_bind, int sym_type)
{
	if (!(s1->do_debug & 2))
		return;

	if (sym_type == STT_FUNC || sym->v >= SYM_FIRST_ANOM)
		return;
	if (s1->dwarf) {
		int debug_type;

		debug_type = tcc_get_dwarf_info(s1, sym);
		dwarf_data1(dwarf_info_section,
			    sym_bind == STB_GLOBAL
			    ? DWARF_ABBREV_VARIABLE_EXTERNAL
			    : DWARF_ABBREV_VARIABLE_STATIC);
		dwarf_strp(dwarf_info_section, get_tok_str(sym->v, NULL));
		dwarf_uleb128(dwarf_info_section, dwarf_line.cur_file);
		dwarf_uleb128(dwarf_info_section, file->line_num);
		tcc_debug_check_forw(s1, sym, dwarf_info_section->data_offset);
		dwarf_data4(dwarf_info_section, debug_type - dwarf_info.start);
		if (sym_bind == STB_GLOBAL)
			dwarf_data1(dwarf_info_section, 1);
		dwarf_data1(dwarf_info_section, PTR_SIZE + 1);
		dwarf_data1(dwarf_info_section, DW_OP_addr);
		greloca(dwarf_info_section, sym, dwarf_info_section->data_offset,
			R_DATA_PTR, 0);

		dwarf_data8(dwarf_info_section, 0);

	} else {
		Section *s = sh_num == SHN_COMMON ? common_section
			     : s1->sections[sh_num];
		CString str;

		cstr_new (&str);
		cstr_printf (&str, "%s:%c",
			     get_tok_str(sym->v, NULL),
			     sym_bind == STB_GLOBAL ? 'G' : func_ind != -1 ? 'V' : 'S'
			    );
		tcc_get_debug_info(s1, sym, &str);
		if (sym_bind == STB_GLOBAL)
			tcc_debug_stabs(s1, str.data, N_GSYM, 0, NULL, 0, 0);
		else
			tcc_debug_stabs(s1, str.data,
					(sym->type.t & VT_STATIC) && data_section == s
					? N_STSYM : N_LCSYM, 0, s, sym->c, 0);
		cstr_free (&str);
	}
}

ST_FUNC void tcc_debug_typedef(TCCState *s1, Sym *sym)
{
	if (!(s1->do_debug & 2))
		return;

	if (s1->dwarf) {
		int debug_type;

		debug_type = tcc_get_dwarf_info(s1, sym);
		if (debug_type != -1) {
			dwarf_data1(dwarf_info_section, DWARF_ABBREV_TYPEDEF);
			dwarf_strp(dwarf_info_section, get_tok_str(sym->v, NULL));
			dwarf_uleb128(dwarf_info_section, dwarf_line.cur_file);
			dwarf_uleb128(dwarf_info_section, file->line_num);
			tcc_debug_check_forw(s1, sym, dwarf_info_section->data_offset);
			dwarf_data4(dwarf_info_section, debug_type - dwarf_info.start);
		}
	} else {
		CString str;
		cstr_new (&str);
		cstr_printf (&str, "%s:t", get_tok_str(sym->v, NULL));
		tcc_get_debug_info(s1, sym, &str);
		tcc_debug_stabs(s1, str.data, N_LSYM, 0, NULL, 0, 0);
		cstr_free (&str);
	}
}
/* ------------------------------------------------------------------------- */
/* for section layout see lib/tcov.c */

ST_FUNC void tcc_tcov_block_end(TCCState *s1, int line);

ST_FUNC void tcc_tcov_block_begin(TCCState *s1)
{
	SValue sv;
	void *ptr;
	unsigned long last_offset = tcov_data.offset;

	tcc_tcov_block_end (tcc_state, 0);
	if (s1->test_coverage == 0 || nocode_wanted)
		return;

	if (tcov_data.last_file_name == 0 ||
	    strcmp ((const char *)(tcov_section->data + tcov_data.last_file_name),
		    file->true_filename) != 0) {
		char wd[1024];
		CString cstr;

		if (tcov_data.last_func_name)
			section_ptr_add(tcov_section, 1);
		if (tcov_data.last_file_name)
			section_ptr_add(tcov_section, 1);
		tcov_data.last_func_name = 0;
		cstr_new (&cstr);
		if (file->true_filename[0] == '/') {
			tcov_data.last_file_name = tcov_section->data_offset;
			cstr_printf (&cstr, "%s", file->true_filename);
		} else {
			getcwd (wd, sizeof(wd));
			tcov_data.last_file_name = tcov_section->data_offset + strlen(wd) + 1;
			cstr_printf (&cstr, "%s/%s", wd, file->true_filename);
		}
		ptr = section_ptr_add(tcov_section, cstr.size + 1);
		strcpy((char *)ptr, cstr.data);

		normalize_slashes((char *)ptr);

		cstr_free (&cstr);
	}
	if (tcov_data.last_func_name == 0 ||
	    strcmp ((const char *)(tcov_section->data + tcov_data.last_func_name),
		    funcname) != 0) {
		size_t len;

		if (tcov_data.last_func_name)
			section_ptr_add(tcov_section, 1);
		tcov_data.last_func_name = tcov_section->data_offset;
		len = strlen (funcname);
		ptr = section_ptr_add(tcov_section, len + 1);
		strcpy((char *)ptr, funcname);
		section_ptr_add(tcov_section, -tcov_section->data_offset & 7);
		ptr = section_ptr_add(tcov_section, 8);
		write64le (ptr, file->line_num);
	}
	if (ind == tcov_data.ind && tcov_data.line == file->line_num)
		tcov_data.offset = last_offset;
	else {
		Sym label = {0};
		label.type.t = VT_LLONG | VT_STATIC;

		ptr = section_ptr_add(tcov_section, 16);
		tcov_data.line = file->line_num;
		write64le (ptr, (tcov_data.line << 8) | 0xff);
		put_extern_sym(&label, tcov_section,
			       ((unsigned char *)ptr - tcov_section->data) + 8, 0);
		sv.type = label.type;
		sv.r = VT_SYM | VT_LVAL | VT_CONST;
		sv.r2 = VT_CONST;
		sv.c.i = 0;
		sv.sym = &label;

		gen_increment_tcov (&sv);

		tcov_data.offset = (unsigned char *)ptr - tcov_section->data;
		tcov_data.ind = ind;
	}
}

ST_FUNC void tcc_tcov_block_end(TCCState *s1, int line)
{
	if (s1->test_coverage == 0)
		return;
	if (line == -1)
		line = tcov_data.line;
	if (tcov_data.offset) {
		void *ptr = tcov_section->data + tcov_data.offset;
		unsigned long long nline = line ? line : file->line_num;

		write64le (ptr, (read64le (ptr) & 0xfffffffffull) | (nline << 36));
		tcov_data.offset = 0;
	}
}

ST_FUNC void tcc_tcov_check_line(TCCState *s1, int start)
{
	if (s1->test_coverage == 0)
		return;
	if (tcov_data.line != file->line_num) {
		if ((tcov_data.line + 1) != file->line_num) {
			tcc_tcov_block_end (s1, -1);
			if (start)
				tcc_tcov_block_begin (s1);
		} else
			tcov_data.line = file->line_num;
	}
}

ST_FUNC void tcc_tcov_start(TCCState *s1)
{
	if (s1->test_coverage == 0)
		return;
	if (!s1->dState)
		s1->dState = tcc_mallocz(sizeof *s1->dState);
	memset (&tcov_data, 0, sizeof (tcov_data));
	if (tcov_section == NULL) {
		tcov_section = new_section(tcc_state, ".tcov", SHT_PROGBITS,
					   SHF_ALLOC | SHF_WRITE);
		section_ptr_add(tcov_section, 4);// pointer to executable name

	}
}

ST_FUNC void tcc_tcov_end(TCCState *s1)
{
	if (s1->test_coverage == 0)
		return;
	if (tcov_data.last_func_name)
		section_ptr_add(tcov_section, 1);
	if (tcov_data.last_file_name)
		section_ptr_add(tcov_section, 1);
}

ST_FUNC void tcc_tcov_reset_ind(TCCState *s1)
{
	tcov_data.ind = 0;
}
/* ------------------------------------------------------------------------- */

#undef last_line_num
#undef new_file
#undef section_sym
#undef debug_next_type
#undef debug_hash_global
#undef debug_hash_local
#undef n_debug_hash_global
#undef n_debug_hash_local
#undef debug_forw_hash_global
#undef debug_forw_hash_local
#undef n_debug_forw_hash_global
#undef n_debug_forw_hash_local
#undef debug_info
#undef debug_info_root
#undef dwarf_sym
#undef dwarf_line
#undef dwarf_info
#undef dwarf_str
#undef dwarf_line_str
#undef tcov_data
/* ==================== tccasm.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) tcc_state->sym
#define TCC_SET_STATE(fn) fn
#undef _tcc_error

static Section *last_text_section;/* to handle .previous asm directive */

static int asmgoto_n;

static int tcc_assemble_internal(TCCState *s1, int do_preprocess, int global);
static Sym *asm_new_label(TCCState *s1, int label, int is_local);
static Sym *asm_new_label1(TCCState *s1, int label, int is_local, int sh_num,
			   int value);
/* output constant with relocation if 'r & VT_SYM' is true */

ST_FUNC void gen_addr64(int r, Sym *sym, int64_t c)
{
	if (r & VT_SYM)
		greloca(cur_text_section, sym, ind, R_DATA_PTR, c), c=0;
	gen_le32(c);
	gen_le32(c>>32);
}

ST_FUNC void gen_expr64(ExprValue *pe)
{
	gen_addr64(pe->sym ? VT_SYM : 0, pe->sym, pe->v);
}

static int asm_get_prefix_name(TCCState *s1, const char *prefix, unsigned int n)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%s%u", prefix, n);
	return tok_alloc_const(buf);
}

ST_FUNC int asm_get_local_label_name(TCCState *s1, unsigned int n)
{
	return asm_get_prefix_name(s1, "L..", n);
}
/* If a C name has an _ prepended then only asm labels that start
   with _ are representable in C, by removing the first _.  ASM names
   without _ at the beginning don't correspond to C names, but we use
   the global C symbol table to track ASM names as well, so we need to
   transform those into ones that don't conflict with a C name,
   so prepend a '.' for them, but force the ELF asm name to be set.  */

static int asm2cname(int v, int *addeddot)
{
	const char *name;
	*addeddot = 0;
	if (!tcc_state->leading_underscore)
		return v;
	name = get_tok_str(v, NULL);
	if (!name)
		return v;
	if (name[0] == '_') {
		v = tok_alloc_const(name + 1);
	} else if (!strchr(name, '.')) {
		char newname[256];
		snprintf(newname, sizeof newname, ".%s", name);
		v = tok_alloc_const(newname);
		*addeddot = 1;
	}
	return v;
}

static Sym *asm_label_find(int v)
{
	Sym *sym;
	int addeddot;
	v = asm2cname(v, &addeddot);
	sym = sym_find(v);
	while (sym && sym->sym_scope && !(sym->type.t & VT_STATIC))
		sym = sym->prev_tok;
	return sym;
}

static Sym *asm_label_push(int v)
{
	int addeddot, v2 = asm2cname(v, &addeddot);
	/* We always add VT_EXTERN, for sym definition that's tentative
	       (for .set, removed for real defs), for mere references it's correct
	       as is.  */

	Sym *sym = global_identifier_push(v2, VT_ASM | VT_EXTERN | VT_STATIC, 0);
	if (addeddot)
		sym->asm_label = v;
	return sym;
}
/* Return a symbol we can use inside the assembler, having name NAME.
   Symbols from asm and C source share a namespace.  If we generate
   an asm symbol it's also a (file-global) C symbol, but it's
   either not accessible by name (like "L.123"), or its type information
   is such that it's not usable without a proper C declaration.

   Sometimes we need symbols accessible by name from asm, which
   are anonymous in C, in this case CSYM can be used to transfer
   all information from that symbol to the (possibly newly created)
   asm symbol.  */
ST_FUNC Sym *get_asm_sym(int name, Sym *csym)
{
	Sym *sym = asm_label_find(name);
	if (!sym) {
		sym = asm_label_push(name);
		if (csym)
			sym->c = csym->c;
	}
	return sym;
}

static Sym *asm_section_sym(TCCState *s1, Section *sec)
{
	char buf[100];
	int label;
	Sym *sym;
	snprintf(buf, sizeof buf, "L.%s", sec->name);
	label = tok_alloc_const(buf);
	sym = asm_label_find(label);
	return sym ? sym : asm_new_label1(s1, label, 1, sec->sh_num, 0);
}
/* We do not use the C expression parser to handle symbols. Maybe the
   C expression parser could be tweaked to do so. */

static void asm_expr_unary(TCCState *s1, ExprValue *pe)
{
	Sym *sym;
	int op, label;
	uint64_t n;
	const char *p;

	switch (tok) {
	case TOK_PPNUM:
		p = tokc.str.data;
		n = strtoull(p, (char **)&p, 0);
		if (*p == 'b' || *p == 'f') {
			/* backward or forward label */

			label = asm_get_local_label_name(s1, n);
			sym = asm_label_find(label);
			if (*p == 'b') {
				/* backward : find the last corresponding defined label */

				if (sym && (!sym->c || elfsym(sym)->st_shndx == SHN_UNDEF))
					sym = sym->prev_tok;
				if (!sym)
					tcc_error("local label '%d' not found backward", (int)n);
			} else {
				/* forward */

				if (!sym || (sym->c && elfsym(sym)->st_shndx != SHN_UNDEF)) {
					/* if the last label is defined, then define a new one */

					sym = asm_label_push(label);
				}
			}
			pe->v = 0;
			pe->sym = sym;
			pe->pcrel = 0;
		} else if (*p == '\0') {
			pe->v = n;
			pe->sym = NULL;
			pe->pcrel = 0;
		} else {
			tcc_error("invalid number syntax");
		}
		next();
		break;
	case '+':
		next();
		asm_expr_unary(s1, pe);
		break;
	case '-':
	case '~':
		op = tok;
		next();
		asm_expr_unary(s1, pe);
		if (pe->sym)
			tcc_error("invalid operation with label");
		if (op == '-')
			pe->v = -pe->v;
		else
			pe->v = ~pe->v;
		break;
	case TOK_CCHAR:
	case TOK_LCHAR:
		pe->v = tokc.i;
		pe->sym = NULL;
		pe->pcrel = 0;
		next();
		break;
	case '(':
		next();
		asm_expr(s1, pe);
		skip(')');
		break;
	case '.':
		pe->v = ind;
		pe->sym = asm_section_sym(s1, cur_text_section);
		pe->pcrel = 0;
		next();
		break;
	default:
		if (tok >= TOK_IDENT) {
			ElfSym *esym;
			/* label case : if the label was not found, add one */

			sym = get_asm_sym(tok, NULL);
			esym = elfsym(sym);
			if (esym && esym->st_shndx == SHN_ABS) {
				/* if absolute symbol, no need to put a symbol value */

				pe->v = esym->st_value;
				pe->sym = NULL;
				pe->pcrel = 0;
			} else {
				pe->v = 0;
				pe->sym = sym;
				pe->pcrel = 0;
			}
			next();
		} else {
			tcc_error("bad expression syntax [%s]", get_tok_str(tok, &tokc));
		}
		break;
	}
}

static void asm_expr_prod(TCCState *s1, ExprValue *pe)
{
	int op;
	ExprValue e2;

	asm_expr_unary(s1, pe);
	for (;;) {
		op = tok;
		if (op != '*' && op != '/' && op != '%' &&
		    op != TOK_SHL && op != TOK_SAR)
			break;
		next();
		asm_expr_unary(s1, &e2);
		if (pe->sym || e2.sym)
			tcc_error("invalid operation with label");
		switch (op) {
		case '*':
			pe->v *= e2.v;
			break;
		case '/':
			if (e2.v == 0) {
div_error:
				tcc_error("division by zero");
			}
			pe->v /= e2.v;
			break;
		case '%':
			if (e2.v == 0)
				goto div_error;
			pe->v %= e2.v;
			break;
		case TOK_SHL:
			pe->v <<= e2.v;
			break;
		default:
		case TOK_SAR:
			pe->v >>= e2.v;
			break;
		}
	}
}

static void asm_expr_logic(TCCState *s1, ExprValue *pe)
{
	int op;
	ExprValue e2;

	asm_expr_prod(s1, pe);
	for (;;) {
		op = tok;
		if (op != '&' && op != '|' && op != '^')
			break;
		next();
		asm_expr_prod(s1, &e2);
		if (pe->sym || e2.sym)
			tcc_error("invalid operation with label");
		switch (op) {
		case '&':
			pe->v &= e2.v;
			break;
		case '|':
			pe->v |= e2.v;
			break;
		default:
		case '^':
			pe->v ^= e2.v;
			break;
		}
	}
}

static inline void asm_expr_sum(TCCState *s1, ExprValue *pe)
{
	int op;
	ExprValue e2;

	asm_expr_logic(s1, pe);
	for (;;) {
		op = tok;
		if (op != '+' && op != '-')
			break;
		next();
		asm_expr_logic(s1, &e2);
		if (op == '+') {
			if (pe->sym != NULL && e2.sym != NULL)
				goto cannot_relocate;
			pe->v += e2.v;
			if (pe->sym == NULL && e2.sym != NULL)
				pe->sym = e2.sym;
		} else {
			pe->v -= e2.v;
			/* NOTE: we are less powerful than gas in that case
			               because we store only one symbol in the expression */

			if (!e2.sym) {
				/* OK */

			} else if (pe->sym == e2.sym) {
				/* OK */

				pe->sym = NULL;/* same symbols can be subtracted to NULL */

			} else {
				ElfSym *esym1, *esym2;
				esym1 = elfsym(pe->sym);
				esym2 = elfsym(e2.sym);
				if (!esym2)
					goto cannot_relocate;
				if (esym1 && esym1->st_shndx == esym2->st_shndx
				    && esym1->st_shndx != SHN_UNDEF) {
					/* we also accept defined symbols in the same section */

					pe->v += (int)(esym1->st_value - esym2->st_value);
					pe->sym = NULL;
				} else if (esym2->st_shndx == cur_text_section->sh_num) {
					/* When subtracting a defined symbol in current section
							       this actually makes the value PC-relative.  */

					pe->v += (int)(0 - esym2->st_value);
					pe->pcrel = 1;
					e2.sym = NULL;
				} else {
cannot_relocate:
					tcc_error("invalid operation with label");
				}
			}
		}
	}
}

static inline void asm_expr_cmp(TCCState *s1, ExprValue *pe)
{
	int op;
	ExprValue e2;

	asm_expr_sum(s1, pe);
	for (;;) {
		op = tok;
		if (op != TOK_EQ && op != TOK_NE
		    && (op > TOK_GT || op < TOK_ULE))
			break;
		next();
		asm_expr_sum(s1, &e2);
		if (pe->sym || e2.sym)
			tcc_error("invalid operation with label");
		switch (op) {
		case TOK_EQ:
			pe->v = pe->v == e2.v;
			break;
		case TOK_NE:
			pe->v = pe->v != e2.v;
			break;
		case TOK_LT:
			pe->v = (int64_t)pe->v < (int64_t)e2.v;
			break;
		case TOK_GE:
			pe->v = (int64_t)pe->v >= (int64_t)e2.v;
			break;
		case TOK_LE:
			pe->v = (int64_t)pe->v <= (int64_t)e2.v;
			break;
		case TOK_GT:
			pe->v = (int64_t)pe->v > (int64_t)e2.v;
			break;
		default:
			break;
		}
		/* GAS compare results are -1/0 not 1/0.  */

		pe->v = -(int64_t)pe->v;
	}
}

ST_FUNC void asm_expr(TCCState *s1, ExprValue *pe)
{
	asm_expr_cmp(s1, pe);
}

ST_FUNC int asm_int_expr(TCCState *s1)
{
	ExprValue e;
	asm_expr(s1, &e);
	if (e.sym)
		expect("constant");
	if ((int)e.v != e.v)
		tcc_error("integer out of range %lld", (long long)e.v);
	return e.v;
}

static Sym *asm_new_label1(TCCState *s1, int label, int is_local,
			   int sh_num, int value)
{
	Sym *sym;
	ElfSym *esym;

	sym = asm_label_find(label);
	if (sym) {
		esym = elfsym(sym);
		/* A VT_EXTERN symbol, even if it has a section is considered
			   overridable.  This is how we "define" .set targets.  Real
			   definitions won't have VT_EXTERN set.  */

		if (esym && esym->st_shndx != SHN_UNDEF) {
			/* the label is already defined */

			if (IS_ASM_SYM(sym)
			    && (is_local == 1 || (sym->type.t & VT_EXTERN)))
				goto new_label;
			if (!(sym->type.t & VT_EXTERN))
				tcc_error("assembler label '%s' already defined",
					  get_tok_str(label, NULL));
		}
	} else {
new_label:
		sym = asm_label_push(label);
	}
	if (!sym->c)
		put_extern_sym2(sym, SHN_UNDEF, 0, 0, 1);
	esym = elfsym(sym);
	esym->st_shndx = sh_num;
	esym->st_value = value;
	if (is_local != 2)
		sym->type.t &= ~VT_EXTERN;
	return sym;
}

static Sym *asm_new_label(TCCState *s1, int label, int is_local)
{
	return asm_new_label1(s1, label, is_local, cur_text_section->sh_num, ind);
}
/* Set the value of LABEL to that of some expression (possibly
   involving other symbols).  LABEL can be overwritten later still.  */

static Sym *set_symbol(TCCState *s1, int label)
{
	long n;
	ExprValue e;
	Sym *sym;
	ElfSym *esym;
	next();
	asm_expr(s1, &e);
	n = e.v;
	esym = elfsym(e.sym);
	if (esym)
		n += esym->st_value;
	sym = asm_new_label1(s1, label, 2, esym ? esym->st_shndx : SHN_ABS, n);
	elfsym(sym)->st_other |= ST_ASM_SET;
	return sym;
}

static void use_section1(TCCState *s1, Section *sec)
{
	cur_text_section->data_offset = ind;
	cur_text_section = sec;
	ind = cur_text_section->data_offset;
}

static void use_section(TCCState *s1, const char *name)
{
	Section *sec;
	sec = find_section(s1, name);
	use_section1(s1, sec);
}

static void push_section(TCCState *s1, const char *name)
{
	Section *sec = find_section(s1, name);
	sec->prev = cur_text_section;
	use_section1(s1, sec);
}

static void pop_section(TCCState *s1)
{
	Section *prev = cur_text_section->prev;
	if (!prev)
		tcc_error(".popsection without .pushsection");
	cur_text_section->prev = NULL;
	use_section1(s1, prev);
}

static void asm_parse_directive(TCCState *s1, int global)
{
	int n, offset, v, size, tok1, c;
	Section *sec;
	uint8_t *ptr;
	/* assembler directive */

	sec = cur_text_section;
	switch (tok) {
	case TOK_ASMDIR_align:
	case TOK_ASMDIR_balign:
	case TOK_ASMDIR_p2align:
	case TOK_ASMDIR_skip:
	case TOK_ASMDIR_space:
		tok1 = tok;
		next();
		n = asm_int_expr(s1);
		if (tok1 == TOK_ASMDIR_p2align) {
			if (n < 0 || n > 30)
				tcc_error("invalid p2align, must be between 0 and 30");
			n = 1 << n;
			tok1 = TOK_ASMDIR_align;
		}
		if (tok1 == TOK_ASMDIR_align || tok1 == TOK_ASMDIR_balign) {
			if (n <= 0 || (n & (n-1)) != 0)
				tcc_error("alignment must be a positive power of two");
			offset = (ind + n - 1) & -n;
			size = offset - ind;
			/* the section must have a compatible alignment */

			if (sec->sh_addralign < n)
				sec->sh_addralign = n;
			c = sec->sh_flags & SHF_EXECINSTR;
		} else {
			if (n < 0)
				n = 0;
			size = n, c = 0;
		}
		v = 0;
		if (tok == ',') {
			next();
			v = asm_int_expr(s1), c = 0;
		}
zero_pad:
		if ((uint64_t)ind + size >= 1<<30)
			tcc_error("too much data");
		if (sec->sh_type != SHT_NOBITS) {
			if (c) {
				gen_fill_nops(size);
				break;
			}
			sec->data_offset = ind;
			ptr = section_ptr_add(sec, size);
			memset(ptr, v, size);
		}
		ind += size;
		break;
	case TOK_ASMDIR_quad:

		size = 8;
		goto asm_data;
	case TOK_ASMDIR_byte:
		size = 1;
		goto asm_data;
	case TOK_ASMDIR_word:
	case TOK_ASMDIR_short:
		size = 2;
		goto asm_data;
	case TOK_ASMDIR_long:
	case TOK_ASMDIR_int:
		size = 4;
asm_data:
		next();
		for (;;) {
			ExprValue e;
			asm_expr(s1, &e);
			if (sec->sh_type != SHT_NOBITS) {
				if (size == 4) {
					gen_expr32(&e);

				} else if (size == 8) {
					gen_expr64(&e);

				} else {
					if (e.sym)
						expect("constant");
					if (size == 1)
						g(e.v);
					else
						gen_le16(e.v);
				}
			} else {
				ind += size;
			}
			if (tok != ',')
				break;
			next();
		}
		break;
	case TOK_ASMDIR_fill: {
		int repeat, size, val, i, j;
		uint8_t repeat_buf[8];
		next();
		repeat = asm_int_expr(s1);
		if (repeat < 0) {
			tcc_error("repeat < 0; .fill ignored");
			break;
		}
		size = 1;
		val = 0;
		if (tok == ',') {
			next();
			size = asm_int_expr(s1);
			if (size < 0) {
				tcc_error("size < 0; .fill ignored");
				break;
			}
			if (size > 8)
				size = 8;
			if (tok == ',') {
				next();
				val = asm_int_expr(s1);
			}
		}
		/* XXX: endianness */

		repeat_buf[0] = val;
		repeat_buf[1] = val >> 8;
		repeat_buf[2] = val >> 16;
		repeat_buf[3] = val >> 24;
		repeat_buf[4] = 0;
		repeat_buf[5] = 0;
		repeat_buf[6] = 0;
		repeat_buf[7] = 0;
		for (i = 0; i < repeat; i++) {
			for (j = 0; j < size; j++) {
				g(repeat_buf[j]);
			}
		}
	}
	break;
	case TOK_ASMDIR_rept: {
		int repeat;
		TokenString *init_str;
		next();
		repeat = asm_int_expr(s1);
		init_str = tok_str_alloc();
		while (next(), tok != TOK_ASMDIR_endr) {
			if (tok == CH_EOF)
				tcc_error("we at end of file, .endr not found");
			tok_str_add_tok(init_str);
		}
		tok_str_add(init_str, TOK_EOF);
		begin_macro(init_str, 1);
		while (repeat-- > 0) {
			tcc_assemble_internal(s1, (parse_flags & PARSE_FLAG_PREPROCESS),
					      global);
			macro_ptr = init_str->str;
		}
		end_macro();
		next();
		break;
	}
	case TOK_ASMDIR_org: {
		ExprValue e;
		ElfSym *esym;
		next();
		asm_expr(s1, &e);
		n = e.v;
		esym = elfsym(e.sym);
		if (esym) {
			if (esym->st_shndx != cur_text_section->sh_num)
				expect("constant or same-section symbol");
			n += esym->st_value;
		}
		if (n < ind)
			tcc_error("attempt to .org backwards");
		v = c = 0;
		size = n - ind;
		goto zero_pad;
	}
	break;
	case TOK_ASMDIR_set:
		next();
		tok1 = tok;
		next();
		/* Also accept '.set stuff', but don't do anything with this.
			   It's used in GAS to set various features like '.set mips16'.  */

		if (tok == ',')
			set_symbol(s1, tok1);
		break;
	case TOK_ASMDIR_globl:
	case TOK_ASMDIR_global:
	case TOK_ASMDIR_weak:
	case TOK_ASMDIR_hidden:
		tok1 = tok;
		do {
			Sym *sym;
			next();
			if (tok < TOK_IDENT)
				expect("identifier");
			sym = get_asm_sym(tok, NULL);
			if (tok1 != TOK_ASMDIR_hidden)
				sym->type.t &= ~VT_STATIC;
			if (tok1 == TOK_ASMDIR_weak)
				sym->a.weak = 1;
			else if (tok1 == TOK_ASMDIR_hidden)
				sym->a.visibility = STV_HIDDEN;
			update_storage(sym);
			next();
		} while (tok == ',');
		break;
	case TOK_ASMDIR_string:
	case TOK_ASMDIR_ascii:
	case TOK_ASMDIR_asciz: {
		const char *p;
		int i, size, t;

		t = tok;
		next();
		for (;;) {
			if (tok != TOK_STR)
				expect("string constant");
			p = tokc.str.data;
			size = tokc.str.size;
			if (t == TOK_ASMDIR_ascii && size > 0)
				size--;
			for (i = 0; i < size; i++)
				g(p[i]);
			next();
			if (tok == ',') {
				next();
			} else if (tok != TOK_STR) {
				break;
			}
		}
	}
	break;
	case TOK_ASMDIR_text:
	case TOK_ASMDIR_data:
	case TOK_ASMDIR_bss: {
		char sname[64];
		tok1 = tok;
		n = 0;
		next();
		if (tok != ';' && tok != TOK_LINEFEED) {
			n = asm_int_expr(s1);
			next();
		}
		if (n)
			sprintf(sname, "%s%d", get_tok_str(tok1, NULL), n);
		else
			sprintf(sname, "%s", get_tok_str(tok1, NULL));
		use_section(s1, sname);
	}
	break;
	case TOK_ASMDIR_file: {
		const char *p;
		parse_flags &= ~PARSE_FLAG_TOK_STR;
		next();
		if (tok == TOK_PPNUM)
			next();
		if (tok == TOK_PPSTR && tokc.str.data[0] == '"') {
			tokc.str.data[tokc.str.size - 2] = 0;
			p = tokc.str.data + 1;
		} else if (tok >= TOK_IDENT) {
			p = get_tok_str(tok, &tokc);
		} else {
			skip_to_eol(0);
			break;
		}
		tccpp_putfile(p);
		next();
	}
	break;
	case TOK_ASMDIR_ident: {
		char ident[256];

		ident[0] = '\0';
		next();
		if (tok == TOK_STR)
			pstrcat(ident, sizeof(ident), tokc.str.data);
		else
			pstrcat(ident, sizeof(ident), get_tok_str(tok, &tokc));
		tcc_warning_c(warn_unsupported)("ignoring .ident %s", ident);
		next();
	}
	break;
	case TOK_ASMDIR_size: {
		Sym *sym;
		ElfSym *esym;

		next();
		if (tok < TOK_IDENT)
			expect("identifier");
		sym = asm_label_find(tok);
		if (!sym)
			tcc_error("label not found: %s", get_tok_str(tok, NULL));
		/* XXX .size name,label2-label1 */

		tcc_warning_c(warn_unsupported)("ignoring .size %s,*", get_tok_str(tok, NULL));
		next();
		skip(',');
		n = asm_int_expr(s1);
		esym = elfsym(sym);
		if (esym) {
			esym->st_size = n;
		}
	}
	break;
	case TOK_ASMDIR_type: {
		Sym *sym;
		const char *newtype;
		int st_type;

		next();
		if (tok < TOK_IDENT)
			expect("identifier");
		sym = get_asm_sym(tok, NULL);
		next();
		skip(',');
		if (tok == TOK_STR) {
			newtype = tokc.str.data;
		} else {
			if (tok == '@' || tok == '%')
				next();
			newtype = get_tok_str(tok, NULL);
		}

		if (!strcmp(newtype, "function") || !strcmp(newtype, "STT_FUNC")) {
			if (IS_ASM_SYM(sym))
				sym->type.t |= VT_ASM_FUNC;
			st_type = STT_FUNC;
set_st_type:
			if (sym->c) {
				ElfSym *esym = elfsym(sym);
				esym->st_info = ELFW(ST_INFO)(ELFW(ST_BIND)(esym->st_info), st_type);
			}
		} else if (!strcmp(newtype, "object") || !strcmp(newtype, "STT_OBJECT")) {
			st_type = STT_OBJECT;
			goto set_st_type;
		} else
			tcc_warning_c(warn_unsupported)("change type of '%s' from 0x%x to '%s' ignored",
							get_tok_str(sym->v, NULL), sym->type.t, newtype);

		next();
	}
	break;
	case TOK_ASMDIR_pushsection:
	case TOK_ASMDIR_section: {
		char sname[256];
		int old_nb_section = s1->nb_sections;
		int flags = SHF_ALLOC;

		tok1 = tok;
		/* XXX: support more options */

		next();
		sname[0] = '\0';
		while (tok != ';' && tok != TOK_LINEFEED && tok != ',') {
			if (tok == TOK_STR)
				pstrcat(sname, sizeof(sname), tokc.str.data);
			else
				pstrcat(sname, sizeof(sname), get_tok_str(tok, NULL));
			next();
		}
		if (tok == ',') {
			const char *p;
			/* skip section options */

			next();
			if (tok != TOK_STR)
				expect("string constant");
			for (p = tokc.str.data; *p; ++p) {
				if (*p == 'w')
					flags |= SHF_WRITE;
				if (*p == 'x')
					flags |= SHF_EXECINSTR;
			}
			next();
			if (tok == ',') {
				next();
				if (tok == '@' || tok == '%')
					next();
				next();
			}
		}
		last_text_section = cur_text_section;
		if (tok1 == TOK_ASMDIR_section)
			use_section(s1, sname);
		else
			push_section(s1, sname);
		/* If we just allocated a new section reset its alignment to
			       1.  new_section normally acts for GCC compatibility and
			       sets alignment to PTR_SIZE.  The assembler behaves different. */

		if (old_nb_section != s1->nb_sections) {
			cur_text_section->sh_addralign = 1;
			/* Make .init and .fini sections executable by default.
			                   GAS does so, too, and musl relies on it. */

			if (!strcmp(sname, ".init") || !strcmp(sname, ".fini"))
				flags |= SHF_EXECINSTR;
			cur_text_section->sh_flags = flags;
		}
	}
	break;
	case TOK_ASMDIR_previous: {
		Section *sec;
		next();
		if (!last_text_section)
			tcc_error("no previous section referenced");
		sec = cur_text_section;
		use_section1(s1, last_text_section);
		last_text_section = sec;
	}
	break;
	case TOK_ASMDIR_popsection:
		next();
		pop_section(s1);
		break;
	/* added for compatibility with GAS */
	case TOK_ASMDIR_code64:
		next();
		break;
	/* TODO: Implement symvar support. FreeBSD >= 14 needs this */
	case TOK_ASMDIR_symver:
		next();
		next();
		skip(',');
		next();
		skip('@');
		next();
		break;
	case TOK_ASMDIR_reloc: {
		ExprValue e;
		const char *reloc_name;
		int reloc_type = -1;

		next();
		asm_expr(s1, &e);
		skip(',');
		reloc_name = get_tok_str(tok, NULL);
		if (reloc_type < 0)
			tcc_error("unimp: reloc '%s' unknown", reloc_name);
		next();
		skip(',');
		greloca(cur_text_section, get_asm_sym(tok, NULL), e.v, reloc_type, 0);
		next();
	}
	break;
	default:
		tcc_error("unknown assembler directive '.%s'", get_tok_str(tok, NULL));
		break;
	}
}
/* assemble a file */

static int tcc_assemble_internal(TCCState *s1, int do_preprocess, int global)
{
	int opcode;
	int saved_parse_flags = parse_flags;

	parse_flags = PARSE_FLAG_ASM_FILE | PARSE_FLAG_TOK_STR;
	if (do_preprocess)
		parse_flags |= PARSE_FLAG_PREPROCESS;
	for (;;) {
		next();
		if (tok == TOK_EOF)
			break;
		tcc_debug_line(s1);
		parse_flags |= PARSE_FLAG_LINEFEED;/* XXX: suppress that hack */

redo:

		if (tok == '#') {
			/* horrible gas comment */

			while (tok != TOK_LINEFEED)
				next();
		} else

			if (tok >= TOK_ASMDIR_FIRST && tok <= TOK_ASMDIR_LAST) {
				asm_parse_directive(s1, global);
			} else if (tok == TOK_PPNUM) {
				const char *p;
				int n;
				p = tokc.str.data;
				n = strtoul(p, (char **)&p, 10);
				if (*p != '\0')
					expect("':'");
				/* new local label */

				asm_new_label(s1, asm_get_local_label_name(s1, n), 1);
				next();
				skip(':');
				goto redo;
			} else if (tok >= TOK_IDENT) {
				/* instruction or label */

				opcode = tok;
				next();
				if (tok == ':') {
					/* new label */

					asm_new_label(s1, opcode, 0);
					next();
					goto redo;
				} else if (tok == '=') {
					set_symbol(s1, opcode);
					goto redo;
				} else {
					asm_opcode(s1, opcode);
				}
			}
		/* end of line */

		if (tok != ';' && tok != TOK_LINEFEED)
			expect("end of line");
		parse_flags &= ~PARSE_FLAG_LINEFEED;/* XXX: suppress that hack */

	}

	parse_flags = saved_parse_flags;
	return 0;
}
/* Assemble the current file */

ST_FUNC int tcc_assemble(TCCState *s1, int do_preprocess)
{
	int ret;
	tcc_debug_start(s1);
	/* default section is text */

	cur_text_section = text_section;
	ind = cur_text_section->data_offset;
	nocode_wanted = 0;
	ret = tcc_assemble_internal(s1, do_preprocess, 1);
	cur_text_section->data_offset = ind;
	tcc_debug_end(s1);
	return ret;
}
/**/
/* GCC inline asm support */
/* assemble the string 'str' in the current C compilation unit without
   C preprocessing. */

static void tcc_assemble_inline(TCCState *s1, const char *str, int len,
				int global)
{
	const int *saved_macro_ptr = macro_ptr;
	int dotid = set_idnum('.', IS_ID);

	tcc_open_bf(s1, ":asm:", len);
	memcpy(file->buffer, str, len);
	macro_ptr = NULL;
	tcc_assemble_internal(s1, 0, global);
	tcc_close();

	set_idnum('.', dotid);
	macro_ptr = saved_macro_ptr;
}
/* find a constraint by its number or id (gcc 3 extended
   syntax). return -1 if not found. Return in *pp in char after the
   constraint */

ST_FUNC int find_constraint(ASMOperand *operands, int nb_operands,
			    const char *name, const char **pp)
{
	int index;
	TokenSym *ts;
	const char *p;

	if (isnum(*name)) {
		index = 0;
		while (isnum(*name)) {
			index = (index * 10) + (*name) - '0';
			name++;
		}
		if ((unsigned)index >= nb_operands)
			index = -1;
	} else if (*name == '[') {
		name++;
		p = strchr(name, ']');
		if (p) {
			ts = tok_alloc(name, p - name);
			for (index = 0; index < nb_operands; index++) {
				if (operands[index].id == ts->tok)
					goto found;
			}
			index = -1;
found:
			name = p + 1;
		} else {
			index = -1;
		}
	} else {
		index = -1;
	}
	if (pp)
		*pp = name;
	return index;
}

static void subst_asm_operands(ASMOperand *operands, int nb_operands,
			       CString *out_str, const char *str)
{
	int c, index, modifier;
	ASMOperand *op;
	SValue sv;

	for (;;) {
		c = *str++;
		if (c == '%') {
			if (*str == '%') {
				str++;
				goto add_char;
			}
			modifier = 0;
			if (*str == 'c' || *str == 'n' ||
			    *str == 'b' || *str == 'w' || *str == 'h' || *str == 'k' ||
			    *str == 'q' || *str == 'l' ||
			    /* P in GCC would add "@PLT" to symbol refs in PIC mode,
			    		   and make literal operands not be decorated with '$'.  */
			    *str == 'P')
				modifier = *str++;
			index = find_constraint(operands, nb_operands, str, &str);
			if (index < 0)
error:
				tcc_error("invalid operand reference after %%");
			op = &operands[index];
			if (modifier == 'l') {
				cstr_cat(out_str, get_tok_str(op->is_label, NULL), -1);
			} else {
				if (op->vt == NULL)
					goto error;
				sv = *op->vt;
				if (op->reg >= 0) {
					sv.r = op->reg;
					if (op->is_memory)
						sv.r |= VT_LVAL;
				}
				subst_asm_operand(out_str, &sv, modifier);
			}
		} else {
add_char:
			cstr_ccat(out_str, c);
			if (c == '\0')
				break;
		}
	}
}

static void parse_asm_operands(ASMOperand *operands, int *nb_operands_ptr,
			       int is_output)
{
	ASMOperand *op;
	int nb_operands;
	char *astr;

	if (tok != ':') {
		nb_operands = *nb_operands_ptr;
		for (;;) {
			if (nb_operands >= MAX_ASM_OPERANDS)
				tcc_error("too many asm operands");
			op = &operands[nb_operands++];
			op->id = 0;
			if (tok == '[') {
				next();
				if (tok < TOK_IDENT)
					expect("identifier");
				op->id = tok;
				next();
				skip(']');
			}
			astr = parse_mult_str("string constant")->data;
			pstrcpy(op->constraint, sizeof op->constraint, astr);
			skip('(');
			gexpr();
			if (is_output) {
				if (!(vtop->type.t & VT_ARRAY))
					test_lvalue();
			} else {
				/* we want to avoid LLOCAL case, except when the 'm'
				                   constraint is used. Note that it may come from
				                   register storage, so we need to convert (reg)
				                   case */

				if ((vtop->r & VT_LVAL) &&
				    ((vtop->r & VT_VALMASK) == VT_LLOCAL ||
				     (vtop->r & VT_VALMASK) < VT_CONST) &&
				    !strchr(op->constraint, 'm')

				   ) {
					gv(RC_INT);
				}
			}
			op->vt = vtop;
			skip(')');
			if (tok == ',') {
				next();
			} else {
				break;
			}
		}
		*nb_operands_ptr = nb_operands;
	}
}
/* parse the GCC asm() instruction */

ST_FUNC void asm_instr(void)
{
	CString astr, *astr1;

	ASMOperand operands[MAX_ASM_OPERANDS];
	int nb_outputs, nb_operands, i, must_subst, out_reg, nb_labels;
	uint8_t clobber_regs[NB_ASM_REGS];
	Section *sec;
	/* since we always generate the asm() instruction, we can ignore
	       volatile */

	while (tok == TOK_VOLATILE1 || tok == TOK_VOLATILE2 || tok == TOK_VOLATILE3
	       || tok == TOK_GOTO) {
		next();
	}

	astr1 = parse_asm_str();
	cstr_new_s(&astr);
	cstr_cat(&astr, astr1->data, astr1->size);

	nb_operands = 0;
	nb_outputs = 0;
	nb_labels = 0;
	must_subst = 0;
	memset(clobber_regs, 0, sizeof(clobber_regs));
	if (tok == ':') {
		next();
		must_subst = 1;
		/* output args */

		parse_asm_operands(operands, &nb_operands, 1);
		nb_outputs = nb_operands;
		if (tok == ':') {
			next();
			if (tok != ')') {
				/* input args */

				parse_asm_operands(operands, &nb_operands, 0);
				if (tok == ':') {
					/* clobber list */
					/* XXX: handle registers */

					next();
					for (;;) {
						if (tok == ':')
							break;
						if (tok != TOK_STR)
							expect("string constant");
						asm_clobber(clobber_regs, tokc.str.data);
						next();
						if (tok == ',') {
							next();
						} else {
							break;
						}
					}
				}
				if (tok == ':') {
					/* goto labels */

					next();
					for (;;) {
						Sym *csym;
						int asmname;
						if (nb_operands + nb_labels >= MAX_ASM_OPERANDS)
							tcc_error("too many asm operands");
						if (tok < TOK_UIDENT)
							expect("label identifier");
						memset(operands + nb_operands + nb_labels, 0,
						       sizeof(operands[0]));
						operands[nb_operands + nb_labels++].id = tok;

						csym = label_find(tok);
						if (!csym) {
							csym = label_push(&global_label_stack, tok,
									  LABEL_FORWARD);
						} else {
							if (csym->r == LABEL_DECLARED)
								csym->r = LABEL_FORWARD;
						}
						next();
						asmname = asm_get_prefix_name(tcc_state, "LG.",
									      ++asmgoto_n);
						if (!csym->c)
							put_extern_sym2(csym, SHN_UNDEF, 0, 0, 1);
						get_asm_sym(asmname, csym);
						operands[nb_operands + nb_labels - 1].is_label = asmname;

						if (tok != ',')
							break;
						next();
					}
				}
			}
		}
	}
	skip(')');
	/* NOTE: we do not eat the ';' so that we can restore the current
	       token after the assembler parsing */

	if (tok != ';')
		expect("';'");
	/* save all values in the memory */

	save_regs(0);
	/* compute constraints */

	asm_compute_constraints(operands, nb_operands, nb_outputs,
				clobber_regs, &out_reg);
	/* substitute the operands in the asm string. No substitution is
	       done if no operands (GCC behaviour) */

	if (must_subst) {
		cstr_reset(astr1);
		cstr_cat(astr1, astr.data, astr.size);
		cstr_reset(&astr);
		subst_asm_operands(operands, nb_operands + nb_labels, &astr, astr1->data);
	}
	/* generate loads */

	asm_gen_code(operands, nb_operands, nb_outputs, 0,
		     clobber_regs, out_reg);
	/* We don't allow switching section within inline asm to
	       bleed out to surrounding code.  */

	sec = cur_text_section;
	/* assemble the string with tcc internal assembler */

	tcc_assemble_inline(tcc_state, astr.data, astr.size - 1, 0);
	cstr_free_s(&astr);
	if (sec != cur_text_section) {
		tcc_warning("inline asm tries to change current section");
		use_section1(tcc_state, sec);
	}
	/* restore the current C token */

	next();
	/* store the output values if needed */

	asm_gen_code(operands, nb_operands, nb_outputs, 1,
		     clobber_regs, out_reg);
	/* free everything */

	for (i=0; i<nb_operands; i++) {
		vpop();
	}

}

ST_FUNC void asm_global_instr(void)
{
	CString *astr;
	int saved_nocode_wanted = nocode_wanted;
	/* Global asm blocks are always emitted.  */

	nocode_wanted = 0;
	next();
	astr = parse_asm_str();
	skip(')');
	/* NOTE: we do not eat the ';' so that we can restore the current
	       token after the assembler parsing */

	if (tok != ';')
		expect("';'");

	cur_text_section = text_section;
	ind = cur_text_section->data_offset;
	/* assemble the string with tcc internal assembler */

	tcc_assemble_inline(tcc_state, astr->data, astr->size - 1, 1);

	cur_text_section->data_offset = ind;
	/* restore the current C token */

	next();

	nocode_wanted = saved_nocode_wanted;
}
/**/
/* CONFIG_TCC_ASM */
/* ==================== tccelf.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) s1->sym
#define TCC_SET_STATE(fn) (tcc_enter_state(s1),fn)
/* Define this to get some debug output during relocation processing.  */

#undef DEBUG_RELOC
/**/
/* global variables */
/* elf version information */

struct sym_version {
	char *lib;
	char *version;
	int out_index;
	int prev_same_lib;
};
/* special flag to indicate that the section should not be linked to the other ones */

#define SHF_PRIVATE 0x80000000
/* section is dynsymtab_section */

#define SHF_DYNSYM 0x40000000

#define shf_RELRO SHF_ALLOC
static const char rdata[] = ".rdata";
/* ------------------------------------------------------------------------- */

ST_FUNC void tccelf_new(TCCState *s)
{
	TCCState *s1 = s;
	/* no section zero */

	dynarray_add(&s->sections, &s->nb_sections, NULL);
	/* create standard sections */

	text_section = new_section(s, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
	data_section = new_section(s, ".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
	/* create ro data section (make ro after relocation done with GNU_RELRO) */

	rodata_section = new_section(s, rdata, SHT_PROGBITS, shf_RELRO);
	bss_section = new_section(s, ".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE);
	common_section = new_section(s, ".common", SHT_NOBITS, SHF_PRIVATE);
	common_section->sh_num = SHN_COMMON;
	/* symbols are always generated for linking stage */

	symtab_section = new_symtab(s, ".symtab", SHT_SYMTAB, 0,
				    ".strtab",
				    ".hashtab", SHF_PRIVATE);
	/* private symbol table for dynamic symbols */

	s->dynsymtab_section = new_symtab(s, ".dynsymtab", SHT_SYMTAB,
					  SHF_PRIVATE|SHF_DYNSYM,
					  ".dynstrtab",
					  ".dynhashtab", SHF_PRIVATE);
	get_sym_attr(s, 0, 1);

	if (s->do_debug) {
		/* add debug sections */

		tcc_debug_new(s);
	}

	/* to make sure that -ltcc1 -Wl,-e,_start will grab the startup code
	   from libtcc1.a (unless _start defined) */

	if (s->elf_entryname)
		set_global_sym(s, s->elf_entryname, NULL, 0); /* SHN_UNDEF */

	/* ndef ELF_OBJ_ONLY */

}

ST_FUNC void free_section(Section *s)
{
	if (!s)
		return;
	tcc_free(s->data);
	s->data = NULL;
	s->data_allocated = s->data_offset = 0;
}

ST_FUNC void tccelf_delete(TCCState *s1)
{
	int i;
	/* free all sections */

	for (i = 1; i < s1->nb_sections; i++)
		free_section(s1->sections[i]);
	dynarray_reset(&s1->sections, &s1->nb_sections);

	for (i = 0; i < s1->nb_priv_sections; i++)
		free_section(s1->priv_sections[i]);
	dynarray_reset(&s1->priv_sections, &s1->nb_priv_sections);

	tcc_free(s1->sym_attrs);
	symtab_section = NULL;/* for tccrun.c:rt_printline() */

}
/* save section data state */

ST_FUNC void tccelf_begin_file(TCCState *s1)
{
	Section *s;
	int i;
	for (i = 1; i < s1->nb_sections; i++) {
		s = s1->sections[i];
		s->sh_offset = s->data_offset;
	}
	/* disable symbol hashing during compilation */

	s = s1->symtab, s->reloc = s->hash, s->hash = NULL;
	s1->uw_sym = 0;
	s1->uw_offs = 0;

}

static void update_relocs(TCCState *s1, Section *s, int *old_to_new_syms,
			  int first_sym);
/* At the end of compilation, convert any UNDEF syms to global, and merge
   with previously existing symbols */

ST_FUNC void tccelf_end_file(TCCState *s1)
{
	Section *s = s1->symtab;
	int first_sym, nb_syms, *tr, i;

	first_sym = s->sh_offset / sizeof (ElfSym);
	nb_syms = s->data_offset / sizeof (ElfSym) - first_sym;
	s->data_offset = s->sh_offset;
	s->link->data_offset = s->link->sh_offset;
	s->hash = s->reloc, s->reloc = NULL;
	tr = tcc_mallocz(nb_syms * sizeof *tr);

	for (i = 0; i < nb_syms; ++i) {
		ElfSym *sym = (ElfSym *)s->data + first_sym + i;
		if (sym->st_shndx == SHN_UNDEF) {
			int sym_bind = ELFW(ST_BIND)(sym->st_info);
			int sym_type = ELFW(ST_TYPE)(sym->st_info);
			if (sym_bind == STB_LOCAL)
				sym_bind = STB_GLOBAL;

			sym->st_info = ELFW(ST_INFO)(sym_bind, sym_type);
		}
		tr[i] = set_elf_sym(s, sym->st_value, sym->st_size, sym->st_info,
				    sym->st_other, sym->st_shndx, (char *)s->link->data + sym->st_name);
	}
	/* now update relocations */

	update_relocs(s1, s, tr, first_sym);
	tcc_free(tr);
	/* record text/data/bss output for -bench info */

	for (i = 0; i < 4; ++i) {
		s = s1->sections[i + 1];
		s1->total_output[i] += s->data_offset - s->sh_offset;
	}
}

ST_FUNC Section *new_section(TCCState *s1, const char *name, int sh_type,
			     int sh_flags)
{
	Section *sec;

	sec = tcc_mallocz(sizeof(Section) + strlen(name));
	sec->s1 = s1;
	strcpy(sec->name, name);
	sec->sh_type = sh_type;
	sec->sh_flags = sh_flags;
	switch (sh_type) {
	case SHT_GNU_versym:
		sec->sh_addralign = 2;
		break;
	case SHT_HASH:
	case SHT_GNU_HASH:
	case SHT_REL:
	case SHT_RELA:
	case SHT_DYNSYM:
	case SHT_SYMTAB:
	case SHT_DYNAMIC:
	case SHT_GNU_verneed:
	case SHT_GNU_verdef:
		sec->sh_addralign = PTR_SIZE;
		break;
	case SHT_STRTAB:
		sec->sh_addralign = 1;
		break;
	default:
		sec->sh_addralign = PTR_SIZE;/* gcc/pcc default alignment */

		break;
	}

	if (sh_flags & SHF_PRIVATE) {
		dynarray_add(&s1->priv_sections, &s1->nb_priv_sections, sec);
	} else {
		sec->sh_num = s1->nb_sections;
		dynarray_add(&s1->sections, &s1->nb_sections, sec);
	}

	return sec;
}

ST_FUNC void init_symtab(Section *s)
{
	int *ptr, nb_buckets = 1;
	put_elf_str(s->link, "");
	section_ptr_add(s, sizeof (ElfW(Sym)));
	ptr = section_ptr_add(s->hash, (2 + nb_buckets + 1) * sizeof(int));
	ptr[0] = nb_buckets;
	ptr[1] = 1;
	memset(ptr + 2, 0, (nb_buckets + 1) * sizeof(int));
}

ST_FUNC Section *new_symtab(TCCState *s1,
			    const char *symtab_name, int sh_type, int sh_flags,
			    const char *strtab_name,
			    const char *hash_name, int hash_sh_flags)
{
	Section *symtab, *strtab, *hash;
	symtab = new_section(s1, symtab_name, sh_type, sh_flags);
	symtab->sh_entsize = sizeof(ElfW(Sym));
	strtab = new_section(s1, strtab_name, SHT_STRTAB, sh_flags);
	symtab->link = strtab;
	hash = new_section(s1, hash_name, SHT_HASH, hash_sh_flags);
	hash->sh_entsize = sizeof(int);
	symtab->hash = hash;
	hash->link = symtab;
	init_symtab(symtab);
	return symtab;
}
/* realloc section and set its content to zero */

ST_FUNC void section_realloc(Section *sec, unsigned long new_size)
{
	unsigned long size;
	unsigned char *data;

	size = sec->data_allocated;
	if (size == 0)
		size = 1;
	while (size < new_size)
		size = size * 2;
	data = tcc_realloc(sec->data, size);
	memset(data + sec->data_allocated, 0, size - sec->data_allocated);
	sec->data = data;
	sec->data_allocated = size;
}
/* reserve at least 'size' bytes aligned per 'align' in section
   'sec' from current offset, and return the aligned offset */

ST_FUNC size_t section_add(Section *sec, addr_t size, int align)
{
	size_t offset, offset1;

	offset = (sec->data_offset + align - 1) & -align;
	offset1 = offset + size;
	if (sec->sh_type != SHT_NOBITS && offset1 > sec->data_allocated)
		section_realloc(sec, offset1);
	sec->data_offset = offset1;
	if (align > sec->sh_addralign)
		sec->sh_addralign = align;
	return offset;
}
/* reserve at least 'size' bytes in section 'sec' from
   sec->data_offset. */

ST_FUNC void *section_ptr_add(Section *sec, addr_t size)
{
	size_t offset = section_add(sec, size, 1);
	return sec->data + offset;
}

static Section *have_section(TCCState *s1, const char *name)
{
	Section *sec;
	int i;
	for (i = 1; i < s1->nb_sections; i++) {
		sec = s1->sections[i];
		if (!strcmp(name, sec->name))
			return sec;
	}
	return NULL;
}
/* return a reference to a section, and create it if it does not
   exists */

ST_FUNC Section *find_section(TCCState *s1, const char *name)
{
	Section *sec = have_section(s1, name);
	if (sec)
		return sec;
	/* sections are created as PROGBITS */

	return new_section(s1, name, SHT_PROGBITS, SHF_ALLOC);
}
/* ------------------------------------------------------------------------- */

ST_FUNC int put_elf_str(Section *s, const char *sym)
{
	int offset, len;
	char *ptr;

	len = strlen(sym) + 1;
	offset = s->data_offset;
	ptr = section_ptr_add(s, len);
	memmove(ptr, sym, len);
	return offset;
}
/* elf symbol hashing function */

static ElfW(Word) elf_hash(const unsigned char *name)
{
	ElfW(Word) h = 0, g;

	while (*name) {
		h = (h << 4) + *name++;
		g = h & 0xf0000000;
		if (g)
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}
/* rebuild hash table of section s */
/* NOTE: we do factorize the hash table code to go faster */

static void rebuild_hash(Section *s, unsigned int nb_buckets)
{
	ElfW(Sym) *sym;
	int *ptr, *hash, nb_syms, sym_index, h;
	unsigned char *strtab;

	strtab = s->link->data;
	nb_syms = s->data_offset / sizeof(ElfW(Sym));

	if (!nb_buckets)
		nb_buckets = ((int *)s->hash->data)[0];

	s->hash->data_offset = 0;
	ptr = section_ptr_add(s->hash, (2 + nb_buckets + nb_syms) * sizeof(int));
	ptr[0] = nb_buckets;
	ptr[1] = nb_syms;
	ptr += 2;
	hash = ptr;
	memset(hash, 0, (nb_buckets + 1) * sizeof(int));
	ptr += nb_buckets + 1;

	sym = (ElfW(Sym) *)s->data + 1;
	for (sym_index = 1; sym_index < nb_syms; sym_index++) {
		if (ELFW(ST_BIND)(sym->st_info) != STB_LOCAL) {
			h = elf_hash(strtab + sym->st_name) % nb_buckets;
			*ptr = hash[h];
			hash[h] = sym_index;
		} else {
			*ptr = 0;
		}
		ptr++;
		sym++;
	}
}
/* return the symbol number */

ST_FUNC int put_elf_sym(Section *s, addr_t value, unsigned long size,
			int info, int other, int shndx, const char *name)
{
	int name_offset, sym_index;
	int nbuckets, h;
	ElfW(Sym) *sym;
	Section *hs;

	sym = section_ptr_add(s, sizeof(ElfW(Sym)));
	if (name && name[0])
		name_offset = put_elf_str(s->link, name);
	else
		name_offset = 0;
	/* XXX: endianness */

	sym->st_name = name_offset;
	sym->st_value = value;
	sym->st_size = size;
	sym->st_info = info;
	sym->st_other = other;
	sym->st_shndx = shndx;
	sym_index = sym - (ElfW(Sym) *)s->data;
	hs = s->hash;
	if (hs) {
		int *ptr, *base;
		ptr = section_ptr_add(hs, sizeof(int));
		base = (int *)hs->data;
		/* only add global or weak symbols. */

		if (ELFW(ST_BIND)(info) != STB_LOCAL) {
			/* add another hashing entry */

			nbuckets = base[0];
			h = elf_hash((unsigned char *)s->link->data + name_offset) % nbuckets;
			*ptr = base[2 + h];
			base[2 + h] = sym_index;
			base[1]++;
			/* we resize the hash table */

			hs->nb_hashed_syms++;
			if (hs->nb_hashed_syms > 2 * nbuckets) {
				rebuild_hash(s, 2 * nbuckets);
			}
		} else {
			*ptr = 0;
			base[1]++;
		}
	}
	return sym_index;
}

ST_FUNC int find_elf_sym(Section *s, const char *name)
{
	ElfW(Sym) *sym;
	Section *hs;
	int nbuckets, sym_index, h;
	const char *name1;

	hs = s->hash;
	if (!hs)
		return 0;
	nbuckets = ((int *)hs->data)[0];
	h = elf_hash((unsigned char *) name) % nbuckets;
	sym_index = ((int *)hs->data)[2 + h];
	while (sym_index != 0) {
		sym = &((ElfW(Sym) *)s->data)[sym_index];
		name1 = (char *) s->link->data + sym->st_name;
		if (!strcmp(name, name1))
			return sym_index;
		sym_index = ((int *)hs->data)[2 + nbuckets + sym_index];
	}
	return 0;
}
/* return elf symbol value, signal error if 'err' is nonzero, decorate
   name if FORC */

ST_FUNC addr_t get_sym_addr(TCCState *s1, const char *name, int err, int forc)
{
	int sym_index;
	ElfW(Sym) *sym;
	char buf[256];
	if (forc && s1->leading_underscore

	    /* win32-32bit stdcall symbols always have _ already */

	    && !strchr(name, '@')

	   ) {
		buf[0] = '_';
		pstrcpy(buf + 1, sizeof(buf) - 1, name);
		name = buf;
	}
	sym_index = find_elf_sym(s1->symtab, name);
	sym = &((ElfW(Sym) *)s1->symtab->data)[sym_index];
	if (!sym_index || sym->st_shndx == SHN_UNDEF) {
		if (err)
			tcc_error_noabort("%s not defined", name);
		return (addr_t)-1;
	}
	return sym->st_value;
}
/* return elf symbol value */

LIBTCCAPI void *tcc_get_symbol(TCCState *s, const char *name)
{
	addr_t addr = get_sym_addr(s, name, 0, 1);
	return addr == -1 ? NULL : (void *)(uintptr_t)addr;
}

LIBTCCAPI int tcc_add_symbol(TCCState *s1, const char *name, const void *val)
{

	/* On x86_64 'val' might not be reachable with a 32bit offset.
	   So it is handled here as if it were in a DLL. */

	pe_putimport(s1, 0, name, (uintptr_t)val);

	return 0;
}
/* list elf symbol names and values */

ST_FUNC void list_elf_symbols(TCCState *s, void *ctx,
			      void (*symbol_cb)(void *ctx, const char *name, const void *val))
{
	ElfW(Sym) *sym;
	Section *symtab;
	int sym_index, end_sym;
	const char *name;
	unsigned char sym_vis, sym_bind;

	symtab = s->symtab;
	end_sym = symtab->data_offset / sizeof (ElfSym);
	for (sym_index = 0; sym_index < end_sym; ++sym_index) {
		sym = &((ElfW(Sym) *)symtab->data)[sym_index];
		if (sym->st_value) {
			name = (char *) symtab->link->data + sym->st_name;
			sym_bind = ELFW(ST_BIND)(sym->st_info);
			sym_vis = ELFW(ST_VISIBILITY)(sym->st_other);
			if (sym_bind == STB_GLOBAL && sym_vis == STV_DEFAULT)
				symbol_cb(ctx, name, (void *)(uintptr_t)sym->st_value);
		}
	}
}
/* list elf symbol names and values */

LIBTCCAPI void tcc_list_symbols(TCCState *s, void *ctx,
				void (*symbol_cb)(void *ctx, const char *name, const void *val))
{
	list_elf_symbols(s, ctx, symbol_cb);
}
/* ndef ELF_OBJ_ONLY */
/* add an elf symbol : check if it is already defined and patch
   it. Return symbol index. NOTE that sh_num can be SHN_UNDEF. */

ST_FUNC int set_elf_sym(Section *s, addr_t value, unsigned long size,
			int info, int other, int shndx, const char *name)
{
	TCCState *s1 = s->s1;
	ElfW(Sym) *esym;
	int sym_bind, sym_index, sym_type, esym_bind;
	unsigned char sym_vis, esym_vis, new_vis;

	sym_bind = ELFW(ST_BIND)(info);
	sym_type = ELFW(ST_TYPE)(info);
	sym_vis = ELFW(ST_VISIBILITY)(other);

	if (sym_bind != STB_LOCAL) {
		/* we search global or weak symbols */

		sym_index = find_elf_sym(s, name);
		if (!sym_index)
			goto do_def;
		esym = &((ElfW(Sym) *)s->data)[sym_index];
		if (esym->st_value == value && esym->st_size == size && esym->st_info == info
		    && esym->st_other == other && esym->st_shndx == shndx)
			return sym_index;
		if (esym->st_shndx != SHN_UNDEF) {
			esym_bind = ELFW(ST_BIND)(esym->st_info);
			/* propagate the most constraining visibility */
			/* STV_DEFAULT(0)<STV_PROTECTED(3)<STV_HIDDEN(2)<STV_INTERNAL(1) */

			esym_vis = ELFW(ST_VISIBILITY)(esym->st_other);
			if (esym_vis == STV_DEFAULT) {
				new_vis = sym_vis;
			} else if (sym_vis == STV_DEFAULT) {
				new_vis = esym_vis;
			} else {
				new_vis = (esym_vis < sym_vis) ? esym_vis : sym_vis;
			}
			esym->st_other = (esym->st_other & ~ELFW(ST_VISIBILITY)(-1))
					 | new_vis;
			if (shndx == SHN_UNDEF) {
				/* ignore adding of undefined symbol if the
				                   corresponding symbol is already defined */

			} else if (sym_bind == STB_GLOBAL && esym_bind == STB_WEAK) {
				/* global overrides weak, so patch */

				goto do_patch;
			} else if (sym_bind == STB_WEAK && esym_bind == STB_GLOBAL) {
				/* weak is ignored if already global */

			} else if (sym_bind == STB_WEAK && esym_bind == STB_WEAK) {
				/* keep first-found weak definition, ignore subsequents */

			} else if (sym_vis == STV_HIDDEN || sym_vis == STV_INTERNAL) {
				/* ignore hidden symbols after */

			} else if (s->sh_flags & SHF_DYNSYM) {
				/* we accept that two DLL define the same symbol */

			} else if ((esym->st_shndx == SHN_COMMON
				    || esym->st_shndx == bss_section->sh_num)
				   && (shndx < SHN_LORESERVE
				       && shndx != bss_section->sh_num)) {
				/* data symbol gets precedence over common/bss */

				goto do_patch;
			} else if (shndx == SHN_COMMON || shndx == bss_section->sh_num) {
				/* data symbol keeps precedence over common/bss */

			} else if (esym->st_other & ST_ASM_SET) {
				/* If the existing symbol came from an asm .set
						   we can override.  */

				goto do_patch;
			} else {

				tcc_error_noabort("link symbol '%s' defined twice", name);
			}
		} else {
			esym->st_other = other;
do_patch:
			esym->st_info = ELFW(ST_INFO)(sym_bind, sym_type);
			esym->st_shndx = shndx;
			esym->st_value = value;
			esym->st_size = size;
		}
	} else {
do_def:
		sym_index = put_elf_sym(s, value, size,
					ELFW(ST_INFO)(sym_bind, sym_type), other,
					shndx, name);
	}
	return sym_index;
}
/* put relocation */

ST_FUNC void put_elf_reloca(Section *symtab, Section *s, unsigned long offset,
			    int type, int symbol, addr_t addend)
{
	TCCState *s1 = s->s1;
	char buf[256];
	Section *sr;
	ElfW_Rel *rel;

	sr = s->reloc;
	if (!sr) {
		/* if no relocation section, create it */

		snprintf(buf, sizeof(buf), REL_SECTION_FMT, s->name);
		/* if the symtab is allocated, then we consider the relocation
		           are also */

		sr = new_section(s->s1, buf, SHT_RELX, symtab->sh_flags);
		sr->sh_entsize = sizeof(ElfW_Rel);
		sr->link = symtab;
		sr->sh_info = s->sh_num;
		s->reloc = sr;
	}
	rel = section_ptr_add(sr, sizeof(ElfW_Rel));
	rel->r_offset = offset;
	rel->r_info = ELFW(R_INFO)(symbol, type);

	rel->r_addend = addend;

	if (SHT_RELX != SHT_RELA && addend)
		tcc_error_noabort("non-zero addend on REL architecture");
}

ST_FUNC void put_elf_reloc(Section *symtab, Section *s, unsigned long offset,
			   int type, int symbol)
{
	put_elf_reloca(symtab, s, offset, type, symbol, 0);
}

ST_FUNC struct sym_attr *get_sym_attr(TCCState *s1, int index, int alloc)
{
	int n;
	struct sym_attr *tab;

	if (index >= s1->nb_sym_attrs) {
		if (!alloc)
			return s1->sym_attrs;
		/* find immediately bigger power of 2 and reallocate array */

		n = 1;
		while (index >= n)
			n *= 2;
		tab = tcc_realloc(s1->sym_attrs, n * sizeof(*s1->sym_attrs));
		s1->sym_attrs = tab;
		memset(s1->sym_attrs + s1->nb_sym_attrs, 0,
		       (n - s1->nb_sym_attrs) * sizeof(*s1->sym_attrs));
		s1->nb_sym_attrs = n;
	}
	return &s1->sym_attrs[index];
}

static void update_relocs(TCCState *s1, Section *s, int *old_to_new_syms,
			  int first_sym)
{
	int i, type, sym_index;
	Section *sr;
	ElfW_Rel *rel;

	for (i = 1; i < s1->nb_sections; i++) {
		sr = s1->sections[i];
		if (sr->sh_type == SHT_RELX && sr->link == s) {
			for_each_elem(sr, 0, rel, ElfW_Rel) {
				sym_index = ELFW(R_SYM)(rel->r_info);
				type = ELFW(R_TYPE)(rel->r_info);
				if ((sym_index -= first_sym) < 0)
					continue;/* zero sym_index in reloc (can happen with asm) */

				sym_index = old_to_new_syms[sym_index];
				rel->r_info = ELFW(R_INFO)(sym_index, type);
			}
		}
	}
}
/* In an ELF file symbol table, the local symbols must appear below
   the global and weak ones. Since TCC cannot sort it while generating
   the code, we must do it after. All the relocation tables are also
   modified to take into account the symbol table sorting */

static void sort_syms(TCCState *s1, Section *s)
{
	int *old_to_new_syms;
	ElfW(Sym) *new_syms;
	int nb_syms, i;
	ElfW(Sym) *p, *q;

	nb_syms = s->data_offset / sizeof(ElfW(Sym));
	new_syms = tcc_malloc(nb_syms * sizeof(ElfW(Sym)));
	old_to_new_syms = tcc_malloc(nb_syms * sizeof(int));
	/* first pass for local symbols */

	p = (ElfW(Sym) *)s->data;
	q = new_syms;
	for (i = 0; i < nb_syms; i++) {
		if (ELFW(ST_BIND)(p->st_info) == STB_LOCAL) {
			old_to_new_syms[i] = q - new_syms;
			*q++ = *p;
		}
		p++;
	}
	/* save the number of local symbols in section header */

	if ( s->sh_size ) /* this 'if' makes IDA happy */

		s->sh_info = q - new_syms;
	/* then second pass for non local symbols */

	p = (ElfW(Sym) *)s->data;
	for (i = 0; i < nb_syms; i++) {
		if (ELFW(ST_BIND)(p->st_info) != STB_LOCAL) {
			old_to_new_syms[i] = q - new_syms;
			*q++ = *p;
		}
		p++;
	}
	/* we copy the new symbols to the old */

	memcpy(s->data, new_syms, nb_syms * sizeof(ElfW(Sym)));
	tcc_free(new_syms);

	update_relocs(s1, s, old_to_new_syms, 0);
	tcc_free(old_to_new_syms);
}
/* relocate symbol table, resolve undefined symbols if do_resolve is
   true and output error if undefined symbol. */

ST_FUNC void relocate_syms(TCCState *s1, Section *symtab, int do_resolve)
{
	ElfW(Sym) *sym;
	int sym_bind, sh_num;
	const char *name;

	for_each_elem(symtab, 1, sym, ElfW(Sym)) {
		sh_num = sym->st_shndx;
		if (sh_num == SHN_UNDEF) {
			if (do_resolve == 2)/* relocating dynsym */

				continue;
			name = (char *) s1->symtab->link->data + sym->st_name;
			/* Use ld.so to resolve symbol for us (for tcc -run) */

			if (do_resolve) {
				/* if dynamic symbol exist, it will be used in relocate_section */

			} else if (s1->dynsym && find_elf_sym(s1->dynsym, name))
				goto found;
			/* XXX: _fp_hw seems to be part of the ABI, so we ignore
			               it */

			if (!strcmp(name, "_fp_hw"))
				goto found;
			/* only weak symbols are accepted to be undefined. Their
			               value is zero */

			sym_bind = ELFW(ST_BIND)(sym->st_info);
			if (sym_bind == STB_WEAK)
				sym->st_value = 0;
			else
				tcc_error_noabort("unresolved reference to '%s'", name);

		} else if (sh_num < SHN_LORESERVE) {
			/* add section base */

			sym->st_value += s1->sections[sym->st_shndx]->sh_addr;
		}
found: ;
	}
}
/* relocate a given section (CPU dependent) by applying the relocations
   in the associated relocation section */

static void relocate_section(TCCState *s1, Section *s, Section *sr)
{
	ElfW_Rel *rel;
	ElfW(Sym) *sym;
	int type, sym_index;
	unsigned char *ptr;
	addr_t tgt, addr;
	int is_dwarf = s->sh_num >= s1->dwlo && s->sh_num < s1->dwhi;

	qrel = (ElfW_Rel *)sr->data;
	for_each_elem(sr, 0, rel, ElfW_Rel) {
		if (s->data == NULL)/* bss */

			continue;
		ptr = s->data + rel->r_offset;
		sym_index = ELFW(R_SYM)(rel->r_info);
		sym = &((ElfW(Sym) *)symtab_section->data)[sym_index];
		type = ELFW(R_TYPE)(rel->r_info);
		tgt = sym->st_value;

		tgt += rel->r_addend;

		if (is_dwarf && type == R_DATA_32DW
		    && sym->st_shndx >= s1->dwlo && sym->st_shndx < s1->dwhi) {
			/* dwarf section relocation to each other */

			add32le(ptr, tgt - s1->sections[sym->st_shndx]->sh_addr);
			continue;
		}
		addr = s->sh_addr + rel->r_offset;
		relocate(s1, rel, type, ptr, addr, tgt);
	}

}
/* relocate all sections */

ST_FUNC void relocate_sections(TCCState *s1)
{
	int i;
	Section *s, *sr;

	for (i = 1; i < s1->nb_sections; ++i) {
		sr = s1->sections[i];
		if (sr->sh_type != SHT_RELX)
			continue;
		s = s1->sections[sr->sh_info];

		if (s != s1->got
		    || s1->static_link
		    || s1->output_type == TCC_OUTPUT_MEMORY)

		{
			relocate_section(s1, s, sr);
		}

	}
}

static int build_got(TCCState *s1)
{
	/* if no got, then create it */

	s1->got = new_section(s1, ".got", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
	s1->got->sh_entsize = 4;
	/* keep space for _DYNAMIC pointer and two dummy got entries */

	section_ptr_add(s1->got, 3 * PTR_SIZE);
	return set_elf_sym(symtab_section, 0, 0, ELFW(ST_INFO)(STB_GLOBAL, STT_OBJECT),
			   0, s1->got->sh_num, "_GLOBAL_OFFSET_TABLE_");
}
/* Create a GOT and (for function call) a PLT entry corresponding to a symbol
   in s1->symtab. When creating the dynamic symbol table entry for the GOT
   relocation, use 'size' and 'info' for the corresponding symbol metadata.
   Returns the offset of the GOT or (if any) PLT entry. */

static struct sym_attr *put_got_entry(TCCState *s1, int dyn_reloc_type,
				      int sym_index)
{
	int need_plt_entry;
	const char *name;
	ElfW(Sym) *sym;
	struct sym_attr *attr;
	unsigned got_offset;
	char plt_name[200];
	int len;
	Section *s_rel;

	need_plt_entry = (dyn_reloc_type == R_JMP_SLOT);
	attr = get_sym_attr(s1, sym_index, 1);
	/* In case a function is both called and its address taken 2 GOT entries
	       are created, one for taking the address (GOT) and the other for the PLT
	       entry (PLTGOT).  */

	if (need_plt_entry ? attr->plt_offset : attr->got_offset)
		return attr;

	s_rel = s1->got;
	if (need_plt_entry) {
		if (!s1->plt) {
			s1->plt = new_section(s1, ".plt", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
			s1->plt->sh_entsize = 4;
		}
		s_rel = s1->plt;
	}
	/* create the GOT entry */

	got_offset = s1->got->data_offset;
	section_ptr_add(s1->got, PTR_SIZE);
	/* Create the GOT relocation that will insert the address of the object or
	       function of interest in the GOT entry. This is a static relocation for
	       memory output (dlsym will give us the address of symbols) and dynamic
	       relocation otherwise (executable and DLLs). The relocation should be
	       done lazily for GOT entry with *_JUMP_SLOT relocation type (the one
	       associated to a PLT entry) but is currently done at load time for an
	       unknown reason. */
	sym = &((ElfW(Sym) *) symtab_section->data)[sym_index];
	name = (char *) symtab_section->link->data + sym->st_name;
//printf("sym %d %s\n", need_plt_entry, name);

	if (s1->dynsym) {
		if (ELFW(ST_BIND)(sym->st_info) == STB_LOCAL) {
			/* Hack alarm.  We don't want to emit dynamic symbols
				       and symbol based relocs for STB_LOCAL symbols, but rather
				       want to resolve them directly.  At this point the symbol
				       values aren't final yet, so we must defer this.  We will later
				       have to create a RELATIVE reloc anyway, so we misuse the
				       relocation slot to smuggle the symbol reference until
				       fill_local_got_entries.  Not that the sym_index is
				       relative to symtab_section, not s1->dynsym!  Nevertheless
				       we use s1->dyn_sym so that if this is the first call
				       that got->reloc is correctly created.  Also note that
				       RELATIVE relocs are not normally created for the .got,
				       so the types serves as a marker for later (and is retained
				       also for the final output, which is okay because then the
				       got is just normal data).  */
			put_elf_reloc(s1->dynsym, s1->got, got_offset, R_RELATIVE,
				      sym_index);
		} else {
			if (0 == attr->dyn_index)
				attr->dyn_index = set_elf_sym(s1->dynsym, sym->st_value,
							      sym->st_size, sym->st_info, 0,
							      sym->st_shndx, name);
			put_elf_reloc(s1->dynsym, s_rel, got_offset, dyn_reloc_type,
				      attr->dyn_index);
		}
	} else {
		put_elf_reloc(symtab_section, s1->got, got_offset, dyn_reloc_type,
			      sym_index);
	}

	if (need_plt_entry) {
		attr->plt_offset = create_plt_entry(s1, got_offset, attr);
		/* create a symbol 'sym@plt' for the PLT jump vector */

		len = strlen(name);
		if (len > sizeof plt_name - 5)
			len = sizeof plt_name - 5;
		memcpy(plt_name, name, len);
		strcpy(plt_name + len, "@plt");
		attr->plt_sym = put_elf_sym(s1->symtab, attr->plt_offset, 0,
					    ELFW(ST_INFO)(STB_GLOBAL, STT_FUNC), 0, s1->plt->sh_num, plt_name);
	} else {
		attr->got_offset = got_offset;
	}

	return attr;
}
/* build GOT and PLT entries */
/* Two passes because R_JMP_SLOT should become first. Some targets
   (arm, arm64) do not allow mixing R_JMP_SLOT and R_GLOB_DAT. */

ST_FUNC void build_got_entries(TCCState *s1, int got_sym)
{
	Section *s;
	ElfW_Rel *rel;
	ElfW(Sym) *sym;
	int i, type, gotplt_entry, reloc_type, sym_index;
	struct sym_attr *attr;
	int pass = 0;
redo:
	for (i = 1; i < s1->nb_sections; i++) {
		s = s1->sections[i];
		if (s->sh_type != SHT_RELX)
			continue;
		/* no need to handle got relocations */

		if (s->link != symtab_section)
			continue;
		for_each_elem(s, 0, rel, ElfW_Rel) {
			type = ELFW(R_TYPE)(rel->r_info);
			gotplt_entry = gotplt_entry_type(type);
			if (gotplt_entry == -1) {
				tcc_error_noabort ("Unknown relocation type for got: %d", type);
				continue;
			}
			sym_index = ELFW(R_SYM)(rel->r_info);
			sym = &((ElfW(Sym) *)symtab_section->data)[sym_index];

			if (gotplt_entry == NO_GOTPLT_ENTRY) {
				continue;
			}
			/* Automatically create PLT/GOT [entry] if it is an undefined
				       reference (resolved at runtime), or the symbol is absolute,
				       probably created by tcc_add_symbol, and thus on 64-bit
				       targets might be too far from application code.  */

			if (gotplt_entry == AUTO_GOTPLT_ENTRY) {
				if (sym->st_shndx == SHN_UNDEF) {
					ElfW(Sym) *esym;
					int dynindex;
					if (!PCRELATIVE_DLLPLT
					    && (s1->output_type & TCC_OUTPUT_DYN))
						continue;
					/* Relocations for UNDEF symbols would normally need
							       to be transferred into the executable or shared object.
							       If that were done AUTO_GOTPLT_ENTRY wouldn't exist.
							       But TCC doesn't do that (at least for exes), so we
							       need to resolve all such relocs locally.  And that
							       means PLT slots for functions in DLLs and COPY relocs for
							       data symbols.  COPY relocs were generated in
							       bind_exe_dynsyms (and the symbol adjusted to be defined),
							       and for functions we were generated a dynamic symbol
							       of function type.  */
					if (s1->dynsym) {
						/* dynsym isn't set for -run :-/  */

						dynindex = get_sym_attr(s1, sym_index, 0)->dyn_index;
						esym = (ElfW(Sym) *)s1->dynsym->data + dynindex;
						if (dynindex
						    && (ELFW(ST_TYPE)(esym->st_info) == STT_FUNC
							|| (ELFW(ST_TYPE)(esym->st_info) == STT_NOTYPE
							    && ELFW(ST_TYPE)(sym->st_info) == STT_FUNC)))
							goto jmp_slot;
					}
				} else if (sym->st_shndx == SHN_ABS) {
					if (sym->st_value == 0)/* from tcc_add_btstub() */

						continue;

					if (PTR_SIZE != 8)
						continue;
					/* from tcc_add_symbol(): on 64 bit platforms these
					                       need to go through .got */

				} else
					continue;
			}
			if ((type == R_X86_64_PLT32 || type == R_X86_64_PC32) &&
			    sym->st_shndx != SHN_UNDEF &&
			    (ELFW(ST_VISIBILITY)(sym->st_other) != STV_DEFAULT ||
			     ELFW(ST_BIND)(sym->st_info) == STB_LOCAL ||
			     s1->output_type & TCC_OUTPUT_EXE)) {
				if (pass != 0)
					continue;
				rel->r_info = ELFW(R_INFO)(sym_index, R_X86_64_PC32);
				continue;
			}

			reloc_type = code_reloc(type);
			if (reloc_type == -1) {
				tcc_error_noabort ("Unknown relocation type: %d", type);
				continue;
			}

			if (reloc_type != 0) {
jmp_slot:
				if (pass != 0)
					continue;
				reloc_type = R_JMP_SLOT;
			} else {
				if (pass != 1)
					continue;
				reloc_type = R_GLOB_DAT;
			}

			if (!s1->got)
				got_sym = build_got(s1);

			if (gotplt_entry == BUILD_GOT_ONLY)
				continue;

			attr = put_got_entry(s1, reloc_type, sym_index);

			if (reloc_type == R_JMP_SLOT)
				rel->r_info = ELFW(R_INFO)(attr->plt_sym, type);
		}
	}
	if (++pass < 2)
		goto redo;
	/* .rel.plt refers to .got actually */

	if (s1->plt && s1->plt->reloc)
		s1->plt->reloc->sh_info = s1->got->sh_num;
	if (got_sym)/* set size */

		((ElfW(Sym) *)symtab_section->data)[got_sym].st_size = s1->got->data_offset;
}

ST_FUNC int set_global_sym(TCCState *s1, const char *name, Section *sec,
			   addr_t offs)
{
	int shn = sec ? sec->sh_num : offs || !name ? SHN_ABS : SHN_UNDEF;
	if (sec && offs == -1)
		offs = sec->data_offset;
	return set_elf_sym(symtab_section, offs, 0,
			   ELFW(ST_INFO)(name ? STB_GLOBAL : STB_LOCAL, STT_NOTYPE), 0, shn, name);
}

static void add_init_array_defines(TCCState *s1, const char *section_name)
{
	Section *s;
	addr_t end_offset;
	char buf[1024];
	s = have_section(s1, section_name);
	if (!s || !(s->sh_flags & SHF_ALLOC)) {
		end_offset = 0;
		s = text_section;
	} else {
		end_offset = s->data_offset;
	}
	snprintf(buf, sizeof(buf), "__%s_start", section_name + 1);
	set_global_sym(s1, buf, s, 0);
	snprintf(buf, sizeof(buf), "__%s_end", section_name + 1);
	set_global_sym(s1, buf, s, end_offset);
}

ST_FUNC void add_array (TCCState *s1, const char *sec, int c)
{
	Section *s;
	s = find_section(s1, sec);
	s->sh_flags = shf_RELRO;
	s->sh_type = sec[1] == 'i' ? SHT_INIT_ARRAY : SHT_FINI_ARRAY;
	put_elf_reloc (s1->symtab, s, s->data_offset, R_DATA_PTR, c);
	section_ptr_add(s, PTR_SIZE);
}
/* set symbol to STB_LOCAL and resolve. The point is to not export it as
   a dynamic symbol to allow so's to have one each with a different value. */
static void set_local_sym(TCCState *s1, const char *name, Section *s,
			  int offset)
{
	int c = find_elf_sym(s1->symtab, name);
	if (c) {
		ElfW(Sym) *esym = (ElfW(Sym) *)s1->symtab->data + c;
		esym->st_info = ELFW(ST_INFO)(STB_LOCAL, STT_NOTYPE);
		esym->st_value = offset;
		esym->st_shndx = s->sh_num;
	}
}
/* avoid generating debug/test_coverage code for stub functions */

static void tcc_compile_string_no_debug(TCCState *s, const char *str)
{
	int save_do_debug = s->do_debug;
	int save_test_coverage = s->test_coverage;

	s->do_debug = 0;
	s->test_coverage = 0;
	tcc_compile_string(s, str);
	s->do_debug = save_do_debug;
	s->test_coverage = save_test_coverage;
}

static void tcc_tcov_add_file(TCCState *s1, const char *filename)
{
	CString cstr;
	void *ptr;
	char wd[1024];

	if (tcov_section == NULL)
		return;
	section_ptr_add(tcov_section, 1);
	write32le (tcov_section->data, tcov_section->data_offset);

	cstr_new (&cstr);
	if (filename[0] == '/')
		cstr_printf (&cstr, "%s.tcov", filename);
	else {
		getcwd (wd, sizeof(wd));
		cstr_printf (&cstr, "%s/%s.tcov", wd, filename);
	}
	ptr = section_ptr_add(tcov_section, cstr.size + 1);
	strcpy((char *)ptr, cstr.data);
	unlink((char *)ptr);

	normalize_slashes((char *)ptr);

	cstr_free (&cstr);

	cstr_new(&cstr);
	cstr_printf(&cstr,
		    "extern char *__tcov_data[];"
		    "extern void __store_test_coverage ();"
		    "__attribute__((destructor)) static void __tcov_exit() {"
		    "__store_test_coverage(__tcov_data);"
		    "}");
	tcc_compile_string_no_debug(s1, cstr.data);
	cstr_free(&cstr);
	set_local_sym(s1, &"___tcov_data"[!s1->leading_underscore], tcov_section, 0);
}
/* ndef TCC_TARGET_PE */
/* set _etext/_edata/_end  f=0:set  f=1:when_needed  f=2,3:just_update */

static void set_linker_sym(TCCState *s1, const char *name, Section *sec, int f)
{
	int sym_index, esym_index, defined;
	ElfW(Sym) *sym, *esym;
	sym_index = find_elf_sym(symtab_section, name);
	sym = (ElfW(Sym) *)symtab_section->data + sym_index;
	esym_index = find_elf_sym(s1->dynsymtab_section, name);
	esym = (ElfW(Sym) *)s1->dynsymtab_section->data + esym_index;
	defined = sym->st_shndx != SHN_UNDEF
		  || (esym->st_shndx != SHN_UNDEF && esym->st_size);
	switch (f) {
	case 1:/* old symbols w/o '_' */

		if (!(sym_index || esym_index) || defined)
			break;
	case 0:
		if (defined) {
			tcc_warning("linker symbol '%s' already defined", name);
			break;
		}
		sym_index = set_global_sym(s1, name, sec, -1);
		get_sym_attr(s1, sym_index, 1)->linker_sym = 1;
		break;
	default:/* update (bss only) */

		if (get_sym_attr(s1, sym_index, 0)->linker_sym)
			sym->st_value = sec->data_offset;
	}

}
/* add various standard linker symbols (must be done after the
   sections are filled (for example after allocating common
   symbols)) */

static void tcc_add_linker_symbols(TCCState *s1)
{
	char buf[1024];
	int i;
	Section *s;

	set_linker_sym(s1, "_etext", text_section, 0);
	set_linker_sym(s1, "_edata", data_section, 0);
	set_linker_sym(s1, "_end", bss_section, 0);
	/* horrible new standard ldscript defines */
	add_init_array_defines(s1, ".preinit_array");
	add_init_array_defines(s1, ".init_array");
	add_init_array_defines(s1, ".fini_array");
	/* add start and stop symbols for sections whose name can be
	       expressed in C */

	for (i = 1; i < s1->nb_sections; i++) {
		s = s1->sections[i];
		if ((s->sh_flags & SHF_ALLOC)
		    && (s->sh_type == SHT_PROGBITS || s->sh_type == SHT_NOBITS
			|| s->sh_type == SHT_STRTAB)) {
			/* check if section name can be expressed in C */

			const char *p0, *p;
			p0 = s->name;
			if (*p0 == '.')
				++p0;
			p = p0;
			for (;;) {
				int c = *p;
				if (!c)
					break;
				if (!isid(c) && !isnum(c))
					goto next_sec;
				p++;
			}
			snprintf(buf, sizeof(buf), "__start_%s", p0);
			set_global_sym(s1, buf, s, 0);
			snprintf(buf, sizeof(buf), "__stop_%s", p0);
			set_global_sym(s1, buf, s, -1);
		}
next_sec: ;
	}
}

ST_FUNC void resolve_common_syms(TCCState *s1)
{
	ElfW(Sym) *sym;
	/* Allocate common symbols in BSS.  */

	for_each_elem(symtab_section, 1, sym, ElfW(Sym)) {
		if (sym->st_shndx == SHN_COMMON && sym->st_size) {
			/* symbol alignment is in st_value for SHN_COMMONs */

			sym->st_value = section_add(bss_section, sym->st_size,
						    sym->st_value);
			sym->st_shndx = bss_section->sh_num;
		}
	}
	/* Now assign linker provided symbols their value.  */

	if (s1->output_type != TCC_OUTPUT_DLL)
		tcc_add_linker_symbols(s1);
}
/* ndef ELF_OBJ_ONLY */
/* Create an ELF file on disk.
   This function handle ELF specific layout requirements */

static int tcc_output_elf(TCCState *s1, FILE *f, int phnum, ElfW(Phdr) *phdr)
{
	int i, shnum, offset, size, file_type;
	Section *s;
	ElfW(Ehdr) ehdr;
	ElfW(Shdr) shdr, *sh;

	file_type = s1->output_type;
	shnum = s1->nb_sections;

	memset(&ehdr, 0, sizeof(ehdr));
	if (phnum > 0) {
		ehdr.e_phentsize = sizeof(ElfW(Phdr));
		ehdr.e_phnum = phnum;
		ehdr.e_phoff = sizeof(ElfW(Ehdr));
	}
	/* fill header */

	ehdr.e_ident[0] = ELFMAG0;
	ehdr.e_ident[1] = ELFMAG1;
	ehdr.e_ident[2] = ELFMAG2;
	ehdr.e_ident[3] = ELFMAG3;
	ehdr.e_ident[4] = ELFCLASSW;
	ehdr.e_ident[5] = ELFDATA2LSB;
	ehdr.e_ident[6] = EV_CURRENT;
	if (file_type == TCC_OUTPUT_OBJ) {
		ehdr.e_type = ET_REL;
	} else {
		if (file_type & TCC_OUTPUT_DYN)
			ehdr.e_type = ET_DYN;
		else
			ehdr.e_type = ET_EXEC;
		if (s1->elf_entryname)
			ehdr.e_entry = get_sym_addr(s1, s1->elf_entryname, 1, 0);
		else
			ehdr.e_entry = get_sym_addr(s1, "_start", !!(file_type & TCC_OUTPUT_EXE), 0);
		if (ehdr.e_entry == (addr_t)-1)
			ehdr.e_entry = text_section->sh_addr;
		if (s1->nb_errors)
			return -1;
	}

	sort_syms(s1, s1->symtab);

	ehdr.e_machine = EM_TCC_TARGET;
	ehdr.e_version = EV_CURRENT;
	ehdr.e_shoff = (sizeof(ElfW(Ehdr)) + phnum * sizeof(ElfW(Phdr)) + 3) & -4;
	ehdr.e_ehsize = sizeof(ElfW(Ehdr));
	ehdr.e_shentsize = sizeof(ElfW(Shdr));
	ehdr.e_shnum = shnum;
	ehdr.e_shstrndx = shnum - 1;

	offset = fwrite(&ehdr, 1, sizeof(ElfW(Ehdr)), f);
	if (phdr)
		offset += fwrite(phdr, 1, phnum * sizeof(ElfW(Phdr)), f);
	/* output section headers */

	while (offset < ehdr.e_shoff) {
		fputc(0, f);
		offset++;
	}

	for (i = 0; i < shnum; i++) {
		sh = &shdr;
		memset(sh, 0, sizeof(ElfW(Shdr)));
		if (i) {
			s = s1->sections[i];
			sh->sh_name = s->sh_name;
			sh->sh_type = s->sh_type;
			sh->sh_flags = s->sh_flags;
			sh->sh_entsize = s->sh_entsize;
			sh->sh_info = s->sh_info;
			if (s->link)
				sh->sh_link = s->link->sh_num;
			sh->sh_addralign = s->sh_addralign;
			sh->sh_addr = s->sh_addr;
			sh->sh_offset = s->sh_offset;
			sh->sh_size = s->sh_size;
		}
		offset += fwrite(sh, 1, sizeof(ElfW(Shdr)), f);
	}
	/* output sections */

	for (i = 1; i < s1->nb_sections; i++) {
		s = s1->sections[i];
		if (s->sh_type != SHT_NOBITS) {
			while (offset < s->sh_offset) {
				fputc(0, f);
				offset++;
			}
			size = s->sh_size;
			if (size)
				offset += fwrite(s->data, 1, size, f);
		}
	}
	return 0;
}

static int tcc_output_binary(TCCState *s1, FILE *f)
{
	Section *s;
	int i, offset, size;

	offset = 0;
	for (i=1; i<s1->nb_sections; i++) {
		s = s1->sections[i];
		if (s->sh_type != SHT_NOBITS &&
		    (s->sh_flags & SHF_ALLOC)) {
			while (offset < s->sh_offset) {
				fputc(0, f);
				offset++;
			}
			size = s->sh_size;
			fwrite(s->data, 1, size, f);
			offset += size;
		}
	}
	return 0;
}
/* Write an elf, coff or "binary" file */

static int tcc_write_elf_file(TCCState *s1, const char *filename, int phnum,
			      ElfW(Phdr) *phdr)
{
	int fd, mode, file_type, ret;
	FILE *f;

	file_type = s1->output_type;
	if (file_type == TCC_OUTPUT_OBJ)
		mode = 0666;
	else
		mode = 0777;
	unlink(filename);
	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode);
	if (fd < 0 || (f = fdopen(fd, "wb")) == NULL)
		return tcc_error_noabort("could not write '%s: %s'", filename, strerror(errno));
	if (s1->verbose)
		printf("<- %s\n", filename);

	if (s1->output_format == TCC_OUTPUT_FORMAT_ELF)
		ret = tcc_output_elf(s1, f, phnum, phdr);
	else
		ret = tcc_output_binary(s1, f);
	fclose(f);

	return ret;
}
/* ndef ELF_OBJ_ONLY */
/* Allocate strings for section names */

static void alloc_sec_names(TCCState *s1, int is_obj)
{
	int i;
	Section *s, *strsec;

	strsec = new_section(s1, ".shstrtab", SHT_STRTAB, 0);
	put_elf_str(strsec, "");
	for (i = 1; i < s1->nb_sections; i++) {
		s = s1->sections[i];
		if (is_obj)
			s->sh_size = s->data_offset;
		if (s->sh_size || s == strsec || (s->sh_flags & SHF_ALLOC) || is_obj)
			s->sh_name = put_elf_str(strsec, s->name);
	}
	strsec->sh_size = strsec->data_offset;
}
/* Output an elf .o file */

static int elf_output_obj(TCCState *s1, const char *filename)
{
	Section *s;
	int i, ret, file_offset;
	/* Allocate strings for section names */

	alloc_sec_names(s1, 1);
	file_offset = (sizeof (ElfW(Ehdr)) + 3) & -4;
	file_offset += s1->nb_sections * sizeof(ElfW(Shdr));
	for (i = 1; i < s1->nb_sections; i++) {
		s = s1->sections[i];
		file_offset = (file_offset + 15) & -16;
		s->sh_offset = file_offset;
		if (s->sh_type != SHT_NOBITS)
			file_offset += s->sh_size;
	}
	/* Create the ELF file with name 'filename' */

	ret = tcc_write_elf_file(s1, filename, 0, NULL);
	return ret;
}

LIBTCCAPI int tcc_output_file(TCCState *s, const char *filename)
{
	s->nb_errors = 0;
	if (s->test_coverage)
		tcc_tcov_add_file(s, filename);
	if (s->output_type == TCC_OUTPUT_OBJ)
		return elf_output_obj(s, filename);

	return pe_output_file(s, filename);

}

ST_FUNC ssize_t full_read(int fd, void *buf, size_t count)
{
	char *cbuf = buf;
	size_t rnum = 0;
	while (1) {
		ssize_t num = read(fd, cbuf, count-rnum);
		if (num < 0)
			return num;
		if (num == 0)
			return rnum;
		rnum += num;
		cbuf += num;
	}
}

ST_FUNC void *load_data(int fd, unsigned long file_offset, unsigned long size)
{
	void *data;

	data = tcc_malloc(size);
	lseek(fd, file_offset, SEEK_SET);
	full_read(fd, data, size);
	return data;
}

typedef struct SectionMergeInfo {
	Section *s;/* corresponding existing section */

	unsigned long offset;/* offset of the new section in the existing section */

	uint8_t new_section;/* true if section 's' was added */

	uint8_t link_once;/* true if link once section */

} SectionMergeInfo;

ST_FUNC int tcc_object_type(int fd, ElfW(Ehdr) *h)
{
	int size = full_read(fd, h, sizeof *h);
	if (size == sizeof *h && 0 == memcmp(h, ELFMAG, 4)) {
		if (h->e_type == ET_REL)
			return AFF_BINTYPE_REL;
		if (h->e_type == ET_DYN)
			return AFF_BINTYPE_DYN;
	} else if (size >= 8) {
		if (0 == memcmp(h, ARMAG, 8))
			return AFF_BINTYPE_AR;

	}
	return 0;
}
/* load an object file and merge it with current files */
/* XXX: handle correctly stab (debug) info */

ST_FUNC int tcc_load_object_file(TCCState *s1,
				 int fd, unsigned long file_offset)
{
	ElfW(Ehdr) ehdr;
	ElfW(Shdr) *shdr, *sh;
	unsigned long size, offset, offseti;
	int i, j, nb_syms, sym_index, ret, seencompressed;
	char *strsec, *strtab;
	int stab_index, stabstr_index;
	int *old_to_new_syms;
	char *sh_name, *name;
	SectionMergeInfo *sm_table, *sm;
	ElfW(Sym) *sym, *symtab;
	ElfW_Rel *rel;
	Section *s;

	lseek(fd, file_offset, SEEK_SET);
	if (tcc_object_type(fd, &ehdr) != AFF_BINTYPE_REL)
		goto invalid;
	/* test CPU specific stuff */

	if (ehdr.e_ident[5] != ELFDATA2LSB ||
	    ehdr.e_machine != EM_TCC_TARGET) {
invalid:
		return tcc_error_noabort("invalid object file");
	}
	/* read sections */

	shdr = load_data(fd, file_offset + ehdr.e_shoff,
			 sizeof(ElfW(Shdr)) * ehdr.e_shnum);
	sm_table = tcc_mallocz(sizeof(SectionMergeInfo) * ehdr.e_shnum);
	/* load section names */

	sh = &shdr[ehdr.e_shstrndx];
	strsec = load_data(fd, file_offset + sh->sh_offset, sh->sh_size);
	/* load symtab and strtab */

	old_to_new_syms = NULL;
	symtab = NULL;
	strtab = NULL;
	nb_syms = 0;
	seencompressed = 0;
	stab_index = stabstr_index = 0;
	ret = -1;

	for (i = 1; i < ehdr.e_shnum; i++) {
		sh = &shdr[i];
		if (sh->sh_type == SHT_SYMTAB) {
			if (symtab) {
				tcc_error_noabort("object must contain only one symtab");
				goto the_end;
			}
			nb_syms = sh->sh_size / sizeof(ElfW(Sym));
			symtab = load_data(fd, file_offset + sh->sh_offset, sh->sh_size);
			sm_table[i].s = symtab_section;
			/* now load strtab */

			sh = &shdr[sh->sh_link];
			strtab = load_data(fd, file_offset + sh->sh_offset, sh->sh_size);
		}
		if (sh->sh_flags & SHF_COMPRESSED)
			seencompressed = 1;
	}
	/* now examine each section and try to merge its content with the
	       ones in memory */

	for (i = 1; i < ehdr.e_shnum; i++) {
		/* no need to examine section name strtab */

		if (i == ehdr.e_shstrndx)
			continue;
		sh = &shdr[i];
		if (sh->sh_type == SHT_RELX)
			sh = &shdr[sh->sh_info];
		/* ignore sections types we do not handle (plus relocs to those) */

		sh_name = strsec + sh->sh_name;
		if (0 == strncmp(sh_name, ".debug_", 7)
		    || 0 == strncmp(sh_name, ".stab", 5)) {
			if (!s1->do_debug || seencompressed)
				continue;

		} else if (0 == strncmp(sh_name, ".eh_frame", 9)) {
			if (NULL == eh_frame_section)
				continue;

		} else if (sh->sh_type != SHT_PROGBITS &&
			   sh->sh_type != SHT_NOTE &&
			   sh->sh_type != SHT_NOBITS &&
			   sh->sh_type != SHT_PREINIT_ARRAY &&
			   sh->sh_type != SHT_INIT_ARRAY &&
			   sh->sh_type != SHT_FINI_ARRAY
			  )
			continue;

		sh = &shdr[i];
		sh_name = strsec + sh->sh_name;
		if (sh->sh_addralign < 1)
			sh->sh_addralign = 1;
		/* find corresponding section, if any */

		for (j = 1; j < s1->nb_sections; j++) {
			s = s1->sections[j];
			if (strcmp(s->name, sh_name))
				continue;
			if (sh->sh_type != s->sh_type
			    && strcmp (s->name, ".eh_frame")
			    /* some crt1.o seem to have two ".note.GNU-stack" (SHT_NOTE & SHT_PROGBITS) */

			    && strcmp (s->name, ".note.GNU-stack")
			   ) {
				tcc_error_noabort("section type conflict: %s %02x <> %02x", s->name,
						  sh->sh_type, s->sh_type);
				goto the_end;
			}
			if (!strncmp(sh_name, ".gnu.linkonce", 13)) {
				/* if a 'linkonce' section is already present, we
				                   do not add it again. It is a little tricky as
				                   symbols can still be defined in
				                   it. */

				sm_table[i].link_once = 1;
				goto next;
			}
			if (stab_section) {
				if (s == stab_section)
					stab_index = i;
				if (s == stab_section->link)
					stabstr_index = i;
			}
			goto found;
		}
		/* not found: create new section */

		s = new_section(s1, sh_name, sh->sh_type, sh->sh_flags & ~SHF_GROUP);
		/* take as much info as possible from the section. sh_link and
		           sh_info will be updated later */

		s->sh_addralign = sh->sh_addralign;
		s->sh_entsize = sh->sh_entsize;
		sm_table[i].new_section = 1;
found:
		size = sh->sh_size;
		/* align start of section */

		offset = section_add(s, size, sh->sh_addralign);
		if (sh->sh_addralign > s->sh_addralign)
			s->sh_addralign = sh->sh_addralign;
		sm_table[i].offset = offset;
		sm_table[i].s = s;
		/* concatenate sections */

		if (sh->sh_type != SHT_NOBITS && size) {
			unsigned char *ptr;
			lseek(fd, file_offset + sh->sh_offset, SEEK_SET);
			ptr = s->data + offset;
			full_read(fd, ptr, size);
		}

next: ;
	}
	/* gr relocate stab strings */

	if (stab_index && stabstr_index) {
		Stab_Sym *a, *b;
		unsigned o;
		s = sm_table[stab_index].s;
		a = (Stab_Sym *)(s->data + sm_table[stab_index].offset);
		b = (Stab_Sym *)(s->data + s->data_offset);
		o = sm_table[stabstr_index].offset;
		while (a < b) {
			if (a->n_strx)
				a->n_strx += o;
			a++;
		}
	}
	/* second short pass to update sh_link and sh_info fields of new
	       sections */

	for (i = 1; i < ehdr.e_shnum; i++) {
		s = sm_table[i].s;
		if (!s || !sm_table[i].new_section)
			continue;
		sh = &shdr[i];
		if (sh->sh_link > 0)
			s->link = sm_table[sh->sh_link].s;
		if (sh->sh_type == SHT_RELX) {
			s->sh_info = sm_table[sh->sh_info].s->sh_num;
			/* update backward link */

			s1->sections[s->sh_info]->reloc = s;
		}
	}

	if (!symtab)
		goto done;
	/* resolve symbols */

	old_to_new_syms = tcc_mallocz(nb_syms * sizeof(int));

	sym = symtab + 1;
	for (i = 1; i < nb_syms; i++, sym++) {
		if (sym->st_shndx != SHN_UNDEF &&
		    sym->st_shndx < SHN_LORESERVE) {
			sm = &sm_table[sym->st_shndx];
			if (sm->link_once) {
				/* if a symbol is in a link once section, we use the
				                   already defined symbol. It is very important to get
				                   correct relocations */

				if (ELFW(ST_BIND)(sym->st_info) != STB_LOCAL) {
					name = strtab + sym->st_name;
					sym_index = find_elf_sym(symtab_section, name);
					if (sym_index)
						old_to_new_syms[i] = sym_index;
				}
				continue;
			}
			/* if no corresponding section added, no need to add symbol */

			if (!sm->s)
				continue;
			/* convert section number */

			sym->st_shndx = sm->s->sh_num;
			/* offset value */

			sym->st_value += sm->offset;
		}
		/* add symbol */

		name = strtab + sym->st_name;
		sym_index = set_elf_sym(symtab_section, sym->st_value, sym->st_size,
					sym->st_info, sym->st_other,
					sym->st_shndx, name);
		old_to_new_syms[i] = sym_index;
	}
	/* third pass to patch relocation entries */

	for (i = 1; i < ehdr.e_shnum; i++) {
		s = sm_table[i].s;
		if (!s)
			continue;
		sh = &shdr[i];
		offset = sm_table[i].offset;
		size = sh->sh_size;
		switch (s->sh_type) {
		case SHT_RELX:
			/* take relocation offset information */

			offseti = sm_table[sh->sh_info].offset;
			for (rel = (ElfW_Rel *) s->data + (offset / sizeof(*rel));
			     rel < (ElfW_Rel *) s->data + ((offset + size) / sizeof(*rel));
			     rel++) {
				int type;
				unsigned sym_index;
				/* convert symbol index */

				type = ELFW(R_TYPE)(rel->r_info);
				sym_index = ELFW(R_SYM)(rel->r_info);
				/* NOTE: only one symtab assumed */

				if (sym_index >= nb_syms)
					goto invalid_reloc;
				sym_index = old_to_new_syms[sym_index];
				/* ignore link_once in rel section. */

				if (!sym_index && !sm_table[sh->sh_info].link_once

				   ) {
invalid_reloc:
					tcc_error_noabort("Invalid relocation entry [%2d] '%s' @ %.8x",
							  i, strsec + sh->sh_name, (int)rel->r_offset);
					goto the_end;
				}
				rel->r_info = ELFW(R_INFO)(sym_index, type);
				/* offset the relocation offset */

				rel->r_offset += offseti;
			}
			break;
		default:
			break;
		}
	}
done:
	ret = !s1->nb_errors - 1;/* errors possibly from set_elf_sym() */

the_end:
	tcc_free(symtab);
	tcc_free(strtab);
	tcc_free(old_to_new_syms);
	tcc_free(sm_table);
	tcc_free(strsec);
	tcc_free(shdr);
	return ret;
}

typedef struct ArchiveHeader {
	char ar_name[16];/* name of this member */

	char ar_date[12];/* file mtime */

	char ar_uid[6];/* owner uid; printed as decimal */

	char ar_gid[6];/* owner gid; printed as decimal */

	char ar_mode[8];/* file mode, printed as octal   */

	char ar_size[10];/* file size, printed as decimal */

	char ar_fmag[2];/* should contain ARFMAG */

} ArchiveHeader;

#define ARFMAG "`\n"

static unsigned long long get_be(const uint8_t *b, int n)
{
	unsigned long long ret = 0;
	while (n)
		ret = (ret << 8) | *b++, --n;
	return ret;
}

static int read_ar_header(int fd, int offset, ArchiveHeader *hdr)
{
	char *p, *e;
	int len;
	lseek(fd, offset, SEEK_SET);
	len = full_read(fd, hdr, sizeof(ArchiveHeader));
	if (len != sizeof(ArchiveHeader))
		return len ? -1 : 0;
	if (memcmp(hdr->ar_fmag, ARFMAG, sizeof hdr->ar_fmag))
		return -1;
	p = hdr->ar_name;
	for (e = p + sizeof hdr->ar_name; e > p && e[-1] == ' ';)
		--e;
	*e = '\0';
	hdr->ar_size[sizeof hdr->ar_size-1] = 0;
	return len;
}
/* load only the objects which resolve undefined symbols */

static int tcc_load_alacarte(TCCState *s1, int fd, int size, int entrysize)
{
	int i, bound, nsyms, sym_index, len, ret = -1;
	unsigned long long off;
	uint8_t *data;
	const char *ar_names, *p;
	const uint8_t *ar_index;
	ElfW(Sym) *sym;
	ArchiveHeader hdr;

	data = tcc_malloc(size);
	if (full_read(fd, data, size) != size)
		goto invalid;
	nsyms = get_be(data, entrysize);
	ar_index = data + entrysize;
	ar_names = (char *) ar_index + nsyms * entrysize;

	do {
		bound = 0;
		for (p = ar_names, i = 0; i < nsyms; i++, p += strlen(p)+1) {
			Section *s = symtab_section;
			sym_index = find_elf_sym(s, p);
			if (!sym_index)
				continue;
			sym = &((ElfW(Sym) *)s->data)[sym_index];
			if (sym->st_shndx != SHN_UNDEF)
				continue;
			off = get_be(ar_index + i * entrysize, entrysize);
			len = read_ar_header(fd, off, &hdr);
			if (len <= 0 || memcmp(hdr.ar_fmag, ARFMAG, 2)) {
invalid:
				tcc_error_noabort("invalid archive");
				goto the_end;
			}
			off += len;
			if (s1->verbose == 2)
				printf("   -> %s\n", hdr.ar_name);
			if (tcc_load_object_file(s1, fd, off) < 0)
				goto the_end;
			++bound;
		}
	} while (bound);
	ret = 0;
the_end:
	tcc_free(data);
	return ret;
}
/* load a '.a' file */

ST_FUNC int tcc_load_archive(TCCState *s1, int fd, int alacarte)
{
	ArchiveHeader hdr;
	/* char magic[8]; */

	int size, len;
	unsigned long file_offset;
	ElfW(Ehdr) ehdr;
	/* skip magic which was already checked */
	/* full_read(fd, magic, sizeof(magic)); */

	file_offset = sizeof ARMAG - 1;

	for (;;) {
		len = read_ar_header(fd, file_offset, &hdr);
		if (len == 0)
			return 0;
		if (len < 0)
			return tcc_error_noabort("invalid archive");
		file_offset += len;
		size = strtol(hdr.ar_size, NULL, 0);
		if (alacarte) {
			/* coff symbol table : we handle it */

			if (!strcmp(hdr.ar_name, "/"))
				return tcc_load_alacarte(s1, fd, size, 4);
			if (!strcmp(hdr.ar_name, "/SYM64/"))
				return tcc_load_alacarte(s1, fd, size, 8);
		} else if (tcc_object_type(fd, &ehdr) == AFF_BINTYPE_REL) {
			if (s1->verbose == 2)
				printf("   -> %s\n", hdr.ar_name);
			if (tcc_load_object_file(s1, fd, file_offset) < 0)
				return -1;
		}
		/* align to even */

		file_offset = (file_offset + size + 1) & ~1;
	}
}
/* ==================== tccrun.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE
#define TCC_SET_STATE(fn) (tcc_enter_state(s1),fn)
/* only native compiler supports -run */

typedef struct rt_frame {
	addr_t ip, fp, sp;
} rt_frame;

static TCCState *g_s1;
/* semaphore to protect it */

TCC_SEM(static rt_sem);
static void rt_wait_sem(void)
{
	WAIT_SEM(&rt_sem);
}
static void rt_post_sem(void)
{
	POST_SEM(&rt_sem);
}
static int rt_get_caller_pc(addr_t *paddr, rt_frame *f, int level);
static void rt_exit(rt_frame *f, int code);
/* ------------------------------------------------------------- */
/* defined when included from lib/bt-exe.c */
static int protect_pages(void *ptr, unsigned long length, int mode);
static int tcc_relocate_ex(TCCState *s1, void *ptr, unsigned ptr_diff);
static void st_link(TCCState *s1);
static void st_unlink(TCCState *s1);

static void *win64_add_function_table(TCCState *s1);
static void win64_del_function_table(void *);

#define PAGESIZE 4096

#define PAGEALIGN(n) ((addr_t)n + (-(addr_t)n & (PAGESIZE-1)))
/* use VirtualAlloc() instead of tcc_malloc() */
static int rt_mem(TCCState *s1, int size)
{
	void *ptr;
	int ptr_diff = 0;
	/* always page-aligned */

	ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!ptr)
		return tcc_error_noabort("tccrun: could not allocate memory");

	s1->run_ptr = ptr;
	s1->run_size = size;
	return ptr_diff;
}
/* ------------------------------------------------------------- */
/* Do all relocations (needed before using tcc_get_symbol())
   Returns -1 on error. */

LIBTCCAPI int tcc_relocate(TCCState *s1)
{
	int size, ret, ptr_diff;

	if (s1->run_ptr)
		exit(tcc_error_noabort("'tcc_relocate()' twice is no longer supported"));

	size = tcc_relocate_ex(s1, NULL, 0);
	if (size < 0)
		return -1;
	ptr_diff = rt_mem(s1, size);
	if (ptr_diff < 0)
		return -1;
	ret = tcc_relocate_ex(s1, s1->run_ptr, ptr_diff);
	if (ret == 0)
		st_link(s1);
	return ret;
}

ST_FUNC void tcc_run_free(TCCState *s1)
{
	unsigned size;
	void *ptr;
	int i;
	/* free any loaded DLLs */

	for ( i = 0; i < s1->nb_loaded_dlls; i++) {
		DLLReference *ref = s1->loaded_dlls[i];
		if ( ref->handle )

			FreeLibrary((HMODULE)ref->handle);

	}
	/* unmap or unprotect and free memory */

	ptr = s1->run_ptr;
	if (NULL == ptr)
		return;
	st_unlink(s1);

	win64_del_function_table(s1->run_function_table);

	size = s1->run_size;

}
/* passed from longjmp instead of '0' */

#define RT_EXIT_ZERO 0xE0E00E0E
/* launch the compiled program with the given arguments */

LIBTCCAPI int tcc_run(TCCState *s1, int argc, char **argv)
{
	int (*prog_main)(int, char **, char **), ret;
	const char *top_sym;
	jmp_buf main_jb;
	char **envp = environ;
	/* tcc -dt -run ... nothing to do if no main() */

	if ((s1->dflag & 16) && (addr_t)-1 == get_sym_addr(s1, "main", 0, 1))
		return 0;

	tcc_add_symbol(s1, "__rt_exit", rt_exit);
	s1->run_main = "_runmain", top_sym = "main";
	if (s1->elf_entryname)
		s1->run_main = top_sym = s1->elf_entryname;
	tcc_add_support(s1, "runmain.o");

	if (tcc_relocate(s1) < 0)
		return -1;

	prog_main = (void *)get_sym_addr(s1, s1->run_main, 1, 1);
	if ((addr_t)-1 == (addr_t)prog_main)
		return -1;
	/* custom stdin for run_main, mainly if stdin is/was an input file.
	     * fileno(stdin) should remain 0, as posix mandates to use the smallest
	     * free fd, which is 0 after the initial fclose in freopen. windows too.
	     * to set stdin to the tty, use /dev/tty (posix) or con (windows).
	     */

	if (s1->run_stdin && !freopen(s1->run_stdin, "r", stdin)) {
		tcc_error_noabort("failed to reopen stdin from '%s'", s1->run_stdin);
		return -1;
	}

	errno = 0;/* clean errno value */

	fflush(stdout);
	fflush(stderr);

	ret = tcc_setjmp(s1, main_jb, tcc_get_symbol(s1, top_sym));
	if (0 == ret) {
		ret = prog_main(argc, argv, envp);
	} else if (RT_EXIT_ZERO == ret) {
		ret = 0;
	}

	if (s1->dflag & 16 && ret)/* tcc -dt -run ... */

		fprintf(s1->ppfp, "[returns %d]\n", ret), fflush(s1->ppfp);
	return ret;
}
/* ------------------------------------------------------------- */
/* remove all STB_LOCAL symbols */

static void cleanup_symbols(TCCState *s1)
{
	Section *s = s1->symtab;
	int sym_index, end_sym = s->data_offset / sizeof (ElfSym);
	/* reset symtab */

	s->data_offset = s->link->data_offset = s->hash->data_offset = 0;
	init_symtab(s);
	/* add global symbols again */

	for (sym_index = 1; sym_index < end_sym; ++sym_index) {
		ElfW(Sym) *sym = &((ElfW(Sym) *)s->data)[sym_index];
		const char *name = (char *)s->link->data + sym->st_name;
		if (ELFW(ST_BIND)(sym->st_info) == STB_LOCAL)
			continue;
//printf("sym %s\n", name);

		put_elf_sym(s, sym->st_value, sym->st_size, sym->st_info, sym->st_other,
			    sym->st_shndx, name);
	}
}
/* free all sections except symbols */

static void cleanup_sections(TCCState *s1)
{
	struct {
		Section **secs;
		int nb_secs;
	} *p = (void *)&s1->sections;
	int i, f = 2;
	do {
		for (i = --f; i < p->nb_secs; i++) {
			Section *s = p->secs[i];
			if (s == s1->symtab || s == s1->symtab->link || s == s1->symtab->hash) {
				s->data = tcc_realloc(s->data, s->data_allocated = s->data_offset);
			} else {
				free_section(s), tcc_free(s), p->secs[i] = NULL;
			}
		}
	} while (++p, f);
}
/* ------------------------------------------------------------- */
/* 0 = .text rwx  other rwx (memory >= 2 pages a 4096 bytes) */
/* 1 = .text rx   other rw (memory >= 3 pages) */
/* 2 = .text rx  .rdata ro  .data/.bss rw (memory >= 4 pages) */
/* Some targets implement secutiry options that do not allow write in
   executable code. These targets need CONFIG_RUNMEM_RO=1.
   The disadvantage of this is that it requires a little bit more memory. */
#ifndef CONFIG_RUNMEM_RO
#define CONFIG_RUNMEM_RO 0
#endif
/* relocate code. Return -1 on error, required size if ptr is NULL,
   otherwise copy code into buffer passed by the caller */

static int tcc_relocate_ex(TCCState *s1, void *ptr, unsigned ptr_diff)
{
	Section *s;
	unsigned offset, length, align, i, k, f;
	unsigned n, copy;
	addr_t mem, addr;

	if (NULL == ptr) {
		s1->nb_errors = 0;

		pe_output_file(s1, NULL);

	}

	offset = copy = 0;
	mem = (addr_t)ptr;
redo:
	if (s1->verbose == 2 && copy)
		printf(&"-----------------------------------------------------\n"[PTR_SIZE*2 -
				8]);
	if (s1->nb_errors)
		return -1;
	if (copy == 3)
		return 0;

	for (k = 0; k < 3; ++k) {/* 0:rx, 1:ro, 2:rw sections */

		n = 0;
		addr = 0;
		for (i = 1; i < s1->nb_sections; i++) {
			static const char shf[] = {
				SHF_ALLOC|SHF_EXECINSTR, SHF_ALLOC, SHF_ALLOC|SHF_WRITE
			};
			s = s1->sections[i];
			if (shf[k] != (s->sh_flags & (SHF_ALLOC|SHF_WRITE|SHF_EXECINSTR)))
				continue;
			length = s->data_offset;
			if (copy == 2) {
				if (addr == 0)
					addr = s->sh_addr;
				n = (s->sh_addr - addr) + length;
				continue;
			}
			if (copy) {/* final step: copy section data to memory */

				if (s1->verbose == 2)
					printf("%d: %-16s %p  len %05x  align %04x\n",
					       k, s->name, (void *)s->sh_addr, length, s->sh_addralign);
				ptr = (void *)s->sh_addr;
				if (k == 0)
					ptr = (void *)(s->sh_addr + ptr_diff);
				if (NULL == s->data || s->sh_type == SHT_NOBITS)
					memset(ptr, 0, length);
				else
					memcpy(ptr, s->data, length);
				continue;
			}

			align = s->sh_addralign;
			if (++n == 1) {
				/* To avoid that x86 processors would reload cached instructions
				                   each time when data is written in the near, we need to make
				                   sure that code and data do not share the same 64 byte unit */

				if (align < 64)
					align = 64;
				/* start new page for different permissions */

				if (k <= CONFIG_RUNMEM_RO)
					align = PAGESIZE;
			}
			s->sh_addralign = align;
			addr = k ? mem + ptr_diff : mem;
			offset += -(addr + offset) & (align - 1);
			s->sh_addr = mem ? addr + offset : 0;
			offset += length;
		}
		if (copy == 2) {/* set permissions */

			if (n == 0)/* no data  */

				continue;

			f = k;
			if (f >= CONFIG_RUNMEM_RO) {
				if (f != 0)
					continue;
				f = 3;/* change only SHF_EXECINSTR to rwx */

			}
			n = PAGEALIGN(n);
			if (s1->verbose == 2) {
				printf("protect         %3s %p  len %05x\n",
				       &"rx\0ro\0rw\0rwx"[f*3], (void *)addr, (unsigned)n);
			}
			if (protect_pages((void *)addr, n, f) < 0)
				return tcc_error_noabort(

					       "VirtualProtect failed");

		}
	}

	if (0 == mem)
		return PAGEALIGN(offset);

	if (++copy == 2) {
		goto redo;
	}
	if (copy == 3) {

		s1->run_function_table = win64_add_function_table(s1);
		/* remove local symbols and free sections except symtab */

		cleanup_symbols(s1);
		cleanup_sections(s1);
		goto redo;
	}
	/* relocate symbols */

	relocate_syms(s1, s1->symtab, 1);
	if (s1->nb_errors)
		goto redo;
	/* relocate sections */

	s1->pe_imagebase = mem;

	relocate_sections(s1);
	goto redo;
}
/* ------------------------------------------------------------- */
/* allow to run code in memory */

static int protect_pages(void *ptr, unsigned long length, int mode)
{

	static const unsigned char protect[] = {
		PAGE_EXECUTE_READ,
		PAGE_READONLY,
		PAGE_READWRITE,
		PAGE_EXECUTE_READWRITE
	};
	DWORD old;
	if (!VirtualProtect(ptr, length, protect[mode], &old))
		return -1;
	return 0;
}

static void *win64_add_function_table(TCCState *s1)
{
	void *p = NULL;
	if (s1->uw_pdata) {
		p = (void *)s1->uw_pdata->sh_addr;
		RtlAddFunctionTable(
			(RUNTIME_FUNCTION *)p,
			s1->uw_pdata->data_offset sizeof (RUNTIME_FUNCTION),
			s1->pe_imagebase
		);
		s1->uw_pdata = NULL;
	}
	return p;
}

static void win64_del_function_table(void *p)
{
	if (p) {
		RtlDeleteFunctionTable((RUNTIME_FUNCTION *)p);
	}
}
static void bt_link(TCCState *s1)
{

}

static void st_link(TCCState *s1)
{
	rt_wait_sem();
	s1->next = g_s1, g_s1 = s1;
	bt_link(s1);
	rt_post_sem();
}
/* remove 'el' from 'list' */

static void ptr_unlink(void *list, void *e, unsigned next)
{
	void **pp, **nn, *p;
	for (pp = list; !!(p = *pp); pp = nn) {
		nn = (void *)((char *)p + next); /* nn = &p->next; */

		if (p == e) {
			*pp = *nn;
			break;
		}
	}
}

static void st_unlink(TCCState *s1)
{
	rt_wait_sem();

	ptr_unlink(&g_s1, s1, offsetof(TCCState, next));
	rt_post_sem();
}

LIBTCCAPI void *_tcc_setjmp(TCCState *s1, void *p_jmp_buf, void *func,
			    void *p_longjmp)
{
	s1->run_lj = p_longjmp;
	s1->run_jb = p_jmp_buf;

	return p_jmp_buf;
}

LIBTCCAPI void tcc_set_backtrace_func(TCCState *s1, void *data, TCCBtFunc *func)
{
	s1->bt_func = func;
	s1->bt_data = data;
}

static TCCState *rt_find_state(rt_frame *f)
{
	TCCState *s;
	int level;
	addr_t pc;

	s = g_s1;
	if (NULL == s || NULL == s->next) {
		/* play it safe in the simple case when there is only one state */

		return s;
	}
	for (level = 0; level < 8; ++level) {
		if (rt_get_caller_pc(&pc, f, level) < 0)
			break;
		for (s = g_s1; s; s = s->next) {
			if (pc >= (addr_t)s->run_ptr
			    && pc < (addr_t)s->run_ptr + s->run_size)
				return s;
		}
	}
	return NULL;
}

static void rt_exit(rt_frame *f, int code)
{
	TCCState *s;
	rt_wait_sem();
	s = rt_find_state(f);
	rt_post_sem();
	if (s && s->run_lj) {

		if (code == 0)
			code = RT_EXIT_ZERO;
		((void(*)(void *,int))s->run_lj)(s->run_jb, code);
	}
	exit(code);
}
/* ------------------------------------------------------------- */
//ndef CONFIG_TCC_BACKTRACE_ONLY
/* ------------------------------------------------------------- */
// for runmain.c:exit(); when CONFIG_TCC_BACKTRACE == 0 */

static int rt_get_caller_pc(addr_t *paddr, rt_frame *f, int level)
{
	if (level)
		return -1;
	*paddr = f->ip;
	return 0;
}
/* ------------------------------------------------------------- */
/* CONFIG_TCC_STATIC */
/* TCC_IS_NATIVE */
/* ------------------------------------------------------------- */

/**/

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) tcc_state->sym
#define TCC_SET_STATE(fn) fn
#undef _tcc_error
#include <assert.h>

ST_DATA const char *const target_machine_defs =
	"__x86_64__\0"
	"__x86_64\0"
	"__amd64__\0"
	;

ST_DATA const int reg_classes[NB_REGS] = {
	/* eax */
	RC_INT | RC_RAX,
	/* ecx */
	RC_INT | RC_RCX,
	/* edx */
	RC_INT | RC_RDX,
	0,
	0,
	0,
	RC_RSI,
	RC_RDI,
	RC_R8,
	RC_R9,
	RC_R10,
	RC_R11,
	0,
	0,
	0,
	0,
	/* xmm0 */
	RC_FLOAT | RC_XMM0,
	/* xmm1 */
	RC_FLOAT | RC_XMM1,
	/* xmm2 */
	RC_FLOAT | RC_XMM2,
	/* xmm3 */
	RC_FLOAT | RC_XMM3,
	/* xmm4 */
	RC_FLOAT | RC_XMM4,
	/* xmm5 */
	RC_FLOAT | RC_XMM5,
	/* xmm6 an xmm7 are included so gv() can be used on them,
	       but they are not tagged with RC_FLOAT because they are
	       callee saved on Windows */

	RC_XMM6,
	RC_XMM7,
	/* st0 */
	RC_ST0
};

static unsigned long func_sub_sp_offset;
static int func_ret_sub;

static int func_scratch, func_alloca;
/* XXX: make it faster ? */
ST_FUNC void g(int c)
{
	int ind1;
	if (nocode_wanted)
		return;
	ind1 = ind + 1;
	if (ind1 > cur_text_section->data_allocated)
		section_realloc(cur_text_section, ind1);
	cur_text_section->data[ind] = c;
	ind = ind1;
}

ST_FUNC void o(unsigned int c)
{
	while (c) {
		g(c);
		c = c >> 8;
	}
}

ST_FUNC void gen_le16(int v)
{
	g(v);
	g(v >> 8);
}

ST_FUNC void gen_le32(int c)
{
	g(c);
	g(c >> 8);
	g(c >> 16);
	g(c >> 24);
}

ST_FUNC void gen_le64(int64_t c)
{
	g(c);
	g(c >> 8);
	g(c >> 16);
	g(c >> 24);
	g(c >> 32);
	g(c >> 40);
	g(c >> 48);
	g(c >> 56);
}

static void orex(int ll, int r, int r2, int b)
{
	if ((r & VT_VALMASK) >= VT_CONST)
		r = 0;
	if ((r2 & VT_VALMASK) >= VT_CONST)
		r2 = 0;
	if (ll || REX_BASE(r) || REX_BASE(r2)) {
		if ((b & 0xff) == 0x66)/* output prefix before rex byte */

			o(0x66), b >>= 8;
		o(0x40 | REX_BASE(r) | (REX_BASE(r2) << 2) | (ll << 3));
	}
	o(b);
}
/* output a symbol and patch all calls to it */

ST_FUNC void gsym_addr(int t, int a)
{
	while (t) {
		unsigned char *ptr = cur_text_section->data + t;
		uint32_t n = read32le(ptr);/* next value */

		write32le(ptr, a < 0 ? -a : a - t - 4);
		t = n;
	}
}

static int is64_type(int t)
{
	return ((t & VT_BTYPE) == VT_PTR ||
		(t & VT_BTYPE) == VT_FUNC ||
		(t & VT_BTYPE) == VT_LLONG);
}
/* instruction + 4 bytes data. Return the address of the data */

static int oad(int c, int s)
{
	int t;
	if (nocode_wanted)
		return s;
	o(c);
	t = ind;
	gen_le32(s);
	return t;
}
/* generate jmp to a label */

#define gjmp2(instr,lbl) oad(instr,lbl)

ST_FUNC void gen_addr32(int r, Sym *sym, int c)
{
	if (r & VT_SYM)
		greloca(cur_text_section, sym, ind, R_X86_64_32S, c), c=0;
	gen_le32(c);
}
/* output constant with relocation if 'r & VT_SYM' is true */

ST_FUNC void gen_addrpc32(int r, Sym *sym, int c)
{
	if (r & VT_SYM)
		greloca(cur_text_section, sym, ind, R_X86_64_PC32, c-4), c=4;
	gen_le32(c-4);
}
/* output got address with relocation */

static void gen_gotpcrel(int r, Sym *sym, int c)
{

	tcc_error("internal error: no GOT on PE: %s %x %x | %02x %02x %02x\n",
		  get_tok_str(sym->v, NULL), c, r,
		  cur_text_section->data[ind-3],
		  cur_text_section->data[ind-2],
		  cur_text_section->data[ind-1]
		 );
	greloca(cur_text_section, sym, ind, R_X86_64_GOTPCREL, -4);
	gen_le32(0);
	if (c) {
		/* we use add c, %xxx for displacement */

		orex(1, r, 0, 0x81);
		o(0xc0 + REG_VALUE(r));
		gen_le32(c);
	}
}
/* generate a modrm reference. 'op_reg' contains the additional 3
   opcode rsp. second register bits */

static void gen_modrm_impl(int opcode, int ll, int op_reg_0, int r, Sym *sym,
			   int c)
{
	int op_reg = REG_VALUE(op_reg_0) << 3;

	if ((r & VT_SYM) && (sym->type.t & VT_TLS)) {

		Sym *s2 = external_global_sym(TOK___tls_index, &int_type);
		r = get_reg(RC_INT);
		gen_modrm_impl(0x8B, 0, r, VT_SYM|VT_CONST, s2, 0);
		o(0x03e0c148 | r << 16); /* shl 2,r */

		o(0x4865), oad(0x250403 | r << 11, 11*PTR_SIZE); /* add gs:0x58,r */

		gen_modrm_impl(0x8B, 1, r, r | VT_LVAL, 0, 0); /* mov (r),r */

		orex(ll, r, op_reg_0, opcode), oad(0x80 | op_reg | r, 0);
		greloca(cur_text_section, sym, ind - 4, R_X86_64_TPOFF32, c);

		return;
	}

	orex(ll, r, op_reg_0, opcode);

	if ((r & VT_VALMASK) == VT_CONST) {
		/* constant memory reference */

		if (!(r & VT_SYM)) {
			/* Absolute memory reference */

			o(0x04 | op_reg);/* [sib] | destreg */

			oad(0x25, c);/* disp32 */

		} else {
			o(0x05 | op_reg);/* (%rip)+disp32 | destreg */

			if (op_reg_0 & TREG_MEM) {
				gen_gotpcrel(r, sym, c);
			} else {
				gen_addrpc32(r, sym, c);
			}
		}
	} else if ((r & VT_VALMASK) == VT_LOCAL) {
		/* currently, we use only ebp as base */

		if (c == (signed char)c) {
			/* short reference */

			o(0x45 | op_reg);
			g(c);
		} else {
			oad(0x85 | op_reg, c);
		}
		/* 'c' (mostly from vtop->c.i) is not valid unless when TREG_MEM is set */

	} else if ((r & TREG_MEM) && c) {
		g(0x80 | op_reg | REG_VALUE(r));
		gen_le32(c);
	} else if (r & VT_LVAL) {
		g(0x00 | op_reg | REG_VALUE(r));
	} else {
		g(0xc0 | op_reg | REG_VALUE(r));
	}
}

static void gen_modrm64(int opcode, int op_reg, int r, Sym *sym, int c)
{
	gen_modrm_impl(opcode, 1, op_reg, r, sym, c);
}

static void gen_modrm32(int opcode, int op_reg, int r, Sym *sym, int c)
{
	gen_modrm_impl(opcode, 0, op_reg, r, sym, c);
}
/* load 'r' from value 'sv' */

void load(int r, SValue *sv)
{
	int v, t, ft, fc, fr;
	SValue v1;

	fr = sv->r;
	ft = sv->type.t & ~(VT_DEFSIGN|VT_VOLATILE|VT_CONSTANT);
	fc = sv->c.i;

	if (fc != sv->c.i && (fr & VT_SYM))
		tcc_error("64 bit addend in load");

	v = fr & VT_VALMASK;
	if (fr & VT_LVAL) {
		int b, ll;
		if (v == VT_LLOCAL) {
			v1.type.t = VT_PTR;
			v1.r = VT_LOCAL | VT_LVAL;
			v1.c.i = fc;
			v1.sym = NULL;
			fr = r;
			if (!(reg_classes[fr] & (RC_INT|RC_R11)))
				fr = get_reg(RC_INT);
			load(fr, &v1);
			fr |= VT_LVAL;
		}
		if (fc != sv->c.i) {
			/* If the addends doesn't fit into a 32bit signed
				       we must use a 64bit move.  We've checked above
				       that this doesn't have a sym associated.  */

			v1.type.t = VT_LLONG;
			v1.r = VT_CONST;
			v1.c.i = sv->c.i;
			v1.sym = NULL;
			fr = r;
			if (!(reg_classes[fr] & (RC_INT|RC_R11)))
				fr = get_reg(RC_INT);
			load(fr, &v1);
			fc = 0;
		}
		ll = 0;
		/* Like GCC we can load from small enough properly sized
			   structs and unions as well.
			   XXX maybe move to generic operand handling, but should
			   occur only with asm, so tccasm.c might also be a better place */

		if ((ft & VT_BTYPE) == VT_STRUCT) {
			int align;
			switch (type_size(&sv->type, &align)) {
			case 1:
				ft = VT_BYTE;
				break;
			case 2:
				ft = VT_SHORT;
				break;
			case 4:
				ft = VT_INT;
				break;
			case 8:
				ft = VT_LLONG;
				break;
			default:
				tcc_error("invalid aggregate type for register load");
				break;
			}
		}
		if ((ft & VT_BTYPE) == VT_FLOAT) {
			b = 0x6e0f66;
			r = REG_VALUE(r);/* movd */

		} else if ((ft & VT_BTYPE) == VT_DOUBLE) {
			b = 0x7e0ff3;/* movq */

			r = REG_VALUE(r);
		} else if ((ft & VT_BTYPE) == VT_LDOUBLE) {
			b = 0xdb, r = 5;/* fldt */

		} else if ((ft & VT_TYPE) == VT_BYTE || (ft & VT_TYPE) == VT_BOOL) {
			b = 0xbe0f;/* movsbl */

		} else if ((ft & VT_TYPE) == (VT_BYTE | VT_UNSIGNED)) {
			b = 0xb60f;/* movzbl */

		} else if ((ft & VT_TYPE) == VT_SHORT) {
			b = 0xbf0f;/* movswl */

		} else if ((ft & VT_TYPE) == (VT_SHORT | VT_UNSIGNED)) {
			b = 0xb70f;/* movzwl */

		} else if ((ft & VT_TYPE) == (VT_VOID)) {
			/* Can happen with zero size structs */

			return;
		} else {
			assert(((ft & VT_BTYPE) == VT_INT)
			       || ((ft & VT_BTYPE) == VT_LLONG)
			       || ((ft & VT_BTYPE) == VT_PTR)
			       || ((ft & VT_BTYPE) == VT_FUNC)
			      );
			ll = is64_type(ft);
			b = 0x8b;
		}
		gen_modrm_impl(b, ll, r, fr, sv->sym, fc);
	} else {
		if (v == VT_CONST) {
			if (fr & VT_SYM) {

				gen_modrm64(0x8d, r, fr, sv->sym, fc);

			} else if (is64_type(ft)) {
				if (sv->c.i >> 32) {
					orex(1,r,0, 0xb8 + REG_VALUE(r));/* movabs $xx, r */

					gen_le64(sv->c.i);
				} else if (sv->c.i > 0) {
					orex(0,r,0, 0xb8 + REG_VALUE(r));/* mov $xx, r */

					gen_le32(sv->c.i);
				} else {
					orex(0, r, r, 0x31);/* xor r, r */

					o(0xc0 + REG_VALUE(r) * 9);
				}
			} else {
				orex(0,r,0, 0xb8 + REG_VALUE(r));/* mov $xx, r */

				gen_le32(fc);
			}
		} else if (v == VT_LOCAL) {
			gen_modrm64(0x8d, r, VT_LOCAL, sv->sym, fc);
		} else if (v == VT_CMP) {
			if (fc & 0x100) {
				v = vtop->cmp_r;
				fc &= ~0x100;
				/* This was a float compare.  If the parity bit is
						   set the result was unordered, meaning false for everything
						   except TOK_NE, and true for TOK_NE.  */

				orex(0, r, 0, 0xb0 + REG_VALUE(r));/* mov $0/1,%al */

				g(v ^ fc ^ (v == TOK_NE));
				o(0x037a + (REX_BASE(r) << 8));
			}
			orex(0,r,0, 0x0f);/* setxx %br */

			o(fc);
			o(0xc0 + REG_VALUE(r));
			orex(0,r,0, 0x0f);
			o(0xc0b6 + REG_VALUE(r) * 0x900);/* movzbl %al, %eax */

		} else if (v == VT_JMP || v == VT_JMPI) {
			t = v & 1;
			orex(0,r,0,0);
			oad(0xb8 + REG_VALUE(r), t);/* mov $1, r */

			o(0x05eb + (REX_BASE(r) << 8));/* jmp after */

			gsym(fc);
			orex(0,r,0,0);
			oad(0xb8 + REG_VALUE(r), t ^ 1);/* mov $0, r */

		} else if (v != r) {
			if ((r >= TREG_XMM0) && (r <= TREG_XMM7)) {
				if (v == TREG_ST0) {
					/* gen_cvt_ftof(VT_DOUBLE); */

					o(0xf0245cdd);/* fstpl -0x10(%rsp) */

					/* movsd -0x10(%rsp),%xmmN */

					o(0x100ff2);
					o(0x44 + REG_VALUE(r)*8);/* %xmmN */

					o(0xf024);
				} else {
					assert((v >= TREG_XMM0) && (v <= TREG_XMM7));
					if ((ft & VT_BTYPE) == VT_FLOAT) {
						o(0x100ff3);
					} else {
						assert((ft & VT_BTYPE) == VT_DOUBLE);
						o(0x100ff2);
					}
					o(0xc0 + REG_VALUE(v) + REG_VALUE(r)*8);
				}
			} else if (r == TREG_ST0) {
				assert((v >= TREG_XMM0) && (v <= TREG_XMM7));
				/* gen_cvt_ftof(VT_LDOUBLE); */
				/* movsd %xmmN,-0x10(%rsp) */

				o(0x110ff2);
				o(0x44 + REG_VALUE(r)*8);/* %xmmN */

				o(0xf024);
				o(0xf02444dd);/* fldl -0x10(%rsp) */

			} else {
				orex(is64_type(ft), r, v, 0x89);
				o(0xc0 + REG_VALUE(r) + REG_VALUE(v) * 8);/* mov v, r */

			}
		}
	}
}
/* store register 'r' in lvalue 'v' */

void store(int r, SValue *v)
{
	int fr, bt, fc;
	/* store the REX prefix in this variable when PIC is enabled */

	int opc = 0, ll = 0;

	fr = v->r;
	fc = v->c.i;
	if (fc != v->c.i && (fr & VT_SYM))
		tcc_error("64 bit addend in store");
	bt = v->type.t & VT_BTYPE;
	/* XXX: incorrect if float reg to reg */

	if (bt == VT_FLOAT) {
		opc = 0x7e0f66;
		r = REG_VALUE(r);
	} else if (bt == VT_DOUBLE) {
		opc = 0xd60f66;
		r = REG_VALUE(r);
	} else if (bt == VT_LDOUBLE) {
		o(0xc0d9);/* fld %st(0) */

		opc = 0xdb;
		r = 7;
	} else if (bt == VT_BYTE || bt == VT_BOOL) {
		opc = 0x88;
	} else {
		opc = 0x89;
		if (bt == VT_SHORT)
			opc = 0x8966;
		else if (is64_type(bt))
			ll = 1;
	}
	gen_modrm_impl(opc, ll, r, fr, v->sym, fc);
}
/* 'is_jmp' is '1' if it is a jump */

static void gcall_or_jmp(int is_jmp)
{
	int r;
	if ((vtop->r & (VT_VALMASK | VT_LVAL)) == VT_CONST &&
	    ((vtop->r & VT_SYM) && (vtop->c.i-4) == (int)(vtop->c.i-4))) {
		/* constant symbolic case -> simple relocation */

		greloca(cur_text_section, vtop->sym, ind + 1, R_X86_64_PLT32,
			(int)(vtop->c.i-4));
		oad(0xe8 + is_jmp, 0);/* call/jmp im */

	} else {
		/* otherwise, indirect call */

		r = TREG_R11;
		load(r, vtop);
		o(0x41);/* REX */

		o(0xff);/* call/jmp *r */

		o(0xd0 + REG_VALUE(r) + (is_jmp << 4));
	}
}

#define REGN 4
static const uint8_t arg_regs[REGN] = {
	TREG_RCX, TREG_RDX, TREG_R8, TREG_R9
};

/* Prepare arguments in R10 and R11 rather than RCX and RDX
   because gv() will not ever use these */

static int arg_prepare_reg(int idx)
{
	if (idx == 0 || idx == 1)
		/* idx=0: r10, idx=1: r11 */

		return idx + 10;
	else
		return idx >= 0 && idx < REGN ? arg_regs[idx] : 0;
}

/* Generate function call. The function address is pushed first, then
   all the parameters in call order. This functions pops all the
   parameters and the function address. */

static void gen_offs_sp(int b, int r, int d)
{
	orex(1,0,r & 0x100 ? 0 : r, b);
	if (d == (signed char)d) {
		o(0x2444 | (REG_VALUE(r) << 3));
		g(d);
	} else {
		o(0x2484 | (REG_VALUE(r) << 3));
		gen_le32(d);
	}
}

static int using_regs(int size)
{
	return !(size > 8 || (size & (size - 1)));
}

/* Return the number of registers needed to return the struct, or 0 if
   returning via struct pointer. */

ST_FUNC int gfunc_sret(CType *vt, int variadic, CType *ret, int *ret_align,
		       int *regsize)
{
	int size, align;
	*ret_align = 1; // Never have to re-align return values for x86-64

	*regsize = 8;
	size = type_size(vt, &align);
	if (!using_regs(size))
		return 0;
	if (size == 8)
		ret->t = VT_LLONG;
	else if (size == 4)
		ret->t = VT_INT;
	else if (size == 2)
		ret->t = VT_SHORT;
	else
		ret->t = VT_BYTE;
	ret->ref = NULL;
	return 1;
}

static int is_sse_float(int t)
{
	int bt;
	bt = t & VT_BTYPE;
	return bt == VT_DOUBLE || bt == VT_FLOAT;
}

static int gfunc_arg_size(CType *type)
{
	int align;
	if (type->t & (VT_ARRAY|VT_BITFIELD))
		return 8;
	return type_size(type, &align);
}

void gfunc_call(int nb_args)
{
	int size, r, args_size, i, d, bt, struct_size;
	int arg;

	save_regs(nb_args);

	args_size = (nb_args < REGN ? REGN : nb_args) * PTR_SIZE;
	arg = nb_args;

	/* for struct arguments, we need to call memcpy and the function
	   call breaks register passing arguments we are preparing.
	   So, we process arguments which will be passed by stack first. */

	struct_size = args_size;
	for (i = 0; i < nb_args; i++) {
		SValue *sv;

		--arg;
		sv = &vtop[-i];
		bt = (sv->type.t & VT_BTYPE);
		size = gfunc_arg_size(&sv->type);

		if (using_regs(size))
			continue; /* arguments smaller than 8 bytes passed in registers or on stack */

		if (bt == VT_STRUCT) {
			/* align to stack align size */

			size = (size + 15) & ~15;
			/* generate structure store */

			r = get_reg(RC_INT);
			gen_offs_sp(0x8d, r, struct_size);
			struct_size += size;

			/* generate memcpy call */

			vset(&sv->type, r | VT_LVAL, 0);
			vpushv(sv);
			vstore();
			--vtop;
		} else if (bt == VT_LDOUBLE) {
			gv(RC_ST0);
			gen_offs_sp(0xdb, 0x107, struct_size);
			struct_size += 16;
		}
	}

	if (func_scratch < struct_size)
		func_scratch = struct_size;

	arg = nb_args;
	struct_size = args_size;

	for (i = 0; i < nb_args; i++) {
		--arg;
		bt = (vtop->type.t & VT_BTYPE);

		size = gfunc_arg_size(&vtop->type);
		if (!using_regs(size)) {
			/* align to stack align size */

			size = (size + 15) & ~15;
			if (arg >= REGN) {
				d = get_reg(RC_INT);
				gen_offs_sp(0x8d, d, struct_size);
				gen_offs_sp(0x89, d, arg*8);
			} else {
				d = arg_prepare_reg(arg);
				gen_offs_sp(0x8d, d, struct_size);
			}
			struct_size += size;
		} else {
			if (is_sse_float(vtop->type.t)) {
				if (tcc_state->nosse)
					tcc_error("SSE disabled");
				if (arg >= REGN) {
					gv(RC_XMM0);
					/* movq %xmm0, j*8(%rsp) */

					gen_offs_sp(0xd60f66, 0x100, arg*8);
				} else {
					/* Load directly to xmmN register */

					gv(RC_XMM0 << arg);
					d = arg_prepare_reg(arg);
					/* mov %xmmN, %rxx */

					o(0x66);
					orex(1,d,0, 0x7e0f);
					o(0xc0 + arg*8 + REG_VALUE(d));
				}
			} else {
				if (bt == VT_STRUCT) {
					vtop->type.ref = NULL;
					vtop->type.t = size > 4 ? VT_LLONG : size > 2 ? VT_INT
						       : size > 1 ? VT_SHORT : VT_BYTE;
				}

				r = gv(RC_INT);
				if (arg >= REGN) {
					gen_offs_sp(0x89, r, arg*8);
				} else {
					d = arg_prepare_reg(arg);
					orex(1,d,r,0x89); /* mov */

					o(0xc0 + REG_VALUE(r) * 8 + REG_VALUE(d));
				}
			}
		}
		vtop--;
	}

	/* Copy R10 and R11 into RCX and RDX, respectively */

	if (nb_args > 0) {
		o(0xd1894c); /* mov %r10, %rcx */

		if (nb_args > 1) {
			o(0xda894c); /* mov %r11, %rdx */

		}
	}

	gcall_or_jmp(0);

	if ((vtop->r & VT_SYM) && vtop->sym->v == TOK_alloca) {
		/* need to add the "func_scratch" area after alloca */

		o(0x48);
		func_alloca = oad(0x05, func_alloca); /* add $NN, %rax */

	}
	vtop--;
}

#define FUNC_PROLOG_SIZE 11

/* generate function prolog of type 't' */

void gfunc_prolog(Sym *func_sym)
{
	CType *func_type = &func_sym->type;
	int addr, reg_param_index, bt, size;
	Sym *sym;
	CType *type;

	func_ret_sub = 0;
	func_scratch = 32;
	func_alloca = 0;
	loc = 0;

	addr = PTR_SIZE * 2;
	ind += FUNC_PROLOG_SIZE;
	func_sub_sp_offset = ind;
	reg_param_index = 0;

	sym = func_type->ref;

	/* if the function returns a structure, then add an
	   implicit pointer parameter */

	size = gfunc_arg_size(&func_vt);
	if (!using_regs(size)) {
		gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
		func_vc = addr;
		reg_param_index++;
		addr += 8;
	}

	/* define parameters */

	while ((sym = sym->next) != NULL) {
		type = &sym->type;
		bt = type->t & VT_BTYPE;
		size = gfunc_arg_size(type);
		if (!using_regs(size)) {
			if (reg_param_index < REGN) {
				gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
			}
			gfunc_set_param(sym, addr, 1);
		} else {
			if (reg_param_index < REGN) {
				/* save arguments passed by register */

				if ((bt == VT_FLOAT) || (bt == VT_DOUBLE)) {
					if (tcc_state->nosse)
						tcc_error("SSE disabled");
					/* movq */

					gen_modrm32(0xd60f66, reg_param_index, VT_LOCAL, NULL, addr);
				} else {
					gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
				}
			}
			gfunc_set_param(sym, addr, 0);
		}
		addr += 8;
		reg_param_index++;
	}

	while (reg_param_index < REGN) {
		if (func_var) {
			gen_modrm64(0x89, arg_regs[reg_param_index], VT_LOCAL, NULL, addr);
			addr += 8;
		}
		reg_param_index++;
	}
}

/* generate function epilog */

void gfunc_epilog(void)
{
	int v, start;

	/* align local size to word & save local variables */

	func_scratch = (func_scratch + 15) & -16;
	loc = (loc & -16) - func_scratch;

	o(0xc9); /* leave */

	if (func_ret_sub == 0) {
		o(0xc3); /* ret */

	} else {
		o(0xc2); /* ret n */

		g(func_ret_sub);
		g(func_ret_sub >> 8);
	}

	v = -loc;
	start = func_sub_sp_offset - FUNC_PROLOG_SIZE;
	cur_text_section->data_offset = ind;
	pe_add_unwind_data(start, ind, v);

	ind = start;
	if (v >= 4096) {
		Sym *sym = external_helper_sym(TOK___chkstk);
		oad(0xb8, v); /* mov stacksize, %eax */

		oad(0xe8, 0); /* call __chkstk, (does the stackframe too) */

		greloca(cur_text_section, sym, ind-4, R_X86_64_PLT32, -4);
		o(0x90); /* fill for FUNC_PROLOG_SIZE = 11 bytes */

	} else {
		o(0xe5894855);  /* push %rbp, mov %rsp, %rbp */

		o(0xec8148);  /* sub rsp, stacksize */

		gen_le32(v);
	}
	ind = cur_text_section->data_offset;

	/* add the "func_scratch" area after each alloca seen */

	gsym_addr(func_alloca, -func_scratch);
}

ST_FUNC void gen_fill_nops(int bytes)
{
	while (bytes--)
		g(0x90);
}
/* generate a jump to a label */

int gjmp(int t)
{
	return gjmp2(0xe9, t);
}
/* generate a jump to a fixed address */

void gjmp_addr(int a)
{
	int r;
	r = a - ind - 2;
	if (r == (signed char)r) {
		g(0xeb);
		g(r);
	} else {
		oad(0xe9, a - ind - 5);
	}
}

ST_FUNC int gjmp_append(int n, int t)
{
	void *p;
	/* insert vtop->c jump list in t */

	if (n) {
		uint32_t n1 = n, n2;
		while ((n2 = read32le(p = cur_text_section->data + n1)))
			n1 = n2;
		write32le(p, t);
		t = n;
	}
	return t;
}

ST_FUNC int gjmp_cond(int op, int t)
{
	if (op & 0x100) {
		/* This was a float compare.  If the parity flag is set
			       the result was unordered.  For anything except != this
			       means false and we don't jump (anding both conditions).
			       For != this means true (oring both).
			       Take care about inverting the test.  We need to jump
			       to our target if the result was unordered and test wasn't NE,
			       otherwise if unordered we don't want to jump.  */

		int v = vtop->cmp_r;
		op &= ~0x100;
		if (op ^ v ^ (v != TOK_NE))
			o(0x067a);/* jp +6 */

		else {
			g(0x0f);
			t = gjmp2(0x8a, t);/* jp t */

		}
	}
	g(0x0f);
	t = gjmp2(op - 16, t);
	return t;
}
/* generate an integer binary operation */

void gen_opi(int op)
{
	int r, fr, opc, c;
	int ll, uu, cc;

	ll = is64_type(vtop[-1].type.t);
	uu = (vtop[-1].type.t & VT_UNSIGNED) != 0;
	cc = (vtop->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST;

	switch (op) {
	case '+':
	case TOK_ADDC1:/* add with carry generation */

		opc = 0;
gen_op8:
		if (cc && (!ll || (int)vtop->c.i == vtop->c.i)) {
			/* constant case */

			vswap();
			r = gv(RC_INT);
			vswap();
			c = vtop->c.i;
			if (c == (signed char)c) {
				/* XXX: generate inc and dec for smaller code ? */

				orex(ll, r, 0, 0x83);
				o(0xc0 | (opc << 3) | REG_VALUE(r));
				g(c);
			} else {
				orex(ll, r, 0, 0x81);
				oad(0xc0 | (opc << 3) | REG_VALUE(r), c);
			}
		} else {
			gv2(RC_INT, RC_INT);
			r = vtop[-1].r;
			fr = vtop[0].r;
			orex(ll, r, fr, (opc << 3) | 0x01);
			o(0xc0 + REG_VALUE(r) + REG_VALUE(fr) * 8);
		}
		vtop--;
		if (op >= TOK_ULT && op <= TOK_GT)
			vset_VT_CMP(op);
		break;
	case '-':
	case TOK_SUBC1:/* sub with carry generation */

		opc = 5;
		goto gen_op8;
	case TOK_ADDC2:/* add with carry use */

		opc = 2;
		goto gen_op8;
	case TOK_SUBC2:/* sub with carry use */

		opc = 3;
		goto gen_op8;
	case '&':
		opc = 4;
		goto gen_op8;
	case '^':
		opc = 6;
		goto gen_op8;
	case '|':
		opc = 1;
		goto gen_op8;
	case '*':
		gv2(RC_INT, RC_INT);
		r = vtop[-1].r;
		fr = vtop[0].r;
		orex(ll, fr, r, 0xaf0f);/* imul fr, r */

		o(0xc0 + REG_VALUE(fr) + REG_VALUE(r) * 8);
		vtop--;
		break;
	case TOK_SHL:
		opc = 4;
		goto gen_shift;
	case TOK_SHR:
		opc = 5;
		goto gen_shift;
	case TOK_SAR:
		opc = 7;
gen_shift:
		opc = 0xc0 | (opc << 3);
		if (cc) {
			/* constant case */

			vswap();
			r = gv(RC_INT);
			vswap();
			orex(ll, r, 0, 0xc1);/* shl/shr/sar $xxx, r */

			o(opc | REG_VALUE(r));
			g(vtop->c.i & (ll ? 63 : 31));
		} else {
			/* we generate the shift in ecx */

			gv2(RC_INT, RC_RCX);
			r = vtop[-1].r;
			orex(ll, r, 0, 0xd3);/* shl/shr/sar %cl, r */

			o(opc | REG_VALUE(r));
		}
		vtop--;
		break;
	case TOK_UDIV:
	case TOK_UMOD:
		uu = 1;
		goto divmod;
	case '/':
	case '%':
	case TOK_PDIV:
		uu = 0;
divmod:
		/* first operand must be in eax */
		/* XXX: need better constraint for second operand */

		gv2(RC_RAX, RC_RCX);
		r = vtop[-1].r;
		fr = vtop[0].r;
		vtop--;
		save_reg(TREG_RDX);
		orex(ll, 0, 0, uu ? 0xd231 : 0x99);/* xor %edx,%edx : cqto */

		orex(ll, fr, 0, 0xf7);/* div fr, %eax */

		o((uu ? 0xf0 : 0xf8) + REG_VALUE(fr));
		if (op == '%' || op == TOK_UMOD)
			r = TREG_RDX;
		else
			r = TREG_RAX;
		vtop->r = r;
		break;
	default:
		opc = 7;
		goto gen_op8;
	}
}

void gen_opl(int op)
{
	gen_opi(op);
}
/* generate a floating point operation 'v = t1 op t2' instruction. The
   two operands are guaranteed to have the same floating point type */
/* XXX: need to use ST1 too */

void gen_opf(int op)
{
	int a, ft, fc, swapped, r, opc;
	int bt = vtop->type.t & VT_BTYPE;
	int float_type = bt == VT_LDOUBLE ? RC_ST0 : RC_FLOAT;

	if (op == TOK_NEG) {/* unary minus */

		gv(float_type);
		if (float_type == RC_ST0) {
			o(0xe0d9);/* fchs */

		} else {
			save_reg(vtop->r);
			/* xor $0x80, $n(rbp) */

			gen_modrm32(0x80, 6, vtop->r, NULL, vtop->c.i + (bt == VT_DOUBLE ? 7 : 3));
			o(0x80);
			gv(float_type);/* -n is not a lvalue */

		}
		return;
	}
	/* convert constants to memory references */

	if ((vtop[-1].r & (VT_VALMASK | VT_LVAL)) == VT_CONST) {
		vswap();
		gv(float_type);
		vswap();
	}
	if ((vtop[0].r & (VT_VALMASK | VT_LVAL)) == VT_CONST)
		gv(float_type);
	/* must put at least one value in the floating point register */

	if ((vtop[-1].r & VT_LVAL) &&
	    (vtop[0].r & VT_LVAL)) {
		vswap();
		gv(float_type);
		vswap();
	}
	swapped = 0;
	/* swap the stack if needed so that t1 is the register and t2 is
	       the memory reference */

	if (vtop[-1].r & VT_LVAL) {
		vswap();
		swapped = 1;
	}
	if ((vtop->type.t & VT_BTYPE) == VT_LDOUBLE) {
		if (op >= TOK_ULT && op <= TOK_GT) {
			/* load on stack second operand */

			load(TREG_ST0, vtop);
			save_reg(TREG_RAX);/* eax is used by FP comparison code */

			if (op == TOK_GE || op == TOK_GT)
				swapped = !swapped;
			else if (op == TOK_EQ || op == TOK_NE)
				swapped = 0;
			if (swapped)
				o(0xc9d9);/* fxch %st(1) */

			if (op == TOK_EQ || op == TOK_NE)
				o(0xe9da);/* fucompp */

			else
				o(0xd9de);/* fcompp */

			o(0xe0df);/* fnstsw %ax */

			if (op == TOK_EQ) {
				o(0x45e480);/* and $0x45, %ah */

				o(0x40fC80);/* cmp $0x40, %ah */

			} else if (op == TOK_NE) {
				o(0x45e480);/* and $0x45, %ah */

				o(0x40f480);/* xor $0x40, %ah */

				op = TOK_NE;
			} else if (op == TOK_GE || op == TOK_LE) {
				o(0x05c4f6);/* test $0x05, %ah */

				op = TOK_EQ;
			} else {
				o(0x45c4f6);/* test $0x45, %ah */

				op = TOK_EQ;
			}
			vtop--;
			vset_VT_CMP(op);
		} else {
			/* no memory reference possible for long double operations */

			load(TREG_ST0, vtop);
			swapped = !swapped;

			switch (op) {
			default:
			case '+':
				a = 0;
				break;
			case '-':
				a = 4;
				if (swapped)
					a++;
				break;
			case '*':
				a = 1;
				break;
			case '/':
				a = 6;
				if (swapped)
					a++;
				break;
			}
			ft = vtop->type.t;
			fc = vtop->c.i;
			o(0xde);/* fxxxp %st, %st(1) */

			o(0xc1 + (a << 3));
			vtop--;
		}
	} else {
		if (op >= TOK_ULT && op <= TOK_GT) {
			/* if saved lvalue, then we must reload it */

			r = vtop->r;
			fc = vtop->c.i;
			if ((r & VT_VALMASK) == VT_LLOCAL) {
				SValue v1;
				r = get_reg(RC_INT);
				v1.type.t = VT_PTR;
				v1.r = VT_LOCAL | VT_LVAL;
				v1.c.i = fc;
				v1.sym = NULL;
				load(r, &v1);
				fc = 0;
				vtop->r = r = r | VT_LVAL;
			}

			if (op == TOK_EQ || op == TOK_NE) {
				swapped = 0;
			} else {
				if (op == TOK_LE || op == TOK_LT)
					swapped = !swapped;
				if (op == TOK_LE || op == TOK_GE) {
					op = 0x93;/* setae */

				} else {
					op = 0x97;/* seta */

				}
			}

			if (swapped) {
				gv(RC_FLOAT);
				vswap();
			}
			assert(!(vtop[-1].r & VT_LVAL));

			if (op == TOK_EQ || op == TOK_NE)
				opc = 0x2e0f;/* ucomisd */

			else
				opc = 0x2f0f;/* comisd */

			if ((vtop->type.t & VT_BTYPE) == VT_DOUBLE)
				opc = opc << 8 | 0x66;

			gen_modrm32(opc, vtop[-1].r, vtop->r, vtop->sym, fc);
			vtop--;
			vset_VT_CMP(op | 0x100);
			vtop->cmp_r = op;
		} else {
			assert((vtop->type.t & VT_BTYPE) != VT_LDOUBLE);
			switch (op) {
			default:
			case '+':
				a = 0;
				break;
			case '-':
				a = 4;
				break;
			case '*':
				a = 1;
				break;
			case '/':
				a = 6;
				break;
			}
			ft = vtop->type.t;
			fc = vtop->c.i;
			assert((ft & VT_BTYPE) != VT_LDOUBLE);
			/* if saved lvalue, then we must reload it */

			if ((vtop->r & VT_VALMASK) == VT_LLOCAL) {
				SValue v1;
				r = get_reg(RC_INT);
				v1.type.t = VT_PTR;
				v1.r = VT_LOCAL | VT_LVAL;
				v1.c.i = fc;
				v1.sym = NULL;
				load(r, &v1);
				fc = 0;
				vtop->r = r | VT_LVAL;
			}

			assert(!(vtop[-1].r & VT_LVAL));
			if (swapped) {
				assert(vtop->r & VT_LVAL);
				gv(RC_FLOAT);
				vswap();
				fc = vtop->c.i;/* bcheck may have saved previous vtop[-1] */

			}

			if ((ft & VT_BTYPE) == VT_DOUBLE) {
				o(0xf2);
			} else {
				o(0xf3);
			}
			o(0x0f);
			opc = 0x58 + a;

			gen_modrm32(opc, vtop[-1].r, vtop->r, vtop->sym, fc);
			vtop--;
		}
	}
}
/* convert integers to fp 't' type. Must handle 'int', 'unsigned int'
   and 'long long' cases. */

void gen_cvt_itof(int t)
{
	if ((t & VT_BTYPE) == VT_LDOUBLE) {
		save_reg(TREG_ST0);
		gv(RC_INT);
		if ((vtop->type.t & VT_BTYPE) == VT_LLONG) {
			/* signed long long to float/double/long double (unsigned case
			               is handled generically) */

			o(0x50 + (vtop->r & VT_VALMASK));/* push r */

			o(0x242cdf);/* fildll (%rsp) */

			o(0x08c48348);/* add $8, %rsp */

		} else if ((vtop->type.t & (VT_BTYPE | VT_UNSIGNED)) ==
			   (VT_INT | VT_UNSIGNED)) {
			/* unsigned int to float/double/long double */

			o(0x6a);/* push $0 */

			g(0x00);
			o(0x50 + (vtop->r & VT_VALMASK));/* push r */

			o(0x242cdf);/* fildll (%rsp) */

			o(0x10c48348);/* add $16, %rsp */

		} else {
			/* int to float/double/long double */

			o(0x50 + (vtop->r & VT_VALMASK));/* push r */

			o(0x2404db);/* fildl (%rsp) */

			o(0x08c48348);/* add $8, %rsp */

		}
		vtop->r = TREG_ST0;
	} else {
		int r = get_reg(RC_FLOAT);
		gv(RC_INT);
		o(0xf2 + ((t & VT_BTYPE) == VT_FLOAT?1:0));
		if ((vtop->type.t & (VT_BTYPE | VT_UNSIGNED)) ==
		    (VT_INT | VT_UNSIGNED) ||
		    (vtop->type.t & VT_BTYPE) == VT_LLONG) {
			o(0x48);/* REX */

		}
		o(0x2a0f);
		o(0xc0 + (vtop->r & VT_VALMASK) + REG_VALUE(r)*8);/* cvtsi2sd */

		vtop->r = r;
	}
}
/* convert from one floating point type to another */

void gen_cvt_ftof(int t)
{
	int ft, bt, tbt;

	ft = vtop->type.t;
	bt = ft & VT_BTYPE;
	tbt = t & VT_BTYPE;

	if (bt == VT_FLOAT) {
		gv(RC_FLOAT);
		if (tbt == VT_DOUBLE) {
			o(0x140f);/* unpcklps */

			o(0xc0 + REG_VALUE(vtop->r)*9);
			o(0x5a0f);/* cvtps2pd */

			o(0xc0 + REG_VALUE(vtop->r)*9);
		} else if (tbt == VT_LDOUBLE) {
			save_reg(RC_ST0);
			/* movss %xmm0,-0x10(%rsp) */

			o(0x110ff3);
			o(0x44 + REG_VALUE(vtop->r)*8);
			o(0xf024);
			o(0xf02444d9);/* flds -0x10(%rsp) */

			vtop->r = TREG_ST0;
		}
	} else if (bt == VT_DOUBLE) {
		gv(RC_FLOAT);
		if (tbt == VT_FLOAT) {
			o(0x140f66);/* unpcklpd */

			o(0xc0 + REG_VALUE(vtop->r)*9);
			o(0x5a0f66);/* cvtpd2ps */

			o(0xc0 + REG_VALUE(vtop->r)*9);
		} else if (tbt == VT_LDOUBLE) {
			save_reg(RC_ST0);
			/* movsd %xmm0,-0x10(%rsp) */

			o(0x110ff2);
			o(0x44 + REG_VALUE(vtop->r)*8);
			o(0xf024);
			o(0xf02444dd);/* fldl -0x10(%rsp) */

			vtop->r = TREG_ST0;
		}
	} else {
		int r;
		gv(RC_ST0);
		r = get_reg(RC_FLOAT);
		if (tbt == VT_DOUBLE) {
			o(0xf0245cdd);/* fstpl -0x10(%rsp) */

			/* movsd -0x10(%rsp),%xmm0 */

			o(0x100ff2);
			o(0x44 + REG_VALUE(r)*8);
			o(0xf024);
			vtop->r = r;
		} else if (tbt == VT_FLOAT) {
			o(0xf0245cd9);/* fstps -0x10(%rsp) */

			/* movss -0x10(%rsp),%xmm0 */

			o(0x100ff3);
			o(0x44 + REG_VALUE(r)*8);
			o(0xf024);
			vtop->r = r;
		}
	}
}
/* convert fp to int 't' type */

void gen_cvt_ftoi(int t)
{
	int ft, bt, size, r;
	ft = vtop->type.t;
	bt = ft & VT_BTYPE;
	if (bt == VT_LDOUBLE) {
		if (t != VT_INT) {
			vpush_helper_func(TOK___fixxfdi);
			vswap();
			gfunc_call(1);
			vpushi(0);
			vtop->r = REG_IRET;
			vtop->r2 = REG_IRE2;
			return;
		}
		gen_cvt_ftof(VT_DOUBLE);
		bt = VT_DOUBLE;
	}

	gv(RC_FLOAT);
	if (t != VT_INT)
		size = 8;
	else
		size = 4;

	r = get_reg(RC_INT);
	if (bt == VT_FLOAT) {
		o(0xf3);
	} else if (bt == VT_DOUBLE) {
		o(0xf2);
	} else {
		assert(0);
	}
	orex(size == 8, r, 0, 0x2c0f);/* cvttss2si or cvttsd2si */

	o(0xc0 + REG_VALUE(vtop->r) + REG_VALUE(r)*8);
	vtop->r = r;
}
// Generate sign extension from 32 to 64 bits:

ST_FUNC void gen_cvt_sxtw(void)
{
	int r = gv(RC_INT);
	/* x86_64 specific: movslq */

	o(0x6348);
	o(0xc0 + (REG_VALUE(r) << 3) + REG_VALUE(r));
}
/* char/short to int conversion */

ST_FUNC void gen_cvt_csti(int t)
{
	int r, sz, xl, ll;
	r = gv(RC_INT);
	sz = !(t & VT_UNSIGNED);
	xl = (t & VT_BTYPE) == VT_SHORT;
	ll = (vtop->type.t & VT_BTYPE) == VT_LLONG;
	orex(ll, r, 0, 0xc0b60f/* mov[sz] %a[xl], %eax */

	     | (sz << 3 | xl) << 8
	     | (REG_VALUE(r) << 3 | REG_VALUE(r)) << 16
	    );
}
/* increment tcov counter */

ST_FUNC void gen_increment_tcov (SValue *sv)
{
	o(0x058348);/* addq $1, xxx(%rip) */

	greloca(cur_text_section, sv->sym, ind, R_X86_64_PC32, -5);
	gen_le32(0);
	o(1);
}
/* computed goto support */

ST_FUNC void ggoto(void)
{
	gcall_or_jmp(1);
	vtop--;
}
/* Save the stack pointer onto the stack and return the location of its address */

ST_FUNC void gen_vla_sp_save(int addr)
{
	/* mov %rsp,addr(%rbp)*/

	gen_modrm64(0x89, TREG_RSP, VT_LOCAL, NULL, addr);
}
/* Restore the SP from a location on the stack */

ST_FUNC void gen_vla_sp_restore(int addr)
{
	gen_modrm64(0x8b, TREG_RSP, VT_LOCAL, NULL, addr);
}

/* Save result of gen_vla_alloc onto the stack */

ST_FUNC void gen_vla_result(int addr)
{
	/* mov %rax,addr(%rbp)*/

	gen_modrm64(0x89, TREG_RAX, VT_LOCAL, NULL, addr);
}
/* Subtract from the stack pointer, and push the resulting value onto the stack */
ST_FUNC void gen_vla_alloc(CType *type, int align)
{
	int use_call = 0;
	/* alloca does more than just adjust %rsp on Windows */

	use_call = 1;

	if (use_call) {
		vpush_helper_func(TOK_alloca);
		vswap();/* Move alloca ref past allocation size */

		gfunc_call(1);
	} else {
		int r;
		r = gv(RC_INT);/* allocation size */

		/* sub r,%rsp */

		o(0x2b48);
		o(0xe0 | REG_VALUE(r));
		/* We align to 16 bytes rather than align */
		/* and ~15, %rsp */

		o(0xf0e48348);
		vpop();
	}
}
/*
 * Assmuing the top part of the stack looks like below,
 *  src dest src
 */

ST_FUNC void gen_struct_copy(int size)
{
	int n = size / PTR_SIZE;

	o(0x5756); /* push rsi, rdi */

	gv2(RC_RDI, RC_RSI);
	if (n <= 4) {
		while (n)
			o(0xa548), --n;
	} else {
		vpushi(n);
		gv(RC_RCX);
		o(0xa548f3);
		vpop();
	}
	if (size & 0x04)
		o(0xa5);
	if (size & 0x02)
		o(0xa566);
	if (size & 0x01)
		o(0xa4);

	o(0x5e5f); /* pop rdi, rsi */

	vpop();
	vpop();
}
/* end of x86-64 code generator */
/**/
/**/

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) s1->sym
#define TCC_SET_STATE(fn) (tcc_enter_state(s1),fn)
/* Returns 1 for a code relocation, 0 for a data relocation. For unknown
   relocations, returns -1. */

ST_FUNC int code_reloc (int reloc_type)
{
	switch (reloc_type) {
	case R_X86_64_32:
	case R_X86_64_32S:
	case R_X86_64_64:
	case R_X86_64_GOTPC32:
	case R_X86_64_GOTPC64:
	case R_X86_64_GOTPCREL:
	case R_X86_64_GOTPCRELX:
	case R_X86_64_REX_GOTPCRELX:
	case R_X86_64_GOTTPOFF:
	case R_X86_64_GOT32:
	case R_X86_64_GOT64:
	case R_X86_64_GLOB_DAT:
	case R_X86_64_COPY:
	case R_X86_64_RELATIVE:
	case R_X86_64_GOTOFF64:
	case R_X86_64_TLSGD:
	case R_X86_64_TLSLD:
	case R_X86_64_DTPOFF32:
	case R_X86_64_TPOFF32:
	case R_X86_64_DTPOFF64:
	case R_X86_64_TPOFF64:
		return 0;

	case R_X86_64_PC32:
	case R_X86_64_PC64:
	case R_X86_64_PLT32:
	case R_X86_64_PLTOFF64:
	case R_X86_64_JUMP_SLOT:
		return 1;
	}
	return -1;
}
/* Returns an enumerator to describe whether and when the relocation needs a
   GOT and/or PLT entry to be created. See tcc.h for a description of the
   different values. */

ST_FUNC int gotplt_entry_type (int reloc_type)
{
	switch (reloc_type) {
	case R_X86_64_GLOB_DAT:
	case R_X86_64_JUMP_SLOT:
	case R_X86_64_COPY:
	case R_X86_64_RELATIVE:
		return NO_GOTPLT_ENTRY;
	/* The following relocs wouldn't normally need GOT or PLT
		   slots, but we need them for simplicity in the link
		   editor part.  See our caller for comments.  */

	case R_X86_64_32:
	case R_X86_64_32S:
	case R_X86_64_64:
	case R_X86_64_PC32:
	case R_X86_64_PC64:
		return AUTO_GOTPLT_ENTRY;

	case R_X86_64_GOTTPOFF:
		return BUILD_GOT_ONLY;

	case R_X86_64_GOT32:
	case R_X86_64_GOT64:
	case R_X86_64_GOTPC32:
	case R_X86_64_GOTPC64:
	case R_X86_64_GOTOFF64:
	case R_X86_64_GOTPCREL:
	case R_X86_64_GOTPCRELX:
	case R_X86_64_TLSGD:
	case R_X86_64_TLSLD:
	case R_X86_64_DTPOFF32:
	case R_X86_64_DTPOFF64:
	case R_X86_64_REX_GOTPCRELX:
	case R_X86_64_PLT32:
	case R_X86_64_PLTOFF64:
		return ALWAYS_GOTPLT_ENTRY;

	case R_X86_64_TPOFF32:
	case R_X86_64_TPOFF64:
		return NO_GOTPLT_ENTRY;
	}

	return -1;
}

ST_FUNC unsigned create_plt_entry(TCCState *s1, unsigned got_offset,
				  struct sym_attr *attr)
{
	Section *plt = s1->plt;
	uint8_t *p;
	int modrm;
	unsigned plt_offset, relofs;

	modrm = 0x25;
	/* empty PLT: create PLT0 entry that pushes the library identifier
	       (GOT + PTR_SIZE) and jumps to ld.so resolution routine
	       (GOT + 2 * PTR_SIZE) */

	if (plt->data_offset == 0) {
		p = section_ptr_add(plt, 16);
		p[0] = 0xff;/* pushl got + PTR_SIZE */

		p[1] = modrm + 0x10;
		write32le(p + 2, PTR_SIZE);
		p[6] = 0xff;/* jmp *(got + PTR_SIZE * 2) */

		p[7] = modrm;
		write32le(p + 8, PTR_SIZE * 2);
	}
	plt_offset = plt->data_offset;
	/* The PLT slot refers to the relocation entry it needs via offset.
	       The reloc entry is created below, so its offset is the current
	       data_offset */

	relofs = s1->plt->reloc ? s1->plt->reloc->data_offset : 0;
	/* Jump to GOT entry where ld.so initially put the address of ip + 4 */

	p = section_ptr_add(plt, 16);
	p[0] = 0xff;/* jmp *(got + x) */

	p[1] = modrm;
	write32le(p + 2, got_offset);
	p[6] = 0x68;/* push $xxx */

	/* On x86-64, the relocation is referred to by _index_ */

	write32le(p + 7, relofs / sizeof (ElfW_Rel) - 1);
	p[11] = 0xe9;/* jmp plt_start */

	write32le(p + 12, -(plt->data_offset));
	return plt_offset;
}
/* relocate the PLT: compute addresses and offsets in the PLT now that final
   address for PLT and GOT are known (see fill_program_header) */

ST_FUNC void relocate_plt(TCCState *s1)
{
	uint8_t *p, *p_end;

	if (!s1->plt)
		return;

	p = s1->plt->data;
	p_end = p + s1->plt->data_offset;

	if (p < p_end) {
		int x = s1->got->sh_addr - s1->plt->sh_addr - 6;
		add32le(p + 2, x);
		add32le(p + 8, x - 6);
		p += 16;
		while (p < p_end) {
			add32le(p + 2, x + (s1->plt->data - p));
			p += 16;
		}
	}

	if (s1->plt->reloc) {
		ElfW_Rel *rel;
		int x = s1->plt->sh_addr + 16 + 6;
		p = s1->got->data;
		for_each_elem(s1->plt->reloc, 0, rel, ElfW_Rel) {
			write64le(p + rel->r_offset, x);
			x += 16;
		}
	}
}

ST_FUNC void relocate(TCCState *s1, ElfW_Rel *rel, int type, unsigned char *ptr,
		      addr_t addr, addr_t val)
{
	int sym_index, esym_index;

	sym_index = ELFW(R_SYM)(rel->r_info);

	switch (type) {
	case R_X86_64_64:
		if (s1->output_type & TCC_OUTPUT_DYN) {
			esym_index = get_sym_attr(s1, sym_index, 0)->dyn_index;
			qrel->r_offset = rel->r_offset;
			if (esym_index) {
				qrel->r_info = ELFW(R_INFO)(esym_index, R_X86_64_64);
				qrel->r_addend = rel->r_addend;
				qrel++;
				break;
			} else {
				qrel->r_info = ELFW(R_INFO)(0, R_X86_64_RELATIVE);
				qrel->r_addend = read64le(ptr) + val;
				qrel++;
			}
		}
		add64le(ptr, val);
		break;
	case R_X86_64_32:
	case R_X86_64_32S:
		if (s1->output_type & TCC_OUTPUT_DYN) {
			/* XXX: this logic may depend on TCC's codegen
			                   now TCC uses R_X86_64_32 even for a 64bit pointer */

			qrel->r_offset = rel->r_offset;
			qrel->r_info = ELFW(R_INFO)(0, R_X86_64_RELATIVE);
			/* Use sign extension! */

			qrel->r_addend = (int)read32le(ptr) + val;
			qrel++;
		}
		if ((type == R_X86_64_32 ? val != (unsigned)val : val != (int)val)
		    /* ignore relocation check for stab section */

		    && (stab_section == NULL ||
			addr < stab_section->sh_addr ||
			addr >= (stab_section->sh_addr + stab_section->data_offset))) {
			tcc_error_noabort("relocation 'R_X86_64_32[S]' out of range");
		}
		add32le(ptr, val);
		break;

	case R_X86_64_PC32:
		if (s1->output_type == TCC_OUTPUT_DLL) {
			/* DLL relocation */

			esym_index = get_sym_attr(s1, sym_index, 0)->dyn_index;
			if (esym_index) {
				qrel->r_offset = rel->r_offset;
				qrel->r_info = ELFW(R_INFO)(esym_index, R_X86_64_PC32);
				/* Use sign extension! */

				qrel->r_addend = (int)read32le(ptr) + rel->r_addend;
				qrel++;
				break;
			}
		}
		goto plt32pc32;

	case R_X86_64_PLT32:
		/* fallthrough: val already holds the PLT slot address */

plt32pc32: {
			long long diff;
			diff = (long long)val - addr;
			if (diff < -2147483648LL || diff > 2147483647LL) {

				/* ignore overflow with undefined weak symbols */

				if (((ElfW(Sym) *)symtab_section->data)[sym_index].st_shndx != SHN_UNDEF)

					tcc_error_noabort("relocation '%d' out of range", type);
			}
			add32le(ptr, diff);
		}
		break;

	case R_X86_64_COPY:
		break;

	case R_X86_64_PLTOFF64:
		add64le(ptr, val - s1->got->sh_addr + rel->r_addend);
		break;

	case R_X86_64_PC64:
		if (s1->output_type == TCC_OUTPUT_DLL) {
			/* DLL relocation */

			esym_index = get_sym_attr(s1, sym_index, 0)->dyn_index;
			if (esym_index) {
				qrel->r_offset = rel->r_offset;
				qrel->r_info = ELFW(R_INFO)(esym_index, R_X86_64_PC64);
				qrel->r_addend = read64le(ptr) + rel->r_addend;
				qrel++;
				break;
			}
		}
		add64le(ptr, val - addr);
		break;

	case R_X86_64_GLOB_DAT:
	case R_X86_64_JUMP_SLOT:
		/* They don't need addend */

		write64le(ptr, val - rel->r_addend);
		break;
	case R_X86_64_GOTPCREL:
	case R_X86_64_GOTPCRELX:
	case R_X86_64_REX_GOTPCRELX:
		add32le(ptr, s1->got->sh_addr - addr +
			get_sym_attr(s1, sym_index, 0)->got_offset - 4);
		break;
	case R_X86_64_GOTPC32:
		add32le(ptr, s1->got->sh_addr - addr + rel->r_addend);
		break;
	case R_X86_64_GOTPC64:
		add64le(ptr, s1->got->sh_addr - addr + rel->r_addend);
		break;
	case R_X86_64_GOTTPOFF:
		add32le(ptr, val - s1->got->sh_addr);
		break;
	case R_X86_64_GOT32:
		/* we load the got offset */

		add32le(ptr, get_sym_attr(s1, sym_index, 0)->got_offset);
		break;
	case R_X86_64_GOT64:
		/* we load the got offset */

		add64le(ptr, get_sym_attr(s1, sym_index, 0)->got_offset);
		break;
	case R_X86_64_GOTOFF64:
		add64le(ptr, val - s1->got->sh_addr);
		break;
	case R_X86_64_TLSGD: {
		static const unsigned char expect[] = {
			/* .byte 0x66; lea 0(%rip),%rdi */

			0x66, 0x48, 0x8d, 0x3d, 0x00, 0x00, 0x00, 0x00,
			/* .word 0x6666; rex64; call __tls_get_addr@PLT */

			0x66, 0x66, 0x48, 0xe8, 0x00, 0x00, 0x00, 0x00
		};
		static const unsigned char replace[] = {
			/* mov %fs:0,%rax */

			0x64, 0x48, 0x8b, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00,
			/* lea -4(%rax),%rax */

			0x48, 0x8d, 0x80, 0x00, 0x00, 0x00, 0x00
		};

		if (memcmp (ptr-4, expect, sizeof(expect)) == 0) {
			ElfW(Sym) *sym;
			Section *sec;
			int32_t x;

			memcpy(ptr-4, replace, sizeof(replace));
			rel[1].r_info = ELFW(R_INFO)(0, R_X86_64_NONE);
			sym = &((ElfW(Sym) *)symtab_section->data)[sym_index];
			sec = s1->sections[sym->st_shndx];
			x = sym->st_value - sec->sh_addr - sec->data_offset;
			add32le(ptr + 8, x);
		} else
			tcc_error_noabort("unexpected R_X86_64_TLSGD pattern");
	}
	break;
	case R_X86_64_TLSLD: {
		static const unsigned char expect[] = {
			/* lea 0(%rip),%rdi */

			0x48, 0x8d, 0x3d, 0x00, 0x00, 0x00, 0x00,
			/* call __tls_get_addr@PLT */

			0xe8, 0x00, 0x00, 0x00, 0x00
		};
		static const unsigned char replace[] = {
			/* data16 data16 data16 mov %fs:0,%rax */

			0x66, 0x66, 0x66, 0x64, 0x48, 0x8b, 0x04, 0x25,
			0x00, 0x00, 0x00, 0x00
		};

		if (memcmp (ptr-3, expect, sizeof(expect)) == 0) {
			memcpy(ptr-3, replace, sizeof(replace));
			rel[1].r_info = ELFW(R_INFO)(0, R_X86_64_NONE);
		} else
			tcc_error_noabort("unexpected R_X86_64_TLSLD pattern");
	}
	break;

	case R_X86_64_DTPOFF32:
	case R_X86_64_TPOFF32:
	case R_X86_64_DTPOFF64:
	case R_X86_64_TPOFF64: {
		int32_t x;
		if (s1->tls_end) {
			x = val - s1->tls_end;
		} else {
			ElfW(Sym) *sym = &((ElfW(Sym) *)symtab_section->data)[sym_index];
			Section *sec = s1->sections[sym->st_shndx];
			x = val - sec->sh_addr - sec->data_offset;
		}
		switch (type) {
		case R_X86_64_DTPOFF64:
		case R_X86_64_TPOFF64:
			add64le(ptr, x);
			break;
		default:
			add32le(ptr, x);
			break;
		}
	}
	break;
	case R_X86_64_NONE:
		break;
	case R_X86_64_RELATIVE:

		add32le(ptr, val - s1->pe_imagebase);
		/* do nothing */

		break;
	default:
		fprintf(stderr,"FIXME: handle reloc type %d at %x [%p] to %x\n",
			type, (unsigned)addr, ptr, (unsigned)val);
		break;
	}
}
/* ==================== i386-asm.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) tcc_state->sym
#define TCC_SET_STATE(fn) fn
#undef _tcc_error

#define MAX_OPERANDS 3

#define TOK_ASM_first TOK_ASM_clc
#define TOK_ASM_last TOK_ASM_emms
#define TOK_ASM_alllast TOK_ASM_subps
/* only used with OPC_WL */

#define OPC_B 0x01
/* accepts w, l or no suffix */
#define OPC_WL 0x02
/* accepts b, w, l or no suffix */
#define OPC_BWL (OPC_B | OPC_WL)
/* register is added to opcode */
#define OPC_REG 0x04
/* modrm encoding */
#define OPC_MODRM 0x08

#define OPCT_MASK 0x70
/* add fwait opcode */
#define OPC_FWAIT 0x10
/* shift opcodes */
#define OPC_SHIFT 0x20
/* arithmetic opcodes */
#define OPC_ARITH 0x30
/* FPU arithmetic opcodes */
#define OPC_FARITH 0x40
/* test opcodes */
#define OPC_TEST 0x50
/* 0x0f01XX (group 7, XX is 2nd opcode,
                               no operands and unstructured mod/rm) */

#define OPC_0F01 0x60
#define OPCT_IS(v,i) (((v) & OPCT_MASK) == (i))
/* Is secondary map (0x0f prefix) */

#define OPC_0F 0x100
/* Always has REX prefix */
#define OPC_48 0x200
/* accepts w, l, q or no suffix */

#define OPC_WLQ 0x1000
/* accepts b, w, l, q or no suffix */
#define OPC_BWLQ (OPC_B | OPC_WLQ)
#define OPC_WLX OPC_WLQ
#define OPC_BWLX OPC_BWLQ

#define OPC_GROUP_SHIFT 13
/* in order to compress the operand type, we use specific operands and
   we or only with EA  */

enum {
	OPT_REG8=0,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_REG16,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_REG32,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_REG64,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_MMX,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_SSE,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_CR,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_TR,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_DB,/* warning: value is hardcoded from TOK_ASM_xxx */

	OPT_SEG,
	OPT_ST,

	OPT_REG8_LOW,/* %spl,%bpl,%sil,%dil, encoded like ah,ch,dh,bh, but
		     with REX prefix, not used in insn templates */

	OPT_IM8,
	OPT_IM8S,
	OPT_IM16,
	OPT_IM32,

	OPT_IM64,

	OPT_EAX,/* %al, %ax, %eax or %rax register */

	OPT_ST0,/* %st(0) register */

	OPT_CL,/* %cl register */

	OPT_DX,/* %dx register */

	OPT_ADDR,/* OP_EA with only offset */

	OPT_INDIR,/* *(expr) */

	/* composite types */

	OPT_COMPOSITE_FIRST,
	OPT_IM,/* IM8 | IM16 | IM32 */

	OPT_REG,/* REG8 | REG16 | REG32 | REG64 */

	OPT_REGW,/* REG16 | REG32 | REG64 */

	OPT_IMW,/* IM16 | IM32 */

	OPT_MMXSSE,/* MMX | SSE */

	OPT_DISP,/* Like OPT_ADDR, but emitted as displacement (for jumps) */

	OPT_DISP8,/* Like OPT_ADDR, but only 8bit (short jumps) */

	/* can be ored with any OPT_xxx */

	OPT_EA = 0x80
};

#define OP_REG8 (1 << OPT_REG8)
#define OP_REG16 (1 << OPT_REG16)
#define OP_REG32 (1 << OPT_REG32)
#define OP_MMX (1 << OPT_MMX)
#define OP_SSE (1 << OPT_SSE)
#define OP_CR (1 << OPT_CR)
#define OP_TR (1 << OPT_TR)
#define OP_DB (1 << OPT_DB)
#define OP_SEG (1 << OPT_SEG)
#define OP_ST (1 << OPT_ST)
#define OP_IM8 (1 << OPT_IM8)
#define OP_IM8S (1 << OPT_IM8S)
#define OP_IM16 (1 << OPT_IM16)
#define OP_IM32 (1 << OPT_IM32)
#define OP_EAX (1 << OPT_EAX)
#define OP_ST0 (1 << OPT_ST0)
#define OP_CL (1 << OPT_CL)
#define OP_DX (1 << OPT_DX)
#define OP_ADDR (1 << OPT_ADDR)
#define OP_INDIR (1 << OPT_INDIR)

#define OP_REG64 (1 << OPT_REG64)
#define OP_REG8_LOW (1 << OPT_REG8_LOW)
#define OP_IM64 (1 << OPT_IM64)
#define OP_EA32 (OP_EA << 1)

#define OP_EA 0x40000000u
#define OP_REG (OP_REG8 | OP_REG16 | OP_REG32 | OP_REG64)

#define TREG_XAX TREG_RAX
#define TREG_XCX TREG_RCX
#define TREG_XDX TREG_RDX
#define TOK_ASM_xax TOK_ASM_rax

typedef struct ASMInstr {
	uint16_t sym;
	uint16_t opcode;
	uint16_t instr_type;
	uint8_t nb_ops;
	uint8_t op_type[MAX_OPERANDS];/* see OP_xxx */

} ASMInstr;

typedef struct Operand {
	uint32_t type;
	int8_t reg;/* register, -1 if none */

	int8_t reg2;/* second register, -1 if none */

	uint8_t shift;
	ExprValue e;
} Operand;

static const uint8_t reg_to_size[9] = {
	/*
	    [OP_REG8] = 0,
	    [OP_REG16] = 1,
	    [OP_REG32] = 2,
	#ifdef TCC_TARGET_X86_64
	    [OP_REG64] = 3,
	#endif
	*/
	0, 0, 1, 0, 2, 0, 0, 0, 3
};

#define NB_TEST_OPCODES 30

static const uint8_t test_bits[NB_TEST_OPCODES] = {
	0x00,/* o */

	0x01,/* no */

	0x02,/* b */

	0x02,/* c */

	0x02,/* nae */

	0x03,/* nb */

	0x03,/* nc */

	0x03,/* ae */

	0x04,/* e */

	0x04,/* z */

	0x05,/* ne */

	0x05,/* nz */

	0x06,/* be */

	0x06,/* na */

	0x07,/* nbe */

	0x07,/* a */

	0x08,/* s */

	0x09,/* ns */

	0x0a,/* p */

	0x0a,/* pe */

	0x0b,/* np */

	0x0b,/* po */

	0x0c,/* l */

	0x0c,/* nge */

	0x0d,/* nl */

	0x0d,/* ge */

	0x0e,/* le */

	0x0e,/* ng */

	0x0f,/* nle */

	0x0f,/* g */

};

static const uint8_t segment_prefixes[] = {
	0x26,/* es */

	0x2e,/* cs */

	0x36,/* ss */

	0x3e,/* ds */

	0x64,/* fs */

	0x65/* gs */

};

static const ASMInstr asm_instrs[] = {
#define ALT(x) x
	/* This removes a 0x0f in the second byte */

#define O(o) ((uint64_t) ((((o) & 0xff00) == 0x0f00) ? ((((o) >> 8) & ~0xff) | ((o) & 0xff)) : (o)))
	/* This constructs instr_type from opcode, type and group.  */

#define T(o,i,g) ((i) | ((g) << OPC_GROUP_SHIFT) | ((((o) & 0xff00) == 0x0f00) ? OPC_0F : 0))
#define DEF_ASM_OP0(name,opcode)
#define DEF_ASM_OP0L(name,opcode,group,instr_type) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 0, { 0 } },
#define DEF_ASM_OP1(name,opcode,group,instr_type,op0) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 1, { op0 }},
#define DEF_ASM_OP2(name,opcode,group,instr_type,op0,op1) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 2, { op0, op1 }},
#define DEF_ASM_OP3(name,opcode,group,instr_type,op0,op1,op2) { TOK_ASM_ ## name, O(opcode), T(opcode, instr_type, group), 3, { op0, op1, op2 }},

	DEF_ASM_OP0(clc, 0xf8)/* must be first OP0 */

	DEF_ASM_OP0(cld, 0xfc)
	DEF_ASM_OP0(cli, 0xfa)
	DEF_ASM_OP0(clts, 0x0f06)
	DEF_ASM_OP0(cmc, 0xf5)
	DEF_ASM_OP0(lahf, 0x9f)
	DEF_ASM_OP0(sahf, 0x9e)
	DEF_ASM_OP0(pushfq, 0x9c)
	DEF_ASM_OP0(popfq, 0x9d)
	DEF_ASM_OP0(pushf, 0x9c)
	DEF_ASM_OP0(popf, 0x9d)
	DEF_ASM_OP0(stc, 0xf9)
	DEF_ASM_OP0(std, 0xfd)
	DEF_ASM_OP0(sti, 0xfb)
	DEF_ASM_OP0(aaa, 0x37)
	DEF_ASM_OP0(aas, 0x3f)
	DEF_ASM_OP0(daa, 0x27)
	DEF_ASM_OP0(das, 0x2f)
	DEF_ASM_OP0(aad, 0xd50a)
	DEF_ASM_OP0(aam, 0xd40a)
	DEF_ASM_OP0(cbw, 0x6698)
	DEF_ASM_OP0(cwd, 0x6699)
	DEF_ASM_OP0(cwde, 0x98)
	DEF_ASM_OP0(cdq, 0x99)
	DEF_ASM_OP0(cbtw, 0x6698)
	DEF_ASM_OP0(cwtl, 0x98)
	DEF_ASM_OP0(cwtd, 0x6699)
	DEF_ASM_OP0(cltd, 0x99)
	DEF_ASM_OP0(cqto, 0x4899)
	DEF_ASM_OP0(int3, 0xcc)
	DEF_ASM_OP0(into, 0xce)
	DEF_ASM_OP0(iret, 0xcf)
	DEF_ASM_OP0(iretw, 0x66cf)
	DEF_ASM_OP0(iretl, 0xcf)
	DEF_ASM_OP0(iretq, 0x48cf)
	DEF_ASM_OP0(rsm, 0x0faa)
	DEF_ASM_OP0(hlt, 0xf4)
	DEF_ASM_OP0(wait, 0x9b)
	DEF_ASM_OP0(nop, 0x90)
	DEF_ASM_OP0(pause, 0xf390)
	DEF_ASM_OP0(xlat, 0xd7)

	DEF_ASM_OP0L(vmcall, 0xc1, 0, OPC_0F01)
	DEF_ASM_OP0L(vmlaunch, 0xc2, 0, OPC_0F01)
	DEF_ASM_OP0L(vmresume, 0xc3, 0, OPC_0F01)
	DEF_ASM_OP0L(vmxoff, 0xc4, 0, OPC_0F01)
	/* strings */

	ALT(DEF_ASM_OP0L(cmpsb, 0xa6, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(scmpb, 0xa6, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(insb, 0x6c, 0, OPC_BWL))
	ALT(DEF_ASM_OP0L(outsb, 0x6e, 0, OPC_BWL))

	ALT(DEF_ASM_OP0L(lodsb, 0xac, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(slodb, 0xac, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(movsb, 0xa4, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(smovb, 0xa4, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(scasb, 0xae, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sscab, 0xae, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(stosb, 0xaa, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sstob, 0xaa, 0, OPC_BWLX))
	/* bits */

	ALT(DEF_ASM_OP2(bsfw, 0x0fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(bsrw, 0x0fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

	ALT(DEF_ASM_OP2(btw, 0x0fa3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btw, 0x0fba, 4, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btsw, 0x0fab, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btsw, 0x0fba, 5, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btrw, 0x0fb3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btrw, 0x0fba, 6, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btcw, 0x0fbb, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btcw, 0x0fba, 7, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(popcntw, 0xf30fb8, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

	ALT(DEF_ASM_OP2(tzcntw, 0xf30fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(lzcntw, 0xf30fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
	/* prefixes */

	DEF_ASM_OP0(lock, 0xf0)
	DEF_ASM_OP0(rep, 0xf3)
	DEF_ASM_OP0(repe, 0xf3)
	DEF_ASM_OP0(repz, 0xf3)
	DEF_ASM_OP0(repne, 0xf2)
	DEF_ASM_OP0(repnz, 0xf2)

	DEF_ASM_OP0(invd, 0x0f08)
	DEF_ASM_OP0(wbinvd, 0x0f09)
	DEF_ASM_OP0(cpuid, 0x0fa2)
	DEF_ASM_OP0(wrmsr, 0x0f30)
	DEF_ASM_OP0(rdtsc, 0x0f31)
	DEF_ASM_OP0(rdmsr, 0x0f32)
	DEF_ASM_OP0(rdpmc, 0x0f33)

	DEF_ASM_OP0(syscall, 0x0f05)
	DEF_ASM_OP0(sysret, 0x0f07)
	DEF_ASM_OP0L(sysretq, 0x480f07, 0, 0)
	DEF_ASM_OP0(ud2, 0x0f0b)
	/* NOTE: we took the same order as gas opcode definition order */
	/* Right now we can't express the fact that 0xa1/0xa3 can't use $eax and a
	   32 bit moffset as operands.
	ALT(DEF_ASM_OP2(movb, 0xa0, 0, OPC_BWLX, OPT_ADDR, OPT_EAX))
	ALT(DEF_ASM_OP2(movb, 0xa2, 0, OPC_BWLX, OPT_EAX, OPT_ADDR)) */

	ALT(DEF_ASM_OP2(movb, 0x88, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movb, 0x8a, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	/* The moves are special: the 0xb8 form supports IM64 (the only insn that
	   does) with REG64.  It doesn't support IM32 with REG64, it would use
	   the full movabs form (64bit immediate).  For IM32->REG64 we prefer
	   the 0xc7 opcode.  So disallow all 64bit forms and code the rest by hand. */

	ALT(DEF_ASM_OP2(movb, 0xb0, 0, OPC_REG | OPC_BWLX, OPT_IM, OPT_REG))
	ALT(DEF_ASM_OP2(mov, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movq, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movb, 0xc6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(movw, 0x8c, 0, OPC_MODRM | OPC_WLX, OPT_SEG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movw, 0x8e, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_SEG))

	ALT(DEF_ASM_OP2(movw, 0x0f20, 0, OPC_MODRM | OPC_WLX, OPT_CR, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f21, 0, OPC_MODRM | OPC_WLX, OPT_DB, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f22, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_CR))
	ALT(DEF_ASM_OP2(movw, 0x0f23, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_DB))

	ALT(DEF_ASM_OP2(movsbw, 0x660fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG16))
	ALT(DEF_ASM_OP2(movsbl, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movsbq, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(movswl, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movswq, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movslq, 0x63, 0, OPC_MODRM, OPT_REG32 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movzbw, 0x0fb6, 0, OPC_MODRM | OPC_WLX, OPT_REG8 | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(movzwl, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movzwq, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP1(pushq, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(push, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x666a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(pushw, 0xff, 6, OPC_MODRM | OPC_WLX, OPT_REG64 | OPT_EA))
	ALT(DEF_ASM_OP1(pushw, 0x6668, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(pushw, 0x68, 0, OPC_WLX, OPT_IM32))
	ALT(DEF_ASM_OP1(pushw, 0x06, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(popw, 0x8f, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP1(popw, 0x07, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_REGW, OPT_EAX))
	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_EAX, OPT_REGW))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))

	ALT(DEF_ASM_OP2(inb, 0xe4, 0, OPC_BWL, OPT_IM8, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xe4, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(inb, 0xec, 0, OPC_BWL, OPT_DX, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xec, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(outb, 0xe6, 0, OPC_BWL, OPT_EAX, OPT_IM8))
	ALT(DEF_ASM_OP1(outb, 0xe6, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(outb, 0xee, 0, OPC_BWL, OPT_EAX, OPT_DX))
	ALT(DEF_ASM_OP1(outb, 0xee, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(leaw, 0x8d, 0, OPC_MODRM | OPC_WLX, OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP2(les, 0xc4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lds, 0xc5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lss, 0x0fb2, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lfs, 0x0fb4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lgs, 0x0fb5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	/* arith */

	ALT(DEF_ASM_OP2(addb, 0x00, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))/* XXX: use D bit ? */

	ALT(DEF_ASM_OP2(addb, 0x02, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	ALT(DEF_ASM_OP2(addb, 0x04, 0, OPC_ARITH | OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(addw, 0x83, 0, OPC_ARITH | OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(addb, 0x80, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0xa8, 0, OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(testb, 0xf6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP1(incb, 0xfe, 0, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(decb, 0xfe, 1, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(notb, 0xf6, 2, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(negb, 0xf6, 3, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(mulb, 0xf6, 4, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(imulb, 0xf6, 5, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(imulw, 0x0faf, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP3(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW))
	ALT(DEF_ASM_OP3(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW))

	ALT(DEF_ASM_OP1(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
	ALT(DEF_ASM_OP1(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
	/* shifts */

	ALT(DEF_ASM_OP2(rolb, 0xc0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_IM8, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(rolb, 0xd2, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_CL, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP1(rolb, 0xd0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP3(shldw, 0x0fa4, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fac, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))

	ALT(DEF_ASM_OP1(call, 0xff, 2, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(call, 0xe8, 0, 0, OPT_DISP))
	DEF_ASM_OP1(callq, 0xff, 2, OPC_MODRM, OPT_INDIR)
	ALT(DEF_ASM_OP1(callq, 0xe8, 0, 0, OPT_DISP))
	ALT(DEF_ASM_OP1(jmp, 0xff, 4, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(jmp, 0xeb, 0, 0, OPT_DISP8))

	ALT(DEF_ASM_OP1(lcall, 0xff, 3, OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(ljmp, 0xff, 5, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(ljmpw, 0x66ff, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(ljmpl, 0xff, 5, OPC_MODRM, OPT_EA)

	ALT(DEF_ASM_OP1(int, 0xcd, 0, 0, OPT_IM8))
	ALT(DEF_ASM_OP1(seto, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	ALT(DEF_ASM_OP1(setob, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	DEF_ASM_OP2(enter, 0xc8, 0, 0, OPT_IM16, OPT_IM8)
	DEF_ASM_OP0(leave, 0xc9)
	DEF_ASM_OP0(ret, 0xc3)
	DEF_ASM_OP0(retq, 0xc3)
	ALT(DEF_ASM_OP1(retq, 0xc2, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(ret, 0xc2, 0, 0, OPT_IM16))
	DEF_ASM_OP0(lret, 0xcb)
	ALT(DEF_ASM_OP1(lret, 0xca, 0, 0, OPT_IM16))

	ALT(DEF_ASM_OP1(jo, 0x70, 0, OPC_TEST, OPT_DISP8))
	DEF_ASM_OP1(loopne, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopnz, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loope, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopz, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loop, 0xe2, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(jecxz, 0x67e3, 0, 0, OPT_DISP8)
	/* float */
	/* specific fcomp handling */

	ALT(DEF_ASM_OP0L(fcomp, 0xd8d9, 0, 0))

	ALT(DEF_ASM_OP1(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(fadd, 0xdcc0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP2(fmul, 0xdcc8, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(fadd, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(faddp, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(fadds, 0xd8, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiaddl, 0xda, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(faddl, 0xdc, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiadds, 0xde, 0, OPC_FARITH | OPC_MODRM, OPT_EA))

	DEF_ASM_OP0(fucompp, 0xdae9)
	DEF_ASM_OP0(ftst, 0xd9e4)
	DEF_ASM_OP0(fxam, 0xd9e5)
	DEF_ASM_OP0(fld1, 0xd9e8)
	DEF_ASM_OP0(fldl2t, 0xd9e9)
	DEF_ASM_OP0(fldl2e, 0xd9ea)
	DEF_ASM_OP0(fldpi, 0xd9eb)
	DEF_ASM_OP0(fldlg2, 0xd9ec)
	DEF_ASM_OP0(fldln2, 0xd9ed)
	DEF_ASM_OP0(fldz, 0xd9ee)

	DEF_ASM_OP0(f2xm1, 0xd9f0)
	DEF_ASM_OP0(fyl2x, 0xd9f1)
	DEF_ASM_OP0(fptan, 0xd9f2)
	DEF_ASM_OP0(fpatan, 0xd9f3)
	DEF_ASM_OP0(fxtract, 0xd9f4)
	DEF_ASM_OP0(fprem1, 0xd9f5)
	DEF_ASM_OP0(fdecstp, 0xd9f6)
	DEF_ASM_OP0(fincstp, 0xd9f7)
	DEF_ASM_OP0(fprem, 0xd9f8)
	DEF_ASM_OP0(fyl2xp1, 0xd9f9)
	DEF_ASM_OP0(fsqrt, 0xd9fa)
	DEF_ASM_OP0(fsincos, 0xd9fb)
	DEF_ASM_OP0(frndint, 0xd9fc)
	DEF_ASM_OP0(fscale, 0xd9fd)
	DEF_ASM_OP0(fsin, 0xd9fe)
	DEF_ASM_OP0(fcos, 0xd9ff)
	DEF_ASM_OP0(fchs, 0xd9e0)
	DEF_ASM_OP0(fabs, 0xd9e1)
	DEF_ASM_OP0(fninit, 0xdbe3)
	DEF_ASM_OP0(fnclex, 0xdbe2)
	DEF_ASM_OP0(fnop, 0xd9d0)
	DEF_ASM_OP0(fwait, 0x9b)
	/* fp load */

	DEF_ASM_OP1(fld, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fldl, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(flds, 0xd9, 0, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fldl, 0xdd, 0, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fildl, 0xdb, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildq, 0xdf, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildll, 0xdf, 5, OPC_MODRM,OPT_EA)
	DEF_ASM_OP1(fldt, 0xdb, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbld, 0xdf, 4, OPC_MODRM, OPT_EA)
	/* fp store */

	DEF_ASM_OP1(fst, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fstl, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fsts, 0xd9, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstps, 0xd9, 3, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fstl, 0xdd, 2, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fstpl, 0xdd, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fist, 0xdf, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistp, 0xdf, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistl, 0xdb, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpl, 0xdb, 3, OPC_MODRM, OPT_EA)

	DEF_ASM_OP1(fstp, 0xddd8, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fistpq, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpll, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstpt, 0xdb, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbstp, 0xdf, 6, OPC_MODRM, OPT_EA)
	/* exchange */

	DEF_ASM_OP0(fxch, 0xd9c9)
	ALT(DEF_ASM_OP1(fxch, 0xd9c8, 0, OPC_REG, OPT_ST))
	/* misc FPU */

	DEF_ASM_OP1(fucom, 0xdde0, 0, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fucomp, 0xdde8, 0, OPC_REG, OPT_ST )

	DEF_ASM_OP0L(finit, 0xdbe3, 0, OPC_FWAIT)
	DEF_ASM_OP1(fldcw, 0xd9, 5, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnstcw, 0xd9, 7, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstcw, 0xd9, 7, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP0(fnstsw, 0xdfe0)
	ALT(DEF_ASM_OP1(fnstsw, 0xdfe0, 0, 0, OPT_EAX ))
	ALT(DEF_ASM_OP1(fnstsw, 0xdd, 7, OPC_MODRM, OPT_EA ))
	DEF_ASM_OP1(fstsw, 0xdfe0, 0, OPC_FWAIT, OPT_EAX )
	ALT(DEF_ASM_OP0L(fstsw, 0xdfe0, 0, OPC_FWAIT))
	ALT(DEF_ASM_OP1(fstsw, 0xdd, 7, OPC_MODRM | OPC_FWAIT, OPT_EA ))
	DEF_ASM_OP0L(fclex, 0xdbe2, 0, OPC_FWAIT)
	DEF_ASM_OP1(fnstenv, 0xd9, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstenv, 0xd9, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(fldenv, 0xd9, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnsave, 0xdd, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fsave, 0xdd, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(frstor, 0xdd, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(ffree, 0xddc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(ffreep, 0xdfc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fxsave, 0x0fae, 0, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fxrstor, 0x0fae, 1, OPC_MODRM, OPT_EA )
	/* The *q forms of fxrstor/fxsave use a REX prefix.
	       If the operand would use extended registers we would have to modify
	       it instead of generating a second one.  Currently that's no
	       problem with TCC, we don't use extended registers.  */

	DEF_ASM_OP1(fxsaveq, 0x0fae, 0, OPC_MODRM | OPC_48, OPT_EA )
	DEF_ASM_OP1(fxrstorq, 0x0fae, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* segments */

	DEF_ASM_OP2(arpl, 0x63, 0, OPC_MODRM, OPT_REG16, OPT_REG16 | OPT_EA)
	ALT(DEF_ASM_OP2(larw, 0x0f02, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
	DEF_ASM_OP1(lgdt, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lgdtq, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidt, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidtq, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lldt, 0x0f00, 2, OPC_MODRM, OPT_EA | OPT_REG)
	DEF_ASM_OP1(lmsw, 0x0f01, 6, OPC_MODRM, OPT_EA | OPT_REG)
	ALT(DEF_ASM_OP2(lslw, 0x0f03, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_REG))
	DEF_ASM_OP1(ltr, 0x0f00, 3, OPC_MODRM, OPT_EA | OPT_REG16)
	DEF_ASM_OP1(sgdt, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sgdtq, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidt, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidtq, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sldt, 0x0f00, 0, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(smsw, 0x0f01, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM, OPT_REG32 | OPT_EA)
	ALT(DEF_ASM_OP1(str, 0x660f00, 1, OPC_MODRM, OPT_REG16))
	ALT(DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM | OPC_48, OPT_REG64))
	DEF_ASM_OP1(verr, 0x0f00, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(verw, 0x0f00, 5, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP0L(swapgs, 0x0f01, 7, OPC_MODRM)
	/* 486 */
	/* bswap can't be applied to 16bit regs */

	DEF_ASM_OP1(bswap, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapl, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapq, 0x0fc8, 0, OPC_REG | OPC_48, OPT_REG64 )

	ALT(DEF_ASM_OP2(xaddb, 0x0fc0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
	ALT(DEF_ASM_OP2(cmpxchgb, 0x0fb0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
	DEF_ASM_OP1(invlpg, 0x0f01, 7, OPC_MODRM, OPT_EA )
	/* pentium */

	DEF_ASM_OP1(cmpxchg8b, 0x0fc7, 1, OPC_MODRM, OPT_EA )
	/* AMD 64 */

	DEF_ASM_OP1(cmpxchg16b, 0x0fc7, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* pentium pro */

	ALT(DEF_ASM_OP2(cmovo, 0x0f40, 0, OPC_MODRM | OPC_TEST | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

	DEF_ASM_OP2(fcmovb, 0xdac0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmove, 0xdac8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovbe, 0xdad0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovu, 0xdad8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnb, 0xdbc0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovne, 0xdbc8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnbe, 0xdbd0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnu, 0xdbd8, 0, OPC_REG, OPT_ST, OPT_ST0 )

	DEF_ASM_OP2(fucomi, 0xdbe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomi, 0xdbf0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fucomip, 0xdfe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomip, 0xdff0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	/* mmx */

	DEF_ASM_OP0(emms, 0x0f77)/* must be last OP0 */

	DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_MMXSSE )
	/* movd shouldn't accept REG64, but AMD64 spec uses it for 32 and 64 bit
	       moves, so let's be compatible. */

	ALT(DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6e, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6f, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_MMX ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG32 ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))
	ALT(DEF_ASM_OP2(movq, 0x0f7f, 0, OPC_MODRM, OPT_MMX, OPT_EA | OPT_MMX ))
	ALT(DEF_ASM_OP2(movq, 0x660fd6, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0xf30f7e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))

	DEF_ASM_OP2(packssdw, 0x0f6b, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packsswb, 0x0f63, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packuswb, 0x0f67, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddb, 0x0ffc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddw, 0x0ffd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddd, 0x0ffe, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsb, 0x0fec, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsw, 0x0fed, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusb, 0x0fdc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusw, 0x0fdd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pand, 0x0fdb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pandn, 0x0fdf, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqb, 0x0f74, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqw, 0x0f75, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqd, 0x0f76, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtb, 0x0f64, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtw, 0x0f65, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtd, 0x0f66, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaddwd, 0x0ff5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmulhw, 0x0fe5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmullw, 0x0fd5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(por, 0x0feb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psllw, 0x0ff1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllw, 0x0f71, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(pslld, 0x0ff2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(pslld, 0x0f72, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psllq, 0x0ff3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllq, 0x0f73, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psraw, 0x0fe1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psraw, 0x0f71, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrad, 0x0fe2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrad, 0x0f72, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlw, 0x0fd1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlw, 0x0f71, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrld, 0x0fd2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrld, 0x0f72, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlq, 0x0fd3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlq, 0x0f73, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psubb, 0x0ff8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubw, 0x0ff9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubd, 0x0ffa, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsb, 0x0fe8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsw, 0x0fe9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusb, 0x0fd8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusw, 0x0fd9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhbw, 0x0f68, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhwd, 0x0f69, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhdq, 0x0f6a, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklbw, 0x0f60, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklwd, 0x0f61, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckldq, 0x0f62, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pxor, 0x0fef, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	/* sse */

	DEF_ASM_OP1(ldmxcsr, 0x0fae, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(stmxcsr, 0x0fae, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP2(movups, 0x0f10, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movups, 0x0f11, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movaps, 0x0f28, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movaps, 0x0f29, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movhps, 0x0f16, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movhps, 0x0f17, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(addps, 0x0f58, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(cvtpi2ps, 0x0f2a, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_SSE )
	DEF_ASM_OP2(cvtps2pi, 0x0f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
	DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
	DEF_ASM_OP2(cvttps2pi, 0x0f2c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(andps, 0x0f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(divps, 0x0f5e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(maxps, 0x0f5f, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(minps, 0x0f5d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(mulps, 0x0f59, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgb, 0x0fe0, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgw, 0x0fe3, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pmaxsw, 0x0fee, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaxub, 0x0fde, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminsw, 0x0fea, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminub, 0x0fda, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(rcpss, 0x0f53, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(rsqrtps, 0x0f52, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtps, 0x0f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtss, 0xf30f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(subps, 0x0f5c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	/* sse2 */

	DEF_ASM_OP2(andpd, 0x660f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	DEF_ASM_OP2(sqrtsd, 0xf20f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	/* movnti should only accept REG32 and REG64, we accept more */

	DEF_ASM_OP2(movnti, 0x0fc3, 0, OPC_MODRM, OPT_REG, OPT_EA)
	DEF_ASM_OP2(movntil, 0x0fc3, 0, OPC_MODRM, OPT_REG32, OPT_EA)
	DEF_ASM_OP2(movntiq, 0x0fc3, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_EA)
	DEF_ASM_OP1(prefetchnta, 0x0f18, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht0, 0x0f18, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht1, 0x0f18, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht2, 0x0f18, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetchw, 0x0f0d, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP0L(lfence, 0x0fae, 5, OPC_MODRM)
	DEF_ASM_OP0L(mfence, 0x0fae, 6, OPC_MODRM)
	DEF_ASM_OP0L(sfence, 0x0fae, 7, OPC_MODRM)
	DEF_ASM_OP1(clflush, 0x0fae, 7, OPC_MODRM, OPT_EA)
	/* Control-Flow Enforcement */

	DEF_ASM_OP0L(endbr64, 0xf30f1e, 7, OPC_MODRM)
#undef ALT
#undef DEF_ASM_OP0
#undef DEF_ASM_OP0L
#undef DEF_ASM_OP1
#undef DEF_ASM_OP2
#undef DEF_ASM_OP3
	/* last operation */

	{
		0,
	}
};

static const uint16_t op0_codes[] = {
#define ALT(x)
#define DEF_ASM_OP0(x,opcode) opcode,
#define DEF_ASM_OP0L(name,opcode,group,instr_type)
#define DEF_ASM_OP1(name,opcode,group,instr_type,op0)
#define DEF_ASM_OP2(name,opcode,group,instr_type,op0,op1)

	DEF_ASM_OP0(clc, 0xf8)/* must be first OP0 */

	DEF_ASM_OP0(cld, 0xfc)
	DEF_ASM_OP0(cli, 0xfa)
	DEF_ASM_OP0(clts, 0x0f06)
	DEF_ASM_OP0(cmc, 0xf5)
	DEF_ASM_OP0(lahf, 0x9f)
	DEF_ASM_OP0(sahf, 0x9e)
	DEF_ASM_OP0(pushfq, 0x9c)
	DEF_ASM_OP0(popfq, 0x9d)
	DEF_ASM_OP0(pushf, 0x9c)
	DEF_ASM_OP0(popf, 0x9d)
	DEF_ASM_OP0(stc, 0xf9)
	DEF_ASM_OP0(std, 0xfd)
	DEF_ASM_OP0(sti, 0xfb)
	DEF_ASM_OP0(aaa, 0x37)
	DEF_ASM_OP0(aas, 0x3f)
	DEF_ASM_OP0(daa, 0x27)
	DEF_ASM_OP0(das, 0x2f)
	DEF_ASM_OP0(aad, 0xd50a)
	DEF_ASM_OP0(aam, 0xd40a)
	DEF_ASM_OP0(cbw, 0x6698)
	DEF_ASM_OP0(cwd, 0x6699)
	DEF_ASM_OP0(cwde, 0x98)
	DEF_ASM_OP0(cdq, 0x99)
	DEF_ASM_OP0(cbtw, 0x6698)
	DEF_ASM_OP0(cwtl, 0x98)
	DEF_ASM_OP0(cwtd, 0x6699)
	DEF_ASM_OP0(cltd, 0x99)
	DEF_ASM_OP0(cqto, 0x4899)
	DEF_ASM_OP0(int3, 0xcc)
	DEF_ASM_OP0(into, 0xce)
	DEF_ASM_OP0(iret, 0xcf)
	DEF_ASM_OP0(iretw, 0x66cf)
	DEF_ASM_OP0(iretl, 0xcf)
	DEF_ASM_OP0(iretq, 0x48cf)
	DEF_ASM_OP0(rsm, 0x0faa)
	DEF_ASM_OP0(hlt, 0xf4)
	DEF_ASM_OP0(wait, 0x9b)
	DEF_ASM_OP0(nop, 0x90)
	DEF_ASM_OP0(pause, 0xf390)
	DEF_ASM_OP0(xlat, 0xd7)

	DEF_ASM_OP0L(vmcall, 0xc1, 0, OPC_0F01)
	DEF_ASM_OP0L(vmlaunch, 0xc2, 0, OPC_0F01)
	DEF_ASM_OP0L(vmresume, 0xc3, 0, OPC_0F01)
	DEF_ASM_OP0L(vmxoff, 0xc4, 0, OPC_0F01)
	/* strings */

	ALT(DEF_ASM_OP0L(cmpsb, 0xa6, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(scmpb, 0xa6, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(insb, 0x6c, 0, OPC_BWL))
	ALT(DEF_ASM_OP0L(outsb, 0x6e, 0, OPC_BWL))

	ALT(DEF_ASM_OP0L(lodsb, 0xac, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(slodb, 0xac, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(movsb, 0xa4, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(smovb, 0xa4, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(scasb, 0xae, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sscab, 0xae, 0, OPC_BWLX))

	ALT(DEF_ASM_OP0L(stosb, 0xaa, 0, OPC_BWLX))
	ALT(DEF_ASM_OP0L(sstob, 0xaa, 0, OPC_BWLX))
	/* bits */

	ALT(DEF_ASM_OP2(bsfw, 0x0fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(bsrw, 0x0fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

	ALT(DEF_ASM_OP2(btw, 0x0fa3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btw, 0x0fba, 4, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btsw, 0x0fab, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btsw, 0x0fba, 5, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btrw, 0x0fb3, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btrw, 0x0fba, 6, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(btcw, 0x0fbb, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP2(btcw, 0x0fba, 7, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW | OPT_EA))

	ALT(DEF_ASM_OP2(popcntw, 0xf30fb8, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

	ALT(DEF_ASM_OP2(tzcntw, 0xf30fbc, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(lzcntw, 0xf30fbd, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))
	/* prefixes */

	DEF_ASM_OP0(lock, 0xf0)
	DEF_ASM_OP0(rep, 0xf3)
	DEF_ASM_OP0(repe, 0xf3)
	DEF_ASM_OP0(repz, 0xf3)
	DEF_ASM_OP0(repne, 0xf2)
	DEF_ASM_OP0(repnz, 0xf2)

	DEF_ASM_OP0(invd, 0x0f08)
	DEF_ASM_OP0(wbinvd, 0x0f09)
	DEF_ASM_OP0(cpuid, 0x0fa2)
	DEF_ASM_OP0(wrmsr, 0x0f30)
	DEF_ASM_OP0(rdtsc, 0x0f31)
	DEF_ASM_OP0(rdmsr, 0x0f32)
	DEF_ASM_OP0(rdpmc, 0x0f33)

	DEF_ASM_OP0(syscall, 0x0f05)
	DEF_ASM_OP0(sysret, 0x0f07)
	DEF_ASM_OP0L(sysretq, 0x480f07, 0, 0)
	DEF_ASM_OP0(ud2, 0x0f0b)
	/* NOTE: we took the same order as gas opcode definition order */
	/* Right now we can't express the fact that 0xa1/0xa3 can't use $eax and a
	   32 bit moffset as operands.
	ALT(DEF_ASM_OP2(movb, 0xa0, 0, OPC_BWLX, OPT_ADDR, OPT_EAX))
	ALT(DEF_ASM_OP2(movb, 0xa2, 0, OPC_BWLX, OPT_EAX, OPT_ADDR)) */

	ALT(DEF_ASM_OP2(movb, 0x88, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movb, 0x8a, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	/* The moves are special: the 0xb8 form supports IM64 (the only insn that
	   does) with REG64.  It doesn't support IM32 with REG64, it would use
	   the full movabs form (64bit immediate).  For IM32->REG64 we prefer
	   the 0xc7 opcode.  So disallow all 64bit forms and code the rest by hand. */

	ALT(DEF_ASM_OP2(movb, 0xb0, 0, OPC_REG | OPC_BWLX, OPT_IM, OPT_REG))
	ALT(DEF_ASM_OP2(mov, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movq, 0xb8, 0, OPC_REG, OPT_IM64, OPT_REG64))
	ALT(DEF_ASM_OP2(movb, 0xc6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(movw, 0x8c, 0, OPC_MODRM | OPC_WLX, OPT_SEG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(movw, 0x8e, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_SEG))

	ALT(DEF_ASM_OP2(movw, 0x0f20, 0, OPC_MODRM | OPC_WLX, OPT_CR, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f21, 0, OPC_MODRM | OPC_WLX, OPT_DB, OPT_REG64))
	ALT(DEF_ASM_OP2(movw, 0x0f22, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_CR))
	ALT(DEF_ASM_OP2(movw, 0x0f23, 0, OPC_MODRM | OPC_WLX, OPT_REG64, OPT_DB))

	ALT(DEF_ASM_OP2(movsbw, 0x660fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG16))
	ALT(DEF_ASM_OP2(movsbl, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movsbq, 0x0fbe, 0, OPC_MODRM, OPT_REG8 | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(movswl, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movswq, 0x0fbf, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movslq, 0x63, 0, OPC_MODRM, OPT_REG32 | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP2(movzbw, 0x0fb6, 0, OPC_MODRM | OPC_WLX, OPT_REG8 | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(movzwl, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(movzwq, 0x0fb7, 0, OPC_MODRM, OPT_REG16 | OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP1(pushq, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(push, 0x6a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x666a, 0, 0, OPT_IM8S))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(pushw, 0x50, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(pushw, 0xff, 6, OPC_MODRM | OPC_WLX, OPT_REG64 | OPT_EA))
	ALT(DEF_ASM_OP1(pushw, 0x6668, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(pushw, 0x68, 0, OPC_WLX, OPT_IM32))
	ALT(DEF_ASM_OP1(pushw, 0x06, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG64))
	ALT(DEF_ASM_OP1(popw, 0x58, 0, OPC_REG | OPC_WLX, OPT_REG16))
	ALT(DEF_ASM_OP1(popw, 0x8f, 0, OPC_MODRM | OPC_WLX, OPT_REGW | OPT_EA))
	ALT(DEF_ASM_OP1(popw, 0x07, 0, OPC_WLX, OPT_SEG))

	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_REGW, OPT_EAX))
	ALT(DEF_ASM_OP2(xchgw, 0x90, 0, OPC_REG | OPC_WLX, OPT_EAX, OPT_REGW))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(xchgb, 0x86, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))

	ALT(DEF_ASM_OP2(inb, 0xe4, 0, OPC_BWL, OPT_IM8, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xe4, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(inb, 0xec, 0, OPC_BWL, OPT_DX, OPT_EAX))
	ALT(DEF_ASM_OP1(inb, 0xec, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(outb, 0xe6, 0, OPC_BWL, OPT_EAX, OPT_IM8))
	ALT(DEF_ASM_OP1(outb, 0xe6, 0, OPC_BWL, OPT_IM8))
	ALT(DEF_ASM_OP2(outb, 0xee, 0, OPC_BWL, OPT_EAX, OPT_DX))
	ALT(DEF_ASM_OP1(outb, 0xee, 0, OPC_BWL, OPT_DX))

	ALT(DEF_ASM_OP2(leaw, 0x8d, 0, OPC_MODRM | OPC_WLX, OPT_EA, OPT_REG))

	ALT(DEF_ASM_OP2(les, 0xc4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lds, 0xc5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lss, 0x0fb2, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lfs, 0x0fb4, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	ALT(DEF_ASM_OP2(lgs, 0x0fb5, 0, OPC_MODRM, OPT_EA, OPT_REG32))
	/* arith */

	ALT(DEF_ASM_OP2(addb, 0x00, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))/* XXX: use D bit ? */

	ALT(DEF_ASM_OP2(addb, 0x02, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	ALT(DEF_ASM_OP2(addb, 0x04, 0, OPC_ARITH | OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(addw, 0x83, 0, OPC_ARITH | OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(addb, 0x80, 0, OPC_ARITH | OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0x84, 0, OPC_MODRM | OPC_BWLX, OPT_EA | OPT_REG, OPT_REG))
	ALT(DEF_ASM_OP2(testb, 0xa8, 0, OPC_BWLX, OPT_IM, OPT_EAX))
	ALT(DEF_ASM_OP2(testb, 0xf6, 0, OPC_MODRM | OPC_BWLX, OPT_IM, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP1(incb, 0xfe, 0, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(decb, 0xfe, 1, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(notb, 0xf6, 2, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(negb, 0xf6, 3, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP1(mulb, 0xf6, 4, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP1(imulb, 0xf6, 5, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))

	ALT(DEF_ASM_OP2(imulw, 0x0faf, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
	ALT(DEF_ASM_OP3(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x6b, 0, OPC_MODRM | OPC_WLX, OPT_IM8S, OPT_REGW))
	ALT(DEF_ASM_OP3(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW | OPT_EA, OPT_REGW))
	ALT(DEF_ASM_OP2(imulw, 0x69, 0, OPC_MODRM | OPC_WLX, OPT_IMW, OPT_REGW))

	ALT(DEF_ASM_OP1(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(divb, 0xf6, 6, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
	ALT(DEF_ASM_OP1(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA))
	ALT(DEF_ASM_OP2(idivb, 0xf6, 7, OPC_MODRM | OPC_BWLX, OPT_REG | OPT_EA, OPT_EAX))
	/* shifts */

	ALT(DEF_ASM_OP2(rolb, 0xc0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_IM8, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP2(rolb, 0xd2, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_CL, OPT_EA | OPT_REG))
	ALT(DEF_ASM_OP1(rolb, 0xd0, 0, OPC_MODRM | OPC_BWLX | OPC_SHIFT, OPT_EA | OPT_REG))

	ALT(DEF_ASM_OP3(shldw, 0x0fa4, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shldw, 0x0fa5, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fac, 0, OPC_MODRM | OPC_WLX, OPT_IM8, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP3(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_CL, OPT_REGW, OPT_EA | OPT_REGW))
	ALT(DEF_ASM_OP2(shrdw, 0x0fad, 0, OPC_MODRM | OPC_WLX, OPT_REGW, OPT_EA | OPT_REGW))

	ALT(DEF_ASM_OP1(call, 0xff, 2, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(call, 0xe8, 0, 0, OPT_DISP))
	DEF_ASM_OP1(callq, 0xff, 2, OPC_MODRM, OPT_INDIR)
	ALT(DEF_ASM_OP1(callq, 0xe8, 0, 0, OPT_DISP))
	ALT(DEF_ASM_OP1(jmp, 0xff, 4, OPC_MODRM, OPT_INDIR))
	ALT(DEF_ASM_OP1(jmp, 0xeb, 0, 0, OPT_DISP8))

	ALT(DEF_ASM_OP1(lcall, 0xff, 3, OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(ljmp, 0xff, 5, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(ljmpw, 0x66ff, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(ljmpl, 0xff, 5, OPC_MODRM, OPT_EA)

	ALT(DEF_ASM_OP1(int, 0xcd, 0, 0, OPT_IM8))
	ALT(DEF_ASM_OP1(seto, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	ALT(DEF_ASM_OP1(setob, 0x0f90, 0, OPC_MODRM | OPC_TEST, OPT_REG8 | OPT_EA))
	DEF_ASM_OP2(enter, 0xc8, 0, 0, OPT_IM16, OPT_IM8)
	DEF_ASM_OP0(leave, 0xc9)
	DEF_ASM_OP0(ret, 0xc3)
	DEF_ASM_OP0(retq, 0xc3)
	ALT(DEF_ASM_OP1(retq, 0xc2, 0, 0, OPT_IM16))
	ALT(DEF_ASM_OP1(ret, 0xc2, 0, 0, OPT_IM16))
	DEF_ASM_OP0(lret, 0xcb)
	ALT(DEF_ASM_OP1(lret, 0xca, 0, 0, OPT_IM16))

	ALT(DEF_ASM_OP1(jo, 0x70, 0, OPC_TEST, OPT_DISP8))
	DEF_ASM_OP1(loopne, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopnz, 0xe0, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loope, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loopz, 0xe1, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(loop, 0xe2, 0, 0, OPT_DISP8)
	DEF_ASM_OP1(jecxz, 0x67e3, 0, 0, OPT_DISP8)
	/* float */
	/* specific fcomp handling */

	ALT(DEF_ASM_OP0L(fcomp, 0xd8d9, 0, 0))

	ALT(DEF_ASM_OP1(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(fadd, 0xd8c0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(fadd, 0xdcc0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP2(fmul, 0xdcc8, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(fadd, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST, OPT_ST0))
	ALT(DEF_ASM_OP2(faddp, 0xdec0, 0, OPC_FARITH | OPC_REG, OPT_ST0, OPT_ST))
	ALT(DEF_ASM_OP0L(faddp, 0xdec1, 0, OPC_FARITH))
	ALT(DEF_ASM_OP1(fadds, 0xd8, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiaddl, 0xda, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(faddl, 0xdc, 0, OPC_FARITH | OPC_MODRM, OPT_EA))
	ALT(DEF_ASM_OP1(fiadds, 0xde, 0, OPC_FARITH | OPC_MODRM, OPT_EA))

	DEF_ASM_OP0(fucompp, 0xdae9)
	DEF_ASM_OP0(ftst, 0xd9e4)
	DEF_ASM_OP0(fxam, 0xd9e5)
	DEF_ASM_OP0(fld1, 0xd9e8)
	DEF_ASM_OP0(fldl2t, 0xd9e9)
	DEF_ASM_OP0(fldl2e, 0xd9ea)
	DEF_ASM_OP0(fldpi, 0xd9eb)
	DEF_ASM_OP0(fldlg2, 0xd9ec)
	DEF_ASM_OP0(fldln2, 0xd9ed)
	DEF_ASM_OP0(fldz, 0xd9ee)

	DEF_ASM_OP0(f2xm1, 0xd9f0)
	DEF_ASM_OP0(fyl2x, 0xd9f1)
	DEF_ASM_OP0(fptan, 0xd9f2)
	DEF_ASM_OP0(fpatan, 0xd9f3)
	DEF_ASM_OP0(fxtract, 0xd9f4)
	DEF_ASM_OP0(fprem1, 0xd9f5)
	DEF_ASM_OP0(fdecstp, 0xd9f6)
	DEF_ASM_OP0(fincstp, 0xd9f7)
	DEF_ASM_OP0(fprem, 0xd9f8)
	DEF_ASM_OP0(fyl2xp1, 0xd9f9)
	DEF_ASM_OP0(fsqrt, 0xd9fa)
	DEF_ASM_OP0(fsincos, 0xd9fb)
	DEF_ASM_OP0(frndint, 0xd9fc)
	DEF_ASM_OP0(fscale, 0xd9fd)
	DEF_ASM_OP0(fsin, 0xd9fe)
	DEF_ASM_OP0(fcos, 0xd9ff)
	DEF_ASM_OP0(fchs, 0xd9e0)
	DEF_ASM_OP0(fabs, 0xd9e1)
	DEF_ASM_OP0(fninit, 0xdbe3)
	DEF_ASM_OP0(fnclex, 0xdbe2)
	DEF_ASM_OP0(fnop, 0xd9d0)
	DEF_ASM_OP0(fwait, 0x9b)
	/* fp load */

	DEF_ASM_OP1(fld, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fldl, 0xd9c0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(flds, 0xd9, 0, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fldl, 0xdd, 0, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fildl, 0xdb, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildq, 0xdf, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fildll, 0xdf, 5, OPC_MODRM,OPT_EA)
	DEF_ASM_OP1(fldt, 0xdb, 5, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbld, 0xdf, 4, OPC_MODRM, OPT_EA)
	/* fp store */

	DEF_ASM_OP1(fst, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fstl, 0xddd0, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fsts, 0xd9, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstps, 0xd9, 3, OPC_MODRM, OPT_EA)
	ALT(DEF_ASM_OP1(fstl, 0xdd, 2, OPC_MODRM, OPT_EA))
	DEF_ASM_OP1(fstpl, 0xdd, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fist, 0xdf, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistp, 0xdf, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistl, 0xdb, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpl, 0xdb, 3, OPC_MODRM, OPT_EA)

	DEF_ASM_OP1(fstp, 0xddd8, 0, OPC_REG, OPT_ST)
	DEF_ASM_OP1(fistpq, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fistpll, 0xdf, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fstpt, 0xdb, 7, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(fbstp, 0xdf, 6, OPC_MODRM, OPT_EA)
	/* exchange */

	DEF_ASM_OP0(fxch, 0xd9c9)
	ALT(DEF_ASM_OP1(fxch, 0xd9c8, 0, OPC_REG, OPT_ST))
	/* misc FPU */

	DEF_ASM_OP1(fucom, 0xdde0, 0, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fucomp, 0xdde8, 0, OPC_REG, OPT_ST )

	DEF_ASM_OP0L(finit, 0xdbe3, 0, OPC_FWAIT)
	DEF_ASM_OP1(fldcw, 0xd9, 5, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnstcw, 0xd9, 7, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstcw, 0xd9, 7, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP0(fnstsw, 0xdfe0)
	ALT(DEF_ASM_OP1(fnstsw, 0xdfe0, 0, 0, OPT_EAX ))
	ALT(DEF_ASM_OP1(fnstsw, 0xdd, 7, OPC_MODRM, OPT_EA ))
	DEF_ASM_OP1(fstsw, 0xdfe0, 0, OPC_FWAIT, OPT_EAX )
	ALT(DEF_ASM_OP0L(fstsw, 0xdfe0, 0, OPC_FWAIT))
	ALT(DEF_ASM_OP1(fstsw, 0xdd, 7, OPC_MODRM | OPC_FWAIT, OPT_EA ))
	DEF_ASM_OP0L(fclex, 0xdbe2, 0, OPC_FWAIT)
	DEF_ASM_OP1(fnstenv, 0xd9, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fstenv, 0xd9, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(fldenv, 0xd9, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fnsave, 0xdd, 6, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fsave, 0xdd, 6, OPC_MODRM | OPC_FWAIT, OPT_EA )
	DEF_ASM_OP1(frstor, 0xdd, 4, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(ffree, 0xddc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(ffreep, 0xdfc0, 4, OPC_REG, OPT_ST )
	DEF_ASM_OP1(fxsave, 0x0fae, 0, OPC_MODRM, OPT_EA )
	DEF_ASM_OP1(fxrstor, 0x0fae, 1, OPC_MODRM, OPT_EA )
	/* The *q forms of fxrstor/fxsave use a REX prefix.
	       If the operand would use extended registers we would have to modify
	       it instead of generating a second one.  Currently that's no
	       problem with TCC, we don't use extended registers.  */

	DEF_ASM_OP1(fxsaveq, 0x0fae, 0, OPC_MODRM | OPC_48, OPT_EA )
	DEF_ASM_OP1(fxrstorq, 0x0fae, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* segments */

	DEF_ASM_OP2(arpl, 0x63, 0, OPC_MODRM, OPT_REG16, OPT_REG16 | OPT_EA)
	ALT(DEF_ASM_OP2(larw, 0x0f02, 0, OPC_MODRM | OPC_WLX, OPT_REG | OPT_EA, OPT_REG))
	DEF_ASM_OP1(lgdt, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lgdtq, 0x0f01, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidt, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lidtq, 0x0f01, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(lldt, 0x0f00, 2, OPC_MODRM, OPT_EA | OPT_REG)
	DEF_ASM_OP1(lmsw, 0x0f01, 6, OPC_MODRM, OPT_EA | OPT_REG)
	ALT(DEF_ASM_OP2(lslw, 0x0f03, 0, OPC_MODRM | OPC_WLX, OPT_EA | OPT_REG, OPT_REG))
	DEF_ASM_OP1(ltr, 0x0f00, 3, OPC_MODRM, OPT_EA | OPT_REG16)
	DEF_ASM_OP1(sgdt, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sgdtq, 0x0f01, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidt, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sidtq, 0x0f01, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(sldt, 0x0f00, 0, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(smsw, 0x0f01, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM, OPT_REG32 | OPT_EA)
	ALT(DEF_ASM_OP1(str, 0x660f00, 1, OPC_MODRM, OPT_REG16))
	ALT(DEF_ASM_OP1(str, 0x0f00, 1, OPC_MODRM | OPC_48, OPT_REG64))
	DEF_ASM_OP1(verr, 0x0f00, 4, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP1(verw, 0x0f00, 5, OPC_MODRM, OPT_REG | OPT_EA)
	DEF_ASM_OP0L(swapgs, 0x0f01, 7, OPC_MODRM)
	/* 486 */
	/* bswap can't be applied to 16bit regs */

	DEF_ASM_OP1(bswap, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapl, 0x0fc8, 0, OPC_REG, OPT_REG32 )
	DEF_ASM_OP1(bswapq, 0x0fc8, 0, OPC_REG | OPC_48, OPT_REG64 )

	ALT(DEF_ASM_OP2(xaddb, 0x0fc0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
	ALT(DEF_ASM_OP2(cmpxchgb, 0x0fb0, 0, OPC_MODRM | OPC_BWLX, OPT_REG, OPT_REG | OPT_EA ))
	DEF_ASM_OP1(invlpg, 0x0f01, 7, OPC_MODRM, OPT_EA )
	/* pentium */

	DEF_ASM_OP1(cmpxchg8b, 0x0fc7, 1, OPC_MODRM, OPT_EA )
	/* AMD 64 */

	DEF_ASM_OP1(cmpxchg16b, 0x0fc7, 1, OPC_MODRM | OPC_48, OPT_EA )
	/* pentium pro */

	ALT(DEF_ASM_OP2(cmovo, 0x0f40, 0, OPC_MODRM | OPC_TEST | OPC_WLX, OPT_REGW | OPT_EA, OPT_REGW))

	DEF_ASM_OP2(fcmovb, 0xdac0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmove, 0xdac8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovbe, 0xdad0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovu, 0xdad8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnb, 0xdbc0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovne, 0xdbc8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnbe, 0xdbd0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcmovnu, 0xdbd8, 0, OPC_REG, OPT_ST, OPT_ST0 )

	DEF_ASM_OP2(fucomi, 0xdbe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomi, 0xdbf0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fucomip, 0xdfe8, 0, OPC_REG, OPT_ST, OPT_ST0 )
	DEF_ASM_OP2(fcomip, 0xdff0, 0, OPC_REG, OPT_ST, OPT_ST0 )
	/* mmx */

	DEF_ASM_OP0(emms, 0x0f77)/* must be last OP0 */

	DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_MMXSSE )
	/* movd shouldn't accept REG64, but AMD64 spec uses it for 32 and 64 bit
	       moves, so let's be compatible. */

	ALT(DEF_ASM_OP2(movd, 0x0f6e, 0, OPC_MODRM, OPT_EA | OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6e, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_MMXSSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f6f, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_MMX ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG32 ))
	ALT(DEF_ASM_OP2(movd, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))
	ALT(DEF_ASM_OP2(movq, 0x0f7f, 0, OPC_MODRM, OPT_MMX, OPT_EA | OPT_MMX ))
	ALT(DEF_ASM_OP2(movq, 0x660fd6, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0xf30f7e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE ))
	ALT(DEF_ASM_OP2(movq, 0x0f7e, 0, OPC_MODRM, OPT_MMXSSE, OPT_EA | OPT_REG64 ))

	DEF_ASM_OP2(packssdw, 0x0f6b, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packsswb, 0x0f63, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(packuswb, 0x0f67, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddb, 0x0ffc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddw, 0x0ffd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddd, 0x0ffe, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsb, 0x0fec, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddsw, 0x0fed, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusb, 0x0fdc, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(paddusw, 0x0fdd, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pand, 0x0fdb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pandn, 0x0fdf, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqb, 0x0f74, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqw, 0x0f75, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpeqd, 0x0f76, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtb, 0x0f64, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtw, 0x0f65, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pcmpgtd, 0x0f66, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaddwd, 0x0ff5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmulhw, 0x0fe5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmullw, 0x0fd5, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(por, 0x0feb, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psllw, 0x0ff1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllw, 0x0f71, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(pslld, 0x0ff2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(pslld, 0x0f72, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psllq, 0x0ff3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psllq, 0x0f73, 6, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psraw, 0x0fe1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psraw, 0x0f71, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrad, 0x0fe2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrad, 0x0f72, 4, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlw, 0x0fd1, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlw, 0x0f71, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrld, 0x0fd2, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrld, 0x0f72, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psrlq, 0x0fd3, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	ALT(DEF_ASM_OP2(psrlq, 0x0f73, 2, OPC_MODRM, OPT_IM8, OPT_MMXSSE ))
	DEF_ASM_OP2(psubb, 0x0ff8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubw, 0x0ff9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubd, 0x0ffa, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsb, 0x0fe8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubsw, 0x0fe9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusb, 0x0fd8, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(psubusw, 0x0fd9, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhbw, 0x0f68, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhwd, 0x0f69, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckhdq, 0x0f6a, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklbw, 0x0f60, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpcklwd, 0x0f61, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(punpckldq, 0x0f62, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pxor, 0x0fef, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	/* sse */

	DEF_ASM_OP1(ldmxcsr, 0x0fae, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(stmxcsr, 0x0fae, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP2(movups, 0x0f10, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movups, 0x0f11, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movaps, 0x0f28, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movaps, 0x0f29, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(movhps, 0x0f16, 0, OPC_MODRM, OPT_EA | OPT_REG32, OPT_SSE )
	ALT(DEF_ASM_OP2(movhps, 0x0f17, 0, OPC_MODRM, OPT_SSE, OPT_EA | OPT_REG32 ))
	DEF_ASM_OP2(addps, 0x0f58, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(cvtpi2ps, 0x0f2a, 0, OPC_MODRM, OPT_EA | OPT_MMX, OPT_SSE )
	DEF_ASM_OP2(cvtps2pi, 0x0f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtss2si, 0xf30f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
	DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_REG32 )
	ALT(DEF_ASM_OP2(cvtsd2si, 0xf20f2d, 0, OPC_MODRM | OPC_48, OPT_EA | OPT_SSE, OPT_REG64 ))
	DEF_ASM_OP2(cvttps2pi, 0x0f2c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_MMX )
	DEF_ASM_OP2(andps, 0x0f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(divps, 0x0f5e, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(maxps, 0x0f5f, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(minps, 0x0f5d, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(mulps, 0x0f59, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgb, 0x0fe0, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pavgw, 0x0fe3, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(pmaxsw, 0x0fee, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pmaxub, 0x0fde, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminsw, 0x0fea, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(pminub, 0x0fda, 0, OPC_MODRM, OPT_EA | OPT_MMXSSE, OPT_MMXSSE )
	DEF_ASM_OP2(rcpss, 0x0f53, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(rsqrtps, 0x0f52, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtps, 0x0f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(sqrtss, 0xf30f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	DEF_ASM_OP2(subps, 0x0f5c, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE )
	/* sse2 */

	DEF_ASM_OP2(andpd, 0x660f54, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	DEF_ASM_OP2(sqrtsd, 0xf20f51, 0, OPC_MODRM, OPT_EA | OPT_SSE, OPT_SSE)
	/* movnti should only accept REG32 and REG64, we accept more */

	DEF_ASM_OP2(movnti, 0x0fc3, 0, OPC_MODRM, OPT_REG, OPT_EA)
	DEF_ASM_OP2(movntil, 0x0fc3, 0, OPC_MODRM, OPT_REG32, OPT_EA)
	DEF_ASM_OP2(movntiq, 0x0fc3, 0, OPC_MODRM | OPC_48, OPT_REG64, OPT_EA)
	DEF_ASM_OP1(prefetchnta, 0x0f18, 0, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht0, 0x0f18, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht1, 0x0f18, 2, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetcht2, 0x0f18, 3, OPC_MODRM, OPT_EA)
	DEF_ASM_OP1(prefetchw, 0x0f0d, 1, OPC_MODRM, OPT_EA)
	DEF_ASM_OP0L(lfence, 0x0fae, 5, OPC_MODRM)
	DEF_ASM_OP0L(mfence, 0x0fae, 6, OPC_MODRM)
	DEF_ASM_OP0L(sfence, 0x0fae, 7, OPC_MODRM)
	DEF_ASM_OP1(clflush, 0x0fae, 7, OPC_MODRM, OPT_EA)
	/* Control-Flow Enforcement */

	DEF_ASM_OP0L(endbr64, 0xf30f1e, 7, OPC_MODRM)
#undef ALT
#undef DEF_ASM_OP0
#undef DEF_ASM_OP0L
#undef DEF_ASM_OP1
#undef DEF_ASM_OP2
#undef DEF_ASM_OP3

};

static inline int get_reg_shift(TCCState *s1)
{
	int shift, v;
	v = asm_int_expr(s1);
	switch (v) {
	case 1:
		shift = 0;
		break;
	case 2:
		shift = 1;
		break;
	case 4:
		shift = 2;
		break;
	case 8:
		shift = 3;
		break;
	default:
		expect("1, 2, 4 or 8 constant");
		shift = 0;
		break;
	}
	return shift;
}

static int asm_parse_numeric_reg(int t, unsigned int *type)
{
	int reg = -1;
	if (t >= TOK_IDENT && t < tok_ident) {
		const char *s = table_ident[t - TOK_IDENT]->str;
		char c;
		*type = OP_REG64;
		if (*s == 'c') {
			s++;
			*type = OP_CR;
		}
		if (*s++ != 'r')
			return -1;
		/* Don't allow leading '0'.  */

		if ((c = *s++) >= '1' && c <= '9')
			reg = c - '0';
		else
			return -1;
		if ((c = *s) >= '0' && c <= '5')
			s++, reg = reg * 10 + c - '0';
		if (reg > 15)
			return -1;
		if ((c = *s) == 0)
			;
		else if (*type != OP_REG64)
			return -1;
		else if (c == 'b' && !s[1])
			*type = OP_REG8;
		else if (c == 'w' && !s[1])
			*type = OP_REG16;
		else if (c == 'd' && !s[1])
			*type = OP_REG32;
		else
			return -1;
	}
	return reg;
}

static int asm_parse_reg(unsigned int *type)
{
	int reg = 0;
	*type = 0;
	if (tok != '%')
		goto error_32;
	next();
	if (tok >= TOK_ASM_eax && tok <= TOK_ASM_edi) {
		reg = tok - TOK_ASM_eax;
		*type = OP_REG32;

	} else if (tok >= TOK_ASM_rax && tok <= TOK_ASM_rdi) {
		reg = tok - TOK_ASM_rax;
		*type = OP_REG64;
	} else if (tok == TOK_ASM_rip) {
		reg = -2;/* Probably should use different escape code. */

		*type = OP_REG64;
	} else if ((reg = asm_parse_numeric_reg(tok, type)) >= 0
		   && (*type == OP_REG32 || *type == OP_REG64)) {
		;

	} else {
error_32:
		expect("register");
	}
	next();
	return reg;
}

static void parse_operand(TCCState *s1, Operand *op)
{
	ExprValue e;
	int reg, indir;
	const char *p;

	indir = 0;
	if (tok == '*') {
		next();
		indir = OP_INDIR;
	}

	if (tok == '%') {
		next();
		if (tok >= TOK_ASM_al && tok <= TOK_ASM_db7) {
			reg = tok - TOK_ASM_al;
			op->type = 1 << (reg >> 3);/* WARNING: do not change constant order */

			op->reg = reg & 7;
			if ((op->type & OP_REG) && op->reg == TREG_XAX)
				op->type |= OP_EAX;
			else if (op->type == OP_REG8 && op->reg == TREG_XCX)
				op->type |= OP_CL;
			else if (op->type == OP_REG16 && op->reg == TREG_XDX)
				op->type |= OP_DX;
		} else if (tok >= TOK_ASM_dr0 && tok <= TOK_ASM_dr7) {
			op->type = OP_DB;
			op->reg = tok - TOK_ASM_dr0;
		} else if (tok >= TOK_ASM_es && tok <= TOK_ASM_gs) {
			op->type = OP_SEG;
			op->reg = tok - TOK_ASM_es;
		} else if (tok == TOK_ASM_st) {
			op->type = OP_ST;
			op->reg = 0;
			next();
			if (tok == '(') {
				next();
				if (tok != TOK_PPNUM)
					goto reg_error;
				p = tokc.str.data;
				reg = p[0] - '0';
				if ((unsigned)reg >= 8 || p[1] != '\0')
					goto reg_error;
				op->reg = reg;
				next();
				skip(')');
			}
			if (op->reg == 0)
				op->type |= OP_ST0;
			goto no_skip;

		} else if (tok >= TOK_ASM_spl && tok <= TOK_ASM_dil) {
			op->type = OP_REG8 | OP_REG8_LOW;
			op->reg = 4 + tok - TOK_ASM_spl;
		} else if ((op->reg = asm_parse_numeric_reg(tok, &op->type)) >= 0) {
			;

		} else {
reg_error:
			tcc_error("unknown register %%%s", get_tok_str(tok, &tokc));
		}
		next();
no_skip: ;
	} else if (tok == '$') {
		/* constant value */

		next();
		asm_expr(s1, &e);
		op->type = OP_IM32;
		op->e = e;
		if (!op->e.sym) {
			if (op->e.v == (uint8_t)op->e.v)
				op->type |= OP_IM8;
			if (op->e.v == (int8_t)op->e.v)
				op->type |= OP_IM8S;
			if (op->e.v == (uint16_t)op->e.v)
				op->type |= OP_IM16;

			if (op->e.v != (int32_t)op->e.v && op->e.v != (uint32_t)op->e.v)
				op->type = OP_IM64;

		}
	} else {
		/* address(reg,reg2,shift) with all variants */

		op->type = OP_EA;
		op->reg = -1;
		op->reg2 = -1;
		op->shift = 0;
		if (tok != '(') {
			asm_expr(s1, &e);
			op->e = e;
		} else {
			next();
			if (tok == '%') {
				unget_tok('(');
				op->e.v = 0;
				op->e.sym = NULL;
			} else {
				/* bracketed offset expression */

				asm_expr(s1, &e);
				if (tok != ')')
					expect(")");
				next();
				op->e.v = e.v;
				op->e.sym = e.sym;
			}
			op->e.pcrel = 0;
		}
		if (tok == '(') {
			unsigned int type = 0;
			next();
			if (tok != ',') {
				op->reg = asm_parse_reg(&type);
			}
			if (tok == ',') {
				next();
				if (tok != ',') {
					op->reg2 = asm_parse_reg(&type);
				}
				if (tok == ',') {
					next();
					op->shift = get_reg_shift(s1);
				}
			}
			if (type & OP_REG32)
				op->type |= OP_EA32;
			skip(')');
		}
		if (op->reg == -1 && op->reg2 == -1)
			op->type |= OP_ADDR;
	}
	op->type |= indir;
}
/* XXX: unify with C code output ? */

ST_FUNC void gen_expr32(ExprValue *pe)
{
	if (pe->pcrel)
		/* If PC-relative, always set VT_SYM, even without symbol,
			   so as to force a relocation to be emitted.  */

		gen_addrpc32(VT_SYM, pe->sym, pe->v + (ind + 4));
	else
		gen_addr32(pe->sym ? VT_SYM : 0, pe->sym, pe->v);
}
/* XXX: unify with C code output ? */

static void gen_disp32(ExprValue *pe)
{
	Sym *sym = pe->sym;
	ElfSym *esym = elfsym(sym);
	if (esym && esym->st_shndx == cur_text_section->sh_num) {
		/* same section: we can output an absolute value. Note
		           that the TCC compiler behaves differently here because
		           it always outputs a relocation to ease (future) code
		           elimination in the linker */

		gen_le32(pe->v + esym->st_value - ind - 4);
	} else {
		if (sym && sym->type.t == VT_VOID) {
			sym->type.t = VT_FUNC;
			sym->type.ref = NULL;
		}

		greloca(cur_text_section, sym, ind, R_X86_64_PLT32, pe->v - 4);
		gen_le32(0);

	}
}
/* generate the modrm operand */

static inline int asm_modrm(int reg, Operand *op)
{
	int mod, reg1, reg2, sib_reg1;

	if (op->type & (OP_REG | OP_MMX | OP_SSE)) {
		g(0xc0 + (reg << 3) + op->reg);
	} else if (op->reg == -1 && op->reg2 == -1) {
		/* displacement only */

		g(0x04 + (reg << 3));
		g(0x25);

		gen_expr32(&op->e);

	} else if (op->reg == -2) {
		ExprValue *pe = &op->e;
		g(0x05 + (reg << 3));
		gen_addrpc32(pe->sym ? VT_SYM : 0, pe->sym, pe->v);
		return ind;

	} else {
		sib_reg1 = op->reg;
		/* fist compute displacement encoding */

		if (sib_reg1 == -1) {
			sib_reg1 = 5;
			mod = 0x00;
		} else if (op->e.v == 0 && !op->e.sym && op->reg != 5) {
			mod = 0x00;
		} else if (op->e.v == (int8_t)op->e.v && !op->e.sym) {
			mod = 0x40;
		} else {
			mod = 0x80;
		}
		/* compute if sib byte needed */

		reg1 = op->reg;
		if (op->reg2 != -1)
			reg1 = 4;
		g(mod + (reg << 3) + reg1);
		if (reg1 == 4) {
			/* add sib byte */

			reg2 = op->reg2;
			if (reg2 == -1)
				reg2 = 4;/* indicate no index */

			g((op->shift << 6) + (reg2 << 3) + sib_reg1);
		}
		/* add offset */

		if (mod == 0x40) {
			g(op->e.v);
		} else if (mod == 0x80 || op->reg == -1) {
			gen_expr32(&op->e);
		}
	}
	return 0;
}
#define REX_R 0x44
#define REX_X 0x42
#define REX_B 0x41

static void asm_rex(int width64, Operand *ops, int nb_ops, int *op_type,
		    int regi, int rmi)
{
	unsigned char rex = width64 ? 0x48 : 0;
	int saw_high_8bit = 0;
	int i;
	if (rmi == -1) {
		/* No mod/rm byte, but we might have a register op nevertheless
		         (we will add it to the opcode later).  */

		for (i = 0; i < nb_ops; i++) {
			if (op_type[i] & (OP_REG | OP_ST)) {
				if (ops[i].reg >= 8) {
					rex |= REX_B;
					ops[i].reg -= 8;
				} else if (ops[i].type & OP_REG8_LOW)
					rex |= 0x40;
				else if (ops[i].type & OP_REG8 && ops[i].reg >= 4)
					/* An 8 bit reg >= 4 without REG8 is ah/ch/dh/bh */

					saw_high_8bit = ops[i].reg;
				break;
			}
		}
	} else {
		if (regi != -1) {
			if (ops[regi].reg >= 8) {
				rex |= REX_R;
				ops[regi].reg -= 8;
			} else if (ops[regi].type & OP_REG8_LOW)
				rex |= 0x40;
			else if (ops[regi].type & OP_REG8 && ops[regi].reg >= 4)
				/* An 8 bit reg >= 4 without REG8 is ah/ch/dh/bh */

				saw_high_8bit = ops[regi].reg;
		}
		if (ops[rmi].type & (OP_REG | OP_MMX | OP_SSE | OP_CR | OP_EA)) {
			if (ops[rmi].reg >= 8) {
				rex |= REX_B;
				ops[rmi].reg -= 8;
			} else if (ops[rmi].type & OP_REG8_LOW)
				rex |= 0x40;
			else if (ops[rmi].type & OP_REG8 && ops[rmi].reg >= 4)
				/* An 8 bit reg >= 4 without REG8 is ah/ch/dh/bh */

				saw_high_8bit = ops[rmi].reg;
		}
		if (ops[rmi].type & OP_EA && ops[rmi].reg2 >= 8) {
			rex |= REX_X;
			ops[rmi].reg2 -= 8;
		}
	}
	if (rex) {
		if (saw_high_8bit)
			tcc_error("can't encode register %%%ch when REX prefix is required",
				  "acdb"[saw_high_8bit-4]);
		g(rex);
	}
}

static void maybe_print_stats (void)
{
	static int already;

	if (0 && !already)
		/* print stats about opcodes */

	{
		const struct ASMInstr *pa;
		int freq[4];
		int op_vals[500];
		int nb_op_vals, i, j;

		already = 1;
		nb_op_vals = 0;
		memset(freq, 0, sizeof(freq));
		for (pa = asm_instrs; pa->sym != 0; pa++) {
			freq[pa->nb_ops]++;
//for(i=0;i<pa->nb_ops;i++) {

			for (j=0; j<nb_op_vals; j++) {
//if (pa->op_type[i] == op_vals[j])

				if (pa->instr_type == op_vals[j])
					goto found;
			}
//op_vals[nb_op_vals++] = pa->op_type[i];

			op_vals[nb_op_vals++] = pa->instr_type;
found: ;
//}

		}
		for (i=0; i<nb_op_vals; i++) {
			int v = op_vals[i];
//if ((v & (v - 1)) != 0)

			printf("%3d: %08x\n", i, v);
		}
		printf("size=%d nb=%d f0=%d f1=%d f2=%d f3=%d\n",
		       (int)sizeof(asm_instrs),
		       (int)sizeof(asm_instrs) / (int)sizeof(ASMInstr),
		       freq[0], freq[1], freq[2], freq[3]);
	}
}

ST_FUNC void asm_opcode(TCCState *s1, int opcode)
{
	const ASMInstr *pa;
	int i, modrm_index, modreg_index, reg, v, op1, seg_prefix, pc, p;
	int nb_ops, s;
	Operand ops[MAX_OPERANDS], *pop;
	int op_type[3];/* decoded op type */

	int alltypes;/* OR of all operand types */

	int autosize;
	int p66;

	int rex64;

	maybe_print_stats();
	/* force synthetic ';' after prefix instruction, so we can handle */
	/* one-line things like "rep stosb" instead of only "rep\nstosb" */

	if (opcode >= TOK_ASM_wait && opcode <= TOK_ASM_repnz)
		unget_tok(';');
	/* get operands */

	pop = ops;
	nb_ops = 0;
	seg_prefix = 0;
	alltypes = 0;
	for (;;) {
		if (tok == ';' || tok == TOK_LINEFEED)
			break;
		if (nb_ops >= MAX_OPERANDS) {
			tcc_error("incorrect number of operands");
		}
		parse_operand(s1, pop);
		if (tok == ':') {
			if (!(pop->type & OP_SEG) || seg_prefix)
				tcc_error("incorrect prefix");
			seg_prefix = segment_prefixes[pop->reg];
			next();
			parse_operand(s1, pop);
			if (!(pop->type & OP_EA)) {
				tcc_error("segment prefix must be followed by memory reference");
			}
		}
		pop++;
		nb_ops++;
		if (tok != ',')
			break;
		next();
	}

	s = 0;/* avoid warning */

again:
	/* optimize matching by using a lookup table (no hashing is needed
	       !) */

	for (pa = asm_instrs; pa->sym != 0; pa++) {
		int it = pa->instr_type & OPCT_MASK;
		s = 0;
		if (it == OPC_FARITH) {
			v = opcode - pa->sym;
			if (!((unsigned)v < 8 * 6 && (v % 6) == 0))
				continue;
		} else if (it == OPC_ARITH) {
			if (!(opcode >= pa->sym && opcode < pa->sym + 8*NBWLX))
				continue;
			s = (opcode - pa->sym) % NBWLX;
			if ((pa->instr_type & OPC_BWLX) == OPC_WLX) {
				/* We need to reject the xxxb opcodes that we accepted above.
						   Note that pa->sym for WLX opcodes is the 'w' token,
						   to get the 'b' token subtract one.  */

				if (((opcode - pa->sym + 1) % NBWLX) == 0)
					continue;
				s++;
			}
		} else if (it == OPC_SHIFT) {
			if (!(opcode >= pa->sym && opcode < pa->sym + 7*NBWLX))
				continue;
			s = (opcode - pa->sym) % NBWLX;
		} else if (it == OPC_TEST) {
			if (!(opcode >= pa->sym && opcode < pa->sym + NB_TEST_OPCODES))
				continue;
			/* cmovxx is a test opcode but accepts multiple sizes.
				       The suffixes aren't encoded in the table, instead we
				       simply force size autodetection always and deal with suffixed
				       variants below when we don't find e.g. "cmovzl".  */

			if (pa->instr_type & OPC_WLX)
				s = NBWLX - 1;
		} else if (pa->instr_type & OPC_B) {
			/* Some instructions don't have the full size but only
				       bwl form.  insb e.g. */

			if ((pa->instr_type & OPC_WLQ) != OPC_WLQ
			    && !(opcode >= pa->sym && opcode < pa->sym + NBWLX-1))
				continue;

			if (!(opcode >= pa->sym && opcode < pa->sym + NBWLX))
				continue;
			s = opcode - pa->sym;
		} else if (pa->instr_type & OPC_WLX) {
			if (!(opcode >= pa->sym && opcode < pa->sym + NBWLX-1))
				continue;
			s = opcode - pa->sym + 1;
		} else {
			if (pa->sym != opcode)
				continue;
		}
		if (pa->nb_ops != nb_ops)
			continue;
		/* Special case for moves.  Selecting the IM64->REG64 form
			   should only be done if we really have an >32bit imm64, and that
			   is hardcoded.  Ignore it here.  */

		if (pa->opcode == 0xb0 && ops[0].type != OP_IM64
		    && (ops[1].type & OP_REG) == OP_REG64
		    && !(pa->instr_type & OPC_0F))
			continue;
		/* now decode and check each operand */

		alltypes = 0;
		for (i = 0; i < nb_ops; i++) {
			int op1, op2;
			op1 = pa->op_type[i];
			op2 = op1 & 0x1f;
			switch (op2) {
			case OPT_IM:
				v = OP_IM8 | OP_IM16 | OP_IM32;
				break;
			case OPT_REG:
				v = OP_REG8 | OP_REG16 | OP_REG32 | OP_REG64;
				break;
			case OPT_REGW:
				v = OP_REG16 | OP_REG32 | OP_REG64;
				break;
			case OPT_IMW:
				v = OP_IM16 | OP_IM32;
				break;
			case OPT_MMXSSE:
				v = OP_MMX | OP_SSE;
				break;
			case OPT_DISP:
			case OPT_DISP8:
				v = OP_ADDR;
				break;
			default:
				v = 1 << op2;
				break;
			}
			if (op1 & OPT_EA)
				v |= OP_EA;
			op_type[i] = v;
			if ((ops[i].type & v) == 0)
				goto next;
			alltypes |= ops[i].type;
		}
		(void)alltypes;/* maybe unused */

		/* all is matching ! */

		break;
next: ;
	}
	if (pa->sym == 0) {
		if (opcode >= TOK_ASM_first && opcode <= TOK_ASM_last) {
			int b;
			b = op0_codes[opcode - TOK_ASM_first];
			if (b & 0xff00)
				g(b >> 8);
			g(b);
			return;
		} else if (opcode <= TOK_ASM_alllast) {
			tcc_error("bad operand with opcode '%s'",
				  get_tok_str(opcode, NULL));
		} else {
			/* Special case for cmovcc, we accept size suffixes but ignore
				       them, but we don't want them to blow up our tables.  */

			TokenSym *ts = table_ident[opcode - TOK_IDENT];
			if (ts->len >= 6
			    && strchr("wlq", ts->str[ts->len-1])
			    && !memcmp(ts->str, "cmov", 4)) {
				opcode = tok_alloc(ts->str, ts->len-1)->tok;
				goto again;
			}
			tcc_error("unknown opcode '%s'", ts->str);
		}
	}
	/* if the size is unknown, then evaluate it (OPC_B or OPC_WL case) */

	autosize = NBWLX-1;
	/* XXX the autosize should rather be zero, to not have to adjust this
	       all the time.  */

	if ((pa->instr_type & OPC_BWLQ) == OPC_B)
		autosize = NBWLX-2;

	if (s == autosize) {
		/* Check for register operands providing hints about the size.
			   Start from the end, i.e. destination operands.  This matters
			   only for opcodes accepting different sized registers, lar and lsl
			   are such opcodes.  */

		for (i = nb_ops - 1; s == autosize && i >= 0; i--) {
			if ((ops[i].type & OP_REG) && !(op_type[i] & (OP_CL | OP_DX)))
				s = reg_to_size[ops[i].type & OP_REG];
		}
		if (s == autosize) {
			if ((opcode == TOK_ASM_push || opcode == TOK_ASM_pop) &&
			    (ops[0].type & (OP_SEG | OP_IM8S | OP_IM32)))
				s = 2;
			else if ((opcode == TOK_ASM_push || opcode == TOK_ASM_pop) &&
				 (ops[0].type & OP_EA))
				s = NBWLX - 2;
			else
				tcc_error("cannot infer opcode suffix");
		}
	}

	rex64 = 0;
	if (pa->instr_type & OPC_48)
		rex64 = 1;
	else if (s == 3 || (alltypes & OP_REG64)) {
		/* generate REX prefix */

		int default64 = 0;
		for (i = 0; i < nb_ops; i++) {
			if (op_type[i] == OP_REG64 && pa->opcode != 0xb8) {
				/* If only 64bit regs are accepted in one operand
						   this is a default64 instruction without need for
						   REX prefixes, except for movabs(0xb8).  */

				default64 = 1;
				break;
			}
		}
		/* XXX find better encoding for the default64 instructions.  */

		if (((opcode != TOK_ASM_push && opcode != TOK_ASM_pop
		      && opcode != TOK_ASM_pushw && opcode != TOK_ASM_pushl
		      && opcode != TOK_ASM_pushq && opcode != TOK_ASM_popw
		      && opcode != TOK_ASM_popl && opcode != TOK_ASM_popq
		      && opcode != TOK_ASM_call && opcode != TOK_ASM_jmp))
		    && !default64)
			rex64 = 1;
	}
	/* now generates the operation */

	if (OPCT_IS(pa->instr_type, OPC_FWAIT))
		g(0x9b);
	if (seg_prefix)
		g(seg_prefix);
	/* Generate addr32 prefix if needed */

	for (i = 0; i < nb_ops; i++) {
		if (ops[i].type & OP_EA32) {
			g(0x67);
			break;
		}
	}
	/* generate data16 prefix if needed */

	p66 = 0;
	if (s == 1)
		p66 = 1;
	else {
		/* accepting mmx+sse in all operands --> needs 0x66 to
			   switch to sse mode.  Accepting only sse in an operand --> is
			   already SSE insn and needs 0x66/f2/f3 handling.  */

		for (i = 0; i < nb_ops; i++)
			if ((op_type[i] & (OP_MMX | OP_SSE)) == (OP_MMX | OP_SSE)
			    && ops[i].type & OP_SSE)
				p66 = 1;
	}
	if (p66)
		g(0x66);

	v = pa->opcode;
	p = v >> 8;/* possibly prefix byte(s) */

	switch (p) {
	case 0:
		break;/* no prefix */

	case 0x48:
		break;/* REX, handled elsewhere */

	case 0x66:
	case 0x67:
	case 0xf2:
	case 0xf3:
		v = v & 0xff;
		g(p);
		break;
	case 0xd4:
	case 0xd5:
		break;/* aam and aad, not prefix, but hardcoded immediate argument "10" */

	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdb:/* x87, no normal prefix */

	case 0xdc:
	case 0xdd:
	case 0xde:
	case 0xdf:
		break;
	default:
		tcc_error("bad prefix 0x%2x in opcode table", p);
		break;
	}
	if (pa->instr_type & OPC_0F)
		v = ((v & ~0xff) << 8) | 0x0f00 | (v & 0xff);
	if ((v == 0x69 || v == 0x6b) && nb_ops == 2) {
		/* kludge for imul $im, %reg */

		nb_ops = 3;
		ops[2] = ops[1];
		op_type[2] = op_type[1];
	} else if (v == 0xcd && ops[0].e.v == 3 && !ops[0].e.sym) {
		v--;/* int $3 case */

		nb_ops = 0;
	} else if ((v == 0x06 || v == 0x07)) {
		if (ops[0].reg >= 4) {
			/* push/pop %fs or %gs */

			v = 0x0fa0 + (v - 0x06) + ((ops[0].reg - 4) << 3);
		} else {
			v += ops[0].reg << 3;
		}
		nb_ops = 0;
	} else if (v <= 0x05) {
		/* arith case */

		v += ((opcode - TOK_ASM_addb) / NBWLX) << 3;
	} else if ((pa->instr_type & (OPCT_MASK | OPC_MODRM)) == OPC_FARITH) {
		/* fpu arith case */

		v += ((opcode - pa->sym) / 6) << 3;
	}
	/* search which operand will be used for modrm */

	modrm_index = -1;
	modreg_index = -1;
	if (pa->instr_type & OPC_MODRM) {
		if (!nb_ops) {
			/* A modrm opcode without operands is a special case (e.g. mfence).
				       It has a group and acts as if there's an register operand 0 */

			i = 0;
			ops[i].type = OP_REG;

			if (pa->sym == TOK_ASM_endbr64)
				ops[i].reg = 2;// dx

			else if (pa->sym >= TOK_ASM_lfence && pa->sym <= TOK_ASM_sfence)
				ops[i].reg = 0;// ax

			else
				tcc_error("bad MODR/M opcode without operands");
			goto modrm_found;
		}
		/* first look for an ea operand */

		for (i = 0; i < nb_ops; i++) {
			if (op_type[i] & OP_EA)
				goto modrm_found;
		}
		/* then if not found, a register or indirection (shift instructions) */

		for (i = 0; i < nb_ops; i++) {
			if (op_type[i] & (OP_REG | OP_MMX | OP_SSE | OP_INDIR))
				goto modrm_found;
		}

modrm_found:
		modrm_index = i;
		/* if a register is used in another operand then it is
		           used instead of group */

		for (i = 0; i < nb_ops; i++) {
			int t = op_type[i];
			if (i != modrm_index &&
			    (t & (OP_REG | OP_MMX | OP_SSE | OP_CR | OP_TR | OP_DB | OP_SEG))) {
				modreg_index = i;
				break;
			}
		}
	}

	asm_rex (rex64, ops, nb_ops, op_type, modreg_index, modrm_index);

	if (pa->instr_type & OPC_REG) {
		/* mov $im, %reg case */

		if (v == 0xb0 && s >= 1)
			v += 7;
		for (i = 0; i < nb_ops; i++) {
			if (op_type[i] & (OP_REG | OP_ST)) {
				v += ops[i].reg;
				break;
			}
		}
	}
	if (pa->instr_type & OPC_B)
		v += s >= 1;
	if (nb_ops == 1 && pa->op_type[0] == OPT_DISP8) {
		ElfSym *esym;
		int jmp_disp;
		/* see if we can really generate the jump with a byte offset */

		esym = elfsym(ops[0].e.sym);
		if (!esym || esym->st_shndx != cur_text_section->sh_num)
			goto no_short_jump;
		jmp_disp = ops[0].e.v + esym->st_value - ind - 2 - (v >= 0xff);
		if (jmp_disp == (int8_t)jmp_disp) {
			/* OK to generate jump */

			ops[0].e.sym = 0;
			ops[0].e.v = jmp_disp;
			op_type[0] = OP_IM8S;
		} else {
no_short_jump:
			/* long jump will be allowed. need to modify the
				       opcode slightly */

			if (v == 0xeb)/* jmp */

				v = 0xe9;
			else if (v == 0x70)/* jcc */

				v += 0x0f10;
			else
				tcc_error("invalid displacement");
		}
	}
	if (OPCT_IS(pa->instr_type, OPC_TEST))
		v += test_bits[opcode - pa->sym];
	else if (OPCT_IS(pa->instr_type, OPC_0F01))
		v |= 0x0f0100;
	op1 = v >> 16;
	if (op1)
		g(op1);
	op1 = (v >> 8) & 0xff;
	if (op1)
		g(op1);
	g(v);

	if (OPCT_IS(pa->instr_type, OPC_SHIFT)) {
		reg = (opcode - pa->sym) / NBWLX;
		if (reg == 6)
			reg = 7;
	} else if (OPCT_IS(pa->instr_type, OPC_ARITH)) {
		reg = (opcode - pa->sym) / NBWLX;
	} else if (OPCT_IS(pa->instr_type, OPC_FARITH)) {
		reg = (opcode - pa->sym) / 6;
	} else {
		reg = (pa->instr_type >> OPC_GROUP_SHIFT) & 7;
	}

	pc = 0;
	if (pa->instr_type & OPC_MODRM) {
		/* if a register is used in another operand then it is
		           used instead of group */

		if (modreg_index >= 0)
			reg = ops[modreg_index].reg;
		pc = asm_modrm(reg, &ops[modrm_index]);
	}
	/* emit constants */
	for (i = 0; i < nb_ops; i++) {
		v = op_type[i];
		if (v & (OP_IM8 | OP_IM16 | OP_IM32 | OP_IM64 | OP_IM8S | OP_ADDR)) {
			/* if multiple sizes are given it means we must look
			               at the op size */

			if ((v | OP_IM8 | OP_IM64) == (OP_IM8 | OP_IM16 | OP_IM32 | OP_IM64)) {
				if (s == 0)
					v = OP_IM8;
				else if (s == 1)
					v = OP_IM16;
				else if (s == 2 || (v & OP_IM64) == 0)
					v = OP_IM32;
				else
					v = OP_IM64;
			}

			if ((v & (OP_IM8 | OP_IM8S | OP_IM16)) && ops[i].e.sym)
				tcc_error("cannot relocate");

			if (v & (OP_IM8 | OP_IM8S)) {
				g(ops[i].e.v);
			} else if (v & OP_IM16) {
				gen_le16(ops[i].e.v);

			} else if (v & OP_IM64) {
				gen_expr64(&ops[i].e);

			} else if (pa->op_type[i] == OPT_DISP || pa->op_type[i] == OPT_DISP8) {
				gen_disp32(&ops[i].e);
			} else {
				gen_expr32(&ops[i].e);
			}
		}
	}
	/* after immediate operands, adjust pc-relative address */

	if (pc)
		add32le(cur_text_section->data + pc - 4, pc - ind);
}
/* return the constraint priority (we allocate first the lowest
   numbered constraints) */

static inline int constraint_priority(const char *str)
{
	int priority, c, pr;
	/* we take the lowest priority */

	priority = 0;
	for (;;) {
		c = *str;
		if (c == '\0')
			break;
		str++;
		switch (c) {
		case 'A':
			pr = 0;
			break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'S':
		case 'D':
			pr = 1;
			break;
		case 'q':
			pr = 2;
			break;
		case 'r':
		case 'R':
		case 'p':
			pr = 3;
			break;
		case 'N':
		case 'M':
		case 'I':
		case 'e':
		case 'i':
		case 'm':
		case 'g':
			pr = 4;
			break;
		default:
			tcc_error("unknown constraint '%c'", c);
			pr = 0;
		}
		if (pr > priority)
			priority = pr;
	}
	return priority;
}

static const char *skip_constraint_modifiers(const char *p)
{
	while (*p == '=' || *p == '&' || *p == '+' || *p == '%')
		p++;
	return p;
}
/* If t (a token) is of the form "%reg" or "reg" return the register number and
   type, otherwise return -1. With GCC the % is optional, too. */

ST_FUNC int asm_parse_regvar (int t)
{
	const char *s;
	Operand op;
	if (t < TOK_IDENT || (t & SYM_FIELD))
		return -1;
	s = table_ident[t - TOK_IDENT]->str;
	if (s[0] == '%')
		++s;
	t = tok_alloc_const(s);
	unget_tok(t);
	/* Internally the % prefix is required. */

	unget_tok('%');
	parse_operand(tcc_state, &op);
	/* Accept only integer regs for now. */

	if (op.type & OP_REG)
		return op.reg;
	else
		return -1;
}

#define REG_OUT_MASK 0x01
#define REG_IN_MASK 0x02

#define is_reg_allocated(reg) (regs_allocated[reg] & reg_mask)

ST_FUNC void asm_compute_constraints(ASMOperand *operands,
				     int nb_operands, int nb_outputs,
				     const uint8_t *clobber_regs,
				     int *pout_reg)
{
	ASMOperand *op;
	int sorted_op[MAX_ASM_OPERANDS];
	int i, j, k, p1, p2, tmp, reg, c, reg_mask;
	const char *str;
	uint8_t regs_allocated[NB_ASM_REGS];
	/* init fields */

	for (i=0; i<nb_operands; i++) {
		op = &operands[i];
		op->input_index = -1;
		op->ref_index = -1;
		op->reg = -1;
		op->is_memory = 0;
		op->is_rw = 0;
	}
	/* compute constraint priority and evaluate references to output
	       constraints if input constraints */

	for (i=0; i<nb_operands; i++) {
		op = &operands[i];
		str = op->constraint;
		str = skip_constraint_modifiers(str);
		if (isnum(*str) || *str == '[') {
			/* this is a reference to another constraint */

			k = find_constraint(operands, nb_operands, str, NULL);
			if ((unsigned)k >= i || i < nb_outputs)
				tcc_error("invalid reference in constraint %d ('%s')",
					  i, str);
			op->ref_index = k;
			if (operands[k].input_index >= 0)
				tcc_error("cannot reference twice the same operand");
			operands[k].input_index = i;
			op->priority = 5;
		} else if ((op->vt->r & VT_VALMASK) == VT_LOCAL
			   && op->vt->sym
			   && (reg = op->vt->sym->r & VT_VALMASK) < VT_CONST) {
			op->priority = 1;
			op->reg = reg;
		} else {
			op->priority = constraint_priority(str);
		}
	}
	/* sort operands according to their priority */

	for (i=0; i<nb_operands; i++)
		sorted_op[i] = i;
	for (i=0; i<nb_operands - 1; i++) {
		for (j=i+1; j<nb_operands; j++) {
			p1 = operands[sorted_op[i]].priority;
			p2 = operands[sorted_op[j]].priority;
			if (p2 < p1) {
				tmp = sorted_op[i];
				sorted_op[i] = sorted_op[j];
				sorted_op[j] = tmp;
			}
		}
	}

	for (i = 0; i < NB_ASM_REGS; i++) {
		if (clobber_regs[i])
			regs_allocated[i] = REG_IN_MASK | REG_OUT_MASK;
		else
			regs_allocated[i] = 0;
	}
	/* esp cannot be used */

	regs_allocated[4] = REG_IN_MASK | REG_OUT_MASK;
	/* ebp cannot be used yet */

	regs_allocated[5] = REG_IN_MASK | REG_OUT_MASK;
	/* allocate registers and generate corresponding asm moves */

	for (i=0; i<nb_operands; i++) {
		j = sorted_op[i];
		op = &operands[j];
		str = op->constraint;
		/* no need to allocate references */

		if (op->ref_index >= 0)
			continue;
		/* select if register is used for output, input or both */

		if (op->input_index >= 0) {
			reg_mask = REG_IN_MASK | REG_OUT_MASK;
		} else if (j < nb_outputs) {
			reg_mask = REG_OUT_MASK;
		} else {
			reg_mask = REG_IN_MASK;
		}
		if (op->reg >= 0) {
			if (is_reg_allocated(op->reg))
				tcc_error("asm regvar requests register that's taken already");
			reg = op->reg;
		}
try_next:
		c = *str++;
		switch (c) {
		case '=':
			goto try_next;
		case '+':
			op->is_rw = 1;
		/* FALL THRU */

		case '&':
			if (j >= nb_outputs)
				tcc_error("'%c' modifier can only be applied to outputs", c);
			reg_mask = REG_IN_MASK | REG_OUT_MASK;
			goto try_next;
		case 'A':
			/* allocate both eax and edx */

			if (is_reg_allocated(TREG_XAX) ||
			    is_reg_allocated(TREG_XDX))
				goto try_next;
			op->is_llong = 1;
			op->reg = TREG_XAX;
			regs_allocated[TREG_XAX] |= reg_mask;
			regs_allocated[TREG_XDX] |= reg_mask;
			break;
		case 'a':
			reg = TREG_XAX;
			goto alloc_reg;
		case 'b':
			reg = 3;
			goto alloc_reg;
		case 'c':
			reg = TREG_XCX;
			goto alloc_reg;
		case 'd':
			reg = TREG_XDX;
			goto alloc_reg;
		case 'S':
			reg = 6;
			goto alloc_reg;
		case 'D':
			reg = 7;
alloc_reg:
			if (op->reg >= 0 && reg != op->reg)
				goto try_next;
			if (is_reg_allocated(reg))
				goto try_next;
			goto reg_found;
		case 'q':
			/* eax, ebx, ecx or edx */

			if (op->reg >= 0) {
				if ((reg = op->reg) < 4)
					goto reg_found;
			} else
				for (reg = 0; reg < 4; reg++) {
					if (!is_reg_allocated(reg))
						goto reg_found;
				}
			goto try_next;
		case 'r':
		case 'R':
		case 'p':/* A general address, for x86(64) any register is acceptable*/

			/* any general register */

			if ((reg = op->reg) >= 0)
				goto reg_found;
			else
				for (reg = 0; reg < NB_ASM_REGS; reg++) {
					if (!is_reg_allocated(reg))
						goto reg_found;
				}
			goto try_next;
reg_found:
			/* now we can reload in the register */

			op->is_llong = 0;
			op->reg = reg;
			regs_allocated[reg] |= reg_mask;
			break;
		case 'e':
		case 'i':
			if (!((op->vt->r & (VT_VALMASK | VT_LVAL)) == VT_CONST))
				goto try_next;
			break;
		case 'I':
		case 'N':
		case 'M':
			if (!((op->vt->r & (VT_VALMASK | VT_LVAL | VT_SYM)) == VT_CONST))
				goto try_next;
			break;
		case 'm':
		case 'g':
			/* nothing special to do because the operand is already in
			               memory, except if the pointer itself is stored in a
			               memory variable (VT_LLOCAL case) */
			/* XXX: fix constant case */
			/* if it is a reference to a memory zone, it must lie
			               in a register, so we reserve the register in the
			               input registers and a load will be generated
			               later */
			if (j < nb_outputs || c == 'm') {
				if ((op->vt->r & VT_VALMASK) == VT_LLOCAL) {
					/* any general register */

					for (reg = 0; reg < NB_ASM_REGS; reg++) {
						if (!(regs_allocated[reg] & REG_IN_MASK))
							goto reg_found1;
					}
					goto try_next;
reg_found1:
					/* now we can reload in the register */

					regs_allocated[reg] |= REG_IN_MASK;
					op->reg = reg;
					op->is_memory = 1;
				}
			}
			break;
		default:
			tcc_error("asm constraint %d ('%s') could not be satisfied",
				  j, op->constraint);
			break;
		}
		/* if a reference is present for that operand, we assign it too */

		if (op->input_index >= 0) {
			operands[op->input_index].reg = op->reg;
			operands[op->input_index].is_llong = op->is_llong;
		}
	}
	/* compute out_reg. It is used to store outputs registers to memory
	       locations references by pointers (VT_LLOCAL case) */

	*pout_reg = -1;
	for (i=0; i<nb_operands; i++) {
		op = &operands[i];
		if (op->reg >= 0 &&
		    (op->vt->r & VT_VALMASK) == VT_LLOCAL &&
		    !op->is_memory) {
			for (reg = 0; reg < NB_ASM_REGS; reg++) {
				if (!(regs_allocated[reg] & REG_OUT_MASK))
					goto reg_found2;
			}
			tcc_error("could not find free output register for reloading");
reg_found2:
			*pout_reg = reg;
			break;
		}
	}
	/* print sorted constraints */
}

ST_FUNC void subst_asm_operand(CString *add_str,
			       SValue *sv, int modifier)
{
	int r, reg, size, val;

	r = sv->r;
	if ((r & VT_VALMASK) == VT_CONST) {
		if (!(r & VT_LVAL) && modifier != 'c' && modifier != 'n' &&
		    modifier != 'P')
			cstr_ccat(add_str, '$');
		if (r & VT_SYM) {
			const char *name = get_tok_str(sv->sym->v, NULL);
			if (sv->sym->v >= SYM_FIRST_ANOM) {
				/* In case of anonymous symbols ("L.42", used
						   for static data labels) we can't find them
						   in the C symbol table when later looking up
						   this name.  So enter them now into the asm label
						   list when we still know the symbol.  */

				get_asm_sym(tok_alloc_const(name), sv->sym);
			}
			if (tcc_state->leading_underscore)
				cstr_ccat(add_str, '_');
			cstr_cat(add_str, name, -1);
			if ((uint32_t)sv->c.i == 0)
				goto no_offset;
			cstr_ccat(add_str, '+');
		}
		val = sv->c.i;
		if (modifier == 'n')
			val = -val;
		cstr_printf(add_str, "%d", val);
no_offset:;

		if (r & VT_LVAL)
			cstr_cat(add_str, "(%rip)", -1);

	} else if ((r & VT_VALMASK) == VT_LOCAL) {
		cstr_printf(add_str, "%d(%%%s)", (int)sv->c.i, get_tok_str(TOK_ASM_xax + 5,
				NULL));
	} else if (r & VT_LVAL) {
		reg = r & VT_VALMASK;
		if (reg >= VT_CONST)
			tcc_internal_error("");
		cstr_printf(add_str, "(%%%s)", get_tok_str(TOK_ASM_xax + reg, NULL));
	} else {
		/* register case */

		reg = r & VT_VALMASK;
		if (reg >= VT_CONST)
			tcc_internal_error("");
		/* choose register operand size */

		if ((sv->type.t & VT_BTYPE) == VT_BYTE ||
		    (sv->type.t & VT_BTYPE) == VT_BOOL)
			size = 1;
		else if ((sv->type.t & VT_BTYPE) == VT_SHORT)
			size = 2;

		else if ((sv->type.t & VT_BTYPE) == VT_LLONG ||
			 (sv->type.t & VT_BTYPE) == VT_PTR)
			size = 8;

		else
			size = 4;
		if (size == 1 && reg >= 4)
			size = 4;

		if (modifier == 'b') {
			if (reg >= 4)
				tcc_error("cannot use byte register");
			size = 1;
		} else if (modifier == 'h') {
			if (reg >= 4)
				tcc_error("cannot use byte register");
			size = -1;
		} else if (modifier == 'w') {
			size = 2;
		} else if (modifier == 'k') {
			size = 4;

		} else if (modifier == 'q') {
			size = 8;

		}

		if (reg >= 8) {
			cstr_printf(add_str, "%%r%d%c", reg,
				    (size == 1) ? 'b' : ((size == 2) ? 'w' : ((size == 4) ? 'd' : ' ')));
			return;
		}
		switch (size) {
		case -1:
			reg = TOK_ASM_ah + reg;
			break;
		case 1:
			reg = TOK_ASM_al + reg;
			break;
		case 2:
			reg = TOK_ASM_ax + reg;
			break;
		default:
			reg = TOK_ASM_eax + reg;
			break;

		case 8:
			reg = TOK_ASM_rax + reg;
			break;

		}
		cstr_printf(add_str, "%%%s", get_tok_str(reg, NULL));
	}
}
/* generate prolog and epilog code for asm statement */

ST_FUNC void asm_gen_code(ASMOperand *operands, int nb_operands,
			  int nb_outputs, int is_output,
			  uint8_t *clobber_regs,
			  int out_reg)
{
	uint8_t regs_allocated[NB_ASM_REGS];
	ASMOperand *op;
	int i, reg;
	/* Strictly speaking %Xbp and %Xsp should be included in the
	       call-preserved registers, but currently it doesn't matter.  */

	static const uint8_t reg_saved[] = { 3, 6, 7, 12, 13, 14, 15 };
	/* mark all used registers */

	memcpy(regs_allocated, clobber_regs, sizeof(regs_allocated));
	for (i = 0; i < nb_operands; i++) {
		op = &operands[i];
		if (op->reg >= 0)
			regs_allocated[op->reg] = 1;
	}
	if (!is_output) {
		/* generate reg save code */

		for (i = 0; i < sizeof(reg_saved)/sizeof(reg_saved[0]); i++) {
			reg = reg_saved[i];
			if (regs_allocated[reg]) {
				if (reg >= 8)
					g(0x41), reg-=8;
				g(0x50 + reg);
			}
		}
		/* generate load code */

		for (i = 0; i < nb_operands; i++) {
			op = &operands[i];
			if (op->reg >= 0) {
				if ((op->vt->r & VT_VALMASK) == VT_LLOCAL &&
				    op->is_memory) {
					/* memory reference case (for both input and
					                       output cases) */

					SValue sv;
					sv = *op->vt;
					sv.r = (sv.r & ~VT_VALMASK) | VT_LOCAL | VT_LVAL;
					sv.type.t = VT_PTR;
					load(op->reg, &sv);
				} else if (i >= nb_outputs || op->is_rw) {
					/* load value in register */

					load(op->reg, op->vt);
					if (op->is_llong) {
						SValue sv;
						sv = *op->vt;
						sv.c.i += 4;
						load(TREG_XDX, &sv);
					}
				}
			}
		}
	} else {
		/* generate save code */

		for (i = 0 ; i < nb_outputs; i++) {
			op = &operands[i];
			if (op->reg >= 0) {
				if ((op->vt->r & VT_VALMASK) == VT_LLOCAL) {
					if (!op->is_memory) {
						SValue sv;
						sv = *op->vt;
						sv.r = (sv.r & ~VT_VALMASK) | VT_LOCAL;
						sv.type.t = VT_PTR;
						load(out_reg, &sv);

						sv = *op->vt;
						sv.r = (sv.r & ~VT_VALMASK) | out_reg;
						store(op->reg, &sv);
					}
				} else {
					store(op->reg, op->vt);
					if (op->is_llong) {
						SValue sv;
						sv = *op->vt;
						sv.c.i += 4;
						store(TREG_XDX, &sv);
					}
				}
			}
		}
		/* generate reg restore code */

		for (i = sizeof(reg_saved)/sizeof(reg_saved[0]) - 1; i >= 0; i--) {
			reg = reg_saved[i];
			if (regs_allocated[reg]) {
				if (reg >= 8)
					g(0x41), reg-=8;
				g(0x58 + reg);
			}
		}
	}
}

ST_FUNC void asm_clobber(uint8_t *clobber_regs, const char *str)
{
	int reg;

	unsigned int type;

	if (!strcmp(str, "memory") ||
	    !strcmp(str, "cc") ||
	    !strcmp(str, "flags"))
		return;
	reg = tok_alloc_const(str);
	if (reg >= TOK_ASM_eax && reg <= TOK_ASM_edi) {
		reg -= TOK_ASM_eax;
	} else if (reg >= TOK_ASM_ax && reg <= TOK_ASM_di) {
		reg -= TOK_ASM_ax;

	} else if (reg >= TOK_ASM_rax && reg <= TOK_ASM_rdi) {
		reg -= TOK_ASM_rax;
	} else if ((reg = asm_parse_numeric_reg(reg, &type)) >= 0) {
		;

	} else {
		tcc_error("invalid clobber register '%s'", str);
	}
	clobber_regs[reg] = 1;
}

/* ==================== tccpe.c ==================== */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) s1->sym
#define TCC_SET_STATE(fn) (tcc_enter_state(s1),fn)

#define PE_MERGE_DATA 1
#define PE_PRINT_SECTIONS 0

#define REL_TYPE_DIRECT R_X86_64_64
#define R_XXX_THUNKFIX R_X86_64_PC32
#define R_XXX_RELATIVE R_X86_64_RELATIVE
#define RSRC_RELTYPE 3
#define IMAGE_FILE_MACHINE 0x8664
#define CHARACTERISTICS_EXE 0x022F
#define CHARACTERISTICS_DLL 0x222E
#define IMAGE_BASE_EXE 0x00400000
#define IMAGE_BASE_DLL 0x10000000
#define DLLCHARACTERISTICS 0
#define OS_VER 0x0400
#define ADDR3264 ULONGLONG
#define PE_MAGIC 0x020B
#define PE_IMAGE_REL IMAGE_REL_BASED_DIR64
#ifndef IMAGE_NT_SIGNATURE
/* cross compiler: windows.h was not included */
/* ----------------------------------------------------------- */
/* definitions below are from winnt.h */
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long long ULONGLONG;
#pragma  pack(push, 1)

typedef struct _IMAGE_DOS_HEADER {/* DOS .EXE header */

	WORD e_magic;/* Magic number */

	WORD e_cblp;/* Bytes on last page of file */

	WORD e_cp;/* Pages in file */

	WORD e_crlc;/* Relocations */

	WORD e_cparhdr;/* Size of header in paragraphs */

	WORD e_minalloc;/* Minimum extra paragraphs needed */

	WORD e_maxalloc;/* Maximum extra paragraphs needed */

	WORD e_ss;/* Initial (relative) SS value */

	WORD e_sp;/* Initial SP value */

	WORD e_csum;/* Checksum */

	WORD e_ip;/* Initial IP value */

	WORD e_cs;/* Initial (relative) CS value */

	WORD e_lfarlc;/* File address of relocation table */

	WORD e_ovno;/* Overlay number */

	WORD e_res[4];/* Reserved words */

	WORD e_oemid;/* OEM identifier (for e_oeminfo) */

	WORD e_oeminfo;/* OEM information; e_oemid specific */

	WORD e_res2[10];/* Reserved words */

	DWORD e_lfanew;/* File address of new exe header */

} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;
/* PE00 */

#define IMAGE_NT_SIGNATURE 0x00004550

typedef struct _IMAGE_FILE_HEADER {
	WORD Machine;
	WORD NumberOfSections;
	DWORD TimeDateStamp;
	DWORD PointerToSymbolTable;
	DWORD NumberOfSymbols;
	WORD SizeOfOptionalHeader;
	WORD Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER 20

typedef struct _IMAGE_DATA_DIRECTORY {
	DWORD VirtualAddress;
	DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER {
	/* Standard fields. */

	WORD Magic;
	BYTE MajorLinkerVersion;
	BYTE MinorLinkerVersion;
	DWORD SizeOfCode;
	DWORD SizeOfInitializedData;
	DWORD SizeOfUninitializedData;
	DWORD AddressOfEntryPoint;
	DWORD BaseOfCode;
	/* NT additional fields. */

	ADDR3264 ImageBase;
	DWORD SectionAlignment;
	DWORD FileAlignment;
	WORD MajorOperatingSystemVersion;
	WORD MinorOperatingSystemVersion;
	WORD MajorImageVersion;
	WORD MinorImageVersion;
	WORD MajorSubsystemVersion;
	WORD MinorSubsystemVersion;
	DWORD Win32VersionValue;
	DWORD SizeOfImage;
	DWORD SizeOfHeaders;
	DWORD CheckSum;
	WORD Subsystem;
	WORD DllCharacteristics;
	ADDR3264 SizeOfStackReserve;
	ADDR3264 SizeOfStackCommit;
	ADDR3264 SizeOfHeapReserve;
	ADDR3264 SizeOfHeapCommit;
	DWORD LoaderFlags;
	DWORD NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER32, IMAGE_OPTIONAL_HEADER64, IMAGE_OPTIONAL_HEADER;
/* Export Directory */

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
/* Import Directory */
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
/* Resource Directory */
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2
/* Exception Directory */
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
/* Security Directory */
#define IMAGE_DIRECTORY_ENTRY_SECURITY 4
/* Base Relocation Table */
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
/* Debug Directory */
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6

/* Architecture Specific Data */

#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE 7
/* RVA of GP */
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8
/* TLS Directory */
#define IMAGE_DIRECTORY_ENTRY_TLS 9
/* Load Configuration Directory */
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
/* Bound Import Directory in headers */
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11
/* Import Address Table */
#define IMAGE_DIRECTORY_ENTRY_IAT 12
/* Delay Load Import Descriptors */
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13
/* COM Runtime descriptor */
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14
/* Section header format. */

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
	BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		DWORD PhysicalAddress;
		DWORD VirtualSize;
	} Misc;
	DWORD VirtualAddress;
	DWORD SizeOfRawData;
	DWORD PointerToRawData;
	DWORD PointerToRelocations;
	DWORD PointerToLinenumbers;
	WORD NumberOfRelocations;
	WORD NumberOfLinenumbers;
	DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER 40

typedef struct _IMAGE_EXPORT_DIRECTORY {
	DWORD Characteristics;
	DWORD TimeDateStamp;
	WORD MajorVersion;
	WORD MinorVersion;
	DWORD Name;
	DWORD Base;
	DWORD NumberOfFunctions;
	DWORD NumberOfNames;
	DWORD AddressOfFunctions;
	DWORD AddressOfNames;
	DWORD AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_TLS_DIRECTORY {
	ADDR3264 StartAddressOfRawData;
	ADDR3264 EndAddressOfRawData;
	ADDR3264 AddressOfIndex;
	ADDR3264 AddressOfCallBacks;
	DWORD SizeOfZeroFill;
	DWORD Characteristics;
} IMAGE_TLS_DIRECTORY;

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
	union {
		DWORD Characteristics;
		DWORD OriginalFirstThunk;
	};
	DWORD TimeDateStamp;
	DWORD ForwarderChain;
	DWORD Name;
	DWORD FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_BASE_RELOCATION {
	DWORD VirtualAddress;
	DWORD SizeOfBlock;
// WORD    TypeOffset[1];

} IMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION 8

#define IMAGE_REL_BASED_ABSOLUTE 0
#define IMAGE_REL_BASED_HIGH 1
#define IMAGE_REL_BASED_LOW 2
#define IMAGE_REL_BASED_HIGHLOW 3
#define IMAGE_REL_BASED_HIGHADJ 4
#define IMAGE_REL_BASED_MIPS_JMPADDR 5
#define IMAGE_REL_BASED_SECTION 6
#define IMAGE_REL_BASED_REL32 7
#define IMAGE_REL_BASED_DIR64 10

#define IMAGE_SCN_CNT_CODE 0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000
#define IMAGE_SCN_MEM_SHARED 0x10000000
#define IMAGE_SCN_MEM_EXECUTE 0x20000000
#define IMAGE_SCN_MEM_READ 0x40000000
#define IMAGE_SCN_MEM_WRITE 0x80000000

#define IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA 0x0020
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT 0x0100
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000

#define IMAGE_FILE_RELOCS_STRIPPED 0x0001
#pragma  pack(pop)
/* ----------------------------------------------------------- */
#endif
/* ndef IMAGE_NT_SIGNATURE */
/* ----------------------------------------------------------- */
#ifndef IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE

/* allow self-host build with tcc 0.9.27 - doesn't have this in winnt.h */

#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040
#endif
#pragma  pack(push, 1)
struct pe_header {
	IMAGE_DOS_HEADER doshdr;
	BYTE dosstub[0x40];
	DWORD nt_sig;
	IMAGE_FILE_HEADER filehdr;

	IMAGE_OPTIONAL_HEADER64 opthdr;

};

struct pe_reloc_header {
	DWORD offset;
	DWORD size;
};

struct pe_rsrc_header {
	struct _IMAGE_FILE_HEADER filehdr;
	struct _IMAGE_SECTION_HEADER sectionhdr;
};

struct pe_rsrc_reloc {
	DWORD offset;
	DWORD size;
	WORD type;
};
#pragma  pack(pop)
/* ------------------------------------------------------------- */
/* internal temporary structures */

enum {
	sec_text = 0,
	sec_rdata,
	sec_data,
	sec_bss,
	sec_idata,
	sec_pdata,
	sec_tls,
	sec_other,
	sec_rsrc,
	sec_debug,
	sec_reloc,
	sec_last
};
struct section_info {
	int cls;
	char name[32];
	ADDR3264 sh_addr;
	DWORD sh_size;
	DWORD pe_flags;
	Section *sec;
	DWORD data_size;
	IMAGE_SECTION_HEADER ish;
};

struct import_symbol {
	int sym_index;
	int iat_index;
	int thk_offset;
};

struct pe_import_info {
	int dll_index;
	int sym_count;
	struct import_symbol **symbols;
};

struct pe_info {
	TCCState *s1;
	Section *reloc;
	Section *thunk;
	Section *coffsym;
	Section *coffstr;
	const char *filename;
	int type;
	DWORD sizeofheaders;
	ADDR3264 imagebase;
	const char *start_symbol;
	DWORD start_addr;
	DWORD imp_offs;
	DWORD imp_size;
	DWORD iat_offs;
	DWORD iat_size;
	DWORD exp_offs;
	DWORD exp_size;
	DWORD tls_dir;
	DWORD tls_data;
	DWORD tls_size;
	int subsystem;
	DWORD section_align;
	DWORD file_align;
	struct section_info **sec_info;
	int sec_count;
	struct pe_import_info **imp_info;
	int imp_count;
	/* output */

	FILE *op;
	DWORD sum;
	unsigned pos;
};
#define PE_DLL 1
#define PE_GUI 2
#define PE_EXE 3
#define PE_RUN 4
/* --------------------------------------------*/

static const char *pe_export_name(TCCState *s1, ElfW(Sym) *sym)
{
	const char *name = (char *)symtab_section->link->data + sym->st_name;
	if (s1->leading_underscore && name[0] == '_'
	    && !(sym->st_other & ST_PE_STDCALL))
		return name + 1;
	return name;
}

static int dynarray_assoc(void **pp, int n, int key)
{
	int i;
	for (i = 0; i < n; ++i, ++pp)
		if (key == **(int **) pp)
			return i;
	return -1;
}

static DWORD umin(DWORD a, DWORD b)
{
	return a < b ? a : b;
}

static DWORD umax(DWORD a, DWORD b)
{
	return a < b ? b : a;
}

static DWORD pe_file_align(struct pe_info *pe, DWORD n)
{
	return (n + (pe->file_align - 1)) & ~(pe->file_align - 1);
}

static ADDR3264 pe_virtual_align(struct pe_info *pe, ADDR3264 n)
{
	return (n + (pe->section_align - 1)) & ~(ADDR3264)(pe->section_align - 1);
}

static void pe_align_section(Section *s, int a)
{
	int i = s->data_offset & (a-1);
	if (i)
		section_ptr_add(s, a - i);
}

static void pe_set_datadir(struct pe_header *hdr, int dir, DWORD addr,
			   DWORD size)
{
	hdr->opthdr.DataDirectory[dir].VirtualAddress = addr;
	hdr->opthdr.DataDirectory[dir].Size = size;
}

static int pe_fwrite(struct pe_info *pe, const void *data, int len)
{
	const WORD *p = data;
	DWORD sum;
	int ret, i;
	pe->pos += (ret = fwrite(data, 1, len, pe->op));
	sum = pe->sum;
	for (i = len; i > 0; i -= 2) {
		sum += (i >= 2) ? *p++ : *(BYTE *)p;
		sum = (sum + (sum >> 16)) & 0xFFFF;
	}
	pe->sum = sum;
	return len == ret ? 0 : -1;
}

static void pe_fpad(struct pe_info *pe, DWORD new_pos)
{
	char buf[256];
	int n, diff = new_pos - pe->pos;
	memset(buf, 0, sizeof buf);
	while (diff > 0) {
		diff -= n = umin(diff, sizeof buf);
		fwrite(buf, n, 1, pe->op);
	}
	pe->pos = new_pos;
}
/*----------------------------------------------------------------------------*/
/* some DWARF support with COFF symbol/string table for gdb */
#pragma  pack(push, 1)

struct syment {
	union {
		char n_name[8];/* old COFF version */

		struct {
			int32_t n_zeroes;/* new == 0 */

			int32_t n_offset;/* offset into string table */

		};
	};
	int32_t n_value;/* value of symbol */

	short n_scnum;/* section number */

	unsigned short n_type;/* type and derived type */

	char n_sclass;/* storage class */

	char n_numaux;/* number of aux. entries */

};
#pragma  pack(pop)

#define SHF_PRIVATE 0x80000000

static void pe_add_coffsym(struct pe_info *pe)
{
	TCCState *s1 = pe->s1;
	ElfSym *esym;
	struct syment *se;
	int n;

	if (NULL == pe->coffsym) {
		pe->coffsym = new_section(s1, ".coffsym", SHT_PROGBITS, SHF_PRIVATE);
		pe->coffstr = new_section(s1, ".coffstr", SHT_PROGBITS, SHF_PRIVATE);
		section_ptr_add(pe->coffstr, 4);/* coff string table size */

		return;
	}
	esym = (ElfSym *)s1->symtab->data;
	for (n = s1->symtab->data_offset / sizeof *esym; ++esym, --n;) {
		int sym_bind = ELFW(ST_BIND)(esym->st_info);
		if (sym_bind == STB_GLOBAL) {
			char *name = esym->st_name + (char *)s1->symtab->link->data;
			int nl = strlen(name);
			addr_t value = esym->st_value;
			int shnum = esym->st_shndx;
			if (shnum != SHN_UNDEF && shnum < s1->nb_sections) {
				Section *s = s1->sections[shnum];
				shnum = s->sh_info;
				value = value - s->sh_addr;
			}
			se = section_ptr_add(pe->coffsym, sizeof *se);
			se->n_value = value;
			se->n_scnum = shnum;
			se->n_sclass = 2;// C_EXT

			if (nl <= 8)
				memcpy(se->n_name, name, nl);
			else
				se->n_offset = put_elf_str(pe->coffstr, name);
		}
	}

	write32le(pe->coffstr->data,
		  pe->coffstr->data_offset);/* coff string table size */

}
/* Run cv2pdb, available at https://github.com/rainers/cv2pdb.  It reads
   and strips the dwarf info and creates a <exename>.pdb file instead */

static void pe_create_pdb(TCCState *s1, const char *exename)
{
	char buf[300];
	int r;
	snprintf(buf, sizeof buf, "cv2pdb.exe \"%s\"", exename);
	r = system(buf);
	strcpy(tcc_fileextension(strcpy(buf, exename)), ".pdb");
	if (r) {
		tcc_error_noabort("could not create '%s'\n(need working cv2pdb from https://github.com/rainers/cv2pdb)",
				  buf);
	} else if (s1->verbose) {
		printf("<- %s\n", buf);
	}
}
/*----------------------------------------------------------------------------*/

static int pe_write(struct pe_info *pe)
{
	static const struct pe_header pe_template = {
		{
			/* IMAGE_DOS_HEADER doshdr */

			0x5A4D,/*WORD e_magic;         Magic number */

			0x0090,/*WORD e_cblp;          Bytes on last page of file */

			0x0003,/*WORD e_cp;            Pages in file */

			0x0000,/*WORD e_crlc;          Relocations */

			0x0004,/*WORD e_cparhdr;       Size of header in paragraphs */

			0x0000,/*WORD e_minalloc;      Minimum extra paragraphs needed */

			0xFFFF,/*WORD e_maxalloc;      Maximum extra paragraphs needed */

			0x0000,/*WORD e_ss;            Initial (relative) SS value */

			0x00B8,/*WORD e_sp;            Initial SP value */

			0x0000,/*WORD e_csum;          Checksum */

			0x0000,/*WORD e_ip;            Initial IP value */

			0x0000,/*WORD e_cs;            Initial (relative) CS value */

			0x0040,/*WORD e_lfarlc;        File address of relocation table */

			0x0000,/*WORD e_ovno;          Overlay number */

			{0,0,0,0},/*WORD e_res[4];     Reserved words */

			0x0000,/*WORD e_oemid;         OEM identifier (for e_oeminfo) */

			0x0000,/*WORD e_oeminfo;       OEM information; e_oemid specific */

			{0,0,0,0,0,0,0,0,0,0},/*WORD e_res2[10];      Reserved words */

			0x00000080/*DWORD   e_lfanew;        File address of new exe header */

		},{
			/* BYTE dosstub[0x40] */
			/* 14 code bytes + "This program cannot be run in DOS mode.\r\r\n$" + 6 * 0x00 */

			0x0e,0x1f,0xba,0x0e,0x00,0xb4,0x09,0xcd,0x21,0xb8,0x01,0x4c,0xcd,0x21,0x54,0x68,
			0x69,0x73,0x20,0x70,0x72,0x6f,0x67,0x72,0x61,0x6d,0x20,0x63,0x61,0x6e,0x6e,0x6f,
			0x74,0x20,0x62,0x65,0x20,0x72,0x75,0x6e,0x20,0x69,0x6e,0x20,0x44,0x4f,0x53,0x20,
			0x6d,0x6f,0x64,0x65,0x2e,0x0d,0x0d,0x0a,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		},
		0x00004550,/* DWORD nt_sig = IMAGE_NT_SIGNATURE */

		{
			/* IMAGE_FILE_HEADER filehdr */

			IMAGE_FILE_MACHINE,/*WORD    Machine; */

			0x0003,/*WORD    NumberOfSections; */

			0x00000000,/*DWORD   TimeDateStamp; */

			0x00000000,/*DWORD   PointerToSymbolTable; */

			0x00000000,/*DWORD   NumberOfSymbols; */

			0x00E0 + (PTR_SIZE-4)*4,/*WORD    SizeOfOptionalHeader; */

			CHARACTERISTICS_EXE,/*WORD    Characteristics; */

		},{
			/* IMAGE_OPTIONAL_HEADER opthdr */
			/* Standard fields. */

			PE_MAGIC,/*WORD    Magic; */

			0x06,/*BYTE    MajorLinkerVersion; */

			0x00,/*BYTE    MinorLinkerVersion; */

			0x00000000,/*DWORD   SizeOfCode; */

			0x00000000,/*DWORD   SizeOfInitializedData; */

			0x00000000,/*DWORD   SizeOfUninitializedData; */

			0x00000000,/*DWORD   AddressOfEntryPoint; */

			0x00000000,/*DWORD   BaseOfCode; */

			/* NT additional fields. */

			0x00000000,/*ADDR3264   ImageBase; */

			0x00001000,/*DWORD   SectionAlignment; */

			0x00000200,/*DWORD   FileAlignment; */

			OS_VER >> 8,/*WORD    MajorOperatingSystemVersion; */

			       OS_VER & 255,/*WORD    MinorOperatingSystemVersion; */

			       0x0000,/*WORD    MajorImageVersion; */

			       0x0000,/*WORD    MinorImageVersion; */

			       OS_VER >> 8,/*WORD    MajorSubsystemVersion; */

			       OS_VER & 255,/*WORD    MinorSubsystemVersion; */

			       0x00000000,/*DWORD   Win32VersionValue; */

			       0x00000000,/*DWORD   SizeOfImage; */

			       0x00000200,/*DWORD   SizeOfHeaders; */

			       0x00000000,/*DWORD   CheckSum; */

			       0x0002,/*WORD    Subsystem; */

			       DLLCHARACTERISTICS,/*WORD    DllCharacteristics; */

			       0x00100000,/*ADDR3264 SizeOfStackReserve; */

			       0x00001000,/*ADDR3264 SizeOfStackCommit; */

			       0x00100000,/*ADDR3264 SizeOfHeapReserve; */

			       0x00001000,/*ADDR3264 SizeOfHeapCommit; */

			       0x00000000,/*DWORD   LoaderFlags; */

			       0x00000010,/*DWORD   NumberOfRvaAndSizes; */

			       /* IMAGE_DATA_DIRECTORY DataDirectory[16]; */

			{	{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
				{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
			}
		}
	};

	struct pe_header pe_header = pe_template;

	int i;
	DWORD file_offset;
	struct section_info *si;
	IMAGE_SECTION_HEADER *psh;
	TCCState *s1 = pe->s1;

	if (s1->do_debug)
		pe_add_coffsym(pe);

	pe->op = fopen(pe->filename, "wb");
	if (NULL == pe->op)
		return tcc_error_noabort("could not write '%s': %s", pe->filename,
					 strerror(errno));

	pe->sizeofheaders = pe_file_align(pe,
					  sizeof (struct pe_header)
					  + pe->sec_count * sizeof (IMAGE_SECTION_HEADER)
					 );

	file_offset = pe->sizeofheaders;

	if (2 == s1->verbose)
		printf("-------------------------------"
		       "\n  virt   file   size  section" "\n");
	for (i = 0; i < pe->sec_count; ++i) {
		DWORD addr, size;
		const char *sh_name;

		si = pe->sec_info[i];
		sh_name = si->name;
		addr = si->sh_addr - pe->imagebase;
		size = si->sh_size;
		psh = &si->ish;

		if (2 == s1->verbose)
			printf("%6x %6x %6x  %s\n",
			       (unsigned)addr, (unsigned)file_offset, (unsigned)size, sh_name);

		switch (si->cls) {
		case sec_text:
			if (!pe_header.opthdr.BaseOfCode)
				pe_header.opthdr.BaseOfCode = addr;
			break;

		case sec_data:

			break;

		case sec_bss:
			break;

		case sec_reloc:
			pe_set_datadir(&pe_header, IMAGE_DIRECTORY_ENTRY_BASERELOC, addr, size);
			break;

		case sec_rsrc:
			pe_set_datadir(&pe_header, IMAGE_DIRECTORY_ENTRY_RESOURCE, addr, size);
			break;

		case sec_pdata:
			pe_set_datadir(&pe_header, IMAGE_DIRECTORY_ENTRY_EXCEPTION, addr, size);
			break;
		}

		if (pe->imp_size) {
			pe_set_datadir(&pe_header, IMAGE_DIRECTORY_ENTRY_IMPORT,
				       pe->imp_offs, pe->imp_size);
			pe_set_datadir(&pe_header, IMAGE_DIRECTORY_ENTRY_IAT,
				       pe->iat_offs, pe->iat_size);
		}
		if (pe->exp_size) {
			pe_set_datadir(&pe_header, IMAGE_DIRECTORY_ENTRY_EXPORT,
				       pe->exp_offs, pe->exp_size);
		}
		if (pe->tls_size) {
			pe_set_datadir(&pe_header, IMAGE_DIRECTORY_ENTRY_TLS,
				       pe->tls_dir + (pe->thunk->sh_addr - pe->imagebase), pe->tls_size);
		}

		memcpy(psh->Name, sh_name, umin(strlen(sh_name), sizeof psh->Name));
		if (pe->coffstr && strlen(sh_name) > 8) {
			/* long section name, for example ".debug_info" */

			snprintf((char *)psh->Name, 8, "/%d", put_elf_str(pe->coffstr, sh_name));
		}

		psh->Characteristics = si->pe_flags;
		psh->VirtualAddress = addr;
		psh->Misc.VirtualSize = size;
		pe_header.opthdr.SizeOfImage =
			umax(pe_virtual_align(pe, size + addr), pe_header.opthdr.SizeOfImage);

		if (si->data_size) {
			psh->PointerToRawData = file_offset;
			file_offset = pe_file_align(pe, file_offset + si->data_size);
			psh->SizeOfRawData = file_offset - psh->PointerToRawData;
			if (si->cls == sec_text)
				pe_header.opthdr.SizeOfCode += psh->SizeOfRawData;
			else
				pe_header.opthdr.SizeOfInitializedData += psh->SizeOfRawData;
		}
	}
//pe_header.filehdr.TimeDateStamp = time(NULL);

	pe_header.filehdr.NumberOfSections = pe->sec_count;
	pe_header.opthdr.AddressOfEntryPoint = pe->start_addr;
	pe_header.opthdr.SizeOfHeaders = pe->sizeofheaders;
	pe_header.opthdr.ImageBase = pe->imagebase;
	pe_header.opthdr.Subsystem = pe->subsystem;
	pe_header.opthdr.DllCharacteristics = s1->pe_dll_characteristics;
	if (s1->pe_stack_size)
		pe_header.opthdr.SizeOfStackReserve = s1->pe_stack_size;
	if (PE_DLL == pe->type)
		pe_header.filehdr.Characteristics = CHARACTERISTICS_DLL;
	pe_header.filehdr.Characteristics |= s1->pe_characteristics;
	if (pe->reloc)
		pe_header.filehdr.Characteristics &= ~IMAGE_FILE_RELOCS_STRIPPED;

	if (pe->coffsym) {
		pe_add_coffsym(pe);
		pe_header.filehdr.PointerToSymbolTable = file_offset;
		pe_header.filehdr.NumberOfSymbols
			= pe->coffsym->data_offset / sizeof (struct syment);
	}

	pe_fwrite(pe, &pe_header, sizeof pe_header);
	for (i = 0; i < pe->sec_count; ++i)
		pe_fwrite(pe, &pe->sec_info[i]->ish, sizeof(IMAGE_SECTION_HEADER));

	file_offset = pe->sizeofheaders;
	for (i = 0; i < pe->sec_count; ++i) {
		Section *s;
		si = pe->sec_info[i];
		if (!si->data_size)
			continue;
		for (s = si->sec; s; s = s->prev) {
			pe_fpad(pe, file_offset);
			pe_fwrite(pe, s->data, s->data_offset);
			if (s->prev)
				file_offset += s->prev->sh_addr - s->sh_addr;
		}
		file_offset = si->ish.PointerToRawData + si->ish.SizeOfRawData;
		pe_fpad(pe, file_offset);
	}

	if (pe->coffsym) {
		pe_fwrite(pe, pe->coffsym->data, pe->coffsym->data_offset);
		pe_fwrite(pe, pe->coffstr->data, pe->coffstr->data_offset);
		file_offset = pe->pos;
	}

	pe->sum += file_offset;
	fseek(pe->op, offsetof(struct pe_header, opthdr.CheckSum), SEEK_SET);
	pe_fwrite(pe, &pe->sum, sizeof (DWORD));

	fclose (pe->op);

	if (2 == s1->verbose)
		printf("-------------------------------\n");
	if (s1->verbose)
		printf("<- %s (%u bytes)\n", pe->filename, (unsigned)file_offset);

	if (s1->do_debug & 16)
		pe_create_pdb(s1, pe->filename);
	return 0;
}
/*----------------------------------------------------------------------------*/

static struct import_symbol *pe_add_import(struct pe_info *pe, int sym_index)
{
	int i;
	int dll_index;
	struct pe_import_info *p;
	struct import_symbol *s;
	ElfW(Sym) *isym;

	isym = (ElfW(Sym) *)pe->s1->dynsymtab_section->data + sym_index;
	dll_index = isym->st_size;

	i = dynarray_assoc ((void **)pe->imp_info, pe->imp_count, dll_index);
	if (-1 != i) {
		p = pe->imp_info[i];
		goto found_dll;
	}
	p = tcc_mallocz(sizeof *p);
	p->dll_index = dll_index;
	dynarray_add(&pe->imp_info, &pe->imp_count, p);

found_dll:
	i = dynarray_assoc ((void **)p->symbols, p->sym_count, sym_index);
	if (-1 != i)
		return p->symbols[i];

	s = tcc_mallocz(sizeof *s);
	dynarray_add(&p->symbols, &p->sym_count, s);
	s->sym_index = sym_index;
	return s;
}

static void pe_free_imports(struct pe_info *pe)
{
	int i;
	for (i = 0; i < pe->imp_count; ++i) {
		struct pe_import_info *p = pe->imp_info[i];
		dynarray_reset(&p->symbols, &p->sym_count);
	}
	dynarray_reset(&pe->imp_info, &pe->imp_count);
}
/*----------------------------------------------------------------------------*/

static void pe_build_imports(struct pe_info *pe)
{
	int thk_ptr, ent_ptr, dll_ptr, sym_cnt, i;
	DWORD rva_base = pe->thunk->sh_addr - pe->imagebase;
	int ndlls = pe->imp_count;
	TCCState *s1 = pe->s1;

	for (sym_cnt = i = 0; i < ndlls; ++i)
		sym_cnt += pe->imp_info[i]->sym_count;

	if (0 == sym_cnt)
		return;

	pe_align_section(pe->thunk, 16);
	pe->imp_size = (ndlls + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	pe->iat_size = (sym_cnt + ndlls) * sizeof(ADDR3264);
	dll_ptr = pe->thunk->data_offset;
	thk_ptr = dll_ptr + pe->imp_size;
	ent_ptr = thk_ptr + pe->iat_size;
	pe->imp_offs = dll_ptr + rva_base;
	pe->iat_offs = thk_ptr + rva_base;
	section_ptr_add(pe->thunk, pe->imp_size + 2*pe->iat_size);

	for (i = 0; i < pe->imp_count; ++i) {
		IMAGE_IMPORT_DESCRIPTOR *hdr;
		int k, n, dllindex;
		ADDR3264 v;
		struct pe_import_info *p = pe->imp_info[i];
		const char *name;
		DLLReference *dllref;

		dllindex = p->dll_index;
		if (dllindex)
			name = tcc_basename((dllref = s1->loaded_dlls[dllindex-1])->name);
		else
			name = "", dllref = NULL;
		/* put the dll name into the import header */

		v = put_elf_str(pe->thunk, name);
		hdr = (IMAGE_IMPORT_DESCRIPTOR *)(pe->thunk->data + dll_ptr);
		hdr->FirstThunk = thk_ptr + rva_base;
		hdr->OriginalFirstThunk = ent_ptr + rva_base;
		hdr->Name = v + rva_base;

		for (k = 0, n = p->sym_count; k <= n; ++k) {
			if (k < n) {
				int iat_index = p->symbols[k]->iat_index;
				int sym_index = p->symbols[k]->sym_index;
				ElfW(Sym) *imp_sym = (ElfW(Sym) *)s1->dynsymtab_section->data + sym_index;
				const char *name = (char *)s1->dynsymtab_section->link->data + imp_sym->st_name;
				int ordinal;
				/* patch symbol (and possibly its underscored alias) */

				do {
					ElfW(Sym) *esym = (ElfW(Sym) *)symtab_section->data + iat_index;
					iat_index = esym->st_value;
					esym->st_value = thk_ptr;
					esym->st_shndx = pe->thunk->sh_num;
				} while (iat_index);

				if (dllref)
					v = 0, ordinal = imp_sym->st_value;/* ordinal from pe_load_def */

				else
					ordinal = 0, v = imp_sym->st_value;/* address from tcc_add_symbol() */

				if (pe->type == PE_RUN) {
					if (dllref) {
						if ( !dllref->handle )
							dllref->handle = LoadLibraryA(dllref->name);
						v = (ADDR3264)GetProcAddress(dllref->handle, ordinal?(char *)0+ordinal:name);
					}
					if (!v)
						tcc_error_noabort("could not resolve symbol '%s'", name);
				} else

					if (ordinal) {
						v = ordinal | (ADDR3264)1 << (sizeof(ADDR3264)*8 - 1);
					} else {
						v = pe->thunk->data_offset + rva_base;
						section_ptr_add(pe->thunk, sizeof(WORD));/* hint, not used */

						put_elf_str(pe->thunk, name);
					}

			} else {
				v = 0;/* last entry is zero */

			}

			*(ADDR3264 *)(pe->thunk->data+thk_ptr) =
				*(ADDR3264 *)(pe->thunk->data+ent_ptr) = v;
			thk_ptr += sizeof (ADDR3264);
			ent_ptr += sizeof (ADDR3264);
		}
		dll_ptr += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	}
}
/* ------------------------------------------------------------- */

struct pe_sort_sym {
	int index;
	const char *name;
};

static int sym_cmp(const void *va, const void *vb)
{
	const char *ca = (*(struct pe_sort_sym **)va)->name;
	const char *cb = (*(struct pe_sort_sym **)vb)->name;
	return strcmp(ca, cb);
}

static void pe_build_exports(struct pe_info *pe)
{
	ElfW(Sym) *sym;
	int sym_index, sym_end;
	DWORD rva_base, base_o, func_o, name_o, ord_o, str_o;
	IMAGE_EXPORT_DIRECTORY *hdr;
	int sym_count, ord;
	struct pe_sort_sym **sorted, *p;
	TCCState *s1 = pe->s1;

	FILE *op;
	char buf[260];
	const char *dllname;
	const char *name;

	rva_base = pe->thunk->sh_addr - pe->imagebase;
	sym_count = 0, sorted = NULL, op = NULL;

	sym_end = symtab_section->data_offset / sizeof(ElfW(Sym));
	for (sym_index = 1; sym_index < sym_end; ++sym_index) {
		sym = (ElfW(Sym) *)symtab_section->data + sym_index;
		name = pe_export_name(s1, sym);
		if (sym->st_other & ST_PE_EXPORT) {
			p = tcc_malloc(sizeof *p);
			p->index = sym_index;
			p->name = name;
			dynarray_add(&sorted, &sym_count, p);
		}

	}

	if (0 == sym_count)
		return;

	qsort (sorted, sym_count, sizeof *sorted, sym_cmp);

	pe_align_section(pe->thunk, 16);
	dllname = tcc_basename(pe->filename);

	base_o = pe->thunk->data_offset;
	func_o = base_o + sizeof(IMAGE_EXPORT_DIRECTORY);
	name_o = func_o + sym_count * sizeof (DWORD);
	ord_o = name_o + sym_count * sizeof (DWORD);
	str_o = ord_o + sym_count * sizeof(WORD);

	hdr = section_ptr_add(pe->thunk, str_o - base_o);
	hdr->Characteristics = 0;
	hdr->Base = 1;
	hdr->NumberOfFunctions = sym_count;
	hdr->NumberOfNames = sym_count;
	hdr->AddressOfFunctions = func_o + rva_base;
	hdr->AddressOfNames = name_o + rva_base;
	hdr->AddressOfNameOrdinals = ord_o + rva_base;
	hdr->Name = str_o + rva_base;
	put_elf_str(pe->thunk, dllname);
	/* automatically write exports to <output-filename>.def */

	pstrcpy(buf, sizeof buf, pe->filename);
	strcpy(tcc_fileextension(buf), ".def");
	op = fopen(buf, "wb");
	if (NULL == op) {
		tcc_error_noabort("could not create '%s': %s", buf, strerror(errno));
	} else {
		fprintf(op, "LIBRARY %s\n\nEXPORTS\n", dllname);
		if (s1->verbose)
			printf("<- %s (%d symbol%s)\n", buf, sym_count, &"s"[sym_count < 2]);
	}

	for (ord = 0; ord < sym_count; ++ord) {
		p = sorted[ord], sym_index = p->index, name = p->name;
		/* insert actual address later in relocate_sections() */

		put_elf_reloc(symtab_section, pe->thunk,
			      func_o, R_XXX_RELATIVE, sym_index);
		*(DWORD *)(pe->thunk->data + name_o)
			= pe->thunk->data_offset + rva_base;
		*(WORD *)(pe->thunk->data + ord_o)
			= ord;
		put_elf_str(pe->thunk, name);
		func_o += sizeof (DWORD);
		name_o += sizeof (DWORD);
		ord_o += sizeof (WORD);
		if (op)
			fprintf(op, "%s\n", name);
	}

	pe->exp_offs = base_o + rva_base;
	pe->exp_size = pe->thunk->data_offset - base_o;
	dynarray_reset(&sorted, &sym_count);
	if (op)
		fclose(op);
}
/* ------------------------------------------------------------- */

static void pe_build_reloc (struct pe_info *pe)
{
	DWORD offset, block_ptr, sh_addr, addr;
	int count, i;
	ElfW_Rel *rel, *rel_end;
	Section *s = NULL, *sr;
	struct pe_reloc_header *hdr;
	TCCState *s1 = pe->s1;
	int dwarf = 0, n;

	sh_addr = offset = block_ptr = count = i = 0;
	rel = rel_end = NULL;

	for (;;) {
		if (rel < rel_end) {
			int type = ELFW(R_TYPE)(rel->r_info);
			addr = rel->r_offset + sh_addr;
			++ rel;
			if (type != REL_TYPE_DIRECT)
				continue;
			if (dwarf) {/* don't runtime-relocate dwarf-to-dwarf */

				n = ((ElfSym *)s1->symtab->data + ELFW(R_SYM)(rel[-1].r_info))->st_shndx;
				if (n >= s1->dwlo && n < s1->dwhi)
					continue;
			}
			if (count == 0) {/* new block */

				block_ptr = pe->reloc->data_offset;
				section_ptr_add(pe->reloc, sizeof(struct pe_reloc_header));
				offset = addr & 0xFFFFFFFF<<12;
			}
			if ((addr -= offset) < (1<<12)) {/* one block spans 4k addresses */

				WORD *wp = section_ptr_add(pe->reloc, sizeof (WORD));
				*wp = addr | PE_IMAGE_REL<<12;
				++count;
				continue;
			}
			-- rel;

		} else if (s) {
			sr = s->reloc;
			if (sr) {
				rel = (ElfW_Rel *)sr->data;
				rel_end = (ElfW_Rel *)(sr->data + sr->data_offset);
				sh_addr = s->sh_addr;
				dwarf = s->sh_num >= s1->dwlo && s->sh_num < s1->dwhi;
			}
			s = s->prev;
			continue;

		} else if (i < pe->sec_count) {
			s = pe->sec_info[i]->sec, ++i;
			continue;

		} else if (!count)
			break;
		/* fill the last block and ready for a new one */

		if (count & 1)/* align for DWORDS */

			section_ptr_add(pe->reloc, sizeof(WORD)), ++count;
		hdr = (struct pe_reloc_header *)(pe->reloc->data + block_ptr);
		hdr -> offset = offset - pe->imagebase;
		hdr -> size = count * sizeof(WORD) + sizeof(struct pe_reloc_header);
		count = 0;
	}
}
/* ------------------------------------------------------------- */

static void pe_build_tls(struct pe_info *pe, Section *s)
{
	TCCState *s1 = pe->s1;
	IMAGE_TLS_DIRECTORY *d;
	int c, n;

	if (0 == s) {
		pe->tls_dir = section_add(pe->thunk, pe->tls_size, 16);
		pe->tls_data = section_add(data_section, PTR_SIZE * (1+3), 16);
		/* put relocations on entries */

		c = put_elf_sym(symtab_section, 0, 0, 0, 0, data_section->sh_num, 0);
		for (n = 0; n < 4; ++n)
			put_elf_reloc(symtab_section, pe->thunk, pe->tls_dir + PTR_SIZE*n,
				      REL_TYPE_DIRECT, c);
		/* for generators */

		set_elf_sym(symtab_section, pe->tls_data, PTR_SIZE * 4,
			    ELFW(ST_INFO)(STB_GLOBAL, STT_OBJECT),
			    0, data_section->sh_num, "__tls_index");
		return;
	}
	if (0 == s1->tls_start)
		s1->tls_start = s->sh_addr;
	d = (void *)(pe->thunk->data + pe->tls_dir);
	d->StartAddressOfRawData = s1->tls_start - data_section->sh_addr;
	d->EndAddressOfRawData = s->sh_addr + s->data_offset - data_section->sh_addr;
	d->AddressOfIndex = pe->tls_data;
	d->AddressOfCallBacks = pe->tls_data + PTR_SIZE;
	d->SizeOfZeroFill = 0;
	d->Characteristics = 0;
	/* to reuse logic from linux in xxx-link.c */

	s1->tls_end = s1->tls_start;
}
/* ------------------------------------------------------------- */

static int pe_section_class(Section *s)
{
	int type, flags;
	const char *name;
	type = s->sh_type;
	flags = s->sh_flags;
	name = s->name;

	if (0 == memcmp(name, ".stab", 5) || 0 == memcmp(name, ".debug_", 7)) {
		return sec_debug;
	} else if (flags & SHF_ALLOC) {
		if (flags & SHF_TLS)
			return sec_tls;
		if (type == SHT_PROGBITS
		    || type == SHT_INIT_ARRAY
		    || type == SHT_FINI_ARRAY) {
			if (flags & SHF_EXECINSTR)
				return sec_text;
			if (flags & SHF_WRITE)
				return sec_data;
			if (0 == strcmp(name, ".rsrc"))
				return sec_rsrc;
			if (0 == strcmp(name, ".iedat"))
				return sec_idata;
			if (0 == strcmp(name, ".pdata"))
				return sec_pdata;
			return sec_rdata;
		} else if (type == SHT_NOBITS) {
			return sec_bss;
		}
		return sec_other;
	} else {
		if (0 == strcmp(name, ".reloc"))
			return sec_reloc;
	}
	return sec_last;
}

static int pe_assign_addresses (struct pe_info *pe)
{
	int i, k, n, c, nbs;
	ADDR3264 addr;
	int *sec_order, *sec_cls;
	struct section_info *si;
	Section *s;
	TCCState *s1 = pe->s1;

	if (PE_DLL == pe->type
	    || (s1->pe_dll_characteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE))
		pe->reloc = new_section(s1, ".reloc", SHT_PROGBITS, 0);
//pe->thunk = new_section(s1, ".iedat", SHT_PROGBITS, SHF_ALLOC);

	nbs = s1->nb_sections;
	sec_order = tcc_mallocz(2 * sizeof (int) * nbs);
	sec_cls = sec_order + nbs;
	for (i = 1; i < nbs; ++i) {
		s = s1->sections[i];
		k = pe_section_class(s);
		for (n = i; n > 1 && k < (c = sec_cls[n - 1]); --n)
			sec_cls[n] = c, sec_order[n] = sec_order[n - 1];
		sec_cls[n] = k, sec_order[n] = i;
		if (k == sec_tls)
			pe->tls_size = sizeof (IMAGE_TLS_DIRECTORY);
	}
	si = NULL;
	addr = pe->imagebase + 1;

	for (i = 1; (c = sec_cls[i]) < sec_last; ++i) {
		s = s1->sections[sec_order[i]];

		if (PE_MERGE_DATA && c == sec_bss)
			c = sec_data;

		if (si && c == si->cls && c != sec_debug) {
			/* merge with previous section */

			s->sh_addr = addr = ((addr - 1) | (16 - 1)) + 1;
		} else {
			si = NULL;
			s->sh_addr = addr = pe_virtual_align(pe, addr);
		}

		if (NULL == pe->thunk
		    && c == (data_section == rodata_section ? sec_data : sec_rdata))
			pe->thunk = s;

		if (s == pe->thunk) {
			pe_build_imports(pe);
			pe_build_exports(pe);
			if (pe->tls_size)
				pe_build_tls(pe, NULL);
		}

		if (s == pe->reloc)
			pe_build_reloc (pe);

		if (0 == s->data_offset)
			continue;

		if (si)
			goto add_section;

		si = tcc_mallocz(sizeof *si);
		dynarray_add(&pe->sec_info, &pe->sec_count, si);

		strcpy(si->name, s->name);
		si->cls = c;
		si->sh_addr = addr;

		si->pe_flags = IMAGE_SCN_MEM_READ;
		if (s->sh_flags & SHF_EXECINSTR)
			si->pe_flags |= IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE;
		else if (s->sh_type == SHT_NOBITS && !(s->sh_flags & SHF_TLS))
			si->pe_flags |= IMAGE_SCN_CNT_UNINITIALIZED_DATA;
		else
			si->pe_flags |= IMAGE_SCN_CNT_INITIALIZED_DATA;
		if (s->sh_flags & SHF_WRITE)
			si->pe_flags |= IMAGE_SCN_MEM_WRITE;
		if (0 == (s->sh_flags & SHF_ALLOC))
			si->pe_flags |= IMAGE_SCN_MEM_DISCARDABLE;

add_section:
		s->sh_info = pe->sec_count;/* section number for coff syms */

		addr += s->data_offset;
		si->sh_size = addr - si->sh_addr;
		if (s->sh_type != SHT_NOBITS) {
			Section **ps = &si->sec;
			while (*ps)
				ps = &(*ps)->prev;
			*ps = s, s->prev = NULL;
			si->data_size = si->sh_size;
		}

		if (s->sh_flags & SHF_TLS) {
			strcpy(si->name, ".tls");
			pe_build_tls(pe, s);
		}
//printf("%08x %05x %08x %s\n", si->sh_addr, si->sh_size, si->pe_flags, s->name);

	}
	tcc_free(sec_order);
	return 0;
}
/*----------------------------------------------------------------------------*/

static int pe_check_symbols(struct pe_info *pe)
{
	int sym_index, sym_end;
	int ret = 0;
	TCCState *s1 = pe->s1;

	pe_align_section(text_section, 8);

	sym_end = symtab_section->data_offset / sizeof(ElfW(Sym));
	for (sym_index = 1; sym_index < sym_end; ++sym_index) {
		ElfW(Sym) *sym = (ElfW(Sym) *)symtab_section->data + sym_index;
		if (sym->st_shndx == SHN_UNDEF) {
			const char *name = (char *)symtab_section->link->data + sym->st_name;
			unsigned type = ELFW(ST_TYPE)(sym->st_info);
			int imp_sym = 0;
			struct import_symbol *is;

			int _imp_, n;
			char buffer[200];
			const char *s, *p;

			n = _imp_ = 0;
			if (sym->st_other & ST_PE_IMPORT)
				_imp_ = 1;
			do {
				s = pe_export_name(s1, sym);
				if (n) {
					/* second try: */

					if (sym->st_other & ST_PE_STDCALL) {
						/* try w/0 stdcall deco (windows API convention) */

						p = strrchr(s, '@');
						if (!p || s[0] != '_')
							break;
						strcpy(buffer, s+1)[p-s-1] = 0, s = buffer;
					} else if (s[0] != '_') {/* try non-ansi function */

						buffer[0] = '_', strcpy(buffer + 1, s), s = buffer;
					} else if (0 == memcmp(s, "_imp__", 6)) {/* mingw 3.7 */

						s += 6, _imp_ = 1;
					} else if (0 == memcmp(s, "__imp_", 6)) {/* mingw 2.0 */

						s += 6, _imp_ = 1;
					} else {
						break;
					}
				}
				imp_sym = find_elf_sym(s1->dynsymtab_section, s);
			} while (0 == imp_sym && ++n < 2);
//printf("pe_find_export (%d) %4x %s\n", n, imp_sym, name);

			if (0 == imp_sym)
				continue;/* will throw the 'undefined' error in relocate_syms() */

			is = pe_add_import(pe, imp_sym);

			if (type == STT_FUNC
			    /* symbols from assembler often have no type */

			    || (type == STT_NOTYPE && 0 == _imp_)) {
				unsigned offset = is->thk_offset;
				if (offset) {
					/* got aliased symbol, like stricmp and _stricmp */

				} else {
					unsigned char *p;
					/* add a helper symbol, will be patched later in
					                       pe_build_imports */

					sprintf(buffer, "IAT.%s", name);
					is->iat_index = put_elf_sym(
								symtab_section, 0, sizeof(DWORD),
								ELFW(ST_INFO)(STB_LOCAL, STT_OBJECT),
								0, SHN_UNDEF, buffer);

					offset = text_section->data_offset;
					is->thk_offset = offset;
					/* add the 'jmp IAT[x]' instruction */
					p = section_ptr_add(text_section, 8);
					write16le(p, 0x25FF);

					write32le(p + 2, (DWORD)-4);

					put_elf_reloc(symtab_section, text_section,
						      offset + 2, R_XXX_THUNKFIX, is->iat_index);

				}
				/* tcc_realloc might have altered sym's address */

				sym = (ElfW(Sym) *)symtab_section->data + sym_index;
				/* patch the original symbol */

				sym->st_value = offset;
				sym->st_shndx = text_section->sh_num;
				sym->st_other &= ~ST_PE_EXPORT;/* do not export */

			} else {/* STT_OBJECT */

				if (0 == _imp_)
					ret = tcc_error_noabort("symbol '%s' is missing __declspec(dllimport)", name);
				/* original symbol will be patched later in pe_build_imports */

				sym->st_value = is->iat_index;/* chain potential alias */

				is->iat_index = sym_index;
			}

		} else if (s1->rdynamic
			   && ELFW(ST_BIND)(sym->st_info) != STB_LOCAL) {
			/* if -rdynamic option, then export all non local symbols */

			sym->st_other |= ST_PE_EXPORT;
		}
	}
	return ret;
}
/*----------------------------------------------------------------------------*/
#if PE_PRINT_SECTIONS
static void pe_print_section(FILE * f, Section * s)
{
	/* just if you're curious */

	BYTE *p, *e, b;
	int i, n, l, m;
	p = s->data;
	e = s->data + s->data_offset;
	l = e - p;

	fprintf(f, "section  \"%s\"", s->name);
	if (s->link)
		fprintf(f, "\nlink     \"%s\"", s->link->name);
	if (s->reloc)
		fprintf(f, "\nreloc    \"%s\"", s->reloc->name);
	fprintf(f, "\nv_addr   %08X", (unsigned)s->sh_addr);
	fprintf(f, "\ncontents %08X", (unsigned)l);
	fprintf(f, "\n\n");

	if (s->sh_type == SHT_NOBITS)
		return;

	if (0 == l)
		return;

	if (s->sh_type == SHT_SYMTAB)
		m = sizeof(ElfW(Sym));
	else if (s->sh_type == SHT_RELX)
		m = sizeof(ElfW_Rel);
	else
		m = 16;

	fprintf(f, "%-8s", "offset");
	for (i = 0; i < m; ++i)
		fprintf(f, " %02x", i);
	n = 56;

	if (s->sh_type == SHT_SYMTAB || s->sh_type == SHT_RELX) {
		const char *fields1[] = {
			"name",
			"value",
			"size",
			"bind",
			"type",
			"other",
			"shndx",
			NULL
		};

		const char *fields2[] = {
			"offs",
			"type",
			"symb",
			NULL
		};

		const char **p;

		if (s->sh_type == SHT_SYMTAB)
			p = fields1, n = 106;
		else
			p = fields2, n = 58;

		for (i = 0; p[i]; ++i)
			fprintf(f, "%6s", p[i]);
		fprintf(f, "  symbol");
	}

	fprintf(f, "\n");
	for (i = 0; i < n; ++i)
		fprintf(f, "-");
	fprintf(f, "\n");

	for (i = 0; i < l;) {
		fprintf(f, "%08X", i);
		for (n = 0; n < m; ++n) {
			if (n + i < l)
				fprintf(f, " %02X", p[i + n]);
			else
				fprintf(f, "   ");
		}

		if (s->sh_type == SHT_SYMTAB) {
			ElfW(Sym) *sym = (ElfW(Sym) *) (p + i);
			const char *name = s->link->data + sym->st_name;
			fprintf(f, "  %04X  %04X  %04X   %02X    %02X    %02X   %04X  \"%s\"",
				(unsigned)sym->st_name,
				(unsigned)sym->st_value,
				(unsigned)sym->st_size,
				(unsigned)ELFW(ST_BIND)(sym->st_info),
				(unsigned)ELFW(ST_TYPE)(sym->st_info),
				(unsigned)sym->st_other,
				(unsigned)sym->st_shndx,
				name);

		} else if (s->sh_type == SHT_RELX) {
			ElfW_Rel *rel = (ElfW_Rel *) (p + i);
			ElfW(Sym) *sym =
				(ElfW(Sym) *) s->link->data + ELFW(R_SYM)(rel->r_info);
			const char *name = s->link->link->data + sym->st_name;
			fprintf(f, "  %04X   %02X   %04X  \"%s\"",
				(unsigned)rel->r_offset,
				(unsigned)ELFW(R_TYPE)(rel->r_info),
				(unsigned)ELFW(R_SYM)(rel->r_info),
				name);
		} else {
			fprintf(f, "   ");
			for (n = 0; n < m; ++n) {
				if (n + i < l) {
					b = p[i + n];
					if (b < 32 || b >= 127)
						b = '.';
					fprintf(f, "%c", b);
				}
			}
		}
		i += m;
		fprintf(f, "\n");
	}
	fprintf(f, "\n\n");
}

static void pe_print_sections(TCCState *s1, const char *fname)
{
	Section *s;
	FILE *f;
	int i;
	f = fopen(fname, "w");
	for (i = 1; i < s1->nb_sections; ++i) {
		s = s1->sections[i];
		pe_print_section(f, s);
	}
	pe_print_section(f, s1->dynsymtab_section);
	fclose(f);
}
#endif
/* ------------------------------------------------------------- */
ST_FUNC int pe_putimport(TCCState *s1, int dllindex, const char *name,
			 addr_t value)
{
	return set_elf_sym(
		       s1->dynsymtab_section,
		       value,
		       dllindex,/* st_size */

		       ELFW(ST_INFO)(STB_GLOBAL, STT_NOTYPE),
		       0,
		       value ? SHN_ABS : SHN_UNDEF,
		       name
	       );
}

static int read_mem(int fd, unsigned offset, void *buffer, unsigned len)
{
	lseek(fd, offset, SEEK_SET);
	return len == read(fd, buffer, len);
}
/* ------------------------------------------------------------- */

static int get_dllexports(int fd, char **pp)
{
	int i, k, l, n, n0, ret;
	char *p;

	IMAGE_SECTION_HEADER ish;
	IMAGE_EXPORT_DIRECTORY ied;
	IMAGE_DOS_HEADER dh;
	IMAGE_FILE_HEADER ih;
	DWORD sig, ref, addr;
	DWORD *namep = NULL, p0 = 0, p1;

	int pef_hdroffset, opt_hdroffset, sec_hdroffset;

	n = n0 = 0;
	p = NULL;
	ret = 1;
	if (!read_mem(fd, 0, &dh, sizeof dh))
		goto the_end;
	if (!read_mem(fd, dh.e_lfanew, &sig, sizeof sig))
		goto the_end;
	if (sig != 0x00004550)
		goto the_end;
	pef_hdroffset = dh.e_lfanew + sizeof sig;
	if (!read_mem(fd, pef_hdroffset, &ih, sizeof ih))
		goto the_end;
	opt_hdroffset = pef_hdroffset + sizeof ih;
	if (ih.Machine == 0x014C || ih.Machine == 0x01C0) {
		IMAGE_OPTIONAL_HEADER32 oh;
		sec_hdroffset = opt_hdroffset + sizeof oh;
		if (!read_mem(fd, opt_hdroffset, &oh, sizeof oh))
			goto the_end;
		if (IMAGE_DIRECTORY_ENTRY_EXPORT >= oh.NumberOfRvaAndSizes)
			goto the_end_0;
		addr = oh.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	} else if (ih.Machine == 0x8664 || ih.Machine == 0xAA64) {
		IMAGE_OPTIONAL_HEADER64 oh;
		sec_hdroffset = opt_hdroffset + sizeof oh;
		if (!read_mem(fd, opt_hdroffset, &oh, sizeof oh))
			goto the_end;
		if (IMAGE_DIRECTORY_ENTRY_EXPORT >= oh.NumberOfRvaAndSizes)
			goto the_end_0;
		addr = oh.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	} else
		goto the_end;
//printf("addr: %08x\n", addr);

	for (i = 0; i < ih.NumberOfSections; ++i) {
		if (!read_mem(fd, sec_hdroffset + i * sizeof ish, &ish, sizeof ish))
			goto the_end;
//printf("vaddr: %08x\n", ish.VirtualAddress);

		if (addr >= ish.VirtualAddress && addr < ish.VirtualAddress + ish.SizeOfRawData)
			goto found;
	}
	goto the_end_0;
found:
	ref = ish.VirtualAddress - ish.PointerToRawData;
	if (!read_mem(fd, addr - ref, &ied, sizeof ied))
		goto the_end;
	k = ied.NumberOfNames;
	if (k) {
		namep = tcc_malloc(l = k * sizeof *namep);
		if (!read_mem(fd, ied.AddressOfNames - ref, namep, l))
			goto the_end;
		for (i = l = 0; i < k; ++i) {
			p1 = namep[i] - ref;
			if (p1 != p0)
				lseek(fd, p0 = p1, SEEK_SET), l = 0;
			do {
				if (0 == l) {
					if (n + 1000 >= n0)
						p = tcc_realloc(p, n0 += 1000);
					if ((l = read(fd, p + n, 1000 - 1)) <= 0)
						goto the_end;
				}
				--l, ++p0;
			} while (p[n++]);
		}
		p[n] = 0;
	}
the_end_0:
	ret = 0;
the_end:
	tcc_free(namep);
	if (ret && p)
		tcc_free(p), p = NULL;
	*pp = p;
	return ret;
}
/* -------------------------------------------------------------
 *  This is for compiled windows resources in 'coff' format
 *  as generated by 'windres.exe -O coff ...'.
 */

static int pe_load_res(TCCState *s1, int fd)
{
	struct pe_rsrc_header hdr;
	Section *rsrc_section;
	int i, ret = -1, sym_index;
	BYTE *ptr;
	unsigned offs;

	if (!read_mem(fd, 0, &hdr, sizeof hdr))
		goto quit;

	if (hdr.filehdr.Machine != IMAGE_FILE_MACHINE
	    || hdr.filehdr.NumberOfSections != 1
	    || strcmp((char *)hdr.sectionhdr.Name, ".rsrc") != 0)
		goto quit;

	rsrc_section = new_section(s1, ".rsrc", SHT_PROGBITS, SHF_ALLOC);
	ptr = section_ptr_add(rsrc_section, hdr.sectionhdr.SizeOfRawData);
	offs = hdr.sectionhdr.PointerToRawData;
	if (!read_mem(fd, offs, ptr, hdr.sectionhdr.SizeOfRawData))
		goto quit;
	offs = hdr.sectionhdr.PointerToRelocations;
	sym_index = put_elf_sym(symtab_section, 0, 0, 0, 0, rsrc_section->sh_num,
				".rsrc");
	for (i = 0; i < hdr.sectionhdr.NumberOfRelocations; ++i) {
		struct pe_rsrc_reloc rel;
		if (!read_mem(fd, offs, &rel, sizeof rel))
			goto quit;
// printf("rsrc_reloc: %x %x %x\n", rel.offset, rel.size, rel.type);

		if (rel.type != RSRC_RELTYPE)
			goto quit;
		put_elf_reloc(symtab_section, rsrc_section,
			      rel.offset, R_XXX_RELATIVE, sym_index);
		offs += sizeof rel;
	}
	ret = 0;
quit:
	return ret;
}
/* ------------------------------------------------------------- */

static char *trimfront(char *p)
{
	while ((unsigned char)*p <= ' ' && *p && *p != '\n')
		++p;
	return p;
}
/*
static char *trimback(char *a, char *e)
{
    while (e > a && (unsigned char)e[-1] <= ' ')
	--e;
    *e = 0;;
    return a;
}*/
static char *get_token(char **s, char *f)
{
	char *p, *e;
	int q;

	p = trimfront(*s);
	q = *p;
	if (q == '"')/* support quoted LIBRARY "xyz.dll" */

		++p;
	else
		q = ' ';
	for (e = p; (unsigned char)*e >= ' ' && *e != q; ++e)
		;
	if (*e == '"')
		*e++ = 0;
	*s = trimfront(e);
	*f = **s, *e = 0;
	return p;
}

static int pe_load_def(TCCState *s1, int fd)
{
	int state = 0, ret = -1, dllindex = 0, ord;
	char dllname[80], *buf, *line, *p, *x, next;

	buf = tcc_load_text(fd);
	if (!buf)
		return ret;

	for (line = buf;; ++line) {
		p = get_token(&line, &next);
		if (!(*p && *p != ';'))
			goto skip;
		switch (state) {
		case 0:
			if (0 != stricmp(p, "LIBRARY") || next == '\n')
				goto quit;
			pstrcpy(dllname, sizeof dllname, get_token(&line, &next));
			if (!*tcc_fileextension(dllname))
				pstrcat(dllname, sizeof dllname, ".dll");
			++state;
			break;
		case 1:
			if (0 != stricmp(p, "EXPORTS"))
				goto quit;
			++state;
			break;
		case 2:
			dllindex = tcc_add_dllref(s1, dllname, 0)->index;
			++state;
		/* fall through */

		default:
			/* get ordinal and will store in sym->st_value */

			ord = 0;
			if (next == '@') {
				x = get_token(&line, &next);
				ord = (int)strtol(x + 1, &x, 10);
			}
//printf("token %s ; %s : %d\n", dllname, p, ord);

			pe_putimport(s1, dllindex, p, ord);
			break;
		}
skip:
		while ((unsigned char)next > ' ')
			get_token(&line, &next);
		if (next != '\n')
			break;
	}
	ret = 0;
quit:
	tcc_free(buf);
	return ret;
}
/* ------------------------------------------------------------- */

static int pe_load_dll(TCCState *s1, int fd, const char *filename)
{
	char *p, *q;
	DLLReference *ref = tcc_add_dllref(s1, filename, 0);
	if (ref->found)
		return 0;
	if (get_dllexports(fd, &p))
		return -1;
	if (p) {
		for (q = p; *q; q += 1 + strlen(q))
			pe_putimport(s1, ref->index, q, 0);
		tcc_free(p);
	}
	return 0;
}

ST_FUNC int pe_load_file(struct TCCState *s1, int fd, const char *filename)
{
	int ret = -1;
	char buf[10];
	if (0 == strcmp(tcc_fileextension(filename), ".def"))
		ret = pe_load_def(s1, fd);
	else if (pe_load_res(s1, fd) == 0)
		ret = 0;
	else if (read_mem(fd, 0, buf, 4) && 0 == memcmp(buf, "MZ", 2))
		ret = pe_load_dll(s1, fd, filename);
	return ret;
}

PUB_FUNC int tcc_get_dllexports(const char *filename, char **pp)
{
	int ret, fd = open(filename, O_RDONLY | O_BINARY);
	if (fd < 0)
		return -1;
	ret = get_dllexports(fd, pp);
	close(fd);
	return ret;
}
/* ------------------------------------------------------------- */

static unsigned pe_add_unwind_info(TCCState *s1)
{
	if (NULL == s1->uw_pdata) {
		s1->uw_pdata = find_section(s1, ".pdata");
		s1->uw_pdata->sh_addralign = 4;
	}
	if (0 == s1->uw_sym)
		s1->uw_sym = put_elf_sym(symtab_section, 0, 0, 0, 0, text_section->sh_num,
					 ".uw_base");
	if (0 == s1->uw_offs) {
		/* As our functions all have the same stackframe, we use one entry for all */

		static const unsigned char uw_info[] = {
			0x01,// UBYTE: 3 Version , UBYTE: 5 Flags

			0x04,// UBYTE Size of prolog

			0x02,// UBYTE Count of unwind codes

			0x05,// UBYTE: 4 Frame Register (rbp), UBYTE: 4 Frame Register offset (scaled)

// USHORT * n Unwind codes array (descending order)
// 0x0b, 0x01, 0xff, 0xff, // stack size
// UBYTE offset of end of instr in prolog + 1, UBYTE:4 operation, UBYTE:4 info

			0x04, 0x03,// 3:0 UWOP_SET_FPREG (mov rsp -> rbp)

			0x01, 0x50,// 0:5 UWOP_PUSH_NONVOL (push rbp)

		};

		Section *s = text_section;
		unsigned char *p;

		section_ptr_add(s, -s->data_offset & 3);/* align */

		s1->uw_offs = s->data_offset;
		p = section_ptr_add(s, sizeof uw_info);
		memcpy(p, uw_info, sizeof uw_info);
	}

	return s1->uw_offs;
}

ST_FUNC void pe_add_unwind_data(unsigned start, unsigned end, unsigned stack)
{
	TCCState *s1 = tcc_state;
	Section *pd;
	unsigned o, n, d;
	struct { /* _RUNTIME_FUNCTION */
		DWORD BeginAddress;
		DWORD EndAddress;
		DWORD UnwindData;
	} *p;

	d = pe_add_unwind_info(s1);
	pd = s1->uw_pdata;
	o = pd->data_offset;
	p = section_ptr_add(pd, sizeof *p);
	/* record this function */

	p->BeginAddress = start;
	p->EndAddress = end;
	p->UnwindData = d;
	/* put relocations on it */

	for (n = o + sizeof *p; o < n; o += sizeof p->BeginAddress)
		put_elf_reloc(symtab_section, pd, o, R_XXX_RELATIVE, s1->uw_sym);
}
/* ------------------------------------------------------------- */
#define PE_STDSYM(n,s) n

static void pe_add_runtime(TCCState *s1, struct pe_info *pe)
{
	const char *start_symbol;
	int pe_type;

	if (TCC_OUTPUT_DLL == s1->output_type) {
		pe_type = PE_DLL;
		start_symbol = PE_STDSYM("__dllstart","@12");
	} else {
		const char *run_symbol;
		if (find_elf_sym(symtab_section, PE_STDSYM("WinMain","@16"))) {
			start_symbol = "__winstart";
			run_symbol = "__runwinmain";
			pe_type = PE_GUI;
		} else if (find_elf_sym(symtab_section, PE_STDSYM("wWinMain","@16"))) {
			start_symbol = "__wwinstart";
			run_symbol = "__runwwinmain";
			pe_type = PE_GUI;
		} else if (find_elf_sym(symtab_section, "wmain")) {
			start_symbol = "__wstart";
			run_symbol = "__runwmain";
			pe_type = PE_EXE;
		} else {
			start_symbol = "__start";
			run_symbol = "__runmain";
			pe_type = PE_EXE;
			if (s1->pe_subsystem == 2)
				pe_type = PE_GUI;
		}

		if (TCC_OUTPUT_MEMORY == s1->output_type && !s1->nostdlib)
			start_symbol = run_symbol;
	}
	if (s1->elf_entryname) {
		pe->start_symbol = start_symbol = s1->elf_entryname;
	} else {
		pe->start_symbol = start_symbol + 1;
		if (!s1->leading_underscore || strchr(start_symbol, '@'))
			++start_symbol;
	}
	/* grab the startup code from libtcc1.a */

	if (TCC_OUTPUT_MEMORY != s1->output_type || s1->run_main)

		set_global_sym(s1, start_symbol, NULL, 0);

	if (0 == s1->nostdlib) {
		static const char *const libs[] = {
			"msvcrt", "kernel32", "", "user32", "gdi32", NULL
		};
		const char *const *pp, *p;
		s1->static_link = 0;/* no static crt for tcc */

		for (pp = libs; 0 != (p = *pp); ++pp) {
			if (*p)
				tcc_add_library(s1, p);
			else if (PE_DLL != pe_type && PE_GUI != pe_type)
				break;
		}
	}
	/* need this for 'tccelf.c:relocate_sections()' */

	if (TCC_OUTPUT_DLL == s1->output_type)
		s1->output_type = TCC_OUTPUT_EXE;
	if (TCC_OUTPUT_MEMORY == s1->output_type)
		pe_type = PE_RUN;
	pe->type = pe_type;
}

ST_FUNC int pe_setsubsy(TCCState *s1, const char *arg)
{
	static const struct subsy {
		const char *p;
		int v;
	} x[] = {

		{ "native", 1 },
		{ "gui", 2 },
		{ "windows", 2 },
		{ "console", 3 },
		{ "posix", 7 },
		{ "efiapp", 10 },
		{ "efiboot", 11 },
		{ "efiruntime", 12 },
		{ "efirom", 13 },

		{ 0, -1 }
	};
	const struct subsy *y;
	for (y = x;; ++y) {
		if (!y->p)
			return -1;
		if (0 == strcmp(y->p, arg)) {
			s1->pe_subsystem = y->v;
			return 0;
		}
	}
}

static void pe_set_options(TCCState * s1, struct pe_info *pe)
{
	if (PE_DLL == pe->type) {
		/* XXX: check if is correct for arm-pe target */

		pe->imagebase = IMAGE_BASE_DLL;
	} else {
		pe->imagebase = IMAGE_BASE_EXE;
	}

	if (PE_DLL == pe->type || PE_GUI == pe->type)
		pe->subsystem = 2;
	else
		pe->subsystem = 3;
	/* Allow override via -Wl,-subsystem=... option */

	if (s1->pe_subsystem != 0)
		pe->subsystem = s1->pe_subsystem;
	/* set default file/section alignment */

	if (pe->subsystem == 1) {
		pe->section_align = 0x20;
		pe->file_align = 0x20;
	} else {
		pe->section_align = 0x1000;
		pe->file_align = 0x200;
	}

	if (s1->section_align != 0)
		pe->section_align = s1->section_align;
	if (s1->pe_file_align != 0)
		pe->file_align = s1->pe_file_align;

	if ((pe->subsystem >= 10) && (pe->subsystem <= 12))
		pe->imagebase = 0;

	if (s1->has_text_addr)
		pe->imagebase = s1->text_addr;
}

ST_FUNC int pe_output_file(TCCState *s1, const char *filename)
{
	struct pe_info pe;

	memset(&pe, 0, sizeof pe);
	pe.filename = filename;
	pe.s1 = s1;
	s1->filetype = 0;

	tcc_add_pragma_libs(s1);
	pe_add_runtime(s1, &pe);
	resolve_common_syms(s1);
	pe_set_options(s1, &pe);
	pe_check_symbols(&pe);
	if (s1->nb_errors)
		goto done;
	if (filename) {
		pe_assign_addresses(&pe);
		relocate_syms(s1, s1->symtab, 0);
		if (s1->nb_errors)
			goto done;
		s1->pe_imagebase = pe.imagebase;
		relocate_sections(s1);
		pe.start_addr = (DWORD)
				(get_sym_addr(s1, pe.start_symbol, 1, 1) - pe.imagebase);
		if (s1->nb_errors)
			goto done;
		pe_write(&pe);
	} else {
		/* -run */

		pe.thunk = data_section;
		pe_build_imports(&pe);
		s1->run_main = pe.start_symbol;

		s1->uw_pdata = find_section(s1, ".pdata");

	}
done:
	dynarray_reset(&pe.sec_info, &pe.sec_count);
	pe_free_imports(&pe);
#if PE_PRINT_SECTIONS
	if (g_debug & 8)
		pe_print_sections(s1, "tcc.log");
#endif

	return s1->nb_errors ? -1 : 0;
}
/* ------------------------------------------------------------- */
/* ONE_SOURCE */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE

#define TCC_STATE_VAR(sym) s1->sym
#define TCC_SET_STATE(fn) (tcc_enter_state(s1),fn)
/**/
/* global variables */
/* XXX: get rid of this ASAP (or maybe not) */

ST_DATA struct TCCState *tcc_state;
TCC_SEM(static tcc_compile_sem);
/* an array of pointers to memory to be free'd after errors */

ST_DATA void **stk_data;
ST_DATA int nb_stk_data;
/* option -d<num> (for general development purposes) */

ST_DATA int g_debug;
/**/

ST_FUNC char *normalize_slashes(char *path)
{
	char *p;
	for (p = path; *p; ++p)
		if (*p == '\\')
			*p = '/';
	return path;
}

#if defined LIBTCC_AS_DLL && !defined CONFIG_TCCDIR
static HMODULE tcc_module;
BOOL WINAPI DllMain (HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved)
{
	if (DLL_PROCESS_ATTACH == dwReason)
		tcc_module = hDll;
	return TRUE;
}
#else
#define tcc_module NULL /* NULL means executable itself */

#endif

#ifndef CONFIG_TCCDIR
/* on win32, we suppose the lib and includes are at the location of 'tcc.exe' */

static inline char *config_tccdir_w32(char *path)
{
	char *p;
	GetModuleFileNameA(tcc_module, path, MAX_PATH);
	p = tcc_basename(normalize_slashes(strlwr(path)));
	if (p > path)
		--p;
	*p = 0;
	return path;
}
#define CONFIG_TCCDIR config_tccdir_w32(alloca(MAX_PATH))
#endif

static void tcc_add_systemdir(TCCState *s)
{
	char buf[1000];
	GetSystemDirectoryA(buf, sizeof buf);
	tcc_add_library_path(s, normalize_slashes(buf));
}
/* for tcc -E : On windows (depending on compiler) a FILE*
   must be created by the same module where it is used. */

PUB_FUNC FILE *tcc_fopen(const char *f, const char *m)
{
	return fopen(f, m);
}
PUB_FUNC int tcc_fclose(FILE *f)
{
	return fclose(f);
}
/**/
PUB_FUNC void tcc_enter_state(TCCState *s1)
{
	if (s1->error_set_jmp_enabled)
		return;
	WAIT_SEM(&tcc_compile_sem);
	tcc_state = s1;
}

PUB_FUNC void tcc_exit_state(TCCState *s1)
{
	if (s1->error_set_jmp_enabled)
		return;
	tcc_state = NULL;
	POST_SEM(&tcc_compile_sem);
}
/**/
/* copy a string and truncate it. */

ST_FUNC char *pstrcpy(char *buf, size_t buf_size, const char *s)
{
	char *q, *q_end;
	int c;

	if (buf_size > 0) {
		q = buf;
		q_end = buf + buf_size - 1;
		while (q < q_end) {
			c = *s++;
			if (c == '\0')
				break;
			*q++ = c;
		}
		*q = '\0';
	}
	return buf;
}
/* strcat and truncate. */

ST_FUNC char *pstrcat(char *buf, size_t buf_size, const char *s)
{
	size_t len;
	len = strlen(buf);
	if (len < buf_size)
		pstrcpy(buf + len, buf_size - len, s);
	return buf;
}

ST_FUNC char *pstrncpy(char *out, size_t buf_size, const char *s, size_t num)
{
	if (num >= buf_size)
		num = buf_size - 1;
	memcpy(out, s, num);
	out[num] = '\0';
	return out;
}
/* extract the basename of a file */

PUB_FUNC char *tcc_basename(const char *name)
{
	char *p = (char *)strchr(name, 0);
	while (p > name && !IS_DIRSEP(p[-1]))
		--p;
	return p;
}
/* extract extension part of a file
 *
 * (if no extension, return pointer to end-of-string)
 */

PUB_FUNC char *tcc_fileextension (const char *name)
{
	char *b = tcc_basename(name);
	char *e = strrchr(b, '.');
	return e ? e : strchr(b, 0);
}

ST_FUNC char *tcc_load_text(int fd)
{
	int len = lseek(fd, 0, SEEK_END);
	char *buf = load_data(fd, 0, len + 1);
	buf[len] = 0;
	return buf;
}
/* replace *pp by copy of 'str' or NULL */

static void tcc_set_str(char **pp, const char *str)
{
	tcc_free(*pp);
	*pp = str ? tcc_strdup(str) : NULL;
}
/* set/append 'str' to *pp (separated by 'sep' unless 0) */

static void tcc_concat_str(char **pp, const char *str, int sep)
{
	int l = *pp ? strlen(*pp) + !!sep : 0;
	*pp = tcc_realloc(*pp, l + strlen(str) + 1);
	if (l && sep)
		((*pp)[l - 1] = sep);
	strcpy(*pp + l, str);
}
/**/
/* memory management */
/* we'll need the actual versions for a minute */

#undef free
#undef realloc

static void *default_reallocator(void *ptr, unsigned long size)
{
	void *ptr1;
	if (size == 0) {
		free(ptr);
		ptr1 = NULL;
	} else {
		ptr1 = realloc(ptr, size);
		if (!ptr1) {
			fprintf(stderr, "tcc: memory full\n");
			exit (1);
		}
	}
	return ptr1;
}

ST_FUNC void libc_free(void *ptr)
{
	free(ptr);
}
/* global so that every tcc_alloc()/tcc_free() call doesn't need to be changed */

static void *(*reallocator)(void *, unsigned long) = default_reallocator;

LIBTCCAPI void tcc_set_realloc(TCCReallocFunc *my_realloc)
{
	reallocator = my_realloc ? my_realloc : default_reallocator;
}
/* in case MEM_DEBUG is #defined */

#undef tcc_free
#undef tcc_malloc
#undef tcc_realloc
#undef tcc_mallocz
#undef tcc_strdup

PUB_FUNC void tcc_free(void *ptr)
{
	reallocator(ptr, 0);
}

PUB_FUNC void *tcc_malloc(unsigned long size)
{
	return reallocator(0, size);
}

PUB_FUNC void *tcc_realloc(void *ptr, unsigned long size)
{
	return reallocator(ptr, size);
}

PUB_FUNC void *tcc_mallocz(unsigned long size)
{
	void *ptr;
	ptr = tcc_malloc(size);
	if (size)
		memset(ptr, 0, size);
	return ptr;
}

PUB_FUNC char *tcc_strdup(const char *str)
{
	char *ptr;
	ptr = tcc_malloc(strlen(str) + 1);
	strcpy(ptr, str);
	return ptr;
}
/* MEM_DEBUG */

#define realpath(file, buf) _fullpath(buf, file, 260)
/* for #pragma once */
ST_FUNC int normalized_PATHCMP(const char *f1, const char *f2)
{
	char *p1, *p2;
	int ret = 1;
	if (!!(p1 = realpath(f1, NULL))) {
		if (!!(p2 = realpath(f2, NULL))) {
			ret = PATHCMP(p1, p2);
			libc_free(p2);/* realpath() requirement */

		}
		libc_free(p1);
	}
	return ret;
}
/**/
/* dynarrays */

ST_FUNC void dynarray_add(void *ptab, int *nb_ptr, void *data)
{
	int nb, nb_alloc;
	void **pp;

	nb = *nb_ptr;
	pp = *(void ***)ptab;
	/* every power of two we double array size */

	if ((nb & (nb - 1)) == 0) {
		if (!nb)
			nb_alloc = 1;
		else
			nb_alloc = nb * 2;
		pp = tcc_realloc(pp, nb_alloc * sizeof(void *));
		*(void ***)ptab = pp;
	}
	pp[nb++] = data;
	*nb_ptr = nb;
}

ST_FUNC void dynarray_reset(void *pp, int *n)
{
	void **p;
	for (p = *(void ***)pp; *n; ++p, --*n)
		if (*p)
			tcc_free(*p);
	tcc_free(*(void **)pp);
	*(void **)pp = NULL;
}

static void dynarray_split(char ***argv, int *argc, const char *p, int sep)
{
	int qot, c;
	CString str;
	for (;;) {
		while (c = (unsigned char)*p, c <= ' ' && c != '\0')
			++p;
		if (c == '\0')
			break;
		cstr_new(&str);
		qot = 0;
		do {
			++p;
			if (sep) {/* e.g. to split -Wl,-opt,arg */

				if (c == sep)
					break;
			} else {
				/* e.g. for tcc_set_options() or "tcc @listfile" */

				if (c == '\\' && (*p == '"' || *p == '\\')) {
					c = *p++;
				} else if (c == '"') {
					qot ^= 1;
					continue;
				} else if (c <= ' ' && !qot) {
					break;
				}
			}
			cstr_ccat(&str, c);
		} while (c = (unsigned char)*p, c != '\0');
		cstr_ccat(&str, '\0');
//printf("<%s>\n", str.data);

		dynarray_add(argv, argc, str.data);
	}
}

static void tcc_split_path(TCCState *s, void *p_ary, int *p_nb_ary,
			   const char *in)
{
	const char *p;
	do {
		int c;
		CString str;

		cstr_new(&str);
		for (p = in; c = *p, c != '\0' && c != PATHSEP[0]; ++p) {
			if (c == '{' && p[1] && p[2] == '}') {
				c = p[1], p += 2;
				if (c == 'B')
					cstr_cat(&str, s->tcc_lib_path, -1);
				if (c == 'R')
					cstr_cat(&str, CONFIG_SYSROOT, -1);
				if (c == 'f' && file) {
					/* substitute current file's dir */

					const char *f = file->true_filename;
					const char *b = tcc_basename(f);
					if (b > f)
						cstr_cat(&str, f, b - f - 1);
					else
						cstr_cat(&str, ".", 1);
				}
			} else {
				cstr_ccat(&str, c);
			}
		}
		if (str.size) {
			cstr_ccat(&str, '\0');
			dynarray_add(p_ary, p_nb_ary, str.data);
		}
		in = p+1;
	} while (*p);
}
/**/
/* warning / error */
/* warn_... option bits */
/* warning is on (-Woption) */

#define WARN_ON 1
/* warning is an error (-Werror=option) */
#define WARN_ERR 2
/* warning is not an error (-Wno-error=option) */
#define WARN_NOE 4
/* error1() mode\s */

enum { ERROR_WARN, ERROR_NOABORT, ERROR_ERROR };

static void error1(int mode, const char *fmt, va_list ap)
{
	BufferedFile **pf, *f;
	TCCState *s1 = tcc_state;
	CString cs;
	int line = 0;

	tcc_exit_state(s1);

	if (mode == ERROR_WARN) {
		if (s1->warn_error)
			mode = ERROR_ERROR;
		if (s1->warn_num) {
			/* handle tcc_warning_c(warn_option)(fmt, ...) */

			int wopt = *(&s1->warn_none + s1->warn_num);
			s1->warn_num = 0;
			if (0 == (wopt & WARN_ON))
				return;
			if (wopt & WARN_ERR)
				mode = ERROR_ERROR;
			if (wopt & WARN_NOE)
				mode = ERROR_WARN;
		}
		if (s1->warn_none)
			return;
	}

	cstr_new(&cs);
	if (fmt[0] == '%' && fmt[1] == 'i' && fmt[2] == ':')
		line = va_arg(ap, int), fmt += 3;
	f = NULL;
	if (s1->error_set_jmp_enabled) {/* we're called while parsing a file */

		/* use upper file if inline ":asm:" or token ":paste:" */

		for (f = file; f && f->filename[0] == ':'; f = f->prev)
			;
	}
	if (f) {
		for (pf = s1->include_stack; pf < s1->include_stack_ptr; pf++)
			cstr_printf(&cs, "In file included from %s:%d:\n",
				    (*pf)->filename, (*pf)->line_num - 1);
		if (0 == line)
			line = f->line_num - ((tok_flags & TOK_FLAG_BOL) && !macro_ptr);
		cstr_printf(&cs, "%s:%d: ", f->filename, line);
	} else if (s1->current_filename) {
		cstr_printf(&cs, "%s: ", s1->current_filename);
	} else {
		cstr_printf(&cs, "tcc: ");
	}
	cstr_printf(&cs, mode == ERROR_WARN ? "warning: " : "error: ");
	if (pp_expr > 1)
		pp_error(&cs);/* special handler for preprocessor expression errors */

	else
		cstr_vprintf(&cs, fmt, ap);
	if (!s1->error_func) {
		/* default case: stderr */

		if (s1 && s1->output_type == TCC_OUTPUT_PREPROCESS && s1->ppfp == stdout)
			printf("\n");/* print a newline during tcc -E */

		fflush(stdout);/* flush -v output */

		fprintf(stderr, "%s\n", (char *)cs.data);
		fflush(stderr);/* print error/warning now (win32) */

	} else {
		s1->error_func(s1->error_opaque, (char *)cs.data);
	}
	cstr_free(&cs);
	if (mode != ERROR_WARN)
		s1->nb_errors++;
	if (mode == ERROR_ERROR && s1->error_set_jmp_enabled) {
		while (nb_stk_data)
			tcc_free(*(void **)stk_data[--nb_stk_data]);
		longjmp(s1->error_jmp_buf, 1);
	}
}

LIBTCCAPI void tcc_set_error_func(TCCState *s, void *error_opaque,
				  TCCErrorFunc *error_func)
{
	s->error_opaque = error_opaque;
	s->error_func = error_func;
}
/* error without aborting current compilation */

PUB_FUNC int _tcc_error_noabort(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	error1(ERROR_NOABORT, fmt, ap);
	va_end(ap);
	return -1;
}

#undef _tcc_error
PUB_FUNC void _tcc_error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	error1(ERROR_ERROR, fmt, ap);
	exit(1);
}

PUB_FUNC void _tcc_warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	error1(ERROR_WARN, fmt, ap);
	va_end(ap);
}
/**/
/* I/O layer */

ST_FUNC void tcc_open_bf(TCCState *s1, const char *filename, int initlen)
{
	BufferedFile *bf;
	int buflen = initlen ? initlen : IO_BUF_SIZE;

	bf = tcc_mallocz(sizeof(BufferedFile) + buflen);
	bf->buf_ptr = bf->buffer;
	bf->buf_end = bf->buffer + initlen;
	bf->buf_end[0] = CH_EOB;/* put eob symbol */

	pstrcpy(bf->filename, sizeof(bf->filename), filename);

	normalize_slashes(bf->filename);

	bf->true_filename = bf->filename;
	bf->line_num = 1;
	bf->ifdef_stack_ptr = s1->ifdef_stack_ptr;
	bf->fd = -1;
	bf->prev = file;
	bf->prev_tok_flags = tok_flags;
	file = bf;
	tok_flags = TOK_FLAG_BOL | TOK_FLAG_BOF;
}

ST_FUNC void tcc_close(void)
{
	TCCState *s1 = tcc_state;
	BufferedFile *bf = file;
	if (bf->fd > 0) {
		close(bf->fd);
		total_lines += bf->line_num - 1;
	}
	if (bf->true_filename != bf->filename)
		tcc_free(bf->true_filename);
	file = bf->prev;
	tok_flags = bf->prev_tok_flags;
	tcc_free(bf);
}

static int _tcc_open(TCCState *s1, const char *filename)
{
	int fd;
	if (strcmp(filename, "-") == 0)
		fd = 0, filename = "<stdin>";
	else
		fd = open(filename, O_RDONLY | O_BINARY);
	if ((s1->verbose == 2 && fd >= 0) || s1->verbose == 3)
		printf("%s %*s%s\n", fd < 0 ? "nf":"->",
		       (int)(s1->include_stack_ptr - s1->include_stack), "", filename);
	return fd;
}

ST_FUNC int tcc_open(TCCState *s1, const char *filename)
{
	int fd = _tcc_open(s1, filename);
	if (fd < 0)
		return -1;
	tcc_open_bf(s1, filename, 0);
	file->fd = fd;
	return 0;
}
/* compile the file opened in 'file'. Return non zero if errors. */

static int tcc_compile(TCCState *s1, int filetype, const char *str, int fd)
{
	/* Here we enter the code section where we use the global variables for
	       parsing and code generation (tccpp.c, tccgen.c, <target>-gen.c).
	       Other threads need to wait until we're done.

	       Alternatively we could use thread local storage for those global
	       variables, which may or may not have advantages */

	tcc_enter_state(s1);
	s1->error_set_jmp_enabled = 1;

	if (setjmp(s1->error_jmp_buf) == 0) {

		if (fd == -1) {
			int len = strlen(str);
			tcc_open_bf(s1, "<string>", len);
			memcpy(file->buffer, str, len);
		} else {
			tcc_open_bf(s1, str, 0);
			file->fd = fd;
		}

		preprocess_start(s1, filetype);
		tccgen_init(s1);

		if (s1->output_type == TCC_OUTPUT_PREPROCESS) {
			tcc_preprocess(s1);
		} else {
			tccelf_begin_file(s1);
			if (filetype & (AFF_TYPE_ASM | AFF_TYPE_ASMPP)) {
				tcc_assemble(s1, !!(filetype & AFF_TYPE_ASMPP));
			} else {
				tccgen_compile(s1);
			}
			tccelf_end_file(s1);
		}
	}
	tccgen_finish(s1);
	preprocess_end(s1);
	s1->error_set_jmp_enabled = 0;
	tcc_exit_state(s1);
	return s1->nb_errors != 0 ? -1 : 0;
}

LIBTCCAPI int tcc_compile_string(TCCState *s, const char *str)
{
	return tcc_compile(s, s->filetype, str, -1);
}
/* define a preprocessor symbol. value can be NULL, sym can be "sym=val" */

LIBTCCAPI void tcc_define_symbol(TCCState *s1, const char *sym,
				 const char *value)
{
	const char *eq;
	if (NULL == (eq = strchr(sym, '=')))
		eq = strchr(sym, 0);
	if (NULL == value)
		value = *eq ? eq + 1 : "1";
	cstr_printf(&s1->cmdline_defs, "#define %.*s %s\n", (int)(eq-sym), sym, value);
}
/* undefine a preprocessor symbol */

LIBTCCAPI void tcc_undefine_symbol(TCCState *s1, const char *sym)
{
	cstr_printf(&s1->cmdline_defs, "#undef %s\n", sym);
}

LIBTCCAPI TCCState *tcc_new(void)
{
	TCCState *s;

	s = tcc_mallocz(sizeof(TCCState));

#undef gnu_ext
	s->gnu_ext = 1;
	s->tcc_ext = 1;
	s->nocommon = 1;
	s->dollars_in_identifiers = 1;/*on by default like in gcc/clang*/

	s->cversion = 199901;/* default unless -std=c11 is supplied */

	s->warn_implicit_function_declaration = 1;
	s->warn_discarded_qualifiers = 1;
	s->ms_extensions = 1;
	s->unwind_tables = 1;
	/* enable this if you want symbols with leading underscore on windows: */
	s->ppfp = stdout;
	/* might be used in error() before preprocess_start() */

	s->include_stack_ptr = s->include_stack;

	tcc_set_lib_path(s, CONFIG_TCCDIR);

	return s;
}

LIBTCCAPI void tcc_delete(TCCState *s1)
{
	/* free sections */

	tccelf_delete(s1);
	/* free library paths */

	dynarray_reset(&s1->library_paths, &s1->nb_library_paths);
	dynarray_reset(&s1->crt_paths, &s1->nb_crt_paths);
	/* free include paths */

	dynarray_reset(&s1->include_paths, &s1->nb_include_paths);
	dynarray_reset(&s1->sysinclude_paths, &s1->nb_sysinclude_paths);

	tcc_free(s1->tcc_lib_path);
	tcc_free(s1->soname);
	tcc_free(s1->rpath);
	tcc_free(s1->elfint);
	tcc_free(s1->elf_entryname);
	tcc_free(s1->init_symbol);
	tcc_free(s1->fini_symbol);
	tcc_free(s1->mapfile);
	tcc_free(s1->outfile);
	tcc_free(s1->deps_outfile);

	dynarray_reset(&s1->files, &s1->nb_files);
	dynarray_reset(&s1->target_deps, &s1->nb_target_deps);
	dynarray_reset(&s1->pragma_libs, &s1->nb_pragma_libs);
	dynarray_reset(&s1->argv, &s1->argc);
	dynarray_reset(&s1->link_argv, &s1->link_argc);
	cstr_free(&s1->cmdline_defs);
	cstr_free(&s1->cmdline_incl);
	tcc_free(s1->dState);
	/* free runtime memory */

	tcc_run_free(s1);
	/* free loaded dlls array */

	dynarray_reset(&s1->loaded_dlls, &s1->nb_loaded_dlls);
	tcc_free(s1);

}

LIBTCCAPI int tcc_set_output_type(TCCState *s, int output_type)
{

	s->output_type = output_type;

	if (!s->nostdinc) {
		/* default include paths */
		/* -isystem paths have already been handled */

		tcc_add_sysinclude_path(s, CONFIG_TCC_SYSINCLUDEPATHS);
	}

	if (output_type == TCC_OUTPUT_PREPROCESS) {
		s->do_debug = 0;
		return 0;
	}
	/* add sections */

	tccelf_new(s);

	if (output_type == TCC_OUTPUT_OBJ) {
		/* always elf for objects */

		s->output_format = TCC_OUTPUT_FORMAT_ELF;
		return 0;
	}

	if (!s->nostdlib_paths)
		tcc_add_library_path(s, CONFIG_TCC_LIBPATHS);

	/* allow linking with system dll's directly */

	tcc_add_systemdir(s);

	return s->nb_errors ? -1 : 0;
}

LIBTCCAPI int tcc_add_include_path(TCCState *s, const char *pathname)
{
	tcc_split_path(s, &s->include_paths, &s->nb_include_paths, pathname);
	return 0;
}

LIBTCCAPI int tcc_add_sysinclude_path(TCCState *s, const char *pathname)
{
	tcc_split_path(s, &s->sysinclude_paths, &s->nb_sysinclude_paths, pathname);
	return 0;
}

LIBTCCAPI int tcc_add_library_path(TCCState *s, const char *pathname)
{
	tcc_split_path(s, &s->library_paths, &s->nb_library_paths, pathname);
	return 0;
}

LIBTCCAPI void tcc_set_lib_path(TCCState *s, const char *path)
{
	tcc_set_str(&s->tcc_lib_path, path);
}
/* add/update a 'DLLReference', Just find if level == -1  */

ST_FUNC DLLReference *tcc_add_dllref(TCCState *s1, const char *dllname,
				     int level)
{
	DLLReference *ref = NULL;
	int i;
	for (i = 0; i < s1->nb_loaded_dlls; i++)
		if (0 == strcmp(s1->loaded_dlls[i]->name, dllname)) {
			ref = s1->loaded_dlls[i];
			break;
		}
	if (level == -1)
		return ref;
	if (ref) {
		if (level < ref->level)
			ref->level = level;
		ref->found = 1;
		return ref;
	}
	ref = tcc_mallocz(sizeof(DLLReference) + strlen(dllname));
	strcpy(ref->name, dllname);
	dynarray_add(&s1->loaded_dlls, &s1->nb_loaded_dlls, ref);
	ref->level = level;
	ref->index = s1->nb_loaded_dlls;
	return ref;
}

static int tcc_add_binary(TCCState *s1, int flags, const char *filename, int fd)
{
	ElfW(Ehdr) ehdr;
	int obj_type;
	const char *saved_filename = s1->current_filename;
	int ret = 0;

	s1->current_filename = filename;
	obj_type = tcc_object_type(fd, &ehdr);
	lseek(fd, 0, SEEK_SET);

	switch (obj_type) {

	case AFF_BINTYPE_REL:
		ret = tcc_load_object_file(s1, fd, 0);
		break;

	case AFF_BINTYPE_AR:
		ret = tcc_load_archive(s1, fd, !(flags & AFF_WHOLE_ARCHIVE));
		break;

	default:
		if (pe_load_file(s1, fd, filename))
			ret = FILE_NOT_RECOGNIZED;
		break;
	}

	close(fd);
	s1->current_filename = saved_filename;
	if (ret == FILE_NOT_RECOGNIZED)
		return tcc_error_noabort("%s: unrecognized file type", filename);
	return ret;
}
/* OpenBSD: choose latest from libxxx.so.x.y versions */

static int guess_filetype(const char *filename)
{
	int filetype = 0;
	if (1) {
		/* use a file extension to detect a filetype */

		const char *ext = tcc_fileextension(filename);
		if (ext[0]) {
			ext++;
			if (!strcmp(ext, "S"))
				filetype = AFF_TYPE_ASMPP;
			else if (!strcmp(ext, "s"))
				filetype = AFF_TYPE_ASM;
			else if (!PATHCMP(ext, "c")
				 || !PATHCMP(ext, "h")
				 || !PATHCMP(ext, "i"))
				filetype = AFF_TYPE_C;
			else
				filetype |= AFF_TYPE_BIN;
		} else {
			filetype = AFF_TYPE_C;
		}
	}
	return filetype;
}

ST_FUNC int tcc_add_file_internal(TCCState *s1, const char *filename, int flags)
{
	int fd;

	if (0 == (flags & AFF_TYPE_MASK))
		flags |= guess_filetype(filename);
	/* ignore binary files with -E */

	if (s1->output_type == TCC_OUTPUT_PREPROCESS
	    && (flags & AFF_TYPE_BIN))
		return 0;
	/* open the file */

	fd = _tcc_open(s1, filename);
	if (fd < 0) {
		if (flags & AFF_PRINT_ERROR)
			tcc_error_noabort("file '%s' not found", filename);
		return FILE_NOT_FOUND;
	}

	if (flags & AFF_TYPE_BIN)
		return tcc_add_binary(s1, flags, filename, fd);

	dynarray_add(&s1->target_deps, &s1->nb_target_deps, tcc_strdup(filename));
	return tcc_compile(s1, flags, filename, fd);
}

LIBTCCAPI int tcc_add_file(TCCState *s, const char *filename)
{
	return tcc_add_file_internal(s, filename, s->filetype | AFF_PRINT_ERROR);
}

static int tcc_add_library_internal(TCCState *s1, const char *fmt,
				    const char *filename, int flags, char **paths, int nb_paths)
{
	char buf[1024];
	int i, ret;

	for (i = 0; i < nb_paths; i++) {
		snprintf(buf, sizeof(buf), fmt, paths[i], filename);
		ret = tcc_add_file_internal(s1, buf, flags & ~AFF_PRINT_ERROR);
		if (ret != FILE_NOT_FOUND)
			return ret;
	}
	if (flags & AFF_PRINT_ERROR)
		tcc_error_noabort("%s '%s' not found",
				  flags & AFF_TYPE_LIB ? "library" : "file", filename);
	return FILE_NOT_FOUND;
}
/* find and load a dll. Return non zero if not found */

ST_FUNC int tcc_add_dll(TCCState *s, const char *filename, int flags)
{
	return tcc_add_library_internal(s, "%s/%s", filename, flags,
					s->library_paths, s->nb_library_paths);
}
/* find [cross-]libtcc1.a and tcc helper objects in library path */

ST_FUNC int tcc_add_support(TCCState *s1, const char *filename)
{
	char buf[100];
	if (CONFIG_TCC_CROSSPREFIX[0])
		filename = strcat(strcpy(buf, CONFIG_TCC_CROSSPREFIX), filename);
	return tcc_add_dll(s1, filename, AFF_PRINT_ERROR);
}
/* the library name is the same as the argument of the '-l' option */

LIBTCCAPI int tcc_add_library(TCCState *s, const char *libraryname)
{
	static const char *const libs[] = {
		"%s/%s.def", "%s/lib%s.def", "%s/%s.dll", "%s/lib%s.dll",

		"%s/lib%s.a",
		NULL
	};
	int flags = AFF_TYPE_LIB | (s->filetype & AFF_WHOLE_ARCHIVE);
	/* if libraryname begins with a colon, it means search lib paths for
	       exactly the following file, without lib prefix or anything */

	if (*libraryname == ':') {
		libraryname++;
	} else {
		const char *const *pp = libs;
		if (s->static_link)
			pp += sizeof(libs) / sizeof(*libs) - 2;/* only "%s/lib%s.a" */

		while (*pp) {
			int ret = tcc_add_library_internal(s, *pp,
							   libraryname, flags, s->library_paths, s->nb_library_paths);
			if (ret != FILE_NOT_FOUND)
				return ret;
			++pp;
		}
	}
	/* fallback to try file without pre- or sufffix */

	return tcc_add_dll(s, libraryname, flags | AFF_PRINT_ERROR);
}
/* handle #pragma comment(lib,) */

ST_FUNC void tcc_add_pragma_libs(TCCState *s1)
{
	int i;
	for (i = 0; i < s1->nb_pragma_libs; i++)
		tcc_add_library(s1, s1->pragma_libs[i]);
}
/**/
/* options parser */

static int strstart(const char *val, const char **str)
{
	const char *p, *q;
	p = *str;
	q = val;
	while (*q) {
		if (*p != *q)
			return 0;
		p++;
		q++;
	}
	*str = p;
	return 1;
}

struct lopt {
	TCCState *s;
	const char *opt, *arg;
	int match;
};
/* match linker option */

static int link_option(struct lopt *o, const char *q)
{
	const char *p;
	int c, r;
redo:
	/* there should be 1 or 2 dashes */

	p = o->opt;
	if (*p++ != '-')
		return 0;
	if (*p == '-')
		p++;
	r = 1;
	if (q[0] == '?') {/* check for no-/disable- prefix */

		++q;
		if (p[0] == 'n' && p[1] == 'o' && p[2] == '-')
			p += 3, r = -1;

		else if (0 == memcmp(p, "disable-", 8))
			p += 8, r = -1;

	}
	while ((c = *q) == *p) {
		if (c == '\0')
			goto succ;/* -Wl,-opt */

		++p;
		if (c == '=')
			goto succ;/* -Wl,-opt=arg */

		++q;
	}
	if (*p == '\0') {
		if (c == '|')
			goto succ;
		if (c == '=' || c == ':') {
			if (o->s->link_optind + 1 < o->s->link_argc) {
				p = o->s->link_argv[++o->s->link_optind];
				goto succ;/* -Wl,-opt,arg */

			}
			o->match = 1;/* -Wl,-opt -Wl,arg */

			return 0;
		}
	} else if (c == ':')
		goto succ;/* -Wl,-Iarg */

	while (*q)
		if (*q++ == '|')
			goto redo;
	return 0;
succ:
	o->arg = p;
//printf("set %s '%s'\n", o->opt, o->arg);

	return r;
}

static void args_parser_add_file(TCCState *s, const char *filename,
				 int filetype);

#define SET_OR_CLEAR(v,f) (v = r > 0 ? v | f : v & ~f)
#define SET_OR_CLEAR_2(v,f1,f2) (v = r > 0 ? v | f1 : v & ~f2)
/* set linker options */

static int tcc_set_linker(TCCState *s, const char *optarg)
{
	TCCState *s1 = s;

	dynarray_split(&s1->link_argv, &s1->link_argc, optarg, ',');

	while (s->link_optind < s->link_argc) {
		char *end = NULL;
		int ignoring = 0;
		struct lopt o = {0};
		int r;
		o.s = s;
		o.opt = s->link_argv[s->link_optind];

		if (link_option(&o, "Bsymbolic")) {
			s->symbolic = 1;
		} else if (link_option(&o, "nostdlib")) {
			s->nostdlib_paths = 1;
		} else if (link_option(&o, "e=|entry=")) {
			tcc_set_str(&s->elf_entryname, o.arg);
		} else if (link_option(&o, "image-base=|Ttext=")) {
			s->text_addr = strtoull(o.arg, &end, 16);
			s->has_text_addr = 1;
		} else if (link_option(&o, "init=")) {
			tcc_set_str(&s->init_symbol, o.arg);
			ignoring = 1;
		} else if (link_option(&o, "fini=")) {
			tcc_set_str(&s->fini_symbol, o.arg);
			ignoring = 1;
		} else if (link_option(&o, "Map=")) {
			tcc_set_str(&s->mapfile, o.arg);
			ignoring = 1;
		} else if (link_option(&o, "oformat=")) {
			if (0 == strncmp("pe-", o.arg, 3))

				s->output_format = TCC_OUTPUT_FORMAT_ELF;
			else if (0==strcmp("binary", o.arg))
				s->output_format = TCC_OUTPUT_FORMAT_BINARY;

			else
				goto err;
		} else if (link_option(&o, "export-all-symbols|export-dynamic|E")) {
			s->rdynamic = 1;
		} else if (link_option(&o, "rpath=")) {
			tcc_concat_str(&s->rpath, o.arg, ':');
		} else if (link_option(&o, "dynamic-linker=|I:")) {
			tcc_set_str(&s->elfint, o.arg);
		} else if (link_option(&o, "enable-new-dtags")) {
			s->enable_new_dtags = 1;
		} else if (link_option(&o, "section-alignment=")) {
			s->section_align = strtoul(o.arg, &end, 16);
		} else if (link_option(&o, "soname=|install_name=")) {
			tcc_set_str(&s->soname, o.arg);
		} else if (!!(r = link_option(&o, "?whole-archive"))) {
			SET_OR_CLEAR(s->filetype, AFF_WHOLE_ARCHIVE);
		} else if (link_option(&o, "znodelete")) {
			s->znodelete = 1;

		} else if (link_option(&o, "large-address-aware")) {
			s->pe_characteristics |= 0x20;
		} else if (!!(r = link_option(&o, "?dynamicbase"))) {
			SET_OR_CLEAR_2(s1->pe_dll_characteristics, 0x40, 0x60);
		} else if (!!(r = link_option(&o, "?high-entropy-va"))) {
			SET_OR_CLEAR_2(s1->pe_dll_characteristics, 0x60, 0x20);
		} else if (!!(r = link_option(&o, "?nxcompat"))) {
			SET_OR_CLEAR(s1->pe_dll_characteristics, 0x100);
		} else if (!!(r = link_option(&o, "?tsaware"))) {
			SET_OR_CLEAR(s1->pe_dll_characteristics, 0x8000);
		} else if (link_option(&o, "file-alignment=")) {
			s->pe_file_align = strtoul(o.arg, &end, 16);
		} else if (link_option(&o, "stack=")) {
			s->pe_stack_size = strtoul(o.arg, &end, 10);
		} else if (link_option(&o, "subsystem=")) {
			if (pe_setsubsy(s, o.arg) < 0)
				goto err;
		} else if (link_option(&o, "as-needed")) {
			ignoring = 1;
		} else if (link_option(&o, "O")) {
			ignoring = 1;
		} else if (link_option(&o, "z=")) {
			ignoring = 1;
		} else if (link_option(&o, "L:")) {
			tcc_add_library_path(s, o.arg);
		} else if (link_option(&o, "l:")) {
			args_parser_add_file(s, o.arg, AFF_TYPE_LIB | (s->filetype & ~AFF_TYPE_MASK));
		} else if (o.match) {
			return 0;/* expecting argument with next '-Wl,' */

		} else {
err:
			return tcc_error_noabort("unsupported linker option '%s'", o.opt);
		}
		if (ignoring)
			tcc_warning_c(warn_unsupported)("unsupported linker option '%s'", o.opt);
		++s->link_optind;
	}
	return 0;
}

typedef struct TCCOption {
	const char *name;
	uint16_t index;
	uint16_t flags;
} TCCOption;

enum {
	TCC_OPTION_ignored = 0,
	TCC_OPTION_HELP,
	TCC_OPTION_HELP2,
	TCC_OPTION_v,
	TCC_OPTION_I,
	TCC_OPTION_D,
	TCC_OPTION_U,
	TCC_OPTION_P,
	TCC_OPTION_L,
	TCC_OPTION_B,
	TCC_OPTION_l,
	TCC_OPTION_bench,
	TCC_OPTION_bt,
	TCC_OPTION_b,
	TCC_OPTION_g,
	TCC_OPTION_c,
	TCC_OPTION_dumpmachine,
	TCC_OPTION_dumpversion,
	TCC_OPTION_d,
	TCC_OPTION_static,
	TCC_OPTION_std,
	TCC_OPTION_shared,
	TCC_OPTION_soname,
	TCC_OPTION_o,
	TCC_OPTION_r,
	TCC_OPTION_Wl,
	TCC_OPTION_Wp,
	TCC_OPTION_W,
	TCC_OPTION_O,
	TCC_OPTION_mfloat_abi,
	TCC_OPTION_m,
	TCC_OPTION_f,
	TCC_OPTION_isystem,
	TCC_OPTION_iwithprefix,
	TCC_OPTION_include,
	TCC_OPTION_nostdinc,
	TCC_OPTION_nostdlib,
	TCC_OPTION_print_search_dirs,
	TCC_OPTION_rdynamic,
	TCC_OPTION_pthread,
	TCC_OPTION_run,
	TCC_OPTION_rstdin,
	TCC_OPTION_w,
	TCC_OPTION_E,
	TCC_OPTION_M,
	TCC_OPTION_MD,
	TCC_OPTION_MF,
	TCC_OPTION_MM,
	TCC_OPTION_MMD,
	TCC_OPTION_MP,
	TCC_OPTION_x,
	TCC_OPTION_ar,
	TCC_OPTION_impdef,
	/* macho */

	TCC_OPTION_dynamiclib,
	TCC_OPTION_flat_namespace,
	TCC_OPTION_two_levelnamespace,
	TCC_OPTION_undefined,
	TCC_OPTION_install_name,
	TCC_OPTION_compatibility_version,
	TCC_OPTION_current_version
};

#define TCC_OPTION_HAS_ARG 0x0001
/* cannot have space before option and arg */
#define TCC_OPTION_NOSEP 0x0002

static const TCCOption tcc_options[] = {
	{ "h", TCC_OPTION_HELP, 0 },
	{ "-help", TCC_OPTION_HELP, 0 },
	{ "?", TCC_OPTION_HELP, 0 },
	{ "hh", TCC_OPTION_HELP2, 0 },
	{ "v", TCC_OPTION_v, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "-version", TCC_OPTION_v, 0 },/* handle as verbose, also prints version*/

	{ "I", TCC_OPTION_I, TCC_OPTION_HAS_ARG },
	{ "D", TCC_OPTION_D, TCC_OPTION_HAS_ARG },
	{ "U", TCC_OPTION_U, TCC_OPTION_HAS_ARG },
	{ "P", TCC_OPTION_P, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "L", TCC_OPTION_L, TCC_OPTION_HAS_ARG },
	{ "B", TCC_OPTION_B, TCC_OPTION_HAS_ARG },
	{ "l", TCC_OPTION_l, TCC_OPTION_HAS_ARG },
	{ "bench", TCC_OPTION_bench, 0 },

	{ "g", TCC_OPTION_g, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "c", TCC_OPTION_c, 0 },
	{ "dumpmachine", TCC_OPTION_dumpmachine, 0},
	{ "dumpversion", TCC_OPTION_dumpversion, 0},
	{ "d", TCC_OPTION_d, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "static", TCC_OPTION_static, 0 },
	{ "std", TCC_OPTION_std, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "shared", TCC_OPTION_shared, 0 },
	{ "soname", TCC_OPTION_soname, TCC_OPTION_HAS_ARG },
	{ "o", TCC_OPTION_o, TCC_OPTION_HAS_ARG },
	{ "pthread", TCC_OPTION_pthread, 0},
	{ "run", TCC_OPTION_run, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "rstdin", TCC_OPTION_rstdin, TCC_OPTION_HAS_ARG },
	{ "rdynamic", TCC_OPTION_rdynamic, 0 },
	{ "r", TCC_OPTION_r, 0 },
	{ "Wl,", TCC_OPTION_Wl, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "Wp,", TCC_OPTION_Wp, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "W", TCC_OPTION_W, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "O", TCC_OPTION_O, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },

	{ "m", TCC_OPTION_m, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "f", TCC_OPTION_f, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "isystem", TCC_OPTION_isystem, TCC_OPTION_HAS_ARG },
	{ "include", TCC_OPTION_include, TCC_OPTION_HAS_ARG },
	{ "nostdinc", TCC_OPTION_nostdinc, 0 },
	{ "nostdlib", TCC_OPTION_nostdlib, 0 },
	{ "print-search-dirs", TCC_OPTION_print_search_dirs, 0 },
	{ "w", TCC_OPTION_w, 0 },
	{ "E", TCC_OPTION_E, 0},
	{ "M", TCC_OPTION_M, 0},
	{ "MM", TCC_OPTION_MM, 0},
	{ "MD", TCC_OPTION_MD, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "MMD", TCC_OPTION_MMD, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP },
	{ "MF", TCC_OPTION_MF, TCC_OPTION_HAS_ARG },
	{ "MP", TCC_OPTION_MP, 0},
	{ "x", TCC_OPTION_x, TCC_OPTION_HAS_ARG },
	/* tcctools */

	{ "ar", TCC_OPTION_ar, 0},

	{ "impdef", TCC_OPTION_impdef, 0},
	/* ignored (silently, except after -Wunsupported) */

	{ "arch", 0, TCC_OPTION_HAS_ARG},
	{ "C", 0, 0 },
	{ "-param", 0, TCC_OPTION_HAS_ARG },
	{ "pedantic", 0, 0 },
	{ "pie", 0, 0 },
	{ "no-pie", 0, 0 },
	{ "pipe", 0, 0 },
	{ "s", 0, 0 },
	{ "traditional", 0, 0 },
	{ NULL, 0, 0 }
};

typedef struct FlagDef {
	uint16_t offset;
	uint16_t flags;
	const char *name;
} FlagDef;
/* warning is activated when using -Wall */

#define WD_ALL 0x0001
/* invert value before storing */
#define FD_INVERT 0x0002

static const FlagDef options_W[] = {
	{ offsetof(TCCState, warn_all), WD_ALL, "all" },
	{ offsetof(TCCState, warn_error), 0, "error" },
	{ offsetof(TCCState, warn_write_strings), 0, "write-strings" },
	{ offsetof(TCCState, warn_unsupported), 0, "unsupported" },
	{ offsetof(TCCState, warn_implicit_function_declaration), WD_ALL, "implicit-function-declaration" },
	{ offsetof(TCCState, warn_discarded_qualifiers), WD_ALL, "discarded-qualifiers" },
	{ 0, 0, NULL }
};

static const FlagDef options_f[] = {
	{ offsetof(TCCState, char_is_unsigned), 0, "unsigned-char" },
	{ offsetof(TCCState, char_is_unsigned), FD_INVERT, "signed-char" },
	{ offsetof(TCCState, nocommon), FD_INVERT, "common" },
	{ offsetof(TCCState, leading_underscore), 0, "leading-underscore" },
	{ offsetof(TCCState, ms_extensions), 0, "ms-extensions" },
	{ offsetof(TCCState, dollars_in_identifiers), 0, "dollars-in-identifiers" },
	{ offsetof(TCCState, test_coverage), 0, "test-coverage" },
	{ offsetof(TCCState, reverse_funcargs), 0, "reverse-funcargs" },
	{ offsetof(TCCState, gnu89_inline), 0, "gnu89-inline" },
	{ offsetof(TCCState, unwind_tables), 0, "asynchronous-unwind-tables" },
	{ 0, 0, NULL }
};

static const FlagDef options_m[] = {
	{ offsetof(TCCState, ms_bitfields), 0, "ms-bitfields" },

	{ offsetof(TCCState, nosse), FD_INVERT, "sse" },

	{ 0, 0, NULL }
};

static int set_flag(TCCState *s, const FlagDef *flags, const char *name)
{
	int value, mask, ret;
	const FlagDef *p;
	const char *r;
	unsigned char *f;

	r = name, value = !strstart("no-", &r), mask = 0;
	/* when called with options_W, look for -W[no-]error=<option> */

	if ((flags->flags & WD_ALL) && strstart("error=", &r))
		value = value ? WARN_ON|WARN_ERR : WARN_NOE, mask = WARN_ON;

	for (ret = -1, p = flags; p->name; ++p) {
		if (ret) {
			if (strcmp(r, p->name))
				continue;
		} else {
			if (0 == (p->flags & WD_ALL))
				continue;
		}

		f = (unsigned char *)s + p->offset;
		*f = (*f & mask) | (value ^ !!(p->flags & FD_INVERT));

		if (ret) {
			ret = 0;
			if (strcmp(r, "all"))
				break;
		}
	}
	return ret;
}

static const char dumpmachine_str[] =
	/* this is a best guess, please refine as necessary */

	"x86_64-pc"
	"-"

	"mingw32"

	;
/* insert args from 'p' (separated by sep or ' ') into argv at position 'optind' */
static void insert_args(TCCState *s1, char ***pargv, int *pargc, int optind,
			const char *p, int sep)
{
	int argc = 0;
	char **argv = NULL;
	int i;
	for (i = 0; i < *pargc; ++i)
		if (i == optind)
			dynarray_split(&argv, &argc, p, sep);
		else
			dynarray_add(&argv, &argc, tcc_strdup((*pargv)[i]));
	dynarray_reset(&s1->argv, &s1->argc);
	*pargc = s1->argc = argc;
	*pargv = s1->argv = argv;
}

static void args_parser_add_file(TCCState *s, const char *filename,
				 int filetype)
{
	struct filespec *f = tcc_malloc(sizeof *f + strlen(filename));
	f->type = filetype;
	strcpy(f->name, filename);
	dynarray_add(&s->files, &s->nb_files, f);
	if (filetype & AFF_TYPE_LIB)
		++s->nb_libraries;
}
/* using * to argc/argv to let "tcc -ar" benefit from @listfile expansion */

PUB_FUNC int tcc_parse_args(TCCState *s, int *pargc, char ***pargv)
{
	TCCState *s1 = s;
	const TCCOption *popt;
	const char *optarg, *r;
	const char *run = NULL;
	int optind = 1, empty = 1, x;
	char **argv = *pargv;
	int argc = *pargc;

	s->link_optind = s->link_argc;

	while (optind < argc) {
		r = argv[optind];
		if (r[0] == '@' && r[1] != '\0') {/* read @listfile */

			int fd;
			char *p;
			fd = open(++r, O_RDONLY | O_BINARY);
			if (fd < 0)
				return tcc_error_noabort("listfile '%s' not found", r);
			p = tcc_load_text(fd);
			insert_args(s1, &argv, &argc, optind, p, 0);
			close(fd), tcc_free(p);
			continue;
		}
		optind++;
		if (r[0] != '-' || r[1] == '\0') {/* file or '-' (stdin) */

			args_parser_add_file(s, r, s->filetype);
			empty = 0;
dorun:
			if (run)
				break;
			continue;
		}
		/* Also allow "tcc <files...> -run -- <args...>" */

		if (r[1] == '-' && r[2] == '\0')
			goto dorun;
		/* find option in table */

		for (popt = tcc_options; ; ++popt) {
			const char *p1 = popt->name;
			const char *r1 = r + 1;
			if (p1 == NULL)
				return tcc_error_noabort("invalid option -- '%s'", r);
			if (!strstart(p1, &r1))
				continue;
			optarg = r1;
			if (popt->flags & TCC_OPTION_HAS_ARG) {
				if (*r1 == '\0' && !(popt->flags & TCC_OPTION_NOSEP)) {
					if (optind >= argc)
						return tcc_error_noabort("argument to '%s' is missing", r);
					optarg = argv[optind++];
				}
			} else if (*r1 != '\0')
				continue;
			break;
		}

		switch (popt->index) {
		case TCC_OPTION_I:
			tcc_add_include_path(s, optarg);
			break;
		case TCC_OPTION_D:
			tcc_define_symbol(s, optarg, NULL);
			break;
		case TCC_OPTION_U:
			tcc_undefine_symbol(s, optarg);
			break;
		case TCC_OPTION_L:
			tcc_add_library_path(s, optarg);
			break;
		case TCC_OPTION_B:
			/* set tcc utilities path (mainly for tcc development) */

			tcc_set_lib_path(s, optarg);
			continue;
		case TCC_OPTION_l:
			args_parser_add_file(s, optarg, AFF_TYPE_LIB | (s->filetype & ~AFF_TYPE_MASK));
			break;
		case TCC_OPTION_pthread:
			s->option_pthread = 1;
			break;
		case TCC_OPTION_bench:
			s->do_bench = 1;
			break;

		case TCC_OPTION_g:
			s->do_debug = 2;
			s->dwarf = CONFIG_DWARF_VERSION;
g_redo:
			if (strstart("dwarf", &optarg)) {
				s->dwarf = (*optarg) ? (0 - atoi(optarg)) : DEFAULT_DWARF_VERSION;
			} else if (0 == strcmp("stabs", optarg)) {
				s->dwarf = 0;
			} else if (isnum(*optarg)) {
				x = *optarg++ - '0';
				/* -g0 = no info, -g1 = lines/functions only, -g2 = full info */

				s->do_debug = x > 2 ? 2 : x == 0 && s->do_backtrace ? 1 : x;
				goto g_redo;

			} else if (0 == strcmp(".pdb", optarg)) {
				s->dwarf = 5, s->do_debug |= 16;

			}
			break;
		case TCC_OPTION_c:
			x = TCC_OUTPUT_OBJ;
set_output_type:
			if (s->output_type)
				tcc_warning("-%s: overriding compiler action already specified", popt->name);
			s->output_type = x;
			break;
		case TCC_OPTION_d:
			if (*optarg == 'D')
				s->dflag = 3;
			else if (*optarg == 'M')
				s->dflag = 7;
			else if (*optarg == 't')
				s->dflag = 16;
			else if (isnum(*optarg))
				g_debug |= atoi(optarg);
			else
				goto unsupported_option;
			break;
		case TCC_OPTION_static:
			s->static_link = 1;
			break;
		case TCC_OPTION_std:
			if (strcmp(optarg, "=c11") == 0 || strcmp(optarg, "=gnu11") == 0)
				s->cversion = 201112;
			break;
		case TCC_OPTION_shared:
			x = TCC_OUTPUT_DLL;
			goto set_output_type;
		case TCC_OPTION_soname:
			tcc_set_str(&s->soname, optarg);
			break;
		case TCC_OPTION_o:
			if (s->outfile) {
				tcc_warning("multiple -o option");
			}
			tcc_set_str(&s->outfile, optarg);
			break;
		case TCC_OPTION_r:
			/* generate a .o merging several output files */

			s->option_r = 1;
			x = TCC_OUTPUT_OBJ;
			goto set_output_type;
		case TCC_OPTION_isystem:
			tcc_add_sysinclude_path(s, optarg);
			break;
		case TCC_OPTION_include:
			cstr_printf(&s->cmdline_incl, "#include \"%s\"\n", optarg);
			break;
		case TCC_OPTION_nostdinc:
			s->nostdinc = 1;
			break;
		case TCC_OPTION_nostdlib:
			s->nostdlib = 1;
			break;
		case TCC_OPTION_run:
			/* When from script "#!/usr/bin/tcc -run <options>",
			               argv[1] is "-run <options>" and argv[2] is <script-name> */

			run = optarg;
			x = TCC_OUTPUT_MEMORY;
			goto set_output_type;

		case TCC_OPTION_rstdin:
			/* custom stdin for run_main */

			s->run_stdin = optarg;
			break;

		case TCC_OPTION_v:
			do
				++s->verbose;
			while (*optarg++ == 'v');
			continue;
		case TCC_OPTION_f:
			if (set_flag(s, options_f, optarg) < 0)
				goto unsupported_option;
			break;
		case TCC_OPTION_m:
			if (set_flag(s, options_m, optarg) < 0) {
				if (x = atoi(optarg), x != 32 && x != 64)
					goto unsupported_option;
				if (PTR_SIZE != x/8)
					return x;
				continue;
			}
			break;
		case TCC_OPTION_W:
			if (optarg[0] && set_flag(s, options_W, optarg) < 0)
				goto unsupported_option;
			break;
		case TCC_OPTION_w:
			s->warn_none = 1;
			break;
		case TCC_OPTION_rdynamic:
			s->rdynamic = 1;
			break;
		case TCC_OPTION_Wl:
			if (tcc_set_linker(s, optarg) < 0)
				return -1;
			break;
		case TCC_OPTION_Wp:
			if (argv[0])/* not with tcc_set_options() */

				insert_args(s, &argv, &argc, --optind, optarg, ',');
			break;
		case TCC_OPTION_E:
			x = TCC_OUTPUT_PREPROCESS;
			goto set_output_type;
		case TCC_OPTION_P:
			s->Pflag = atoi(optarg) + 1;
			break;

		case TCC_OPTION_M:
			s->include_sys_deps = 1;
// fall through

		case TCC_OPTION_MM:
			s->just_deps = 1;
			s->gen_deps = 1;
			if (!s->deps_outfile)
				tcc_set_str(&s->deps_outfile, "-");
			break;
		case TCC_OPTION_MD:
			s->include_sys_deps = 1;
// fall through

		case TCC_OPTION_MMD:
			s->gen_deps = 1;
			/* usually, only "-MMD" is used */
			/* but the Linux Kernel uses "-MMD,depfile" */

			if (*optarg != ',')
				break;
			++optarg;
// fall through

		case TCC_OPTION_MF:
			tcc_set_str(&s->deps_outfile, optarg);
			break;
		case TCC_OPTION_MP:
			s->gen_phony_deps = 1;
			break;

		case TCC_OPTION_dumpmachine:
			printf("%s\n", dumpmachine_str);
			exit(0);
		case TCC_OPTION_dumpversion:
			printf ("%s\n", TCC_VERSION);
			exit(0);

		case TCC_OPTION_x:
			x = 0;
			if (*optarg == 'c')
				x = AFF_TYPE_C;
			else if (*optarg == 'a')
				x = AFF_TYPE_ASMPP;
			else if (*optarg == 'b')
				x = AFF_TYPE_BIN;
			else if (*optarg == 'n')
				x = AFF_TYPE_NONE;
			else
				tcc_warning("unsupported language '%s'", optarg);
			s->filetype = x | (s->filetype & ~AFF_TYPE_MASK);
			break;
		case TCC_OPTION_O:
			s->optimize = isnum(optarg[0]) ? optarg[0]-'0' : 1/* -O -Os */
				      ;
			break;
		case TCC_OPTION_HELP:
			x = OPT_HELP;
			goto extra_action;
		case TCC_OPTION_HELP2:
			x = OPT_HELP2;
			goto extra_action;
		case TCC_OPTION_print_search_dirs:
			x = OPT_PRINT_DIRS;
			goto extra_action;
		case TCC_OPTION_impdef:
			x = OPT_IMPDEF;
			goto extra_action;
		case TCC_OPTION_ar:
			x = OPT_AR;
extra_action:
			if (NULL == argv[0])/* from tcc_set_options() */

				return -1;
			if (!empty && x)
				return tcc_error_noabort("cannot parse %s here", r);
			--optind;
			*pargc = argc - optind;
			*pargv = argv + optind;
			return x;
		default:
unsupported_option:
			tcc_warning_c(warn_unsupported)("unsupported option '%s'", r);
			break;
		}
		empty = 0;
	}
	if (s->link_optind < s->link_argc)
		return tcc_error_noabort("argument to '-Wl,%s' is missing",
					 s->link_argv[s->link_optind]);
	if (run) {
		if (*run && tcc_set_options(s, run) < 0)
			return -1;
		x = 0, r = 0;
		goto extra_action;
	}
	if (!empty)
		return 0;
	if (s->verbose == 2)
		return OPT_PRINT_DIRS;
	if (s->verbose)
		return OPT_V;
	return OPT_HELP;
}

LIBTCCAPI int tcc_set_options(TCCState *s, const char *r)
{
	char **argv = NULL;
	int argc = 0, ret;
	dynarray_add(&argv, &argc, 0);
	dynarray_split(&argv, &argc, r, 0);
	ret = tcc_parse_args(s, &argc, &argv);
	dynarray_reset(&argv, &argc);
	return ret;
}

PUB_FUNC void tcc_print_stats(TCCState *s1, unsigned total_time)
{
	if (!total_time)
		total_time = 1;
	fprintf(stderr, "# %d idents, %d lines, %u bytes\n"
		"# %0.3f s, %u lines/s, %0.1f MB/s\n",
		total_idents, total_lines, total_bytes,
		(double)total_time/1000,
		(unsigned)total_lines*1000/total_time,
		(double)total_bytes/1000/total_time);
	fprintf(stderr, "# text %u, data.rw %u, data.ro %u, bss %u bytes\n",
		s1->total_output[0],
		s1->total_output[1],
		s1->total_output[2],
		s1->total_output[3]
	       );
}

/* ==================== tcctools.c ==================== */
/* -------------------------------------------------------------- */
/*
 *  TCC - Tiny C Compiler
 *
 *  tcctools.c - extra tools and and -m32/64 support
 *
 */
/* -------------------------------------------------------------- */

/* _TCC_H */
#undef TCC_STATE_VAR
#undef TCC_SET_STATE
#define TCC_SET_STATE(fn) (tcc_enter_state(s1),fn)
//#define ARMAG  "!<arch>\n"

#define ARFMAG "`\n"

typedef struct {
	char ar_name[16];
	char ar_date[12];
	char ar_uid[6];
	char ar_gid[6];
	char ar_mode[8];
	char ar_size[10];
	char ar_fmag[2];
} ArHdr;

static unsigned long le2belong(unsigned long ul)
{
	return ((ul & 0xFF0000)>>8)+((ul & 0xFF000000)>>24) +
	       ((ul & 0xFF)<<24)+((ul & 0xFF00)<<8);
}

static int ar_usage(int ret)
{
	fprintf(stderr, "usage: tcc -ar [crstvx] lib [files]\n");
	fprintf(stderr, "create library ([abdiopN] not supported).\n");
	return ret;
}

ST_FUNC int tcc_tool_ar(int argc, char **argv)
{
	static const ArHdr arhdr_init = {
		"/               ",
		"0           ",
		"0     ",
		"0     ",
		"0       ",
		"0         ",
		ARFMAG
	};

	ArHdr arhdr = arhdr_init;
	ArHdr arhdro = arhdr_init;

	FILE *fi, *fh = NULL, *fo = NULL;
	const char *created_file = NULL;// must delete on error

	ElfW(Ehdr) *ehdr;
	ElfW(Shdr) *shdr;
	ElfW(Sym) *sym;
	int i, fsize, i_lib, i_obj;
	char *buf, *shstr, *symtab, *strtab;
	int symtabsize = 0;//, strtabsize = 0;

	char *anames = NULL;
	int *afpos = NULL;
	int istrlen, strpos = 0, fpos = 0, funccnt = 0, funcmax, hofs;
	char tfile[260], stmp[20];
	char *file, *name;
	int ret = 2;
	const char *ops_conflict = "habdiopN";// unsupported but destructive if ignored.

	int extract = 0;
	int table = 0;
	int verbose = 0;

	i_lib = 0;
	i_obj = 0;// will hold the index of the lib and first obj

	for (i = 1; i < argc; i++) {
		const char *a = argv[i];
		if (*a == '-' && strchr(a, '.'))
			ret = 1;// -x.y is always invalid (same as gnu ar)

		if ((*a == '-') || (i == 1 && !strchr(a, '.'))) {// options argument

			if (strpbrk(a, ops_conflict))
				ret = 1;
			if (strchr(a, 'x'))
				extract = 1;
			if (strchr(a, 't'))
				table = 1;
			if (strchr(a, 'v'))
				verbose = 1;
		} else {// lib or obj files: don't abort - keep validating all args.

			if (!i_lib)// first file is the lib

				i_lib = i;
			else if (!i_obj)// second file is the first obj

				i_obj = i;
		}
	}

	if (!i_lib)// i_obj implies also i_lib.

		ret = 1;
	i_obj = i_obj ? i_obj :
		argc;// An empty archive will be generated if no input file is given

	if (ret == 1)
		return ar_usage(ret);

	if (extract || table) {
		if ((fh = fopen(argv[i_lib], "rb")) == NULL) {
			fprintf(stderr, "tcc: ar: can't open file %s\n", argv[i_lib]);
			goto finish;
		}
		fread(stmp, 1, 8, fh);
		if (memcmp(stmp,ARMAG,8)) {
no_ar:
			fprintf(stderr, "tcc: ar: not an ar archive %s\n", argv[i_lib]);
			goto finish;
		}
		while (fread(&arhdr, 1, sizeof(arhdr), fh) == sizeof(arhdr)) {
			char *p, *e;

			if (memcmp(arhdr.ar_fmag, ARFMAG, 2))
				goto no_ar;
			p = arhdr.ar_name;
			for (e = p + sizeof arhdr.ar_name; e > p && e[-1] == ' ';)
				e--;
			*e = '\0';
			arhdr.ar_size[sizeof arhdr.ar_size-1] = 0;
			fsize = atoi(arhdr.ar_size);
			buf = tcc_malloc(fsize + 1);
			fread(buf, fsize, 1, fh);
			if (strcmp(arhdr.ar_name,"/") && strcmp(arhdr.ar_name,"/SYM64/")) {
				if (e > p && e[-1] == '/')
					e[-1] = '\0';
				/* tv not implemented */

				if (table || verbose)
					printf("%s%s\n", extract ? "x - " : "", arhdr.ar_name);
				if (extract) {
					if ((fo = fopen(arhdr.ar_name, "wb")) == NULL) {
						fprintf(stderr, "tcc: ar: can't create file %s\n",
							arhdr.ar_name);
						tcc_free(buf);
						goto finish;
					}
					fwrite(buf, fsize, 1, fo);
					fclose(fo);
					/* ignore date/uid/gid/mode */

				}
			}
			if (fsize & 1)
				fgetc(fh);
			tcc_free(buf);
		}
		ret = 0;
finish:
		if (fh)
			fclose(fh);
		return ret;
	}

	if ((fh = fopen(argv[i_lib], "wb")) == NULL) {
		fprintf(stderr, "tcc: ar: can't create file %s\n", argv[i_lib]);
		goto the_end;
	}
	created_file = argv[i_lib];

	sprintf(tfile, "%s.tmp", argv[i_lib]);
	if ((fo = fopen(tfile, "wb+")) == NULL) {
		fprintf(stderr, "tcc: ar: can't create temporary file %s\n", tfile);
		goto the_end;
	}

	funcmax = 250;
	afpos = tcc_realloc(NULL, funcmax * sizeof *afpos);// 250 func

	memcpy(&arhdro.ar_mode, "100644", 6);
// i_obj = first input object file

	while (i_obj < argc) {
		if (*argv[i_obj] == '-') {// by now, all options start with '-'

			i_obj++;
			continue;
		}
		if ((fi = fopen(argv[i_obj], "rb")) == NULL) {
			fprintf(stderr, "tcc: ar: can't open file %s \n", argv[i_obj]);
			goto the_end;
		}
		if (verbose)
			printf("a - %s\n", argv[i_obj]);

		fseek(fi, 0, SEEK_END);
		fsize = ftell(fi);
		fseek(fi, 0, SEEK_SET);
		buf = tcc_malloc(fsize + 1);
		fread(buf, fsize, 1, fi);
		fclose(fi);
// elf header

		ehdr = (ElfW(Ehdr) *)buf;
		if (ehdr->e_ident[4] != ELFCLASSW) {
			fprintf(stderr, "tcc: ar: Unsupported Elf Class: %s\n", argv[i_obj]);
			goto the_end;
		}

		shdr = (ElfW(Shdr) *) (buf + ehdr->e_shoff + ehdr->e_shstrndx *
				       ehdr->e_shentsize);
		shstr = (char *)(buf + shdr->sh_offset);
		symtab = strtab = NULL;
		for (i = 0; i < ehdr->e_shnum; i++) {
			shdr = (ElfW(Shdr) *) (buf + ehdr->e_shoff + i * ehdr->e_shentsize);
			if (!shdr->sh_offset)
				continue;
			if (shdr->sh_type == SHT_SYMTAB) {
				symtab = (char *)(buf + shdr->sh_offset);
				symtabsize = shdr->sh_size;
			}
			if (shdr->sh_type == SHT_STRTAB) {
				if (!strcmp(shstr + shdr->sh_name, ".strtab")) {
					strtab = (char *)(buf + shdr->sh_offset);
//strtabsize = shdr->sh_size;

				}
			}
		}

		if (symtab && strtab) {
			int nsym = symtabsize / sizeof(ElfW(Sym));
//printf("symtab: info size shndx name\n");

			for (i = 1; i < nsym; i++) {
				sym = (ElfW(Sym) *) (symtab + i * sizeof(ElfW(Sym)));
				if (sym->st_shndx &&
				    (sym->st_info == 0x10
				     || sym->st_info == 0x11
				     || sym->st_info == 0x12
				     || sym->st_info == 0x20
				     || sym->st_info == 0x21
				     || sym->st_info == 0x22
				    )) {
//printf("symtab: %2Xh %4Xh %2Xh %s\n", sym->st_info, sym->st_size, sym->st_shndx, strtab + sym->st_name);

					istrlen = strlen(strtab + sym->st_name)+1;
					anames = tcc_realloc(anames, strpos+istrlen);
					strcpy(anames + strpos, strtab + sym->st_name);
					strpos += istrlen;
					if (++funccnt >= funcmax) {
						funcmax += 250;
						afpos = tcc_realloc(afpos, funcmax * sizeof *afpos);// 250 func more

					}
					afpos[funccnt] = fpos;
				}
			}
		}

		file = argv[i_obj];
		for (name = strchr(file, 0);
		     name > file && name[-1] != '/' && name[-1] != '\\';
		     --name);
		istrlen = strlen(name);
		if (istrlen >= sizeof(arhdro.ar_name))
			istrlen = sizeof(arhdro.ar_name) - 1;
		memset(arhdro.ar_name, ' ', sizeof(arhdro.ar_name));
		memcpy(arhdro.ar_name, name, istrlen);
		arhdro.ar_name[istrlen] = '/';
		sprintf(stmp, "%-10d", fsize);
		memcpy(&arhdro.ar_size, stmp, 10);
		fwrite(&arhdro, sizeof(arhdro), 1, fo);
		fwrite(buf, fsize, 1, fo);
		tcc_free(buf);
		i_obj++;
		fpos += (fsize + sizeof(arhdro));
		if (fpos & 1)
			fputc(0, fo), ++fpos;
	}
	hofs = 8 + sizeof(arhdr) + strpos + (funccnt+1) * sizeof(int);
	fpos = 0;
	if ((hofs & 1))// align

		hofs++, fpos = 1;
// write header

	fwrite(ARMAG, 8, 1, fh);
// create an empty archive

	if (!funccnt) {
		ret = 0;
		goto the_end;
	}
	sprintf(stmp, "%-10d", (int)(strpos + (funccnt+1) * sizeof(int)) + fpos);
	memcpy(&arhdr.ar_size, stmp, 10);
	fwrite(&arhdr, sizeof(arhdr), 1, fh);
	afpos[0] = le2belong(funccnt);
	for (i=1; i<=funccnt; i++)
		afpos[i] = le2belong(afpos[i] + hofs);
	fwrite(afpos, (funccnt+1) * sizeof(int), 1, fh);
	fwrite(anames, strpos, 1, fh);
	if (fpos)
		fwrite("", 1, 1, fh);
// write objects

	fseek(fo, 0, SEEK_END);
	fsize = ftell(fo);
	fseek(fo, 0, SEEK_SET);
	buf = tcc_malloc(fsize + 1);
	fread(buf, fsize, 1, fo);
	fwrite(buf, fsize, 1, fh);
	tcc_free(buf);
	ret = 0;
the_end:
	if (anames)
		tcc_free(anames);
	if (afpos)
		tcc_free(afpos);
	if (fh)
		fclose(fh);
	if (created_file && ret != 0)
		remove(created_file);
	if (fo)
		fclose(fo), remove(tfile);
	return ret;
}
/* -------------------------------------------------------------- */

ST_FUNC int tcc_tool_impdef(int argc, char **argv)
{
	int ret, v, i;
	char infile[260];
	char outfile[260];

	const char *file;
	char *p, *q;
	FILE *fp, *op;

	char path[260];

	infile[0] = outfile[0] = 0;
	fp = op = NULL;
	ret = 1;
	p = NULL;
	v = 0;

	for (i = 1; i < argc; ++i) {
		const char *a = argv[i];
		if ('-' == a[0]) {
			if (0 == strcmp(a, "-v")) {
				v = 1;
			} else if (0 == strcmp(a, "-o")) {
				if (++i == argc)
					goto usage;
				strcpy(outfile, argv[i]);
			} else
				goto usage;
		} else if (0 == infile[0])
			strcpy(infile, a);
		else
			goto usage;
	}

	if (0 == infile[0]) {
usage:
		fprintf(stderr,
			"usage: tcc -impdef library.dll [-v] [-o outputfile]\n"
			"create export definition file (.def) from dll\n"
		       );
		goto the_end;
	}

	if (0 == outfile[0]) {
		strcpy(outfile, tcc_basename(infile));
		q = strrchr(outfile, '.');
		if (NULL == q)
			q = strchr(outfile, 0);
		strcpy(q, ".def");
	}

	file = infile;
	if (SearchPath(NULL, file, ".dll", sizeof path, path, NULL))
		file = path;
	ret = tcc_get_dllexports(file, &p);
	if (ret || !p) {
		fprintf(stderr, "tcc: impdef: %s '%s'\n",
			ret == -1 ? "can't find file" :
			ret == 1 ? "can't read symbols" :
			ret == 0 ? "no symbols found in" :
			"unknown file type", file);
		ret = 1;
		goto the_end;
	}

	if (v)
		printf("-> %s\n", file);

	op = fopen(outfile, "wb");
	if (NULL == op) {
		fprintf(stderr, "tcc: impdef: could not create output file: %s\n", outfile);
		goto the_end;
	}

	fprintf(op, "LIBRARY %s\n\nEXPORTS\n", tcc_basename(file));
	for (q = p, i = 0; *q; ++i) {
		fprintf(op, "%s\n", q);
		q += strlen(q) + 1;
	}

	if (v)
		printf("<- %s (%d symbol%s)\n", outfile, i, &"s"[i<2]);

	ret = 0;

the_end:
	if (p)
		tcc_free(p);
	if (fp)
		fclose(fp);
	if (op)
		fclose(op);
	return ret;
}

/* -------------------------------------------------------------- */

/* re-execute the i386/x86_64 cross-compilers with tcc -m32/-m64: */

#include <process.h>
/* - Empty argument or with space/tab (not newline) requires quoting.
 * - Double-quotes at the value require '\'-escape, regardless of quoting.
 * - Consecutive (or 1) backslashes at the value all need '\'-escape only if
 *   followed by [escaped] double quote, else taken literally, e.g. <x\\y\>
 *   remains literal without quoting or esc, but <x\\"y\> becomes <x\\\\\"y\>.
 * - This "before double quote" rule applies also before delimiting quoting,
 *   e.g. <x\y \"z\> becomes <"x\y \\\"z\\"> (quoting required because space).
 *
 * https://learn.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments
 */
static char *quote_win32(const char *s)
{
	char *o, *r = tcc_malloc(2 * strlen(s) + 3);/* max-esc, quotes, \0 */

	int cbs = 0, quoted = !*s;/* consecutive backslashes before current */

	for (o = r; *s; *o++ = *s++) {
		quoted |= *s == ' ' || *s == '\t';
		if (*s == '\\' || *s == '"')
			*o++ = '\\';
		else
			o -= cbs;/* undo cbs escapes, if any (not followed by DQ) */

		cbs = *s == '\\' ? cbs + 1 : 0;
	}
	if (quoted) {
		memmove(r + 1, r, o++ - r);
		*r = *o++ = '"';
	} else {
		o -= cbs;
	}

	*o = 0;
	return r;/* don't bother with realloc(r, o-r+1) */

}

static int execvp_win32(const char *prog, char **argv)
{
	int ret;
	char **p;
	/* replace all " by \" */

	for (p = argv; *p; ++p)
		*p = quote_win32(*p);
	ret = _spawnvp(P_NOWAIT, prog, (const char *const *)argv);
	if (-1 == ret)
		return ret;
	_cwait(&ret, ret, WAIT_CHILD);
	exit(ret);
}
#define execvp execvp_win32

ST_FUNC int tcc_tool_cross(char **argv, int target)
{
	char program[4096];
	char *a0 = argv[0];
	int prefix = tcc_basename(a0) - a0;

	snprintf(program, sizeof program,
		 "%.*s%s"

		 "-win32"

		 "-tcc"

		 ".exe"

		 , prefix, a0, target == 64 ? "x86_64" : "i386");

	if (strcmp(a0, program))
		execvp(argv[0] = program, argv);
	fprintf(stderr, "tcc: could not run '%s'\n", program);
	return 1;
}
/* TCC_TARGET_I386 && TCC_TARGET_X86_64 */
/* -------------------------------------------------------------- */
/* enable commandline wildcard expansion (tcc -o x.exe *.c) */

const int _CRT_glob = 1;
#ifndef _CRT_glob
const int _dowildcard = 1;
#endif
/* -------------------------------------------------------------- */
/* generate xxx.d file */
static char *escape_target_dep(const char *s)
{
	char *res = tcc_malloc(strlen(s) * 2 + 1);
	int j;
	for (j = 0; *s; s++, j++) {
		if (is_space(*s)) {
			res[j++] = '\\';
		}
		res[j] = *s;
	}
	res[j] = '\0';
	return res;
}

ST_FUNC int gen_makedeps(TCCState *s1, const char *target, const char *filename)
{
	FILE *depout;
	char buf[1024];
	char **escaped_targets;
	int i, k, num_targets;

	if (!filename) {
		/* compute filename automatically: dir/file.o -> dir/file.d */

		snprintf(buf, sizeof buf, "%.*s.d",
			 (int)(tcc_fileextension(target) - target), target);
		filename = buf;
	}

	if (!strcmp(filename, "-"))
		depout = fdopen(1, "w");
	else
		/* XXX return err codes instead of error() ? */

		depout = fopen(filename, "w");
	if (!depout)
		return tcc_error_noabort("could not open '%s'", filename);
	if (s1->verbose)
		printf("<- %s\n", filename);

	escaped_targets = tcc_malloc(s1->nb_target_deps * sizeof(*escaped_targets));
	num_targets = 0;
	for (i = 0; i<s1->nb_target_deps; ++i) {
		for (k = 0; k < i; ++k)
			if (0 == strcmp(s1->target_deps[i], s1->target_deps[k]))
				goto next;
		escaped_targets[num_targets++] = escape_target_dep(s1->target_deps[i]);
next:;
	}

	fprintf(depout, "%s:", target);
	for (i = 0; i < num_targets; ++i)
		fprintf(depout, " \\\n  %s", escaped_targets[i]);
	fprintf(depout, "\n");
	if (s1->gen_phony_deps) {
		/* Skip first file, which is the c file.
		         * Only works for single file give on command-line,
		         * but other compilers have the same limitation */

		for (i = 1; i < num_targets; ++i)
			fprintf(depout, "%s:\n", escaped_targets[i]);
	}
	for (i = 0; i < num_targets; ++i)
		tcc_free(escaped_targets[i]);
	tcc_free(escaped_targets);
	fclose(depout);
	return 0;
}
/* -------------------------------------------------------------- */

static const char help[] =
	"Tiny C Compiler "TCC_VERSION" - Copyright (C) 2001-2006 Fabrice Bellard\n"
	"Usage: tcc [options...] [-o outfile] [-c] infile(s)...\n"
	"       tcc [options...] -run infile (or --) [arguments...]\n"
	"General options:\n"
	"  -c           compile only - generate an object file\n"
	"  -o outfile   set output filename\n"
	"  -run         run compiled source\n"
	"  -fflag       set or reset (with 'no-' prefix) 'flag' (see tcc -hh)\n"
	"  -Wwarning    set or reset (with 'no-' prefix) 'warning' (see tcc -hh)\n"
	"  -w           disable all warnings\n"
	"  -v --version show version\n"
	"  -vv          show search paths or loaded files\n"
	"  -h -hh       show this, show more help\n"
	"  -bench       show compilation statistics\n"
	"  -            use stdin pipe as infile\n"
	"  @listfile    read arguments from listfile\n"
	"Preprocessor options:\n"
	"  -Idir        add include path 'dir'\n"
	"  -Dsym[=val]  define 'sym' with value 'val'\n"
	"  -Usym        undefine 'sym'\n"
	"  -E           preprocess only\n"
	"  -nostdinc    do not use standard system include paths\n"
	"Linker options:\n"
	"  -Ldir        add library path 'dir'\n"
	"  -llib        link with dynamic or static library 'lib'\n"
	"  -nostdlib    do not link with standard crt and libraries\n"
	"  -r           generate (relocatable) object file\n"
	"  -rdynamic    export all global symbols to dynamic linker\n"
	"  -shared      generate a shared library/dll\n"
	"  -soname      set name for shared library to be used at runtime\n"
	"  -Wl,-opt[=val]  set linker option (see tcc -hh)\n"
	"Debugger options:\n"
	"  -g           generate stab runtime debug info\n"
	"  -gdwarf[-x]  generate dwarf runtime debug info\n"

	"  -g.pdb       create .pdb debug database\n"

	"Misc. options:\n"
	"  -std=version define __STDC_VERSION__ according to version (c11/gnu11)\n"
	"  -x[c|a|b|n]  specify type of the next infile (C,ASM,BIN,NONE)\n"
	"  -Bdir        set tcc's private include/library dir\n"
	"  -M[M]D       generate make dependency file [ignore system files]\n"
	"  -M[M]        as above but no other output\n"
	"  -MF file     specify dependency file name\n"

	"  -m32/64      defer to i386/x86_64 cross compiler\n"

	"Tools:\n"
	"  create library  : tcc -ar [crstvx] lib [files]\n"

	"  create def file : tcc -impdef lib.dll [-v] [-o lib.def]\n"

	"Discussion & bug reports:\n"
	"  https://lists.nongnu.org/mailman/listinfo/tinycc-devel\n"
	;

static const char help2[] =
	"Tiny C Compiler "TCC_VERSION" - More Options\n"
	"Special options:\n"
	"  -P -P1                        with -E: no/alternative #line output\n"
	"  -dD -dM                       with -E: output #define directives\n"
	"  -pthread                      same as -D_REENTRANT and -lpthread\n"
	"  -On                           same as -D__OPTIMIZE__ for n > 0\n"
	"  -Wp,-opt                      same as -opt\n"
	"  -include file                 include 'file' above each input file\n"
	"  -nostdlib                     do not link with standard crt/libs\n"
	"  -isystem dir                  add 'dir' to system include path\n"
	"  -static                       link to static libraries (not recommended)\n"
	"  -dumpversion                  print version\n"
	"  -print-search-dirs            print search paths\n"
	"  -rstdin file                  with -run: use 'file' as custom stdin\n"
	"  -dt                           with -run/-E: auto-define 'test_...' macros\n"
	"Ignored options:\n"
	"  -arch -C --param -pedantic -pipe -s -traditional\n"
	"-W[no-]... warnings:\n"
	"  all                           turn on some (*) warnings\n"
	"  error[=warning]               stop after warning (any or specified)\n"
	"  write-strings                 strings are const\n"
	"  unsupported                   warn about ignored options, pragmas, etc.\n"
	"  implicit-function-declaration warn for missing prototype (*)\n"
	"  discarded-qualifiers          warn when const is dropped (*)\n"
	"-f[no-]... flags:\n"
	"  unsigned-char                 default char is unsigned\n"
	"  signed-char                   default char is signed\n"
	"  common                        use common section instead of bss\n"
	"  leading-underscore            decorate extern symbols\n"
	"  ms-extensions                 allow anonymous struct in struct\n"
	"  dollars-in-identifiers        allow '$' in C symbols\n"
	"  reverse-funcargs              evaluate function arguments right to left\n"
	"  gnu89-inline                  'extern inline' is like 'static inline'\n"
	"  asynchronous-unwind-tables    create eh_frame section [on]\n"
	"  test-coverage                 create code coverage code\n"
	"-m... target specific options:\n"
	"  ms-bitfields                  use MSVC bitfield layout\n"

	"  no-sse                        disable floats on x86_64\n"

	"-Wl,... linker options:\n"
	"  -nostdlib                     do not search standard library paths\n"
	"  -[no-]whole-archive           load lib(s) fully/only as needed\n"
	"  -export-all-symbols           same as -rdynamic\n"
	"  -export-dynamic               same as -rdynamic\n"
	"  -image-base= -Ttext=          set base address of executable\n"
	"  -section-alignment=           set section alignment in executable\n"

	"  -file-alignment=              set PE file alignment\n"
	"  -stack=                       set PE stack reserve\n"
	"  -large-address-aware          set related PE option\n"
	"  -subsystem=[console/windows]  set PE subsystem\n"
	"  -oformat=[pe-* binary]        set executable output format\n"
	"Predefined macros:\n"
	"  tcc -E -dM - < nul\n"

	"See also the manual for more details.\n"
	;

static const char version[] =
	"tcc version "TCC_VERSION

	" ("

	"x86_64"

	" Windows"

	")\n"
	;

static void print_dirs(const char *msg, char **paths, int nb_paths)
{
	int i;
	printf("%s:\n%s", msg, nb_paths ? "" : "  -\n");
	for (i = 0; i < nb_paths; i++)
		printf("  %s\n", paths[i]);
}

static void print_search_dirs(TCCState *s)
{
	printf("install: %s\n", s->tcc_lib_path);
	/* print_dirs("programs", NULL, 0); */

	print_dirs("include", s->sysinclude_paths, s->nb_sysinclude_paths);
	print_dirs("libraries", s->library_paths, s->nb_library_paths);
	printf("libtcc1:\n  %s/%s\n", s->library_paths[0],
	       CONFIG_TCC_CROSSPREFIX TCC_LIBTCC1);

}

static void set_environment(TCCState *s)
{
	char *path;

	path = getenv("C_INCLUDE_PATH");
	if (path != NULL) {
		tcc_add_sysinclude_path(s, path);
	}
	path = getenv("CPATH");
	if (path != NULL) {
		tcc_add_include_path(s, path);
	}
	path = getenv("LIBRARY_PATH");
	if (path != NULL) {
		tcc_add_library_path(s, path);
	}
}

static char *default_outputfile(TCCState *s, const char *first_file)
{
	char buf[1024];
	char *ext;
	const char *name = "a";

	if (first_file && strcmp(first_file, "-"))
		name = tcc_basename(first_file);
	if (strlen(name) + 4 >= sizeof buf)
		name = "a";
	strcpy(buf, name);
	ext = tcc_fileextension(buf);

	if (s->output_type == TCC_OUTPUT_DLL)
		strcpy(ext, ".dll");
	else if (s->output_type == TCC_OUTPUT_EXE)
		strcpy(ext, ".exe");
	else if ((s->just_deps || s->output_type == TCC_OUTPUT_OBJ) && !s->option_r
		 && *ext)
		strcpy(ext, ".o");
	else
		strcpy(buf, "a.out");
	return tcc_strdup(buf);
}

static unsigned getclock_ms(void)
{

	return GetTickCount();

}

int main(int argc, char **argv)
{
	TCCState *s, *s1;
	int ret, opt, n = 0, t = 0, done;
	unsigned start_time = 0, end_time = 0;
	const char *first_file;
	int argc0 = argc;
	char **argv0 = argv;
	FILE *ppfp = NULL;

redo:
	argc = argc0, argv = argv0;
	s = s1 = tcc_new();
	opt = tcc_parse_args(s, &argc, &argv);

	if (n == 0) {
		ret = 0;
		if (opt == OPT_HELP) {
			fputs(help, stdout);
			if (s->verbose)
				goto help2;
		} else if (opt == OPT_HELP2) {
help2: fputs(help2, stdout);
		} else if (opt == OPT_M32 || opt == OPT_M64) {
			ret = tcc_tool_cross(argv, opt);
		} else if (s->verbose)
			printf("%s", version);

		if (opt == OPT_AR)
			ret = tcc_tool_ar(argc, argv);

		if (opt == OPT_IMPDEF)
			ret = tcc_tool_impdef(argc, argv);

		if (opt == OPT_PRINT_DIRS) {
			/* initialize search dirs */

			set_environment(s);
			tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
			print_search_dirs(s);
		}
		if (opt) {
			if (opt < 0)
err:
				ret = 1;
			tcc_delete(s);
			return ret;
		}
		if (s->nb_files == 0) {
			tcc_error_noabort("no input files");
		} else if (s->output_type == TCC_OUTPUT_PREPROCESS) {
			if (s->outfile && 0!=strcmp("-",s->outfile)) {
				ppfp = tcc_fopen(s->outfile, "wb");
				if (!ppfp)
					tcc_error_noabort("could not write '%s'", s->outfile);
			}
		} else if (s->output_type == TCC_OUTPUT_OBJ && !s->option_r) {
			if (s->nb_libraries)
				tcc_error_noabort("cannot specify libraries with -c");
			else if (s->nb_files > 1 && s->outfile)
				tcc_error_noabort("cannot specify output file with -c many files");
		}
		if (s->nb_errors)
			goto err;
		if (s->do_bench)
			start_time = getclock_ms();
	}

	set_environment(s);
	if (s->output_type == 0)
		s->output_type = TCC_OUTPUT_EXE;
	ret = tcc_set_output_type(s, s->output_type);
	if (ppfp)
		s->ppfp = ppfp;

	if ((s->output_type == TCC_OUTPUT_MEMORY
	     || s->output_type == TCC_OUTPUT_PREPROCESS)
	    && (s->dflag & 16)) {/* -dt option */

		if (t)
			s->dflag |= 32;
		s->run_test = ++t;
		if (n)
			--n;
	}
	/* compile or add each files or library */

	first_file = NULL;
	while (0 == ret) {
		struct filespec *f = s->files[n];
		s->filetype = f->type;
		if (f->type & AFF_TYPE_LIB) {
			ret = tcc_add_library(s, f->name);
		} else {
			if (1 == s->verbose)
				printf("-> %s\n", f->name);
			if (!first_file)
				first_file = f->name;
			ret = tcc_add_file(s, f->name);
		}
		if (++n == s->nb_files)
			break;
		if (s->output_type == TCC_OUTPUT_OBJ && !s->option_r)
			break;
	}

	if (s->do_bench)
		end_time = getclock_ms();

	if (s->run_test) {
		t = 0;
	} else if (s->output_type == TCC_OUTPUT_PREPROCESS) {
		;
	} else if (0 == ret) {
		if (s->output_type == TCC_OUTPUT_MEMORY) {

			ret = tcc_run(s, argc, argv);

		} else {
			if (!s->outfile)
				s->outfile = default_outputfile(s, first_file);
			if (!s->just_deps)
				ret = tcc_output_file(s, s->outfile);
			if (!ret && s->gen_deps)
				gen_makedeps(s, s->outfile, s->deps_outfile);
		}
	}

	done = 1;
	if (t)
		done = 0;/* run more tests with -dt -run */

	else if (ret) {
		if (s->nb_errors)
			ret = 1;
		/* else keep the original exit code from tcc_run() */

	} else if (n < s->nb_files)
		done = 0;/* compile more files with -c */

	else if (s->do_bench)
		tcc_print_stats(s, end_time - start_time);

	tcc_delete(s);
	if (!done)
		goto redo;
	if (ppfp)
		tcc_fclose(ppfp);
	return ret;
}
/* TCC runtime library.
   Parts of this code are (c) 2002 Fabrice Bellard

   Copyright (C) 1987, 1988, 1992, 1994, 1995 Free Software Foundation, Inc.

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs,
and to distribute those combinations without any restriction coming
from the use of this file.  (The General Public License restrictions
do apply in other respects; for example, they cover modification of
the file, and distribution when not linked into a combine
executable.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

typedef int Wtype;
typedef unsigned int UWtype;
typedef unsigned int USItype;
typedef long long DWtype;
typedef unsigned long long UDWtype;

struct DWstruct {
	Wtype low, high;
};

typedef union {
	struct DWstruct s;
	DWtype ll;
} DWunion;

typedef long double XFtype;

/* the following deal with IEEE single-precision numbers */
#define EXCESS		126
#define SIGNBIT		0x80000000
#define HIDDEN		(1 << 23)
#define SIGN(fp)	((fp) & SIGNBIT)
#define EXP(fp)		(((fp) >> 23) & 0xFF)
#define MANT(fp)	(((fp) & 0x7FFFFF) | HIDDEN)

/* the following deal with IEEE double-precision numbers */
#define EXCESSD		1022
#define EXPD(fp)	(((fp.l.upper) >> 20) & 0x7FF)
#define SIGND(fp)	((fp.l.upper) & SIGNBIT)
#define HIDDEND_LL	((long long)1 << 52)
#define MANTD_LL(fp)	((fp.ll & (HIDDEND_LL-1)) | HIDDEND_LL)

/* the following deal with x86 long double-precision numbers */
#define EXCESSLD	16382
#define EXPLD(fp)	(fp.l.upper & 0x7fff)
#define SIGNLD(fp)	((fp.l.upper) & 0x8000)

/* only for x86 */
union ldouble_long {
	long double ld;
	struct {
		unsigned long long lower;
		unsigned short upper;
	} l;
};

union double_long {
	double d;
	struct {
		unsigned int lower;
		int upper;
	} l;
	long long ll;
};

union float_long {
	float f;
	unsigned int l;
};

/* XXX: we don't support several builtin supports for now */

/* XXX: fix tcc's code generator to do this instead */
float __floatundisf(unsigned long long a)
{
	DWunion uu;
	XFtype r;

	uu.ll = a;
	if (uu.s.high >= 0) {
		return (float)uu.ll;
	} else {
		r = (XFtype)uu.ll;
		r += 18446744073709551616.0;
		return (float)r;
	}
}

double __floatundidf(unsigned long long a)
{
	DWunion uu;
	XFtype r;

	uu.ll = a;
	if (uu.s.high >= 0) {
		return (double)uu.ll;
	} else {
		r = (XFtype)uu.ll;
		r += 18446744073709551616.0;
		return (double)r;
	}
}

long double __floatundixf(unsigned long long a)
{
	DWunion uu;
	XFtype r;

	uu.ll = a;
	if (uu.s.high >= 0) {
		return (long double)uu.ll;
	} else {
		r = (XFtype)uu.ll;
		r += 18446744073709551616.0;
		return (long double)r;
	}
}

unsigned long long __fixunssfdi (float a1)
{
	register union float_long fl1;
	register int exp;
	register unsigned long long l;

	fl1.f = a1;

	if (fl1.l == 0)
		return (0);

	exp = EXP (fl1.l) - EXCESS - 24;
	l = MANT(fl1.l);

	if (exp >= 41)
		return 1ULL << 63;
	else if (exp >= 0)
		l <<= exp;
	else if (exp >= -23)
		l >>= -exp;
	else
		return 0;
	if (SIGN(fl1.l))
		l = (unsigned long long)-l;
	return l;
}

long long __fixsfdi (float a1)
{
	long long ret;
	int s;
	ret = __fixunssfdi((s = a1 >= 0) ? a1 : -a1);
	return s ? ret : -ret;
}

unsigned long long __fixunsdfdi (double a1)
{
	register union double_long dl1;
	register int exp;
	register unsigned long long l;

	dl1.d = a1;

	if (dl1.ll == 0)
		return (0);

	exp = EXPD (dl1) - EXCESSD - 53;
	l = MANTD_LL(dl1);

	if (exp >= 12)
		return 1ULL << 63; /* overflow result (like gcc, somewhat) */
	else if (exp >= 0)
		l <<= exp;
	else if (exp >= -52)
		l >>= -exp;
	else
		return 0;
	if (SIGND(dl1))
		l = (unsigned long long)-l;
	return l;
}

long long __fixdfdi (double a1)
{
	long long ret;
	int s;
	ret = __fixunsdfdi((s = a1 >= 0) ? a1 : -a1);
	return s ? ret : -ret;
}

unsigned long long __fixunsxfdi (long double a1)
{
	register union ldouble_long dl1;
	register int exp;
	register unsigned long long l;

	dl1.ld = a1;

	if (dl1.l.lower == 0 && dl1.l.upper == 0)
		return (0);

	exp = EXPLD (dl1) - EXCESSLD - 64;
	l = dl1.l.lower;
	if (exp > 0)
		return 1ULL << 63;
	if (exp < -63)
		return 0;
	l >>= -exp;
	if (SIGNLD(dl1))
		l = (unsigned long long)-l;
	return l;
}

long long __fixxfdi (long double a1)
{
	long long ret;
	int s;
	ret = __fixunsxfdi((s = a1 >= 0) ? a1 : -a1);
	return s ? ret : -ret;
}

/* runtime bits libtcc1.a would otherwise provide, needed only when this file
   is compiled by tcc itself.  x86_64 PE only. */
#ifdef __TINYC__

__asm__
(
	".globl alloca \n "
	"alloca: \n "
	"pop     %rdx \n "
	"mov     %rcx,%rax \n "
	"add     $15,%rax \n "
	"and     $-16,%rax \n "
	"jz      p3 \n "
	"p1: \n "
	"cmp     $4096,%rax \n "
	"jbe     p2 \n "
	"test    %rax,-4096(%rsp) \n "
	"sub     $4096,%rsp \n "
	"sub     $4096,%rax \n "
	"jmp p1 \n "
	"p2: \n "
	"sub     %rax,%rsp \n "
	"mov     %rsp,%rax \n "
	"p3: \n "
	"push    %rdx \n "
	"ret \n "
);

__asm__
(
	".globl __chkstk\n"
	"__chkstk:\n"
	"xchg    (%rsp),%rbp\n"     /* store ebp, get ret.addr */
	"push    %rbp\n"            /* push ret.addr */
	"lea     8(%rsp),%rbp\n"    /* setup frame ptr */
	"push    %rcx\n"            /* save ecx */
	"mov     %rbp,%rcx\n"
	"movslq  %eax,%rax\n"
	"P0:\n"
	"sub     $4096,%rcx\n"
	"test    %rax,(%rcx)\n"
	"sub     $4096,%rax\n"
	"cmp     $4096,%rax\n"
	"jge     P0\n"
	"sub     %rax,%rcx\n"
	"test    %rax,(%rcx)\n"
	"mov     %rsp,%rax\n"
	"mov     %rcx,%rsp\n"
	"mov     (%rax),%rcx\n"     /* restore ecx */
	"jmp     *8(%rax)\n"
);

/* setjmp/longjmp support */
__asm__
(
	".globl tinyc_getbp\n"
	"tinyc_getbp:\n"
	"mov %rbp,%rax\n"
	"ret\n"
);

#include <tchar.h>
#define __UNKNOWN_APP    0
#define __CONSOLE_APP    1
#define __GUI_APP        2

extern void __set_app_type(int);
typedef struct {
	int newmode;
} _startupinfo;
extern int __cdecl __getmainargs(int *pargc, _TCHAR ***pargv, _TCHAR ***penv,
				 int globb, _startupinfo *);

int _start()
{
	_startupinfo start_info = {0};
	__set_app_type(__GUI_APP);
	//assume no unicode.
	__getmainargs( &__argc, &__targv, &_tenviron, 0, &start_info);
	main(__argc, __targv);
	return 1;
}

#endif
