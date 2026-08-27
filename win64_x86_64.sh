#!/bin/sh

IFS=$(printf ' \t\n@'); IFS=${IFS%@}

amal -DONE_SOURCE -E tcc.c > all.c
# cleanup multiline macros for compat with unifdef
EXINIT="$(printf '$?\\%%s/(defined[^\n]*?)[ \t]*[\\\\]\n[ \t]*/\\1 /gm:wq')" vi -e all.c

# cleanup license turds
EXINIT='>ifndef ONE_SOURCE>,$s#/\*([^*]|\*+[^*/])*Copyright([^*]|\*+[^*/])*\*+/##mg:wq' vi -e all.c

cp all.c __all.c

cat lib/libtcc1.c >> __all.c
# nextvi's `:0i` swallows the rest of EXINIT as insert text and never
# writes, so bake the version line in with a substitute carrying a real
# newline in the replacement instead
NL='
'
EXINIT="1,#>/\*>-1d:1s@^@#define TCC_VERSION \"$(head -n1 VERSION)\"$NL#define TCC_GITHASH \"$(git rev-parse --verify HEAD)\"$NL:g/if \(TCC_LIBTCC1\[0\]\)/.,.+1d:wq" vi -e __all.c
grep -q '^#define TCC_VERSION "' __all.c ||
	{ echo "TCC_VERSION header not inserted" >&2; exit 1; }

# unlike the SysV amalgamation this needs no va_list.c: tcc's x86_64 PE
# __builtin_va_arg macro (tccdefs.h) folds the argument off the stack inline.
# What it does need embedded, under the same __TINYC__ guard as the other
# runtime bits, so tcc can compile and link this file (and anything it
# compiles) without libtcc1.a: the Windows alloca/__chkstk pair and the
# msvcrt _start entry point.
cat <<\EOF >> __all.c

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

out=tcc_win64_x86_64.c
CC=${CC:-gcc}
# headers for the PE target: the win32 environment tcc ships, predef-
# compatible with tcc rather than with a host gcc, so preprocessing works
# but full compilation is only possible under a real mingw
INCS="-isystem win32/include -isystem win32/include/winapi"

# Symbols a fixed win64/PE build decides.  Values matter as much as the
# plain -D/-U: without -DPTR_SIZE=8, unifdef cannot fold
# `#if PTR_SIZE == 8 && !defined TCC_TARGET_PE` and keeps both LONG_SIZE
# arms -- which is why LONG_SIZE=4 would lose to its own #undef.
# ELF_OBJ_ONLY and TCC_USING_DOUBLE_FOR_LDOUBLE are given explicitly even
# though tcc.h would define them itself: unifdef cannot follow a #define
# the file makes, so every later test of them would stay undecided.
DEFS="\
-DTCC_TARGET_X86_64 \
-D__x86_64__ \
-DTCC_TARGET_PE \
-D_WIN32 \
-D_WIN64 \
-DONE_SOURCE \
-DTCC_IS_NATIVE \
-DPTR_SIZE=8 \
-DLONG_SIZE=4 \
-DLDOUBLE_SIZE=16 \
-DPROMOTE_RET \
-DTCC_TARGET_NATIVE_STRUCT_COPY \
-DCONFIG_TCC_ASM \
-DCONFIG_TCC_SEMLOCK=1 \
-DELF_OBJ_ONLY \
-DTCC_USING_DOUBLE_FOR_LDOUBLE \
-DCONFIG_RUNMEM_VIRTUALALLOC=1 \
-Dprecedence_parser \
-UTCC_TARGET_UNIX \
-UTCC_EH_FRAME \
-UTCC_TARGET_MACHO \
-UTCC_TARGET_ARM \
-UTCC_TARGET_ARM64 \
-UTCC_TARGET_I386 \
-UTCC_TARGET_RISCV64 \
-UTCC_TARGET_C67 \
-UTCC_TARGET_COFF \
-UCONFIG_NEW_MACHO \
-UTCC_ARM_EABI \
-UTCC_ARM_VFP \
-UTCC_ARM_HARDFLOAT \
-UUDIV_NEEDS_NORMALIZATION \
-UCHAR_IS_UNSIGNED \
-U_MSC_VER \
-U_M_ARM64 \
-U__arm__ \
-U__i386__ \
-U__APPLE__ \
-U__GNU__ \
-U__clang__ \
-U__cplusplus \
-U__OpenBSD__ \
-U__NetBSD__ \
-U__FreeBSD__ \
-UTARGETOS_BSD \
-UTARGETOS_OpenBSD \
-UTARGETOS_FreeBSD \
-UTARGETOS_NetBSD \
-UTARGETOS_FreeBSD_kernel \
-UTARGETOS_ANDROID \
-UCONFIG_TCC_MUSL \
-UCONFIG_SELINUX \
-UCONFIG_TCC_PIE \
-UCONFIG_TCC_BCHECK \
-UCONFIG_TCC_BACKTRACE \
-UCONFIG_TCC_BACKTRACE_ONLY \
-UCONFIG_TCC_STATIC \
-UCONFIG_TCCBOOT \
-UTCC_LIBGCC \
-UTCC_PROFILE \
-UTCC_CROSS_TEST \
-UPARSE_DEBUG \
-UPP_DEBUG \
-UINC_DEBUG \
-UMEM_DEBUG \
-UDMEM_DEBUG \
-UASM_DEBUG \
-UTAL_DEBUG \
-UTAL_INFO \
-UBF_DEBUG \
-USYM_DEBUG \
-UDEBUG_RELOC \
-UDEBUG_VERSION \
-U_TCC_H \
-U_ELF_H \
-U_DWARF_H \
-U__GNU_STAB__ \
-ULIBTCC_H"

