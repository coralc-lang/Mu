/* One libtcc.a, 12 emit-targets, one host process.
   Verifies namespaced libtcc APIs can each drive their own target. */
#include <stdio.h>
#include <string.h>

typedef struct TCCState TCCState;
typedef void TCCErrorFunc(void *opaque, const char *msg);

#define DECL(T) \
    extern TCCState *tcc_new__##T(void); \
    extern void tcc_delete__##T(TCCState *); \
    extern void tcc_set_lib_path__##T(TCCState *, const char *); \
    extern void tcc_set_error_func__##T(TCCState *, void *, TCCErrorFunc *); \
    extern int tcc_set_options__##T(TCCState *, const char *); \
    extern int tcc_set_output_type__##T(TCCState *, int); \
    extern int tcc_compile_string__##T(TCCState *, const char *); \
    extern int tcc_add_library_path__##T(TCCState *, const char *); \
    extern int tcc_add_include_path__##T(TCCState *, const char *); \
    extern int tcc_output_file__##T(TCCState *, const char *); \
    extern int tcc_run__##T(TCCState *, int, char **); \
    extern int tcc_relocate__##T(TCCState *); \
    extern void *tcc_get_symbol__##T(TCCState *, const char *);

DECL(x86_64_win32)
DECL(arm64_win32)
DECL(x86_64)
DECL(arm64)
DECL(i386)

#define TCC_OUTPUT_MEMORY 1
#define TCC_OUTPUT_EXE    2
#define TCC_OUTPUT_OBJ    3

static int failures;

static void errcb(void *op, const char *msg)
{
    (void)op;
    fprintf(stderr, "  tcc error: %s\n", msg);
}

static const char *src =
    "int printf(const char *, ...);\n"
    "int main(void) { printf(\"hello from merged libtcc\\n\"); return 0; }\n";

/* emit a PE exe for target T via namespaced API */
static int emit_pe(const char *tag, TCCState *(*fnew)(void),
                   void (*fdel)(TCCState *),
                   void (*flibpath)(TCCState *, const char *),
                   void (*ferr)(TCCState *, void *, TCCErrorFunc *),
                   int (*fsetopt)(TCCState *, const char *),
                   int (*fouttype)(TCCState *, int),
                   int (*fcomp)(TCCState *, const char *),
                   int (*flibpath2)(TCCState *, const char *),
                   int (*fout)(TCCState *, const char *),
                   const char *outpath)
{
    TCCState *s = fnew();
    if (!s) { fprintf(stderr, "%s: tcc_new failed\n", tag); return -1; }
    ferr(s, 0, errcb);
    flibpath(s, ".");
    fsetopt(s, "-nostdinc");
    if (fouttype(s, TCC_OUTPUT_EXE) == -1) { fdel(s); return -1; }
    flibpath2(s, ".");
    flibpath2(s, "./win32/lib");
    if (fcomp(s, src) == -1) {
        fprintf(stderr, "%s: compile failed\n", tag);
        fdel(s);
        return -1;
    }
    if (fout(s, outpath) == -1) {
        fprintf(stderr, "%s: output failed\n", tag);
        fdel(s);
        return -1;
    }
    fdel(s);
    printf("%s: wrote %s\n", tag, outpath);
    return 0;
}

int main(void)
{
    /* --- native in-memory JIT run (x86_64 linux variant) --- */
    {
        TCCState *s = tcc_new__x86_64();
        char *argv[] = { "test" };
        int r = -1;
        if (s) {
            tcc_set_error_func__x86_64(s, 0, errcb);
            tcc_set_lib_path__x86_64(s, ".");
            tcc_set_output_type__x86_64(s, TCC_OUTPUT_MEMORY);
            if (tcc_compile_string__x86_64(s, src) != -1)
                r = tcc_run__x86_64(s, 1, argv);
            tcc_delete__x86_64(s);
        }
        if (r != 0) { fprintf(stderr, "FAIL: native tcc_run = %d\n", r); failures++; }
        else printf("native x86_64 tcc_run: OK\n");
    }

    /* --- native JIT: compile + relocate + call a function --- */
    {
        TCCState *s = tcc_new__x86_64();
        int (*add)(int, int) = 0;
        if (s) {
            tcc_set_error_func__x86_64(s, 0, errcb);
            tcc_set_lib_path__x86_64(s, ".");
            tcc_set_output_type__x86_64(s, TCC_OUTPUT_MEMORY);
            if (tcc_compile_string__x86_64(s, "int add(int a,int b){return a+b;}") != -1
                && tcc_relocate__x86_64(s) != -1)
                add = (int (*)(int, int))tcc_get_symbol__x86_64(s, "add");
            if (!add || add(2, 40) != 42) {
                fprintf(stderr, "FAIL: native JIT add\n");
                failures++;
            } else {
                printf("native x86_64 JIT add(2,40): OK\n");
            }
            tcc_delete__x86_64(s);
        }
    }

    /* --- cross: PE exe for windows-x64 --- */
    if (emit_pe("x86_64-win32",
                tcc_new__x86_64_win32, tcc_delete__x86_64_win32,
                tcc_set_lib_path__x86_64_win32, tcc_set_error_func__x86_64_win32,
                tcc_set_options__x86_64_win32, tcc_set_output_type__x86_64_win32,
                tcc_compile_string__x86_64_win32, tcc_add_library_path__x86_64_win32,
                tcc_output_file__x86_64_win32,
                "parts/hi-x86_64-win32.exe") == -1)
        failures++;

    /* --- cross: PE exe for windows-arm64 --- */
    if (emit_pe("arm64-win32",
                tcc_new__arm64_win32, tcc_delete__arm64_win32,
                tcc_set_lib_path__arm64_win32, tcc_set_error_func__arm64_win32,
                tcc_set_options__arm64_win32, tcc_set_output_type__arm64_win32,
                tcc_compile_string__arm64_win32, tcc_add_library_path__arm64_win32,
                tcc_output_file__arm64_win32,
                "parts/hi-arm64-win32.exe") == -1)
        failures++;

    /* --- cross: ELF obj for linux-arm64 --- */
    {
        TCCState *s = tcc_new__arm64();
        if (s) {
            tcc_set_error_func__arm64(s, 0, errcb);
            tcc_set_lib_path__arm64(s, ".");
            tcc_set_output_type__arm64(s, TCC_OUTPUT_OBJ);
            if (tcc_compile_string__arm64(s, src) == -1 ||
                tcc_output_file__arm64(s, "parts/hi-arm64.o") == -1) {
                fprintf(stderr, "FAIL: arm64 obj\n");
                failures++;
            } else {
                printf("arm64: wrote parts/hi-arm64.o\n");
            }
            tcc_delete__arm64(s);
        }
    }

    /* --- cross: ELF obj for linux-i386 --- */
    {
        TCCState *s = tcc_new__i386();
        if (s) {
            tcc_set_error_func__i386(s, 0, errcb);
            tcc_set_lib_path__i386(s, ".");
            tcc_set_output_type__i386(s, TCC_OUTPUT_OBJ);
            if (tcc_compile_string__i386(s, src) == -1 ||
                tcc_output_file__i386(s, "parts/hi-i386.o") == -1) {
                fprintf(stderr, "FAIL: i386 obj\n");
                failures++;
            } else {
                printf("i386: wrote parts/hi-i386.o\n");
            }
            tcc_delete__i386(s);
        }
    }

    printf(failures ? "RESULT: %d FAILURES\n" : "RESULT: ALL OK\n", failures);
    return failures != 0;
}
