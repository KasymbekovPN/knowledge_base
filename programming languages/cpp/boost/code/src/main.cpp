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
#include "test_graph.h"
#include "test_asio.h"
#include "test_beast.h"

int main() {
    // test_optional::test();
    // test_variant::test();
    // test_any::test();
    // test_smart_ptr::test();
    // test_container::test();
    // test_filesystem::test();
    // test_regex::test();
    // test_spirit::test();
    // test_multiprecision::test();
    // test_graph::test_dijkstra();
    // test_graph::test_bundled_properties();
    // test_asio::test_timer();
    // test_asio::test_buffer();
    // test_asio::test_sync_tcp();
    // test_asio::test_async_tcp();
    // test_asio::test_strands_for_counter();
    // test_asio::test_coroutine();
    // test_beast::test_sync_http_client();
    // test_beast::test_sync_http_server();
    // test_beast::test_async_http_server();
    test_beast::test_ws();
}