# Symbols that only become decidable after the TARGET_DEFS_ONLY / USING_GLOBALS
# double inclusions below are gone -- unifdef leaves everything nested in an
# undecided region untouched, so these need a second round.  No _SC_PAGESIZE
# on PE, so tccrun.c falls back to its flat 4096 PAGESIZE like a real
# Windows build does.
# NEED_RELOC_TYPE and NEED_BUILD_GOT are defined by tcc.h only under
# `#ifndef TCC_TARGET_PE', so a real Windows build never sees them: the whole
# ELF GOT/PLT machinery in tccelf.c and x86_64-link.c is compiled out, along
# with the prototypes and the gotplt_entry enum.  The linux amalgamation has
# to force them true; here forcing them true kept code calling declarations
# that had been folded away.  Unlike the linux script, these are -U.
DEFS2="\
-UNEED_RELOC_TYPE \
-UNEED_BUILD_GOT \
-DSHT_RELX=4 \
-DSHT_RELA=4 \
-DRC_IRE2 \
-DRC_FRE2 \
-DUSE_TAL \
-DTARGET_DEFS_ONLY_ON \
-DUSING_GLOBALS_ON \
-UTARGET_DEFS_ONLY \
-UUSING_GLOBALS \
-U_SC_PAGESIZE \
-UPAGESIZE \
-UCONFIG_TCC_PREDEFS \
-UCONFIG_NEW_DTAGS \
-UCONFIG_TCC_SWITCHES \
-UCONFIG_TRIPLET \
-UPUB_FUNC \
-ULIBTCCAPI"

# The oracles and the verification below preprocess with the fixed target's
# decisions as the environment: ref still carries the #define/#ifdef
# scaffolding that round two folded away in $out, and gcc must resolve it
# the same way or the two texts diverge with no pass being wrong.
CPPFLAGS="$DEFS $DEFS2 $INCS"

# names of everything folded above, for wiping the `#endif` trailer comments
# that unifdef leaves behind as orphans
GONE="TARGET_DEFS_ONLY|USING_GLOBALS|NEED_BUILD_GOT|NEED_RELOC_TYPE|SHT_RELX\
|PTR_SIZE|LONG_SIZE|PE|_WIN32|_WIN64|TCC_TARGET_PE|TCC_TARGET_UNIX|TCC_TARGET_MACHO\
|TCC_TARGET_ARM|TCC_TARGET_ARM64|TCC_TARGET_I386|TCC_TARGET_RISCV64\
|TCC_TARGET_C67|TCC_TARGET_COFF|CONFIG_TCC_BCHECK|CONFIG_TCC_BACKTRACE\
|ELF_OBJ_ONLY|TCC_USING_DOUBLE_FOR_LDOUBLE|CONFIG_RUNMEM_VIRTUALALLOC\
|TARGETOS_Linux|TCC_EH_FRAME"

