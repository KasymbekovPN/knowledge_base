/*
cmake --preset default
cmake --build .build
.\build\boost_learning.exe
 */

#include "test_optional.h"
#include "test_variant.h"
#include "test_any.h"
#include "test_smart_ptr.h"

int main() {
    // test_optional::test();
    // test_variant::test();
    // test_any::test();
    test_smart_ptr::test();
}
