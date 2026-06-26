---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Filesystem

Boost.Filesystem — работа с путями, файлами и директориями на разных ОС. **Требует линковки** (`Boost::filesystem`). Прообраз `std::filesystem` (C++17).

```cpp
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(app PRIVATE Boost::filesystem)
```

## 1. Класс `path`

Представление пути в файловой системе (с учётом особенностей ОС).

### Конструкторы

|Конструктор|Описание|
|---|---|
|`path()`|Пустой путь|
|`path(const string& s)`|Из строки|
|`path(const char* s)`|Из C-строки|
|`path(InputIterator, InputIterator)`|Из диапазона символов|

### Методы — разбор пути

|Метод|Описание|
|---|---|
|`filename()`|Имя файла с расширением (`/a/b.txt` → `b.txt`)|
|`stem()`|Имя без расширения (`b.txt` → `b`)|
|`extension()`|Расширение (`b.txt` → `.txt`)|
|`parent_path()`|Родительский каталог (`/a/b.txt` → `/a`)|
|`root_name()`|Корневое имя (`C:` на Windows)|
|`root_directory()`|Корневой разделитель (`/`)|
|`root_path()`|`root_name()` + `root_directory()`|
|`relative_path()`|Путь после корня|
|`is_absolute()` / `is_relative()`|Тип пути|
|`has_filename()`, `has_extension()`, ...|Проверки наличия компонентов|

### Методы — изменение пути

|Метод|Описание|
|---|---|
|`operator/=` / `operator/`|Соединить компоненты с разделителем|
|`operator+=`|Добавить без разделителя|
|`append(...)`|Добавить компонент|
|`replace_extension(ext)`|Заменить расширение|
|`remove_filename()`|Убрать имя файла|
|`clear()`|Очистить|
|`make_preferred()`|Привести разделители к «родным» для ОС|

### Методы — конвертация

|Метод|Описание|
|---|---|
|`string()`|В `std::string` (UTF-8/системная кодировка)|
|`wstring()`|В `std::wstring`|
|`generic_string()`|С разделителями `/` независимо от ОС|
|`c_str()`|Сырой указатель|
|`native()`|Нативное представление|

```cpp
fs::path p = "/home/user/doc.txt";
std::cout << p.filename()    << "\n"; // doc.txt
std::cout << p.stem()        << "\n"; // doc
std::cout << p.extension()   << "\n"; // .txt
std::cout << p.parent_path() << "\n"; // /home/user

fs::path full = fs::path("/home") / "user" / "file.txt"; // соединение через /
```

## 2. Запросы о статусе (свободные функции)

|Функция|Описание|
|---|---|
|`exists(p)`|Существует ли путь|
|`is_regular_file(p)`|Обычный файл?|
|`is_directory(p)`|Каталог?|
|`is_symlink(p)`|Символическая ссылка?|
|`is_empty(p)`|Пустой файл/каталог?|
|`file_size(p)`|Размер файла в байтах|
|`last_write_time(p)`|Время последней модификации|
|`status(p)`|Объект `file_status`|
|`equivalent(p1, p2)`|Указывают ли на один объект ФС|
|`space(p)`|Сведения о свободном/общем месте (`space_info`)|

## 3. Операции с файлами и каталогами

|Функция|Описание|
|---|---|
|`create_directory(p)`|Создать один каталог|
|`create_directories(p)`|Создать всю цепочку каталогов|
|`copy(from, to)`|Копировать файл/каталог|
|`copy_file(from, to)`|Копировать файл|
|`copy_file(from, to, option)`|С опцией перезаписи (`copy_options`)|
|`rename(from, to)`|Переименовать/переместить|
|`remove(p)`|Удалить файл/пустой каталог|
|`remove_all(p)`|Рекурсивно удалить (возвращает число удалённых)|
|`resize_file(p, n)`|Изменить размер файла|
|`create_symlink(target, link)`|Создать символьную ссылку|
|`create_hard_link(target, link)`|Создать жёсткую ссылку|
|`permissions(p, perms)`|Изменить права доступа|

> ⚠️ Заметь: `remove`, `remove_all`, `rename`, `permissions` — операции с побочными эффектами/необратимые. В реальных скриптах используй с осторожностью.

## 4. Навигация и текущий каталог

|Функция|Описание|
|---|---|
|`current_path()`|Текущий рабочий каталог|
|`current_path(p)`|Сменить текущий каталог|
|`absolute(p)`|Привести к абсолютному пути|
|`canonical(p)`|Канонический путь (резолвит `.`, `..`, ссылки)|
|`relative(p, base)`|Относительный путь от base|
|`temp_directory_path()`|Системный каталог для временных файлов|
|`unique_path(model)`|Сгенерировать уникальное имя (boost-специфично)|

## 5. Итерация по каталогам

|Класс|Описание|
|---|---|
|`directory_iterator`|Перебор содержимого одного каталога|
|`recursive_directory_iterator`|Рекурсивный обход поддерева|
|`directory_entry`|Элемент каталога (путь + кешированный статус)|

