---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

**Linux (apt, Ubuntu/Debian-подобные) — проверено вживую**

```bash
sudo apt update
sudo apt install -y protobuf-compiler-grpc libgrpc++-dev
```

Ставит `grpc_cpp_plugin` (в `/usr/bin`), заголовки (`grpcpp/grpcpp.h` и т.д.) и `libgrpc++.so`/`libgrpc.so`. Версия в репозитории Ubuntu 22.04 — **1.30.2** (2020 год, довольно старая, но полностью рабочая для учебных целей). Если нужна свежая версия — через vcpkg/Conan или сборку из исходников (см. ниже).

Fedora/RHEL:

```bash
sudo dnf install grpc-devel grpc-plugins
```

Arch:

```bash
sudo pacman -S grpc
```

**Через vcpkg (рекомендуется, если нужна свежая версия и одинаковый способ на Linux/Windows)**

```bash
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh        # Linux/macOS
.\vcpkg\bootstrap-vcpkg.bat        # Windows

./vcpkg/vcpkg install grpc                 # Linux
.\vcpkg\vcpkg install grpc:x64-windows      # Windows
```

vcpkg сам подтянет все зависимости gRPC (protobuf, abseil, c-ares, re2, zlib, BoringSSL/OpenSSL) и соберёт их — учти, что это тяжелее, чем сборка одного protobuf: gRPC заметно больше по объёму кода и зависимостей, полная сборка с нуля может занять заметно дольше (десятки минут на слабой машине), чем protobuf в одиночку.

**Через Conan**

```bash
pip install conan
conan install grpc/<version>@ --build=missing
```

**Сборка из исходников (Linux и Windows)**

```bash
git clone --recurse-submodules -b v1.6x.x https://github.com/grpc/grpc
cd grpc
mkdir -p cmake/build && cd cmake/build
cmake ../.. -DgRPC_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

На Windows та же последовательность команд в `cmd`/PowerShell с Visual Studio генератором (`cmake ../.. -G "Visual Studio 17 2022" -A x64 -DgRPC_BUILD_TESTS=OFF`), либо через MSYS2 (`pacman -S mingw-w64-x86_64-grpc`), аналогично тому, как раньше ставили protobuf.

**Windows (без сборки из исходников) — практические варианты**

1. **vcpkg** — основной рекомендуемый путь, см. выше.
2. **MSYS2**: `pacman -S mingw-w64-x86_64-grpc mingw-w64-x86_64-protobuf` — даёт `grpc_cpp_plugin.exe`, заголовки и `.dll`/`.a` сразу, без ручной сборки.
3. **Conan** — так же, как на Linux.

**Проверка после установки**

```bash
which grpc_cpp_plugin
grpc_cpp_plugin --help    # покажет опции генератора, а не версию (проверено выше)
```

Дальше кодогенерация будет выглядеть так:

```bash
protoc -I. --cpp_out=. --grpc_out=. \
  --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
  service.proto
```

`--plugin=protoc-gen-grpc=<путь>` — обязательная часть команды, protoc сам не знает про grpc-плагин, нужно явно указать, где искать исполняемый файл `grpc_cpp_plugin`.

---

**Минимальный `vcpkg.json`**

```json
{
  "name": "grpc-cpp-demo",
  "version": "1.0.0",
  "dependencies": [
    "grpc",
    "protobuf"
  ]
}
```

**Почему так**

Сам порт `grpc` в vcpkg (проверил актуальный `ports/grpc/vcpkg.json` из репозитория, текущая версия портфеля — 1.81.1) уже указывает `protobuf`, `abseil`, `openssl`, `c-ares`, `re2`, `zlib` как свои зависимости — vcpkg подтянет их автоматически. Явно прописывать `protobuf` в своём `vcpkg.json` не обязательно (придёт транзитивно), но на практике так делают почти всегда — твой код напрямую использует `protobuf::libprotobuf` (для сообщений, не только для RPC), так что зависимость должна быть явной и в `CMakeLists.txt`.

Важная деталь из реального манифеста: порт `grpc` содержит **самоссылающуюся** host-зависимость —

```json
{
  "name": "grpc",
  "host": true,
  "features": ["codegen"]
}
```

Это гарантирует, что генератор кода (`grpc_cpp_plugin`) всегда собирается для хост-платформы, даже при кросс-компиляции под другую архитектуру. Из этого следует практический вывод: просто `"grpc"` в зависимостях уже даёт тебе и библиотеку, и плагин кодогенерации — отдельно указывать `grpc[codegen]` не требуется, это встроено в порт безусловно.

**Что это даёт в CMake**

```cmake
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Библиотеки:
#   protobuf::libprotobuf, protobuf::protoc
#   gRPC::grpc++, gRPC::grpc
# Исполняемый плагин кодогенерации (проверил в gRPCTargets-vcpkg-tools.cmake
# из порта — таргет создаётся динамически по маске grpc_*_plugin*):
#   gRPC::grpc_cpp_plugin
```

Дальше кодогенерация через `add_custom_command` с `$<TARGET_FILE:gRPC::grpc_cpp_plugin>` — тот же паттерн, что мы использовали с `protobuf::protoc` в демо с `FetchContent`, просто с дополнительным `--plugin=protoc-gen-grpc=...` флагом.
