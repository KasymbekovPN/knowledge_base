---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Graph (BGL)

Boost.Graph Library — структуры данных и алгоритмы для работы с графами. Header-only, но с **крутой кривой обучения** из-за интенсивного использования шаблонов и концепции property maps. Один из самых сложных модулей наряду со Spirit.

```cpp
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
```

## Архитектурная идея

BGL построен на **обобщённом программировании**: алгоритмы отделены от структур данных через концепции (Concepts) и property maps. Один алгоритм (например, Dijkstra) работает с любым типом графа, удовлетворяющим нужной концепции. Это мощно, но именно отсюда сложность.

Три ключевых слоя:

- **Структуры графов** — как хранится граф (`adjacency_list`, `adjacency_matrix`).
- **Property maps** — как associating данных с вершинами/рёбрами.
- **Алгоритмы** — обходы, кратчайшие пути, потоки и т.д.

## 1. Структуры графов

### `adjacency_list` — основной тип

Граф как список смежности. Параметризуется пятью аргументами:

```cpp
adjacency_list
    OutEdgeList,   // контейнер для рёбер вершины
    VertexList,    // контейнер для вершин
    Directed,      // направленность
    VertexProperty,// данные на вершинах
    EdgeProperty,  // данные на рёбрах
    GraphProperty> // данные на графе
```

### Селекторы контейнеров (OutEdgeList / VertexList)

|Селектор|Контейнер|Особенность|
|---|---|---|
|`vecS`|`std::vector`|Быстрый доступ по индексу, дескрипторы — числа|
|`listS`|`std::list`|Стабильные дескрипторы при удалении|
|`setS`|`std::set`|Без дублирующихся рёбер, сортировка|
|`multisetS`|`std::multiset`|С дубликатами|
|`hash_setS`|hash-таблица|Быстрый поиск рёбер|

### Селекторы направленности (Directed)

|Селектор|Описание|
|---|---|
|`directedS`|Направленный, только исходящие рёбра|
|`undirectedS`|Ненаправленный|
|`bidirectionalS`|Направленный + доступ к входящим рёбрам (`in_edges`)|

```cpp
using namespace boost;

// Направленный граф с весами на рёбрах
typedef adjacency_list
    vecS, vecS, directedS,
    no_property,                        // нет данных на вершинах
    property<edge_weight_t, int>        // вес на рёбрах
> Graph;
```

### `adjacency_matrix`

Граф как матрица смежности — выгоден для **плотных** графов (O(1) проверка наличия ребра, но O(V²) памяти).

## 2. Дескрипторы и базовые операции

### Дескрипторы (handles на вершины/рёбра)

|Тип|Описание|
|---|---|
|`graph_traits<Graph>::vertex_descriptor`|Идентификатор вершины|
|`graph_traits<Graph>::edge_descriptor`|Идентификатор ребра|

> При `vecS` для вершин дескриптор вершины — это просто `std::size_t` (индекс).

### Построение графа

|Функция|Описание|
|---|---|
|`add_vertex(g)`|Добавить вершину → vertex_descriptor|
|`add_vertex(prop, g)`|С свойством|
|`add_edge(u, v, g)`|Добавить ребро → `pair<edge_descriptor, bool>`|
|`add_edge(u, v, prop, g)`|С свойством (например, весом)|
|`remove_edge(u, v, g)`|Удалить ребро|
|`remove_vertex(v, g)`|Удалить вершину|
|`clear_vertex(v, g)`|Удалить все рёбра вершины|

```cpp
Graph g;
auto v0 = add_vertex(g);
auto v1 = add_vertex(g);
add_edge(v0, v1, 10, g); // ребро с весом 10
```

Граф можно создать сразу с N вершинами: `Graph g(5);`

## 3. Обход графа (итерация)

