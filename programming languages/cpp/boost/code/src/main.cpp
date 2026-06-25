/*
cmake --preset default
cmake --build .build
.\.build\boost_learning.exe
 */

#include "test_optional.h"
#include "test_variant.h"
#include "test_any.h"
#include "test_smart_ptr.h"
#include "test_container.h"
#include "test_filesystem.h"
#include "test_regex.h"
#include "test_spirit.h"
#include "test_multiprecision.h"

int main() {
    // test_optional::test();
    // test_variant::test();
    // test_any::test();
    // test_smart_ptr::test();
    // test_container::test();
    // test_filesystem::test();
    // test_regex::test();
    // test_spirit::test();
    test_multiprecision::test();
}
