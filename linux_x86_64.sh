#!/bin/sh

amal -DONE_SOURCE -E tcc.c > all.c
# cleanup multiline macros for compat with unifdef
EXINIT="$(printf '$?\\%%s/(defined[^\n]*?)[ \t]*[\\\\]\n[ \t]*/\\1 /gm:wq')" vi -e all.c

# cleanup license turds
EXINIT='>ifndef ONE_SOURCE>,$s#/\*([^*]|\*+[^*/])*Copyright([^*]|\*+[^*/])*\*+/##mg:wq' vi -e all.c

cp all.c __all.c

cat lib/libtcc1.c >> __all.c
EXINIT="1,#>/\*>-1d:0i #define TCC_VERSION \"$(git rev-parse --verify HEAD)\"
:g/if \(TCC_LIBTCC1\[0\]\)/.,.+1d:wq" vi -e __all.c

cat <<\EOF >> __all.c

/* runtime bits libtcc1.a would otherwise provide, needed only when this file
   is compiled by tcc itself.  x86_64 SysV only: __chkstk is a PE/Windows
   stack-probe helper (tcc never emits a call to it here) and _start/<tchar.h>
   are the msvcrt entry point, so neither belongs in a linux amalgamation. */
#ifdef __TINYC__

__asm__
(
	".globl alloca \n "
	"alloca: \n "
	"pop     %rdx \n "
	"mov     %rdi,%rax \n "
	"add     $15,%rax \n "
	"and     $-16,%rax \n "
	"jz      p3 \n "
	"sub     %rax,%rsp \n "
	"mov     %rsp,%rax \n "
	"p3: \n "
	"push    %rdx \n "
	"ret \n "
);

/* setjmp/longjmp support */
__asm__
(
	".globl tinyc_getbp\n"
	"tinyc_getbp:\n"
	"mov %rbp,%rax\n"
	"ret\n"
);

#endif
EOF

out=tcc_linux_x86_64.c
CC=${CC:-gcc}

# Symbols a fixed linux/x86_64 build decides.  Values matter as much as the
# plain -D/-U: without -DPTR_SIZE=8, unifdef cannot fold `#if PTR_SIZE == 8 &&
# !defined TCC_TARGET_PE` and keeps the whole region -- including every #ifdef
# nested inside it, which is why TCC_TARGET_PE used to survive its own -U.
DEFS="\
-DTCC_TARGET_X86_64 \
-D__x86_64__ \
-DONE_SOURCE \
-DTCC_IS_NATIVE \
-DTCC_TARGET_UNIX \
-DPTR_SIZE=8 \
-DLONG_SIZE=8 \
-DLDOUBLE_SIZE=16 \
-DPROMOTE_RET \
-DTCC_TARGET_NATIVE_STRUCT_COPY \
-DCONFIG_TCC_ASM \
-DCONFIG_TCC_SEMLOCK=1 \
-DTCC_EH_FRAME=1 \
-Dprecedence_parser \
-UTCC_TARGET_PE \
-UTCC_TARGET_MACHO \
-UTCC_TARGET_ARM \
-UTCC_TARGET_ARM64 \
-UTCC_TARGET_I386 \
-UTCC_TARGET_RISCV64 \
-UTCC_TARGET_C67 \
-UTCC_TARGET_COFF \
-UCONFIG_NEW_MACHO \
-UELF_OBJ_ONLY \
-UTCC_ARM_EABI \
-UTCC_ARM_VFP \
-UTCC_ARM_HARDFLOAT \
-UTCC_USING_DOUBLE_FOR_LDOUBLE \
-UUDIV_NEEDS_NORMALIZATION \
-UCHAR_IS_UNSIGNED \
-U_WIN32 \
-U_WIN64 \
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
-UCONFIG_RUNMEM_VIRTUALALLOC \
-UCONFIG_TCC_PIE \
-UCONFIG_TCC_BCHECK \
-UCONFIG_TCC_BACKTRACE \
-UCONFIG_TCC_BACKTRACE_ONLY \
-UCONFIG_TCC_STATIC \
-UCONFIG_TCCBOOT \
-UTCC_LIBGCC \
-UTCC_PROFILE \
-UTCC_GITHASH \
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
# undecided region untouched, so these need a second round.
DEFS2="\
-DNEED_RELOC_TYPE \
-DNEED_BUILD_GOT \
-DSHT_RELX=4 \
-DSHT_RELA=4 \
-DRC_IRE2 \
-DRC_FRE2 \
-DUSE_TAL \
-D_SC_PAGESIZE=30 \
-DTARGET_DEFS_ONLY_ON \
-DUSING_GLOBALS_ON \
-UTARGET_DEFS_ONLY \
-UUSING_GLOBALS \
-UPAGESIZE \
-UCONFIG_TCC_PREDEFS \
-UCONFIG_NEW_DTAGS \
-UCONFIG_TCC_SWITCHES \
-UCONFIG_TRIPLET \
-UPUB_FUNC \
-ULIBTCCAPI"