|Функция|Возвращает|Описание|
|---|---|---|
|`vertices(g)`|пара итераторов|Все вершины|
|`edges(g)`|пара итераторов|Все рёбра|
|`out_edges(v, g)`|пара итераторов|Исходящие рёбра вершины|
|`in_edges(v, g)`|пара итераторов|Входящие (только bidirectional)|
|`adjacent_vertices(v, g)`|пара итераторов|Смежные вершины|
|`num_vertices(g)`|число|Количество вершин|
|`num_edges(g)`|число|Количество рёбер|
|`source(e, g)`|vertex|Начало ребра|
|`target(e, g)`|vertex|Конец ребра|
|`out_degree(v, g)`|число|Исходящая степень|

```cpp
// Перебор всех рёбер
for (auto [it, end] = edges(g); it != end; ++it) {
    auto e = *it;
    std::cout << source(e, g) << " -> " << target(e, g) << "\n";
}

// Перебор соседей вершины v
for (auto [it, end] = adjacent_vertices(v0, g); it != end; ++it) {
    std::cout << "сосед: " << *it << "\n";
}
```

---

## 4. Property Maps — самая сложная концепция

Property map связывает данные с вершинами/рёбрами. Это абстракция «функции», отображающей дескриптор в значение.

### Внутренние свойства (хранятся в графе)

Задаются через шаблон `property<Tag, Type>`:

|Тег|Назначение|
|---|---|
|`vertex_index_t`|Индекс вершины|
|`vertex_distance_t`|Расстояние (для алгоритмов)|
|`vertex_color_t`|Цвет (для обходов)|
|`vertex_predecessor_t`|Предшественник в дереве путей|
|`edge_weight_t`|Вес ребра|
|`edge_capacity_t`|Пропускная способность (для потоков)|

```cpp
// Доступ к property map
auto weight_map = get(edge_weight, g);
auto w = weight_map[some_edge];   // вес конкретного ребра
// или
auto w2 = get(edge_weight, g, some_edge);
```

### Внешние свойства (отдельные контейнеры)

Часто данные хранят снаружи и оборачивают в property map:

```cpp
std::vector<int> distances(num_vertices(g));
auto dist_map = make_iterator_property_map(
    distances.begin(), get(vertex_index, g));
```

### Bundled properties — удобная альтернатива

Современный, более читаемый способ — структуры прямо на вершинах/рёбрах:

```cpp
struct VertexData { std::string name; int population; };
struct EdgeData   { double distance; };

typedef adjacency_list<vecS, vecS, directedS,
    VertexData, EdgeData> Graph;

Graph g;
auto v = add_vertex(g);
g[v].name = "Москва";       // доступ через operator[]
g[v].population = 12000000;

auto [e, ok] = add_edge(v0, v1, g);
g[e].distance = 650.5;
```

## 5. Алгоритмы

### Кратчайшие пути

|Алгоритм|Функция|Применение|
|---|---|---|
|Dijkstra|`dijkstra_shortest_paths`|Неотрицательные веса|
|Bellman-Ford|`bellman_ford_shortest_paths`|Допускает отрицательные веса|
|Johnson|`johnson_all_pairs_shortest_paths`|Все пары вершин|
|Floyd-Warshall|`floyd_warshall_all_pairs_shortest_paths`|Все пары, плотные графы|
|A*|`astar_search`|С эвристикой|

### Обходы

|Алгоритм|Функция|
|---|---|
|Поиск в ширину|`breadth_first_search`|
|Поиск в глубину|`depth_first_search`|
|Топологическая сортировка|`topological_sort`|

### Остовные деревья и компоненты

|Алгоритм|Функция|
|---|---|
|Минимальное остовное дерево (Kruskal)|`kruskal_minimum_spanning_tree`|
|MST (Prim)|`prim_minimum_spanning_tree`|
|Компоненты связности|`connected_components`|
|Сильно связные компоненты|`strong_components`|

### Потоки

|Алгоритм|Функция|
|---|---|
|Максимальный поток|`edmonds_karp_max_flow`, `push_relabel_max_flow`, `boykov_kolmogorov_max_flow`|

## 6. Визиторы — вмешательство в ход алгоритма

Многие алгоритмы (BFS/DFS) принимают **visitor** — объект с колбэками на события обхода:

