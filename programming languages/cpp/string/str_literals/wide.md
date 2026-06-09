---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/str_literals/_|<=]]


```cpp
const wchar_t* wide_str = L"こんにちは";
```

- Используется для работы с широкими символами.
- Размер `wchar_t` зависит от платформы:
  - **Windows**: 2 байта (UTF-16),
  - **Unix/Linux**: 4 байта (UTF-32).
