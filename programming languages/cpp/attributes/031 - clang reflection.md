---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

Минимальный аналог Qt moc / protobuf-генератора: разметка C++ класса через `[[clang::annotate("...")]]`, парсинг через **libclang** (не через регэкспы!) и генерация кода сериализации без единой строчки, написанной вручную под конкретный класс `Person`.

### model.h
```cpp
#pragma once  
  
#include <string>  
  
// Класс помечен для генератора — "reflect:serializable" сигнализирует,  
// что для этого типа нужно сгенерировать (де)сериализацию. Компилятору  
// эта строка ничего не говорит, она существует только для инструментов,  
// читающих AST (libclang) — примерно как Q_OBJECT для moc.  
class [[clang::annotate("reflect:serializable")]] Person {  
public:  
    Person() = default;  
    Person(const int age, std::string name, const double salary):  
        age{age}, name{std::move(name)}, salary{salary} {}  
  
    // Поля помечены "reflect:field" — генератор включит их в вывод.  
    // У salary есть модификатор "secret" — генератор подставит маску    
    // вместо реального значения.    
    int age [[clang::annotate("reflect:field")]] = 0;  
    std::string name [[clang::annotate("reflect:field")]];  
    double salary [[clang::annotate("reflect:field,secret")]] = 0.0;  
  
private:  
    // Без аннотации — генератор его вообще не увидит и не включит в вывод.  
    mutable int accessCount_{0};  
};
```

### generate.py
```Python
#!/usr/bin/env python3  
"""  
Простейший аналог Qt moc / protobuf-генератора: парсит C++ заголовок  
через libclang, находит классы и поля, помеченные [[clang::annotate(...)]],  
и генерирует код (де)сериализации на их основе — без единой строчки  
кода, написанной вручную для конкретного класса Person.  
"""  
import sys  
import clang.cindex as cindex  
  
INPUT_HEADER = "model.h"  
OUTPUT_HEADER = "model.generated.h"  
  
  
def get_annotation(cursor):  
    """Текст первого дочернего AnnotateAttr-узла данного cursor, если есть."""  
    for child in cursor.get_children():  
        if child.kind == cindex.CursorKind.ANNOTATE_ATTR:  
            return child.spelling  
    return None  
  
  
def parse_reflect_tag(annotation):  
    """'reflect:field,secret' -> ('field', ['secret'])"""  
    if annotation is None or not annotation.startswith("reflect:"):  
        return None, []  
    rest = annotation[len("reflect:"):]  
    parts = rest.split(",")  
    return parts[0], parts[1:]  
  
  
def main():  
    index = cindex.Index.create()  
    tu = index.parse(INPUT_HEADER, args=["-std=c++17", "-x", "c++", "-IC:/msys64/ucrt64/include/c++/16.1.0", "-IC:/msys64/ucrt64/include/c++/16.1.0/x86_64-w64-mingw32"])  
  
    for d in tu.diagnostics:  
        print(f"clang: {d}", file=sys.stderr)  
  
    classes = []  
  
    def visit(cursor):  
        if cursor.kind in (cindex.CursorKind.CLASS_DECL, cindex.CursorKind.STRUCT_DECL):  
            tag, _ = parse_reflect_tag(get_annotation(cursor))  
            if tag == "serializable":  
                fields = []  
                for child in cursor.get_children():  
                    if child.kind == cindex.CursorKind.FIELD_DECL:  
                        ftag, fflags = parse_reflect_tag(get_annotation(child))  
                        if ftag == "field":  
                            fields.append((child.spelling, child.type.spelling, fflags))  
                classes.append((cursor.spelling, fields))  
        for child in cursor.get_children():  
            visit(child)  
  
    visit(tu.cursor)  
  
    with open(OUTPUT_HEADER, "w") as out:  
        out.write("// АВТОСГЕНЕРИРОВАНО generate.py из model.h. Не редактировать вручную.\n")  
        out.write("#pragma once\n\n")  
        out.write('#include "model.h"\n')  
        out.write("#include <sstream>\n#include <string>\n\n")  
  
        for class_name, fields in classes:  
            out.write(f"inline std::string to_json(const {class_name}& obj) {{\n")  
            out.write("    std::ostringstream oss;\n")  
            out.write('    oss << "{";\n')  
            for i, (fname, ftype, fflags) in enumerate(fields):  
                if i > 0:  
                    out.write('    oss << ",";\n')  
                if "secret" in fflags:  
                    out.write(f'    oss << "\\"{fname}\\":\\"***\\"";\n')  
                elif ftype == "std::string":  
                    out.write(f'    oss << "\\"{fname}\\":\\"" << obj.{fname} << "\\"";\n')  
                else:  
                    out.write(f'    oss << "\\"{fname}\\":" << obj.{fname};\n')  
            out.write('    oss << "}";\n')  
            out.write("    return oss.str();\n")  
            out.write("}\n\n")  
  
    print(f"Generated {OUTPUT_HEADER}: {len(classes)} class(es)")  
    for class_name, fields in classes:  
        desc = ", ".join(f"{n}:{t}" + ("[secret]" if "secret" in f else "") for n, t, f in fields)  
        print(f"  {class_name} -> {desc}")  
  
  
if __name__ == "__main__":  
    main()
```

