#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <format>

namespace test_graph {

void test_dijkstra() {
    typedef boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, int>
    > Graph;

    Graph g{5};
    boost::add_edge(0, 1, 2, g);
    boost::add_edge(0, 2, 5, g);
    boost::add_edge(1, 2, 1, g);
    boost::add_edge(1, 3, 7, g);
    boost::add_edge(2, 4, 3, g);
    boost::add_edge(4, 3, 1, g);

    std::vector<int> distances(boost::num_vertices(g));
    std::vector<Graph::vertex_descriptor> predecessors(boost::num_vertices(g));

    auto start = boost::vertex(0, g);
    boost::dijkstra_shortest_paths(
        g,
        start,
        boost::distance_map(
            boost::make_iterator_property_map(
                distances.begin(),
                boost::get(boost::vertex_index, g))
        ).predecessor_map(
            boost::make_iterator_property_map(
                predecessors.begin(),
                boost::get(boost::vertex_index, g)
            )
        )
    );

    for (std::size_t i{}; i < distances.size(); ++i) {
        std::cout << std::format("0 -> {} : distance = {}, prev = {}\n", i, distances[i], predecessors[i]);
    }
}

struct City { std::string name; };
struct Road { double km; };

void test_bundled_properties() {
    typedef boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::undirectedS,
        City,
        Road
    > Map;

    Map map;
    auto moscow = boost::add_vertex(map);
    map[moscow].name = "Moscow";
    auto spb = boost::add_vertex(map);
    map[spb].name = "SPB";

    auto [road, ok] = boost::add_edge(moscow, spb, map);
    map[road].km = 705.0;

    for (auto [it, end] = boost::edges(map); it != end; ++it) {
        auto e = *it;
        std::cout << std::format(
            "{} - {}: {} km\n",
            map[boost::source(e, map)].name,
            map[boost::target(e, map)].name,
            map[e].km);
    }
}

}
