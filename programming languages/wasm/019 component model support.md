---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

В таблице стабильности WASM-предложений (`stability-wasm-proposals`) Component Model для Rust API отмечен как полностью поддерживаемый (✅), а для C API — как «work-in-progress» (🚧), с прямой формулировкой «mostly supported in the C API but gaps remain» и ссылкой на открытые issue, которые эти пробелы отслеживают. То есть это не домыслы, а официально признанный разрыв в зрелости между Rust- и C/C++-обвязкой одного и того же движка.

По WAMR — картина ещё менее определённая: в его README Component Model вообще не упоминается среди поддерживаемых post-MVP фич (там перечислены SIMD, Reference Types, GC, exception handling и т.д., а компонент-модели нет), и независимый обзор состояния экосистемы (WASI/Component Model, февраль 2025) описывает WAMR как рантайм, сфокусированный на минимальном footprint и WASI Preview1, без акцента на Component Model — в отличие от Wasmtime, который там прямо назван первым рантаймом с полной поддержкой загрузки компонентов.

И это не абстрактная оговорка — мы буквально на это наткнулись в `host_component.cc`: полярность `bool` у сгенерированного шима (`ret.is_err = !process(...)`) нигде явно не задокументирована в заголовках, я понял её только разобрав сгенерированный `plugin.c` руками и поймав баг по факту неверного вывода. Для Rust-стороны (родной язык Component Model в этой экосистеме) такого рода сюрпризов в биндингах ожидается ощутимо меньше — там API считается основным и обкатанным.

Практический вывод для реального проекта с плагинами на C++: если Component Model обязателен (например, важна кросс-языковая интероперабельность плагинов), стоит на момент принятия решения заново свериться с текущим состоянием issue-трекера Wasmtime C API и с тем, какой рантайм вообще выбирается — картина меняется быстро, эта информация протухает за месяцы, а не годы. Если это не обязательное требование — «ручной» core-module ABI, который мы строили до Дня 8 (`plugin_abi.h` + `Module`/`Instance`), полностью зрелый, стабильный и не зависит от этого разрыва вообще, поскольку не использует Component Model.

Sources:

- [Wasm Proposals - Wasmtime](https://docs.wasmtime.dev/stability-wasm-proposals.html)
- [wasm-micro-runtime/README.md at main · bytecodealliance/wasm-micro-runtime](https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/README.md)
- [WASI and the WebAssembly Component Model: Current Status | eunomia](https://eunomia.dev/blog/2025/02/16/wasi-and-the-webassembly-component-model-current-status/)
