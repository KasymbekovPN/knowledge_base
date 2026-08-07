/*
protoc -I. --cpp_out=. numbers.proto
*/

#include <iomanip>
#include <iostream>
#include <format>

#include "numbers.pb.h"

namespace {
    void dump(const std::string& label, const myapp::Numbers& numbers) {
        std::string bytes;
        numbers.SerializeToString(&bytes);
        std::cout << std::format("{} -> {} bytes:\n", label, bytes.size());
        for (unsigned char c: bytes) {
            std::cout
                << std::hex
                << std::setw(2)
                << std::setfill('0')
                << static_cast<int>(c) << " ";
        }
        std::cout << std::dec << '\n';
    }
}

int main() {
    // --- 1. Малое положительное число: varint даёт 1 байт ---
    {
      myapp::Numbers n;
      n.set_a(1);
      dump("a=1        (int32)", n);
    }

    // --- 2. int32 = -1: БЕЗ zigzag varint кодирует как 64-битное число,
    //        т.к. отрицательные int32 сначала расширяются до int64 —
    //        получаем классические 10 байт "все единицы" ---
    {
      myapp::Numbers n;
      n.set_a(-1);
      dump("a=-1       (int32, without zigzag!)", n);
    }

    // --- 3. sint32 = -1: С zigzag компактно кодируется в 1 байт ---
    {
      myapp::Numbers n;
      n.set_b(-1);
      dump("b=-1       (sint32, with zigzag)", n);
    }

    // --- 4. sint32 небольшие значения по возрастанию модуля ---
    {
      myapp::Numbers n;
      n.set_b(0);
      dump("b=0        (sint32)", n);
    }
    {
      myapp::Numbers n;
      n.set_b(1);
      dump("b=1        (sint32)", n);
    }
    {
      myapp::Numbers n;
      n.set_b(-2);
      dump("b=-2       (sint32)", n);
    }
    {
      myapp::Numbers n;
      n.set_b(2);
      dump("b=2        (sint32)", n);
    }

    // --- 5. Число, требующее 2 байта varint (>127) ---
    {
      myapp::Numbers n;
      n.set_a(300);
      dump("a=300      (int32)", n);
    }

    // --- 6. int64 = -1 без zigzag: 10 байт ---
    {
      myapp::Numbers n;
      n.set_c(-1);
      dump("c=-1       (int64, with zigzag)", n);
    }

    // --- 7. sint64 = -1 с zigzag: 1 байт ---
    {
      myapp::Numbers n;
      n.set_d(-1);
      dump("d=-1       (sint64, with zigzag)", n);
    }

    // --- 8. fixed32 (не varint вообще, всегда 4 байта, little-endian) ---
    {
      myapp::Numbers n;
      n.set_e(1);
      dump("e=1        (fixed32)", n);
    }

    return 0;
}
