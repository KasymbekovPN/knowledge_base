#!/usr/bin/env bash
# Пересобирает plugin_component.wasm из wit/plugin.wit + impl.c с нуля.
# Проверено вживую в этой сессии -- результат побайтово совпадает (с
# точностью до незначащих отличий) с тем, что уже лежит в
# ../plugin_component.wasm.
#
# Нужны: wasi-sdk (clang с таргетами wasm32-wasip1/wasip2),
# wit-bindgen-cli, wasm-tools -- все три уже стоят в этой сессии.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLANG="${WASI_SDK_CLANG:-/home/claude/wasi-sdk/bin/clang}"
# Адаптер лежит прямо в пакете (plugin_src/adapters/) -- раньше здесь
# был абсолютный путь внутрь этой песочницы (/home/claude/wit_plugin/...),
# который на машине пользователя не существует. WASI_SDK_CLANG всё ещё
# указывает в песочницу по умолчанию -- сам wasi-sdk в пакет не входит
# (слишком большой), его нужно поставить отдельно и передать путь через
# env/-параметр.
ADAPTER="${WASI_ADAPTER:-$ROOT/plugin_src/adapters/wasi_snapshot_preview1.reactor.wasm}"

cd "$ROOT"

# 1) WIT -> биндинги на C (generated/plugin.c, generated/plugin.h,
#    generated/plugin_component_type.o -- последний хранит секцию с
#    описанием типа компонента, без него wasm-tools не поймёт, что
#    экспортирует core-модуль).
wit-bindgen c wit/plugin.wit --out-dir plugin_src/generated

# 2) impl.c + сгенерированные биндинги -> core wasm-модуль в
#    reactor-режиме (без _start/main, как и все плагины в этой сессии).
"$CLANG" --target=wasm32-wasip1 -mexec-model=reactor -nostartfiles -Wl,--no-entry \
  -I plugin_src/generated -O2 \
  plugin_src/impl.c plugin_src/generated/plugin.c plugin_src/generated/plugin_component_type.o \
  -o plugin_src/plugin_core.wasm

# 3) core-модуль -> настоящий компонент. Адаптер нужен, потому что
#    wasi-sdk даже для чистого C реактора тянет пути через
#    wasi_snapshot_preview1 (abort/exit); он конвертирует их в
#    компонентные wasi:cli/wasi:io импорты. Сам plugin_component.wasm
#    их при этом не импортирует вообще (см. `wasm-tools component wit`) --
#    адаптер тут просто техническая необходимость toolchain'а wasi-sdk,
#    а не то, что реально требуется в рантайме.
wasm-tools component new plugin_src/plugin_core.wasm \
  --adapt wasi_snapshot_preview1="$ADAPTER" \
  -o plugin_component.wasm

echo "Готово: $ROOT/plugin_component.wasm"
wasm-tools validate plugin_component.wasm --features component-model