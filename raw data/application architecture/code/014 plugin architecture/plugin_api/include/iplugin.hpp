// Стабильный контракт между хостом и плагинами. Это ЕДИНСТВЕННОЕ, что
// хост и плагин обязаны знать друг о друге - никаких общих CMake-целей,
// никакой совместной сборки, только этот заголовок с двух сторон.

#pragma once

// Версия API плагинов. Меняется ТОЛЬКО при несовместимых изменениях
// интерфейса IPlugin (добавили чисто виртуальный метод, поменяли
// сигнатуру существующего). Хост и плагин сверяют её до того, как
// хоть что-то вызвать друг у друга - это и есть versioning на практике,
// не просто номер в комментарии.
constexpr int PLUGIN_API_VERSION{1};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual const char* name() const = 0;
    virtual const char* execute(const char* input) = 0;
};

// Сигнатуры фабричных функций - точки входа в плагин. extern "C" убирает
// C++ name mangling: dlsym ищет функцию по простому строковому имени,
// а не по мангленному имени, которое зависит от компилятора/ABI и может
// отличаться даже между двумя сборками одним и тем же компилятором с
// разными флагами. Без extern "C" dlsym("createPlugin") почти наверняка
// ничего не найдёт.
extern "C" {
    using CreatePluginFn = IPlugin* (*)();
    using DestroyPluginFn = void (*)(IPlugin*);
    using GetApiVersionFn = int (*)();
}

#define PLUGIN_CREATE_FN_NAME "createPlugin"
#define PLUGIN_DESTROY_FN_NAME "destroyPlugin"
#define PLUGIN_API_VERSION_FN_NAME "pluginApiVersion"
