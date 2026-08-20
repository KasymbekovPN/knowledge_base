/*

Плагин 3: подсчёт слов, результат -- десятичная строка ("3", "12"...).
Тот же контракт, но выход не совпадает по длине со входом -- проверка,
что паттерн "указатель + длина" в контракте не завязан на in_len==out_len.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugins/wordcount.wasm wordcount.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_ABI_VERSION 1

static int g_initialized = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_initialized = 1;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

static int32_t count_words(const char* s, int32_t len) {
    int32_t count = 0;
    int in_word = 0;
    for (int32_t i = 0; i < len; i++) {
        int is_space = (s[i] == ' ' || s[i] == '\t' || s[i] == '\n');
        if (!is_space && !in_word) {
            count++;
            in_word = 1;
        } else {
            in_word = 0;
        }
    }

    return count;
}

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_initialized) return 0;

    int32_t n = count_words(in_ptr, in_len);

    // Ручная конвертация числа в ASCII-десятичную строку -- без snprintf,
    // чтобы не тянуть лишнего в такой мелкий плагин.
    char digits[12];
    int32_t digitCount = 0;
    if (n == 0) {
        digits[digitCount++] = '0';
    } else {
        int32_t tmp = n;
        while (tmp > 0) {
            digits[digitCount++] = (char)('0' + (tmp % 10));
            tmp /= 10;
        }
    }

    char *out = (char*)plugin_alloc(digitCount);
    if (out == NULL) return 0;
    for (int32_t i = 0; i < digitCount; i++) {
        // реверс, т.к. цифры набирались с конца
        out[i] = digits[digitCount - 1 - i];
    }

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)digitCount;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_initialized = 0; }
