// Тот же to_upper, что и в plugin_str.c, но теперь память полностью
// на попечении Extism -- никаких ручных malloc/free "напрямую в хост".
#define EXTISM_IMPLEMENTATION
#define EXTISM_USE_LIBC

#include "prism-sdk.h"

int32_t EXTISM_EXPORTED_FUNCTION(to_upper) {
    size_t len = 0;
    // extism_load_input_dup сам: узнаёт длину входа, malloc'ит буфер нужного
    // размера, копирует туда байты из "Extism-памяти" -- без единого сырого
    // смещения в коде плагина.
    char* buf = extism_load_input_dup(&len);
    if (!buf) return 1;

    for (size_t i = 0; i < len; i++) {
        if (buf[i] >= 'a' && buf[i] <= 'z') {
            buf[i] -= 32;
        }
    }

    // Аллоцирует Extism-хендл, копирует туда buf и сообщает хосту, что это
    // и есть результат вызова -- один вызов вместо
    // malloc + store + output_set_from_handle по отдельности.
    extism_output_buf(buf, len);
    free(buf);

    return 0;
}