# Nextvi regex has no \t or \n escapes, and <:> is the ex separator, so a
# non-capturing group is written (?\:...) -- and the parity rule applies once
# per parsing level, so inside a `$?` loop argument it needs (?\\\:...), which
# is what NC is for.  X is a scratch marker; it never survives a step.
NL='
'
TAB=$(printf '\t')
X=$(printf '\001')
# one line that is blank or holds a single-line comment.  a comment spanning
# lines deliberately does not match: that is what keeps the licence header.
NC='(?\\\:'
BC="[ $TAB]*$NC/\*[^$NL]*\*/)?[ $TAB]*"
# a doomed line, its \-continuations, and the doc comment sitting on top of it.
# anchored on the preceding newline because in multiline mode ^ only asserts at
# the start of the whole region, not at every line.
DOOMED="$NL$NC$BC$NL)*$X$NC[^$NL]*\\\\$NL)*[^$NL]*"

# Run an ex script over $out and abort if a command in it reported an error.
# `:??!p EXFAIL` prints only when the command before it failed.  A command that
# fails *inside* a :g aborts the whole global, and that is the dangerous case:
# the leftover dead text still passes the verification at the bottom, so the
# run would report success having skipped most of its work.  Catching it here
# needs ec_glob to forward the inner status; without that fix this still
# catches a :g whose own pattern matched nothing.
ex() {
	EXINIT="$1" vi -e $out </dev/null 2>&1 | tr -d '\r' | grep -q EXFAIL &&
		{ echo "ex failed: $2" >&2; exit 1; }
	return 0
}

# a killed run must not leave a $out.prev that short-circuits the loop below
rm -f $out.prev $out.ex $out.once $out.alt $out.rp $out.pp $out.diff

unifdef -k $DEFS __all.c > $out
cp $out $out.ref

# ONE_SOURCE pulls tcc.h, x86_64-gen.c and x86_64-link.c in twice -- once under
# TARGET_DEFS_ONLY, once not -- and flips USING_GLOBALS about ten times on the
# way.  unifdef cannot follow a symbol the file defines and undefines itself, so
# it keeps both copies.  Rename each #ifdef after the state it actually sees,
# then let round two fold them.
ex ">^#define TARGET_DEFS_ONLY\$>,>^#undef TARGET_DEFS_ONLY\$>s/^#ifdef TARGET_DEFS_ONLY\$/#ifdef TARGET_DEFS_ONLY_ON/:??!p EXFAIL\
:%g/^#define USING_GLOBALS\$/>^#ifdef USING_GLOBALS\$>s/\$/_ON/:??!p EXFAIL\
:%g/^#(?\:define|undef) (?\:TARGET_DEFS_ONLY|USING_GLOBALS)\$/d:??!p EXFAIL:wq" "resolving the double inclusion"

unifdef -k $DEFS2 $out > $out.2   # unifdef exits 1 whenever it
mv $out.2 $out                    # changed something, so never chain on it

# An error in any command inside a :g stops the whole global, so one failed
# search would quietly skip every later match -- and the verification at the
# bottom cannot see that, because text left behind is still dead text.
# Every pass therefore states a post-condition.
grep -q "^#.*\(TARGET_DEFS_ONLY\|USING_GLOBALS\)" $out &&
	{ echo "double-inclusion left unresolved in $out" >&2; exit 1; }

$CC -E -P -w -DNDEBUG $CPPFLAGS $out >/dev/null ||
	{ echo "preprocessing broke $out" >&2; exit 1; }

