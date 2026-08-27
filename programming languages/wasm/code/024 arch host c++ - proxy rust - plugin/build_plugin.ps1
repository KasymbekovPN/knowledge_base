# Windows-аналог build_plugin.sh -- та же самая последовательность
# команд (WIT -> C-биндинги -> core wasm-модуль -> компонент), только
# под PowerShell. ВНИМАНИЕ: в отличие от build_plugin.sh, этот скрипт
# НЕ прогнан вживую -- в этой (Linux-)песочнице нет Windows, чтобы его
# реально исполнить. Команды и флаги -- те же самые, что уже проверены
# в build_plugin.sh (wasi-sdk clang, wit-bindgen, wasm-tools кросс-
# платформенны и на Windows принимают идентичные аргументы), но сам
# .ps1 стоит прогнать и свериться с выводом при первом запуске.
#
# Нужны (все три -- обычные Windows-бинарники, .exe):
#   - wasi-sdk для Windows: https://github.com/WebAssembly/wasi-sdk/releases
#     (архив содержит bin\clang.exe с теми же таргетами wasm32-wasip1/wasip2)
#   - wit-bindgen-cli:  cargo install wit-bindgen-cli
#   - wasm-tools:       cargo install wasm-tools
#   (два последних ставятся через cargo одинаково что на Linux, что на
#   Windows -- crates.io кросс-платформенный)
#

param(
    #[string]$WasiSdkClang = $(if ($env:WASI_SDK_CLANG) { $env:WASI_SDK_CLANG } else { "C:\wasi-sdk\bin\clang.exe" }),
    [string]$WasiSdkClang = $(if ($env:WASI_SDK_CLANG) { $env:WASI_SDK_CLANG } else { "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" }),
    # Адаптер лежит прямо в пакете, рядом с этим скриптом
    # (plugin_src\adapters\) -- не нужно ничего скачивать отдельно.
    [string]$WasiAdapter  = $(if ($env:WASI_ADAPTER)  { $env:WASI_ADAPTER }  else { Join-Path $PSScriptRoot "adapters\wasi_snapshot_preview1.reactor.wasm" })
)

$ErrorActionPreference = "Stop"

$Root = $PSScriptRoot
Set-Location $Root

if (-not (Test-Path $WasiSdkClang)) {
    throw "clang.exe not found: $WasiSdkClang (pass -WasiSdkClang <path> or set env:WASI_SDK_CLANG)"
}
if (-not (Test-Path $WasiAdapter)) {
    throw "wasi_snapshot_preview1.reactor.wasm not found: $WasiAdapter (pass -WasiAdapter <path> or env:WASI_ADAPTER)"
}

New-Item -ItemType Directory -Force -Path "plugin_src\generated" | Out-Null

# 1) WIT -> биндинги на C.
& wit-bindgen c "wit\plugin.wit" --out-dir "plugin_src\generated"
if ($LASTEXITCODE -ne 0) { throw "wit-bindgen returned code $LASTEXITCODE" }

# 2) impl.c + сгенерированные биндинги -> core wasm-модуль (reactor,
#    без _start/main -- те же флаги, что в build_plugin.sh).
& $WasiSdkClang `
    --target=wasm32-wasip1 -mexec-model=reactor -nostartfiles "-Wl,--no-entry" `
    -I "plugin_src\generated" -O2 `
    "impl.c" "plugin_src\generated\plugin.c" "plugin_src\generated\plugin_component_type.o" `
    -o "plugin_src\plugin_core.wasm"
if ($LASTEXITCODE -ne 0) { throw "clang returned code $LASTEXITCODE" }

# 3) core-модуль -> компонент (адаптер нужен по той же причине, что и
#    в build_plugin.sh -- см. комментарий там; сам плагин WASI-импортов
#    не использует, это техническая деталь toolchain'а wasi-sdk).
& wasm-tools component new "plugin_src\plugin_core.wasm" `
    --adapt "wasi_snapshot_preview1=$WasiAdapter" `
    -o "plugin_component.wasm"
if ($LASTEXITCODE -ne 0) { throw "wasm-tools component new returned code $LASTEXITCODE" }

Write-Host "Done: $Root\plugin_component.wasm"
& wasm-tools validate "plugin_component.wasm" --features component-model
if ($LASTEXITCODE -ne 0) { throw "validation failed (code $LASTEXITCODE)" }