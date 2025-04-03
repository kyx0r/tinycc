#!/bin/sh

amal -DONE_SOURCE -E tcc.c > all.c
# cleanup multiline macros for compat with unifdef
EXINIT='se noled:&/\\$
DJ$DJnDJnDJ$DJ$DJnDJ$DJnDJ$DJnDJ$DJnDJnDJnDJ$DJn:q' vi -e all.c

#cleanup license turds
EXINIT='se noled:/ifndef ONE_SOURCE/:.,$g/\/\*$/.,.+1g/.*\*.*TCC - /.-1,.d\\:&d/\*/$
dd:wq' vi -e all.c

cp all.c __all.c

cat lib/libtcc1.c >> __all.c
EXINIT="1,18d:i\#define TCC_VERSION \"$(git rev-parse --verify HEAD)\"
:g/if \(TCC_LIBTCC1\[0\]\)/.,.+1d:wq" vi -e __all.c

cat <<\EOF >> __all.c

#ifdef __TINYC__

#ifdef TCC_TARGET_I386
__asm__
(
	".global alloca\n"
	"alloca:\n"
	"pop %edx\n"
	"pop %eax\n"
	"add $3, %eax\n"
	"and $-4,%eax\n"
	"je exit\n"
	"p1:\n"
	"cmp $0x1000,%eax\n"
	"jbe inter\n"
	"test   %eax,-0x1000(%esp)\n"
	"sub    $0x1000,%esp\n"
	"sub    $0x1000,%eax\n"
	"jmp p1\n"
	"inter:\n"
	"sub %eax,%esp\n"
	"mov %esp,%eax\n"
	"exit:\n"
	"push %edx\n"
	"push %edx\n"
	"ret\n"
);

__asm__
(
	".global __chkstk \n"
	"__chkstk: \n"
	"xchg    (%esp),%ebp\n"     /* store ebp, get ret.addr */
	"push    %ebp\n"            /* push ret.addr */
	"lea     4(%esp),%ebp\n"    /* setup frame ptr */
	"push    %ecx\n"            /* save ecx */
	"mov     %ebp,%ecx\n"
	"P0:\n"
	"sub     $4096,%ecx\n"
	"test    %eax,(%ecx)\n"
	"sub     $4096,%eax\n"
	"cmp     $4096,%eax\n"
	"jge     P0\n"
	"sub     %eax,%ecx\n"
	"test    %eax,(%ecx)\n"
	"mov     %esp,%eax\n"
	"mov     %ecx,%esp\n"
	"mov     (%eax),%ecx\n"     /* restore ecx */
	"jmp     *4(%eax)\n"
);
#endif

#ifdef TCC_TARGET_X86_64
__asm__
(
	".globl alloca \n "
	"alloca: \n "
	"pop     %rdx \n "
#ifdef _WIN32
	"mov     %rcx,%rax \n "
#else
	"mov     %rdi,%rax \n "
#endif
	"add     $15,%rax \n "
	"and     $-16,%rax \n "
	"jz      p3 \n "
#ifdef _WIN32
	"p1: \n "
	"cmp     $4096,%rax \n "
	"jbe     p2 \n "
	"test    %rax,-4096(%rsp) \n "
	"sub     $4096,%rsp \n "
	"sub     $4096,%rax \n "
	"jmp p1 \n "
	"p2: \n "
#endif
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

/* ---------------------------------------------- */
/* setjmp/longjmp support */
__asm__
(
	".globl tinyc_getbp\n"
	"tinyc_getbp:\n"
	"mov %rbp,%rax\n"
	"ret\n"
);

#endif

#include <tchar.h>
#define __UNKNOWN_APP    0
#define __CONSOLE_APP    1
#define __GUI_APP        2

extern void __set_app_type(int);
typedef struct
{
        int newmode;
} _startupinfo;
extern int __cdecl __getmainargs(int *pargc, _TCHAR ***pargv, _TCHAR ***penv, int globb, _startupinfo*);

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
EOF

unifdef -k \
-DTCC_TARGET_PE \
-DTCC_TARGET_X86_64 \
-D__x86_64__ \
-DONE_SOURCE \
-DTCC_IS_NATIVE \
-D_WIN64 \
-D_WIN32 \
-UCONFIG_NEW_MACHO \
-UTCC_TARGET_MACHO \
-UTCC_TARGET_ARM \
-UTCC_TARGET_UNIX \
-UTARGETOS_BSD \
-UTARGETOS_OpenBSD \
-UTARGETOS_FreeBSD \
-UTARGETOS_NetBSD \
-UTARGETOS_FreeBSD_kernel \
-UTCC_TARGET_I386 \
-U__arm__ \
-UTCC_TARGET_ARM64 \
-UTCC_ARM_EABI \
-UTCC_ARM_VFP \
-UTCC_TARGET_COFF \
-UTCC_TARGET_RISCV64 \
-UTCC_TARGET_C67 \
-U__APPLE__ \
-UCONFIG_SELINUX \
-U_MSC_VER \
__all.c > tcc_win64_x86_64.c

astyle -n -H --style=linux --max-code-length=80 --indent=force-tab=8 --squeeze-ws --squeeze-lines=1 --align-pointer=name tcc_win64_x86_64.c

