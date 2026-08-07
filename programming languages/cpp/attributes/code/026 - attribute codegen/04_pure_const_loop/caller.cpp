extern int global;
__attribute__((pure)) int pureRead(int x);
__attribute__((const)) int constCompute(int x);

int loopPureRead(int x, int n) {
    int sum{};
    for (int i{}; i < n; i++) {
        global = i;
        sum += pureRead(x);
    }

    return sum;
}

int loopConst(int x, int n) {
    int sum{};
    for (int i{}; i < n; i++) {
        global = i;
        sum += constCompute(x);
    }
    return sum;
}

/*

g++ -O2 -S -o - 04_pure_const_loop/caller.cpp | c++filt


        .file   "caller.cpp"
        .text
        .p2align 4
        .globl  loopPureRead(int, int)
        .def    loopPureRead(int, int); .scl    2;      .type   32;     .endef
        .seh_proc       loopPureRead(int, int)
loopPureRead(int, int):
.LFB0:
        pushq   %r12
        .seh_pushreg    %r12
        pushq   %rbp
        .seh_pushreg    %rbp
        pushq   %rdi
        .seh_pushreg    %rdi
        pushq   %rsi
        .seh_pushreg    %rsi
        pushq   %rbx
        .seh_pushreg    %rbx
        subq    $32, %rsp
        .seh_stackalloc 32
        .seh_endprologue
        movl    %ecx, %ebp
        movl    %edx, %edi
        testl   %edx, %edx
        jle     .L4
        movq    .refptr.global(%rip), %r12
        xorl    %ebx, %ebx
        xorl    %esi, %esi
        .p2align 4
        .p2align 3
.L3:
        movl    %ebp, %ecx
        movl    %ebx, (%r12)
        addl    $1, %ebx
        call    pureRead(int)
        addl    %eax, %esi
        cmpl    %ebx, %edi
        jne     .L3
        movl    %esi, %eax
        addq    $32, %rsp
        popq    %rbx
        popq    %rsi
        popq    %rdi
        popq    %rbp
        popq    %r12
        ret
        .p2align 4,,10
        .p2align 3
.L4:
        xorl    %esi, %esi
        movl    %esi, %eax
        addq    $32, %rsp
        popq    %rbx
        popq    %rsi
        popq    %rdi
        popq    %rbp
        popq    %r12
        ret
        .seh_endproc
        .p2align 4
        .globl  loopConst(int, int)
        .def    loopConst(int, int);    .scl    2;      .type   32;     .endef
        .seh_proc       loopConst(int, int)
loopConst(int, int):
.LFB1:
        pushq   %rbp
        .seh_pushreg    %rbp
        pushq   %rdi
        .seh_pushreg    %rdi
        pushq   %rsi
        .seh_pushreg    %rsi
        pushq   %rbx
        .seh_pushreg    %rbx
        subq    $40, %rsp
        .seh_stackalloc 40
        .seh_endprologue
        movl    %ecx, %ebp
        movl    %edx, %edi
        testl   %edx, %edx
        jle     .L10
        xorl    %ebx, %ebx
        xorl    %esi, %esi
        .p2align 4
        .p2align 3
.L9:
        movl    %ebp, %ecx
        call    constCompute(int)
        addl    %eax, %esi
        movl    %ebx, %eax
        addl    $1, %ebx
        cmpl    %ebx, %edi
        jne     .L9
        movq    .refptr.global(%rip), %rdx
        movl    %eax, (%rdx)
        movl    %esi, %eax
        addq    $40, %rsp
        popq    %rbx
        popq    %rsi
        popq    %rdi
        popq    %rbp
        ret
        .p2align 4,,10
        .p2align 3
.L10:
        xorl    %esi, %esi
        movl    %esi, %eax
        addq    $40, %rsp
        popq    %rbx
        popq    %rsi
        popq    %rdi
        popq    %rbp
        ret
        .seh_endproc
        .ident  "GCC: (Rev5, Built by MSYS2 project) 16.1.0"
        .def    pureRead(int);  .scl    2;      .type   32;     .endef
        .def    constCompute(int);      .scl    2;      .type   32;     .endef
        .section        .rdata$.refptr.global, "dr"
        .p2align        3, 0
        .globl  .refptr.global
        .linkonce       discard
.refptr.global:
        .quad   global

*/