#include <iostream>
#include <format>
#include "player_proto2.pb.h"
#include "player_proto3.pb.h"
#include "player_editions.pb.h"

int main() {

    auto p2 = com::example::v2::Player();
    p2.set_id(42);
    std::cout << std::format("v2:\n{}\n", p2.Utf8DebugString());

    auto p3 = com::example::v3::Player();
    p3.set_id(43);
    std::cout << std::format("v3:\n{}\n", p3.Utf8DebugString());

    auto pe = com::example::ed::Player();
    pe.set_id(44);
    std::cout << std::format("ed:\n{}\n", pe.Utf8DebugString());

    return 0;
}