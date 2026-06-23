---
tags:
  - build
---
[[programming languages/vcpkg/_|<=]]

## VS Code

Подключение vcpkg происходит **не через расширение, а через настройку**. Два основных способа:

Через `CMakePresets.json` (рекомендуемый, переносимый):

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "default",
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    }
  ]
}
```

Или через настройки CMake Tools (`settings.json`):

```json
{
  "cmake.configureSettings": {
    "CMAKE_TOOLCHAIN_FILE": "${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
  }
}
```

Есть сторонние расширения вроде «vcpkg» в маркетплейсе, но они в основном дают мелкие удобства (автоподстановка пути, helper-команды). Для нормальной работы они не обязательны — toolchain-файл решает всё.

## CLion

Что касается vcpkg, в CLion есть **встроенная интеграция с vcpkg** (появилась в версии 2023.1). Через неё можно:

- управлять vcpkg прямо из IDE (View → Tool Windows → vcpkg или через настройки),
- добавлять/устанавливать пакеты из UI,
- IDE сама пропишет toolchain-файл в CMake-профиль.

То есть **никакого плагина ставить не надо** — функциональность уже в коробке. Если версия CLion старше 2023.1, тогда подключаете vcpkg вручную через CMake options:

```
-DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

в Settings → Build, Execution, Deployment → CMake.

## Итог

| |Нужны расширения для vcpkg?|Что ставить|
|---|---|---|
|**VS Code**|Плагинов именно под vcpkg нет и не требуется. Нужны общие C++ расширения|C/C++ + CMake Tools, дальше toolchain-файл|
|**CLion**|Ничего ставить не нужно|Встроенная поддержка vcpkg (2023.1+)|

Главная мысль: vcpkg подключается через CMake toolchain, а не через IDE-плагины. Расширения нужны лишь для комфортной работы с C++/CMake как таковыми.