# Now drop what the fixed target made unreachable.  Three oracles, no guessing:
#
#   macros        $CC -Wunused-macros, which knows about expansion and #ifdef
#                 tests, so ElfW()-style ## pasting cannot be missed.  -E only:
#                 without a mingw the host compiler cannot syntax-check a PE
#                 build, but the warnings come from the preprocessor, which is
#                 all this oracle needs
#   types/enums   an identifier occurring exactly once in the *preprocessed*
#                 text is only its own definition.  preprocessed, because
#                 Elf64_Shdr is never written literally -- ElfW(Shdr) pastes it
#
# repeated until the file stops shrinking.
until cmp -s $out $out.prev 2>/dev/null; do
	cp $out $out.prev

	# --- macros: ~2100 elf.h/dwarf.h/stab.h defines for architectures this
	# build dropped, plus every debug scaffold unifdef just switched off
	$CC -E -Wunused-macros $CPPFLAGS $out 2>&1 >/dev/null |
		sed -n "s/^[^:]*:\([0-9][0-9]*\):.*-Wunused-macros.*/\1s@^@$X@/p" |
		tr '\n' ':' > $out.ex
	if [ -s $out.ex ]; then
		EXINIT="$(cat $out.ex)wq" vi -e $out </dev/null
		EXINIT="\$?\\%s@$DOOMED@@gm:wq" vi -e $out </dev/null
	fi

	# --- the once-only identifier table, shared by the next two steps
	$CC -E -P -DNDEBUG $CPPFLAGS $out 2>/dev/null | tr -c 'A-Za-z0-9_' '\n' |
		sort | uniq -c | sed -n 's/^ *1 //p' | sort > $out.once
	[ -s $out.once ] ||
		{ echo "empty identifier table: preprocessing failed" >&2; exit 1; }

	# --- typedefs: every Elf32_* plus the Elf64_* records tcc never touches
	sed -n "s/^}[ $TAB]*\([A-Za-z_][A-Za-z0-9_]*\);\$/\1/p" $out |
		sort -u | comm -12 - $out.once | tr '\n' '|' | sed 's/|$//' > $out.alt
	if [ -s $out.alt ]; then
		ex "%g@^\}[ $TAB]*(?\:$(cat $out.alt));@<^typedef<,.d:??!p EXFAIL:wq" \
			"deleting unused typedefs"
	fi

	# --- enum constants: DW_TAG_*, DW_AT_*, ...  Only ones written `= value`
	# whose successor is also `= value` (or the closing brace) are cut, so no
	# implicit numbering can shift.  The guard is a captured group put back by
	# \1.  A leading (?=...) asserting the same span plus the next line is the
	# cleaner spelling and gives the same result, but it re-scans the whole span
	# at every position: 0.1s here versus over a minute on this file.
	sed -n "s/^[ $TAB]*\([A-Za-z_][A-Za-z0-9_]*\)[ $TAB]*=.*/\1/p" $out |
		sort -u | comm -12 - $out.once | tr '\n' '|' | sed 's/|$//' > $out.alt
	if [ -s $out.alt ]; then
		EXINIT="%g@^[ $TAB]*(?\:$(cat $out.alt))[ $TAB]*=@s@^@$X@:wq" vi -e $out </dev/null
		EXINIT="\$?\\%s@$NL$NC$BC$NL)*$X[^$NL]*($NL$NC$BC$NL)*[^$NL]*$NC=|\}))@\\1@gm\
:\$?\\%s@$NL$NC$BC$NL)*[ $TAB]*enum[ $TAB]*$NL[ $TAB]*\{[ $TAB]*$NL$NC$BC$NL)*[ $TAB]*\};@@gm\
:\$?\\%s@,($NL[ $TAB]*)\}@\\1}@gm\
:%s@$X@@g:wq" vi -e $out </dev/null
		# whatever the guard refused to cut keeps its marker; strip it
		EXINIT="%s@$X@@g:wq" vi -e $out </dev/null
	fi
done

n=$($CC -E -Wunused-macros $CPPFLAGS $out 2>&1 >/dev/null | grep -c "is not used")
[ "$n" -eq 0 ] ||
	{ echo "$n unused macros left in $out: a pass was cut short" >&2; exit 1; }
sed -n "s/^}[ $TAB]*\([A-Za-z_][A-Za-z0-9_]*\);\$/\1/p" $out |
	sort -u | comm -12 - $out.once > $out.alt
[ -s $out.alt ] &&
	{ echo "unused typedefs left in $out: $(tr '\n' ' ' <$out.alt)" >&2; exit 1; }
# enum constants are not asserted empty: the numbering guard keeps a few on
# purpose, e.g. TOK_LAST, whose successor is a #define rather than a member.

# cpp line markers -> one banner per source file; `#endif` trailers orphaned by
# the folding above; the emptied `#define`/`#undef` shells unifdef leaves.
for f in $(sed -n "s@^// [0-9]* \"\([^\"]*\)\".*@\1@p" $out | sort -u); do
	case $f in
	*"<"*|*">"*|*" "*) continue;;	# <command line> and friends: no banner
	esac
	e=$(echo "$f" | sed 's/[.]/\\./g')
	ex ">^// [0-9]* \"$e\"[ 0-9]*\$>s@.*@/* ==================== $f ==================== */@:??!p EXFAIL:wq" \
		"banner for $f"
