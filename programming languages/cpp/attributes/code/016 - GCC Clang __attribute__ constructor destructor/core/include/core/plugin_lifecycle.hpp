#pragma once

// Кроссплатформенный способ выполнить код автоматически при загрузке и
// при выгрузке плагина (.so на Linux/macOS, .dll на Windows), без
// явного вызова init-функции извне.
//
// - GCC/Clang (Linux, macOS, и MinGW на Windows):
//   __attribute__((constructor))/((destructor)) — понимается всеми
//   компиляторами на базе GCC/Clang вне зависимости от целевой ОС.
// - Windows БЕЗ MinGW (обычный cl.exe, а также clang/clang-cl,
//   таргетящиеся на MSVC ABI): __attribute__((constructor)) у clang
//   на этом таргете действительно вызывается при DLL_PROCESS_ATTACH
//   (через .CRT$XCU), а вот __attribute__((destructor)) НЕ подключён
//   к DLL_PROCESS_DETACH — деструктор просто никогда не вызывается
//   (проверено: registerPlugin срабатывает, unregisterPlugin — нет,
//   ни при FreeLibrary, ни при завершении процесса). Поэтому здесь
//   нужен DllMain — единственная гарантированная точка входа, которую
//   вызывает загрузчик Windows при LoadLibrary/FreeLibrary, — для
//   ЛЮБОГО компилятора, если сборка не под MinGW.
//
// ВАЖНЫЙ WINDOWS-НЮАНС: код внутри DllMain выполняется под loader lock —
// нельзя грузить другие DLL через LoadLibrary, создавать/join'ить потоки
// и т.п. (см. "DllMain restrictions" в документации Microsoft). Простая
// регистрация в структуре данных внутри своего процесса (как здесь)
// безопасна, но при усложнении плагина об этом ограничении важно помнить.
//
// PLUGIN_LIFECYCLE(OnLoad, OnUnload) объявляет OnLoad()/OnUnload() как
// обычные static-функции; их тела пишутся отдельно как определения.

#if defined(_WIN32) && !defined(__MINGW32__)

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #define PLUGIN_LIFECYCLE(OnLoad, OnUnload)\
        static void OnLoad();\
        static void OnUnload();\
        extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {\
            if (reason == DLL_PROCESS_ATTACH) { OnLoad(); }\
            else if (reason == DLL_PROCESS_DETACH) { OnUnload(); }\
            return TRUE;\
        }

#else

    #define PLUGIN_LIFECYCLE(OnLoad, OnUnload) \
        static void OnLoad();\
        static void OnUnload();\
        __attribute__((constructor)) static void OnLoad##_ctor_trampoline() { \
            OnLoad(); \
        } \
        __attribute__((destructor)) static void OnUnload##_ctor_trampoline() { \
            OnUnload(); \
        } \

#endif