|Событие BFS|Когда вызывается|
|---|---|
|`discover_vertex`|Вершина впервые обнаружена|
|`examine_vertex`|Вершина взята из очереди|
|`examine_edge`|Ребро рассматривается|
|`tree_edge`|Ребро вошло в дерево обхода|
|`finish_vertex`|Обработка вершины завершена|

```cpp
#include <boost/graph/breadth_first_search.hpp>

struct MyVisitor : boost::default_bfs_visitor {
    void discover_vertex(auto v, const auto& g) const {
        std::cout << "Обнаружена вершина: " << v << "\n";
    }
};

breadth_first_search(g, start, boost::visitor(MyVisitor{}));
```

## 7. Именованные параметры (bgl_named_params)

Многие алгоритмы используют «именованные параметры» — цепочку вызовов для необязательных аргументов:

```cpp
dijkstra_shortest_paths(g, start,
    distance_map(dist_map)
    .predecessor_map(pred_map)
    .weight_map(weight_map));
```

Это исторический паттерн BGL (до C++11). Выглядит непривычно, но позволяет задавать только нужные параметры.

## Сводка концепций

|Концепция|Суть|
|---|---|
|`adjacency_list`|Основная структура; 5 параметров-селекторов|
|Селекторы `vecS`/`listS`/...|Выбор контейнеров для вершин/рёбер|
|`directedS`/`undirectedS`/`bidirectionalS`|Направленность|
|Дескрипторы|Handles на вершины/рёбра (`graph_traits`)|
|Функции обхода|`vertices`, `edges`, `out_edges`, `adjacent_vertices`|
|Property maps|Связь данных с элементами графа (сложнейшая часть)|
|Bundled properties|Структуры на вершинах/рёбрах + `operator[]` (проще)|
|Визиторы|Колбэки на события алгоритмов|
|Именованные параметры|`.distance_map(...).weight_map(...)`|
## Практические советы

- **Начинай с bundled properties** — они куда читаемее, чем теги `property<edge_weight_t, int>` и внешние property maps. К тегам переходи, когда алгоритм их требует.
- **`vecS` для вершин** — выбор по умолчанию: дескрипторы = индексы, есть готовый `vertex_index`. `listS` бери, если часто удаляешь вершины и нужна стабильность дескрипторов.
- **Property maps — главное препятствие.** Потрать время именно на них: разберись, что `get(vertex_index, g)` даёт отображение вершина→индекс, а `make_iterator_property_map` оборачивает внешний вектор.
- **Сообщения компилятора** при ошибках в BGL огромны (как в Spirit) — читай первую строку про несовпадение концепции, остальное обычно шум.
- Для простых задач (обойти небольшой граф) BGL может быть избыточен — иногда проще свой `std::vector<std::vector<int>>`. BGL оправдан, когда нужны готовые алгоритмы (потоки, MST, A*) и обобщённость.

## Отличия от стандарта и альтернатив

- В стандартной библиотеке **графов нет** — BGL уникален и остаётся эталонной C++-библиотекой графов.
- Альтернативы вне Boost: igraph (C, с обёртками), Lemon, NetworkX (Python), или собственные структуры для простых случаев.
- Сила BGL — обобщённость и богатый набор алгоритмов; цена — сложность шаблонного API и тяжёлая диагностика ошибок.

### include/test_graph.h
```cpp
#pragma once  
  
namespace test_graph {  
    void test_dijkstra();  
    void test_bundled_properties();  
}
```

### src/test_graph.cpp
```cpp
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
            )        )    );  
    for (std::size_t i{}; i < distances.size(); ++i) {  
        std::cout << std::format("0 -> {} : distance = {}, prev = {}\n", i, distances[i], predecessors[i]);  
    }}  
  
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
    }}    
}
```

```
0 -> 0 : distance = 0, prev = 0
0 -> 1 : distance = 2, prev = 0
0 -> 2 : distance = 3, prev = 1
0 -> 3 : distance = 7, prev = 4
0 -> 4 : distance = 6, prev = 2
Moscow - SPB: 705 km
```
