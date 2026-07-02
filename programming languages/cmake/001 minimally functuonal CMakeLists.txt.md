---
tags:
  - programming-language
  - cmake
---
[[programming languages/cmake/_|<=]]

Минимальный рабочий `CMakeLists.txt` состоит всего из трёх команд:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyApp)

add_executable(myapp main.cpp)
```

**`cmake_minimum_required(VERSION 3.16)`**

Задаёт минимальную версию CMake, необходимую для сборки проекта. Должна идти **самой первой** командой. Это влияет на поведение CMake: включает соответствующий набор политик (policies), определяющих, как интерпретируются те или иные команды. Указывайте реальную версию, на которую рассчитан проект, а не самую старую «на всякий случай».

**`project(MyApp)`**

Объявляет проект и задаёт его имя. Должна идти сразу после `cmake_minimum_required`. Эта команда выполняет важную работу под капотом: определяет используемые языки (по умолчанию C и C++), запускает проверку компиляторов и задаёт ряд переменных. Можно указать дополнительные параметры:

```cmake
project(MyApp
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "My application"
)
```

После этого становятся доступны переменные вроде `PROJECT_NAME`, `MyApp_VERSION`, `PROJECT_SOURCE_DIR` и т.д.

**`add_executable(myapp main.cpp)`**

Создаёт цель сборки — исполняемый файл с именем `myapp` из перечисленных исходников. Первый аргумент — имя цели (по нему потом обращаются другие команды, например `target_link_libraries`), остальные — файлы с исходным кодом:

```cmake
add_executable(myapp
    main.cpp
    utils.cpp
    parser.cpp
)
```

**Сборка**

```
cmake -B build
cmake --build build
```

Результат — исполняемый файл `myapp` внутри каталога `build/`.

Эти три команды образуют скелет любого проекта; всё остальное (библиотеки, флаги, зависимости) наращивается поверх этой основы.

### CMakeLilsts.txt
```cmake
cmake_minimum_required(VERSION 4.0.0)  
  
project(MyApp)  
  
add_executable(myapp main.cpp)
```

### main.cpp
```cpp
/*  
cmake -B .build  
cmake --build .build  
*/  
  
#include <iostream>  
  
int main() {  
    std::cout << "Hello CMakeLists world!\n";  
}
```

```
Hello CMakeLists world!
```
