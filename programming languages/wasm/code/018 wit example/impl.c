/*

Реализация плагина поверх сгенерированных wit-bindgen биндингов.
Обрати внимание: никакого ручного malloc/free буфера под результат,
никакой упаковки (ptr<<32)|len -- это всё уже сделано генератором
(plugin_string_dup/plugin_string_set) и каноническим ABI компонент-модели.

*/

#include <string.h>
#include <stdlib.h>

#include "generated/plugin.h"

static int g_initialized = 0;

void exports_example_plugin_transform_init(void) {
    g_initialized = 1;
}

void exports_example_plugin_transform_info(exports_example_plugin_transform_plugin_info_t *ret) {
    plugin_string_set(&ret->name, "upper-wit-demo");
    ret->abi_version = 1;
}

bool exports_example_plugin_transform_process(plugin_string_t *input,
                                              plugin_string_t *ret,
                                              exports_example_plugin_transform_process_error_t *err) {
    // Внимание: сгенерированный шим делает `ret.is_err = !process(...)`
    // (см. generated/plugin.c) -- то есть возврат true здесь означает
    // Ok (заполнен ret), а false -- Err (заполнен err). Это НЕ то же
    // самое, что "true = есть ошибка", как можно интуитивно подумать.
    if (!g_initialized) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_NOT_INITIALIZED;
        return false;
    }

    if (input->len == 1) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_EMPTY_INPUT;
        return false;
    }

    // Та же трансформация, что в upper.c из Дня 8 -- но вход/выход уже
    // не (ptr,len) пара, а plugin_string_t, и хосту не нужно самому
    // звать plugin_alloc/plugin_free для этого буфера.
    char* buf = (char*)malloc(input->len);
    for (size_t i = 0; i < input->len; i++) {
        char c = ((char*)input->ptr)[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        buf[i] = c;
    }

    ret->ptr = (uint8_t*) buf;
    ret->len = input->len;
    return true;
}

void exports_example_plugin_transform_shutdown(void) {
    g_initialized = 0;
}
