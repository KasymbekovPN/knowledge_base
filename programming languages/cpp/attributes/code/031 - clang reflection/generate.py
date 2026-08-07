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