# names of everything folded above, for wiping the `#endif` trailer comments
# that unifdef leaves behind as orphans
GONE="TARGET_DEFS_ONLY|USING_GLOBALS|NEED_BUILD_GOT|NEED_RELOC_TYPE|SHT_RELX\
|PTR_SIZE|LONG_SIZE|PE|_WIN32|TCC_TARGET_PE|TCC_TARGET_UNIX|TCC_TARGET_MACHO\
|TCC_TARGET_ARM|TCC_TARGET_ARM64|TCC_TARGET_I386|TCC_TARGET_RISCV64\
|TCC_TARGET_C67|TCC_TARGET_COFF|CONFIG_TCC_BCHECK|CONFIG_TCC_BACKTRACE\
|ELF_OBJ_ONLY"

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
# the leftover dead text still passes the object comparison at the bottom, so
# the run would report success having skipped most of its work.  Catching it
# here needs ec_glob to forward the inner status; without that fix this still
# catches a :g whose own pattern matched nothing.
ex() {
	EXINIT="$1" vi -e $out </dev/null 2>&1 | tr -d '\r' | grep -q EXFAIL &&
		{ echo "ex failed: $2" >&2; exit 1; }
	return 0
}

# a killed run must not leave a $out.prev that short-circuits the loop below
rm -f $out.prev $out.ex $out.once $out.alt

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
# search would quietly skip every later match -- and the object-code check at
# the bottom cannot see that, because text left behind is still dead text.
# Every pass therefore states a post-condition.
grep -q "^#.*\(TARGET_DEFS_ONLY\|USING_GLOBALS\)" $out &&
	{ echo "double-inclusion left unresolved in $out" >&2; exit 1; }

# Now drop what the fixed target made unreachable.  Three oracles, no guessing:
#
#   macros        $CC -Wunused-macros, which knows about expansion and #ifdef
#                 tests, so ElfW()-style ## pasting cannot be missed
#   types/enums   an identifier occurring exactly once in the *preprocessed*
#                 text is only its own definition.  preprocessed, because
#                 Elf64_Shdr is never written literally -- ElfW(Shdr) pastes it
#
# repeated until the file stops shrinking.
$CC -fsyntax-only -w $out || { echo "folding broke $out" >&2; exit 1; }
until cmp -s $out $out.prev 2>/dev/null; do
	cp $out $out.prev

	# --- macros: ~2100 elf.h/dwarf.h/stab.h defines for architectures this
	# build dropped, plus every debug scaffold unifdef just switched off
	$CC -fsyntax-only -Wunused-macros $out 2>&1 |
		sed -n "s/^[^:]*:\([0-9][0-9]*\):.*-Wunused-macros.*/\1s@^@$X@/p" |
		tr '\n' ':' > $out.ex
	if [ -s $out.ex ]; then
		EXINIT="$(cat $out.ex)wq" vi -e $out </dev/null
		EXINIT="\$?\\%s@$DOOMED@@gm:wq" vi -e $out </dev/null
	fi

	# --- the once-only identifier table, shared by the next two steps
	$CC -E -P -DNDEBUG $out 2>/dev/null | tr -c 'A-Za-z0-9_' '\n' |
		sort | uniq -c | sed -n 's/^ *1 //p' | sort > $out.once

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

n=$($CC -fsyntax-only -Wunused-macros $out 2>&1 | grep -c "is not used")
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

# Prove none of it changed the program: compile the untouched round-one output
# and the finished file and compare the objects byte for byte.  __LINE__ and
# assert() bake source positions into .rodata, so neutralise both first.
for f in $out.ref $out; do
	sed 's/\([^A-Za-z0-9_]\)__LINE__\([^A-Za-z0-9_]\)/\10\2/g' $f > $out.v.c
	$CC -c -DNDEBUG -w $out.v.c -o $out.v.$$.o || exit 1
	mv $out.v.$$.o $out.v.$(basename $f).o
done
if cmp $out.v.$(basename $out.ref).o $out.v.$(basename $out).o; then
	echo "verified: $out is byte-identical to the unstripped amalgamation"
else
	echo "VERIFY FAILED: $out differs from $out.ref" >&2
	exit 1
fi

rm -f $out.ref $out.prev $out.ex $out.once $out.alt $out.v.c $out.v.*.o
