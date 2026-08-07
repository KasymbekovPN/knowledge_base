// Только объявления — тела в funcs.cpp, в отдельной единице трансляции.
// Это важно: если бы caller.cpp видел сами тела, GCC вывел бы отсутствие
// побочных эффектов самостоятельно (IPA pure-const анализ), и эффект
// атрибутов был бы незаметен на фоне этого. Раздельная компиляция
// заставляет компилятор доверять именно объявленному атрибуту.

int normalFunc(int x);
__attribute__((pure)) int pureFunc(int x);
__attribute__((const)) int constFunc(int x);

int testNormal(int x) {
    return normalFunc(x) + normalFunc(x);
}

int testPure(int x) {
    return pureFunc(x) + pureFunc(x);
}

int testConst(int x) {
    return constFunc(x) + constFunc(x);
}

/*

g++ -O2 -S -o - 03_pure_const_cse/caller.cpp | c++filt

.file   "caller.cpp"
        .text
        .p2align 4
        .globl  testNormal(int)
        .def    testNormal(int);        .scl    2;      .type   32;     .endef
        .seh_proc       testNormal(int)
testNormal(int):
.LFB0:
        pushq   %rsi
        .seh_pushreg    %rsi
        pushq   %rbx
        .seh_pushreg    %rbx
        subq    $40, %rsp
        .seh_stackalloc 40
        .seh_endprologue
        movl    %ecx, %ebx
        call    normalFunc(int)
        movl    %ebx, %ecx
        movl    %eax, %esi
        call    normalFunc(int)
        addl    %esi, %eax
        addq    $40, %rsp
        popq    %rbx
        popq    %rsi
        ret
        .seh_endproc
        .p2align 4
        .globl  testPure(int)
        .def    testPure(int);  .scl    2;      .type   32;     .endef
        .seh_proc       testPure(int)
testPure(int):
.LFB1:
        subq    $40, %rsp
        .seh_stackalloc 40
        .seh_endprologue
        call    pureFunc(int)
        addl    %eax, %eax
        addq    $40, %rsp
        ret
        .seh_endproc
        .p2align 4
        .globl  testConst(int)
        .def    testConst(int); .scl    2;      .type   32;     .endef
        .seh_proc       testConst(int)
testConst(int):
.LFB2:
        subq    $40, %rsp
        .seh_stackalloc 40
        .seh_endprologue
        call    constFunc(int)
        addl    %eax, %eax
        addq    $40, %rsp
        ret
        .seh_endproc
        .ident  "GCC: (Rev5, Built by MSYS2 project) 16.1.0"
        .def    normalFunc(int);        .scl    2;      .type   32;     .endef
        .def    pureFunc(int);  .scl    2;      .type   32;     .endef
        .def    constFunc(int); .scl    2;      .type   32;     .endef

*/