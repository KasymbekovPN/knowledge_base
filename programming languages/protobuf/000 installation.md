---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

**Linux**

Debian/Ubuntu:

```
sudo apt update
sudo apt install protobuf-compiler libprotobuf-dev
protoc --version
```

Fedora/RHEL:

```
sudo dnf install protobuf-compiler protobuf-devel
```

Arch:

```
sudo pacman -S protobuf
```

Через vcpkg (любой дистрибутив):

```
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install protobuf
```

Через Conan:

```
pip install conan
conan install protobuf/<version>@ --build=missing
```

Сборка из исходников (нужна, если требуется свежая версия, которой нет в репозиториях дистрибутива):

```
git clone --recurse-submodules https://github.com/protocolbuffers/protobuf
cd protobuf
cmake . -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF
cmake --build . --parallel
sudo cmake --install .
```

**Windows**

```
winget install --id Google.Protobuf -e
```

Через vcpkg (рекомендуемый способ для C++/CMake):

```
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install protobuf:x64-windows
```

Дальше в CMake подключается через `-DCMAKE_TOOLCHAIN_FILE=<путь>/vcpkg/scripts/buildsystems/vcpkg.cmake`.

Через Conan:

```
pip install conan
conan install protobuf/<version>@ --build=missing -s arch=x86_64
```

Через Chocolatey (только сам компилятор protoc, без dev-библиотек для линковки):

```
choco install protoc
```

Сборка из исходников (Visual Studio + CMake):

```
git clone --recurse-submodules https://github.com/protocolbuffers/protobuf
cd protobuf
cmake . -G "Visual Studio 17 2022" -A x64 -Dprotobuf_BUILD_TESTS=OFF
cmake --build . --config Release
cmake --install . --config Release
```

Практический совет: если проект уже на CMake, проще всего взять protobuf через vcpkg (или Conan) — тогда `find_package(Protobuf REQUIRED)` и `protobuf_generate_cpp()` сразу подхватят пути к protoc и библиотекам без ручной настройки, одинаково на Linux и Windows.
