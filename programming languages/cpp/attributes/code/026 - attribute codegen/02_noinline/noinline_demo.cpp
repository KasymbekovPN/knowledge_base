__attribute__((noinline)) int noinlineAdd(int a, int b) {
    return a + b;
}

int callNoInline(int x) {
    return noinlineAdd(x, 10);
}

int plainAdd2(int a, int b) {
    return a + b;
}

int callPlain2(int x) {
    return plainAdd2(x, 10);
}

/*

g++ -O2 -S -o - 02_noinline/noinline_demo.cpp | c++filt

.file   "noinline_demo.cpp"
        .text
        .p2align 4
        .globl  noinlineAdd(int, int)
        .def    noinlineAdd(int, int);  .scl    2;      .type   32;     .endef
        .seh_proc       noinlineAdd(int, int)
noinlineAdd(int, int):
.LFB0:
        .seh_endprologue
        leal    (%rcx,%rdx), %eax
        ret
        .seh_endproc
        .p2align 4
        .globl  callNoInline(int)
        .def    callNoInline(int);      .scl    2;      .type   32;     .endef
        .seh_proc       callNoInline(int)
callNoInline(int):
.LFB1:
        .seh_endprologue
        movl    $10, %edx
        jmp     noinlineAdd(int, int)
        .seh_endproc
        .p2align 4
        .globl  plainAdd2(int, int)
        .def    plainAdd2(int, int);    .scl    2;      .type   32;     .endef
        .seh_proc       plainAdd2(int, int)
plainAdd2(int, int):
.LFB2:
        .seh_endprologue
        leal    (%rcx,%rdx), %eax
        ret
        .seh_endproc
        .p2align 4
        .globl  callPlain2(int)
        .def    callPlain2(int);        .scl    2;      .type   32;     .endef
        .seh_proc       callPlain2(int)
callPlain2(int):
.LFB3:
        .seh_endprologue
        leal    10(%rcx), %eax
        ret
        .seh_endproc
        .ident  "GCC: (Rev5, Built by MSYS2 project) 16.1.0"

*/