// Тот же контракт example:plugin/transform@0.1.0, что в impl.c, но
// реализация уже на C++: состояние плагина -- класс, а не static int,
// трансформация -- std::string + std::transform, а не ручной цикл по
// char*. Единственное, что должно остаться "по-сишному" -- сигнатуры
// экспортируемых функций: они обязаны совпадать с тем, что сгенерировал
// wit-bindgen в plugin.h, и должны быть extern "C" (иначе линковщик не
// найдёт символ example:plugin/transform@0.1.0#process -- C++ исказит
// имя мэнглингом, как мы уже ловили на Extism-плагине).

#include <algorithm>
#include <cctype>
#include <string>

#include "generated/plugin.h"

namespace {
    // Состояние плагина как обычный C++ класс -- никакой разницы с "хостовым"
    // C++, кроме того, что живёт это всё внутри памяти WASM-инстанса.
    class TransformPlugin {
    public:
        void init() { initialized_ = true; }

        [[nodiscard]] bool initialized() const { return initialized_; }

        [[nodiscard]] std::string toUpper(const std::string& input) const {
            std::string out{input};
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
              return static_cast<char>(std::toupper(c));
            });
            return out;
        }

        void shutdown() { initialized_ = false; }

    private:
        bool initialized_{false};
    };

    // Единственный экземпляр состояния на весь инстанс WASM-модуля -- ровно
    // та же модель, что и static int g_initialized в C-версии, просто
    // обёрнутая в класс.
    TransformPlugin g_plugin;
}

extern "C" {

void exports_example_plugin_transform_init(void) { g_plugin.init(); }

void exports_example_plugin_transform_info(exports_example_plugin_transform_plugin_info_t *ret) {
    // plugin_string_dup копирует C-строку через тот же аллокатор
    // (cabi_realloc), которым пользуется сгенерированный код -- так что
    // post_return потом сможет её корректно освободить через free().
    plugin_string_dup(&ret->name, "upper-wit-demo-cpp");
    ret->abi_version = 1;
}

bool exports_example_plugin_transform_process(plugin_string_t *input,
                                              plugin_string_t *ret,
                                              exports_example_plugin_transform_process_error_t *err) {
    // Как и в C-версии: true здесь означает Ok (см. сгенерированный шим
    // __wasm_export_..._process в plugin.c -- `ret.is_err = !process(...)`).
    if (!g_plugin.initialized()) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_NOT_INITIALIZED;
        return false;
    }
    if (input->len == 0) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_EMPTY_INPUT;
        return false;
    }

    const std::string text{reinterpret_cast<const char *>(input->ptr), input->len};
    const std::string upper{g_plugin.toUpper(text)};

    // plugin_string_dup_n сам выделяет буфер нужного размера через
    // cabi_realloc и копирует туда данные -- аналог plugin_alloc+memcpy
    // из ручного ABI, но уже сгенерированный, а не написанный руками.
    plugin_string_dup_n(ret, upper.data(), upper.size());

    return true;
}

void exports_example_plugin_transform_shutdown(void) {
    g_plugin.shutdown();
}

}
