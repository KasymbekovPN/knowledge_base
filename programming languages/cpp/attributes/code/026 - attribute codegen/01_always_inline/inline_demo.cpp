__attribute__((always_inline)) inline int alwaysInlineAdd(int a, int b) {
    return a + b;
}

int callAlwaysInline(int x) {
    return alwaysInlineAdd(x, 10);
}

int plainAdd(int a, int b) {
    return a + b;
}

int callPlain(int a) {
    return plainAdd(a, 10);
}

/*

g++ -O0 -S -o - 01_always_inline/inline_demo.cpp | c++filt

.file   "inline_demo.cpp"
        .text
        .globl  callAlwaysInline(int)
        .def    callAlwaysInline(int);  .scl    2;      .type   32;     .endef
        .seh_proc       callAlwaysInline(int)
callAlwaysInline(int):
.LFB1:
        pushq   %rbp
        .seh_pushreg    %rbp
        movq    %rsp, %rbp
        .seh_setframe   %rbp, 0
        subq    $16, %rsp
        .seh_stackalloc 16
        .seh_endprologue
        movl    %ecx, 16(%rbp)
        movl    16(%rbp), %eax
        movl    %eax, -4(%rbp)
        movl    $10, -8(%rbp)
        movl    -4(%rbp), %edx
        movl    -8(%rbp), %eax
        addl    %edx, %eax
        addq    $16, %rsp
        popq    %rbp
        ret
        .seh_endproc
        .globl  plainAdd(int, int)
        .def    plainAdd(int, int);     .scl    2;      .type   32;     .endef
        .seh_proc       plainAdd(int, int)
plainAdd(int, int):
.LFB2:
        pushq   %rbp
        .seh_pushreg    %rbp
        movq    %rsp, %rbp
        .seh_setframe   %rbp, 0
        .seh_endprologue
        movl    %ecx, 16(%rbp)
        movl    %edx, 24(%rbp)
        movl    16(%rbp), %edx
        movl    24(%rbp), %eax
        addl    %edx, %eax
        popq    %rbp
        ret
        .seh_endproc
        .globl  callPlain(int)
        .def    callPlain(int); .scl    2;      .type   32;     .endef
        .seh_proc       callPlain(int)
callPlain(int):
.LFB3:
        pushq   %rbp
        .seh_pushreg    %rbp
        movq    %rsp, %rbp
        .seh_setframe   %rbp, 0
        subq    $32, %rsp
        .seh_stackalloc 32
        .seh_endprologue
        movl    %ecx, 16(%rbp)
        movl    16(%rbp), %eax
        movl    $10, %edx
        movl    %eax, %ecx
        call    plainAdd(int, int)
        addq    $32, %rsp
        popq    %rbp
        ret
        .seh_endproc
        .ident  "GCC: (Rev5, Built by MSYS2 project) 16.1.0"

*/