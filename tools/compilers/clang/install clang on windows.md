---
tags:
  - install
  - tool
  - clang
  - compiler
  - windows
  - llvm
---
[[__clang index__|<=]]

[Первая программа на Windows. Компилятор Clang](https://metanit.com/cpp/tutorial/1.8.php)
[Getting Started: Building and Running Clang](https://clang.llvm.org/get_started.html)
[How to Install Clang on Windows](https://www.wikihow.com/Install-Clang-on-Windows)
[Поддержка Clang и LLVM в проектах Visual Studio](https://learn.microsoft.com/ru-ru/cpp/build/clang-support-msbuild?view=msvc-170)
[llvm on github]( [https://github.com/llvm/llvm-project](https://github.com/llvm/llvm-project))

Для загрузки установщика под Windows перейдем на страницу релизов в данном репозитории по ссылке [https://github.com/llvm/llvm-project/releases/](https://github.com/llvm/llvm-project/releases/)

Скачиваем и устанавливаем LLVM-XX.X.X-win64.exe

Чтобы проверить установку Clang, в терминале/командной строке следует ввести команду clang --version. Результатом должно быть:
```
clang version 18.1.8
Target: x86_64-pc-windows-msvc
Thread model: posix
InstalledDir: C:\Program Files\LLVM\bin
```