done
# <!> starts an external-command block in ex args, so it has to be escaped
ex "%g@^// [0-9]* \"[^\"]*\"[ 0-9]*\$@d:??!p EXFAIL\
:%g@^[ $TAB]*/\* *(?\:\!|def |not |end of )? *(?\:$GONE) *\*/[ $TAB]*\$@d:??!p EXFAIL:wq" \
	"stripping line markers and orphaned #endif trailers"

astyle -xb -n -H --style=linux --max-code-length=80 --indent=force-tab=8 \
	--squeeze-ws --squeeze-lines=1 --align-pointer=name $out

# Prove none of it changed the program.  The linux amalgamation compiles the
# untouched round-one output and the finished file and compares objects byte
# for byte; here there is no mingw for $CC to compile a PE build with, so the
# preprocessed text is compared instead -- as token multisets, because astyle
# reformats $out but not $out.ref.  Every transformation above is silent in
# preprocessed text -- dropped #defines, comments and line markers leave no
# trace -- except the ones deleting real declarations: the round-two folding,
# unused typedefs and unused enum members.  So no token may appear that ref
# does not have, and of the identifiers allowed to vanish every occurrence
# must sit on a line the passes deleted (their bodies), or be a keyword, or
# have occurred exactly once in ref: its own, now deleted, definition.
tok() {
	# __LINE__ is baked into tcc_error() calls: the two files do not have
	# the same line numbers, so neutralise it before preprocessing, like
	# the object comparison in the linux amalgamation does
	sed 's/\([^A-Za-z0-9_]\)__LINE__\([^A-Za-z0-9_]\)/\10\2/g' "$1" > $out.l.c
	$CC -E -P -DNDEBUG $CPPFLAGS $out.l.c 2>/dev/null |
		tr -c 'A-Za-z0-9_' '\n' | grep -v '^$' | sort | uniq -c
}
# gcc only preprocesses *.c files -- a $out.ref is "linker input, unused"
# and yields empty output with exit 0 -- so give the copy a .c name first.
cp $out.ref $out.v.c
tok $out.v.c > $out.rt && [ -s $out.rt ] ||
	{ echo "preprocessing $out.ref failed" >&2; exit 1; }
tok $out > $out.ot && [ -s $out.ot ] ||
	{ echo "preprocessing $out failed" >&2; exit 1; }
if cmp -s $out.rt $out.ot; then
	echo "verified: $out preprocesses identical to the unstripped amalgamation"
else
	# everything the passes deleted, snapshot before astyle muddies the diff
	diff $out.ref $out > $out.gone 2>/dev/null
	[ $? -le 1 ] || { echo "diff of $out.ref failed" >&2; exit 1; }
	grep '^< ' $out.gone | tr -c 'A-Za-z0-9_' '\n' |
		grep -v '^$' | sort -u > $out.gt
	[ -s $out.gt ] || echo 0GONE > $out.gt
	awk '
		FNR == 1 { phase++ }
		phase == 1 { gone[$1] = 1 }
		phase == 2 { r[$2] = $1 }
		phase == 3 { o[$2] = $1 }
		END {
			kw = " auto break case char const continue default do" \
			     " double else enum extern float for goto if inline" \
			     " int long register restrict return short signed" \
			     " sizeof static struct switch typedef union" \
			     " unsigned void volatile while _Bool"
			n = split(kw, a, " ")
			for (t in o)
				if (!(t in r) || o[t] > r[t]) {
					print "new token: " t
					bad = 1
				}
			for (t in r) {
				d = r[t] - (t in o ? o[t] : 0)
				if (d <= 0) continue
				if (t !~ /^[A-Za-z_][A-Za-z0-9_]*$/) continue
				iskw = 0
				for (i = 1; i <= n; i++) if (a[i] == t) iskw = 1
				if (!iskw && !(t in gone) && r[t] != 1) {
					print "dropped token: " t
					bad = 1
				}
			}
			exit bad
		}
	' $out.gt $out.rt $out.ot ||
		{ echo "VERIFY FAILED: $out differs from $out.ref" >&2; exit 1; }
	echo "verified: $out differs from the unstripped amalgamation by dropped typedefs and enum members only"
fi

rm -f $out.ref $out.prev $out.ex $out.once $out.alt $out.gone $out.gt \
	$out.rt $out.ot $out.v.c $out.l.c