```cpp
// Один уровень
for (auto& entry : fs::directory_iterator("/some/dir")) {
    std::cout << entry.path().filename().string() << "\n";
}

// Рекурсивно
for (auto& entry : fs::recursive_directory_iterator("/some/dir")) {
    if (fs::is_regular_file(entry))
        std::cout << entry.path() << " (" << fs::file_size(entry) << " B)\n";
}
```

`recursive_directory_iterator` дополнительно имеет:

|Метод|Описание|
|---|---|
|`depth()`|Текущая глубина вложенности|
|`disable_recursion_pending()`|Не входить в текущий подкаталог|
|`pop()`|Подняться на уровень вверх|

## 6. Обработка ошибок: два стиля

Большинство функций имеют **две перегрузки**:

```cpp
// 1. С исключениями (бросает filesystem_error)
try {
    fs::file_size("/no/such/file");
} catch (const fs::filesystem_error& e) {
    std::cerr << e.what() << "\n";
    std::cerr << e.path1() << "\n"; // путь, вызвавший ошибку
}

// 2. С error_code (без исключений)
boost::system::error_code ec;
auto sz = fs::file_size("/no/such/file", ec);
if (ec) std::cerr << "Ошибка: " << ec.message() << "\n";
```

|Сущность|Описание|
|---|---|
|`filesystem_error`|Исключение; методы `path1()`, `path2()`, `code()`|
|`error_code` (из Boost.System)|Код ошибки для «тихого» режима|

## 7. Вспомогательные типы

| Тип                 | Описание                                                  |
| ------------------- | --------------------------------------------------------- |
| `file_status`       | Статус файла: тип и права                                 |
| `file_type`         | Перечисление: `regular_file`, `directory`, `symlink`, ... |
| `perms`             | Биты прав доступа                                         |
| `space_info`        | `capacity`, `free`, `available`                           |
| `copy_options`      | Опции копирования (`overwrite_existing`, ...)             |
| `directory_options` | Опции обхода каталогов                                    |

## Отличия от `std::filesystem`

- API почти идентичен: `std::filesystem` создавался на основе Boost.Filesystem (v3).
- Пространство имён: `boost::filesystem` ↔ `std::filesystem`; исключение `filesystem_error` — в обоих.
- **Не требует линковки**: `std::filesystem` встроен в стандартную библиотеку (хотя на старых GCC нужен флаг `-lstdc++fs`), boost-версию надо линковать.
- `unique_path()` есть в boost, в стандарте отсутствует.
- В стандарте `file_size`/`last_write_time` возвращают типы из `std::chrono`; в boost — свои представления времени (`std::time_t`).
- Boost доступен на компиляторах без поддержки C++17 — главная причина всё ещё его использовать.
- В целом миграция Boost → std обычно сводится к замене пространства имён.

### include/test_filesystem.h
```cpp
#pragma once  
  
namespace test_filesystem {  
    void test();  
}
```

### src/test_filesystem.cpp
```cpp
#include "test_filesystem.h"  
  
#include <iostream>  
#include <format>  
#include <boost/filesystem.hpp>  
#include <boost/filesystem/fstream.hpp>  
  
namespace fs = boost::filesystem;  
  
namespace test_filesystem {  
  
void test() {  
    fs::path dir = fs::temp_directory_path() / "boost_demo";  
    std::cout << std::format("Directory path: {}\n", dir.string());  
    fs::create_directory(dir);  
  
    fs::path file_path = dir / "text.txt";  
    std::cout << std::format("File path: {}\n", file_path.string());  
  
    // запись в файл через boost::filesystem::ofstream  
    fs::ofstream ofs(file_path);  
    ofs << "Hello from Boost.Filesystem!\n";  
    ofs << "Line 2\n";  
    ofs.close();  
  
    if (fs::exists(file_path)) {  
        std::cout << std::format("Size: {}\n", fs::file_size(file_path));  
        std::cout << std::format("Extension: {}\n", file_path.extension().string());  
  
        // чтение обратно через boost::filesystem::ifstream  
        fs::ifstream ifs(file_path);  
        std::string line;  
        while (std::getline(ifs, line)) {  
            std::cout << std::format("  > {}\n", line);  
        }    }  
    boost::system::error_code ec;  
    for (auto& e : fs::directory_iterator(dir, ec)) {  
        std::cout << std::format("Entry: {}\n", e.path().filename().string());  
    }  
    fs::remove_all(dir);  
    std::cout << std::format("Remove: {}\n", dir.string());  
}  
  
}
```

```
Directory path: C:\Users\PAVELK~1\AppData\Local\Temp\boost_demo
File path: C:\Users\PAVELK~1\AppData\Local\Temp\boost_demo\text.txt
Size: 38
Extension: .txt
  > Hello from Boost.Filesystem!
  > Line 2
Entry: text.txt
Remove: C:\Users\PAVELK~1\AppData\Local\Temp\boost_demo
```