### main.cpp
```cpp
#include "model.generated.h"  
  
#include <iostream>  
  
int main() {  
    Person p(30, "Ada Lovelace", 123456.78);  
    std::cout << to_json(p) << "\n";  
  
    return 0;  
}  
  
/*  
  
& "C:\Users\Pavel Kasymbekov\AppData\Local\Programs\Python\Python311\python.exe" -m venv .venv  
pip install libclang --break-system-packages  
.venv\Scripts\Activate.ps1  
python.exe .\generate.py  
clang++.exe -std=c++17 main.cpp  
  
*/
```

## Как это работает

1. **`model.h`** — обычный C++ заголовок. Единственное отличие — поля и класс помечены `[[clang::annotate("reflect:...")]]`. Это чистая метаданные: компилятору эта строка ничего не говорит (GCC даже выдаёт `-Wattributes` warning "scoped attribute directive ignored" и компилирует как обычно — атрибут в неизвестном namespace обязан быть проигнорирован по стандарту, а не привести к ошибке).
    
2. **`generate.py`** — использует Python-биндинги libclang (`clang.cindex`) для разбора `model.h` в настоящее Clang AST (то же дерево, которое строит сам компилятор, а не текстовый парсинг заголовка регэкспами). Обходит дерево, находит `CLASS_DECL`/`STRUCT_DECL` с дочерним узлом `ANNOTATE_ATTR` равным `"reflect:serializable"`, затем для каждого `FIELD_DECL` внутри — проверяет его собственный `ANNOTATE_ATTR` (`"reflect:field"`, опционально с флагами через запятую, например `"reflect:field,secret"`).
    
3. По собранному списку класс/поля/типы/флаги генерируется **`model.generated.h`** — обычный C++ заголовок с функцией `to_json(const Person&)`. Поле `salary`, помеченное флагом `secret`, генератор автоматически заменяет на маску `"***"` вместо реального значения — это уже логика самого генератора, а не что-то, что умеет делать компилятор.
    
4. **`main.cpp`** просто использует сгенерированный `to_json()` — как будто он был написан руками.
    

## Запуск

```bash
pip install libclang --break-system-packages   # чистые Python-биндинги + сам libclang.so, системный clang не нужен
python3 generate.py                             # -> model.generated.h
g++ -std=c++17 -o app main.cpp
./app
```

Ожидаемый вывод:

```
{"age":30,"name":"Ada Lovelace","salary":"***"}
```

## Важные нюансы

- **libclang не требует установленного `clang`/`clang++` в системе.** Пакет `libclang` с PyPI поставляет готовую `libclang.so` (библиотеку синтаксического разбора, часть LLVM) — сборка финального бинарника делается любым компилятором (здесь — GCC), libclang нужен только на этапе генерации кода.
    
- **Аннотации переживают только сам AST, не попадают в скомпилированный бинарник** (в отличие от, например, `__attribute__((section(...)))`). Это чисто compile-time/tooling механизм — рантайм ничего про них не знает. Если нужна reflection-информация именно в рантайме (не на этапе генерации кода), это отдельная, более сложная задача (RTTI, ручные таблицы метаданных, либо будущая C++26 static reflection).
    
- **`generate.py` передаёт libclang те же include-пути, что использует системный GCC** (`-I/usr/include/c++/11` и т.п.) — без этого парсинг падает с `fatal error: 'stddef.h' file not found`, потому что у чистого libclang нет собственных системных заголовков. В реальном проекте эти пути обычно берут из `compile_commands.json` (CMake с `CMAKE_EXPORT_COMPILE_COMMANDS=ON`), а не хардкодят.
    
- **Именно так устроены реальные генераторы** такого рода: Qt moc парсит `Q_OBJECT`/`Q_PROPERTY` (правда, собственным упрощённым парсером, не через полноценный Clang AST), а более современные инструменты (например, генераторы биндингов, ORM, сериализаторы) всё чаще используют именно libclang/LibTooling, потому что это даёт настоящее понимание C++ (шаблоны, using-алиасы, наследование), а не хрупкий текстовый парсинг заголовков.
