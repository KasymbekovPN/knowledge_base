---
tags:
  - programming-language
  - architecture
---
[[raw data/application architecture/_|<=]]

### vcpkg.json
```json
{  
    "name": "project",  
    "version": "0.1.0",  
    "dependencies": []  
}
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {            
	        "name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {            
	        "name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
        }  
    ],    
    "testPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug",  
            "output": {  
                "outputOnFailure": true  
            }  
        }    
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)  
project(ESC CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
// ECS: сущность (Entity) - просто число, компонент (Component) - только  
// данные, система (System) - только поведение, работающее над множеством  
// сущностей с нужным набором компонентов. Никакой иерархии наследования.  
  
#include <algorithm>  
#include <cstdint>  
#include <vector>  
#include <iostream>  
#include <format>  
  
using EntityId = std::uint32_t;  
  
// ---------------------------------------------------------------------------  
// ComponentStorage<T> - sparse set: dense_ хранит компоненты ПОДРЯД в памяти  
// (важно для кэша - системы бегают по dense_ линейно, без прыжков по куче,  
// в отличие от массива указателей на полиморфные объекты при наследовании).  
// sparse_ даёт O(1) ответ "есть ли у entity компонент T" и O(1) доступ к нему.  
// ---------------------------------------------------------------------------  
template <typename T>  
class ComponentStorage {  
public:  
    void add(const EntityId e, T component) {  
        if (e >= sparse_.size()) sparse_.resize(e + 1, kInvalid);  
        if (sparse_[e] != kInvalid) {  
            dense_[sparse_[e]] = std::move(component);  
            return;  
        }  
        sparse_[e] = static_cast<EntityId>(dense_.size());  
        dense_.push_back(std::move(component));  
        denseToEntity_.push_back(e);  
    }  
    bool has(const EntityId e) const {  
        return e < sparse_.size() && sparse_[e] != kInvalid;  
    }  
    T* get(const EntityId e) {  
        return has(e) ? &dense_[sparse_[e]] : nullptr;  
    }  
    void remove(const EntityId e) {  
        if (!has(e)) return;  
        const EntityId IDX{sparse_[e]};  
        const EntityId LAST_IDX{static_cast<EntityId>(dense_.size() - 1)};  
        const EntityId LAST_ENTITY{denseToEntity_[LAST_IDX]};  
        // swap-with-last: O(1), dense_ без дыр  
        dense_[IDX] = std::move(dense_[LAST_IDX]);  
        denseToEntity_[IDX] = LAST_ENTITY;  
        sparse_[LAST_ENTITY] = IDX;  
        dense_.pop_back();  
        denseToEntity_.pop_back();  
        sparse_[e] = kInvalid;  
    }  
    const std::vector<EntityId>& entities() const { return denseToEntity_; }  
    std::size_t size() const { return dense_.size(); }  
  
private:  
    static constexpr EntityId kInvalid = static_cast<EntityId>(-1);  
    // entity id -> индекс в dense_ (или kInvalid)  
    std::vector<EntityId> sparse_;  
    // сами компоненты, ПОДРЯД в памяти  
    std::vector<T> dense_;  
    // индекс в dense_ -> entity id  
    std::vector<EntityId> denseToEntity_;  
};  
  
// ---------------------------------------------------------------------------  
// Компоненты - только данные, ни одного метода с логикой.  
// ---------------------------------------------------------------------------  
struct Position { float x{0.0f}, y{0.0f}; };  
struct Velocity { float dx{0.0f}, dy{0.0f}; };  
struct Health { int current{100}, max{100}; };  
struct Renderable { char glyph{'?'}; };  
  
// ---------------------------------------------------------------------------  
// World - контейнер сущностей и хранилищ компонентов. Entity - просто id,  
// сам по себе не хранит вообще ничего.  
// ---------------------------------------------------------------------------  
class World {  
public:  
    EntityId createEntity() { return nextId_++; }  
  
    ComponentStorage<Position> positions;  
    ComponentStorage<Velocity> velocities;  
    ComponentStorage<Health> healths;  
    ComponentStorage<Renderable> renderables;  
  
private:  
    EntityId nextId_{0};  
};  
  
// ---------------------------------------------------------------------------  
// Системы - свободные функции, работающие только с теми сущностями, у  
// которых есть нужная комбинация компонентов. Ни одна система не знает  
// про "типы" сущностей (Player/Rock/Ghost) - только про наличие данных.  
// ---------------------------------------------------------------------------  
void movementSystem(World& world, const float dt) {  
    for (const EntityId e : world.velocities.entities()) {  
        if (Position* pos = world.positions.get(e)) {  
            const Velocity* vel = world.velocities.get(e);  
            pos->x += vel->dx * dt;  
            pos->y += vel->dy * dt;  
        }        
        // Сущность с Velocity, но без Position - просто игнорируется этой  
        // системой, никакой ошибки. Комбинации компонентов - дело каждой        
        // системы по отдельности, не общей иерархии типов.    }  
}  
  
void damageSystem(World& world, const EntityId target, const int amount) {  
    if (Health* hp = world.healths.get(target)) {  
        hp->current = std::max(0, hp->current - amount);  
        std::cout << std::format("  entity {} received {}, hp = {}\n", amount, hp->current, hp->max);  
    } else {  
        std::cout << std::format("  entity {} does not have Health\n", target);  
    }
}  
  
void renderSystem(World& world) {  
    for (EntityId e : world.renderables.entities()) {  
        Renderable& r = *world.renderables.get(e);  
        if (Position* pos = world.positions.get(e)) {  
            std::cout << std::format("  [{}] entity {} ({}, {})\n", r.glyph, e, pos->x, pos->y);  
        } else {  
            std::cout << std::format("  [{}] entity {} (without Position)\n", r.glyph, e);  
        }    
    }
}  
  
int main() {  
    World world;  
  
    // player: движется, получает урон, отрисовывается  
    EntityId player = world.createEntity();  
    world.positions.add(player, Position{.x = 0.0f, .y = 0.0f});  
    world.velocities.add(player, Velocity{.dx = 1, .dy = 0.5f});  
    world.healths.add(player, Health{.current = 100, .max = 100});  
    world.renderables.add(player, Renderable{.glyph = 'P'});  
  
    // rock: только позиция и отрисовка - статичный декор, урон получить не может  
    EntityId rock = world.createEntity();  
    world.positions.add(rock, Position{.x = 5.0f, .y = 5.0f});  
    world.renderables.add(rock, Renderable{.glyph = '#'});  
  
    // ghost: движется и отрисовывается, но НЕ имеет Health - бессмертен  
    EntityId ghost = world.createEntity();  
    world.positions.add(ghost, Position{.x = 2.0f, .y = 2.0f});  
    world.velocities.add(ghost, Velocity{.dx = -0.5f, .dy = 0.2f});  
    world.renderables.add(ghost, Renderable{.glyph = 'G'});  
  
    // turret: неподвижна (нет Velocity), но имеет Health и отрисовку  
    EntityId turret = world.createEntity();  
    world.positions.add(turret, {.x = 8, .y = 1});  
    world.healths.add(turret, {.current = 50, .max = 50});  
    world.renderables.add(turret, {'T'});  
  
    std::cout << "-- begin condition --\n";  
    renderSystem(world);  
  
    std::cout << "\n-- movementSystem x3 (dt=1.0) --\n";  
    for (int i = 0; i < 3; ++i) movementSystem(world, 1.0f);  
    renderSystem(world);  
  
    std::cout << "\n-- damage --\n";  
    damageSystem(world, player, 30);  // есть Health -> применяется  
    damageSystem(world, rock, 10);    // нет Health -> игнорируется  
    damageSystem(world, turret, 60);  // есть Health -> применяется, но clamp до 0  
  
    std::cout << "\n-- remove Velocity  --\n";  
    world.velocities.remove(player);  
    for (int i = 0; i < 3; ++i) movementSystem(world, 1.0f);  // player больше не двигается  
    renderSystem(world);  
  
    return 0;  
}
```

**Проблема, которую ECS решает**

Классическая иерархия наследования плохо масштабируется на комбинации способностей. Представим `GameObject → Character → FlyingCharacter → ArmoredFlyingCharacter` — как только появляется существо "летающее, но неуязвимое" или "неподвижное, но с здоровьем" (как `turret` в примере), иерархия начинает комбинаторно взрываться: либо городить множественное наследование (`FlyingCharacter : Character, IFlyable` — с классическими проблемами ромбовидного наследования и общих данных базового класса), либо плодить подклассы под каждую комбинацию свойств. В примере у меня четыре сущности с четырьмя разными наборами возможностей (`player` — всё сразу, `rock` — только позиция и отрисовка, `ghost` — движение и отрисовка без здоровья, `turret` — здоровье и отрисовка без движения) — через одиночное наследование это потребовало бы отдельного класса на каждую комбинацию, а таких комбинаций растёт экспоненциально с числом независимых способностей.

**Инверсия относительно обычного ООП**

Обычно объект = данные + поведение вместе (метод класса имеет доступ к его же полям). ECS разрывает эту связку сознательно: `Entity` — просто число (`EntityId`), вообще без данных и поведения. Компонент (`Position`, `Velocity`, `Health`, `Renderable`) — чистые данные, ни одного метода с логикой. Система (`movementSystem`, `damageSystem`, `renderSystem`) — чистое поведение, свободная функция, которая проходит по каким сущностям есть нужный набор компонентов, и ничего не знает про "типы" сущностей вообще. `player` физически — просто число `0`, которому в разных хранилищах сопоставлены четыре записи; ничто в коде не говорит "это Player" — только факт наличия конкретных компонентов.

**Почему это удобно для расширения**

Добавить сущности новую способность — значит просто добавить в неё компонент, ничего не переписывая. В демо это буквально показано: `world.velocities.remove(player)` — и `player` мгновенно перестаёт двигаться, при том что и `movementSystem`, и весь остальной код не изменились ни на строчку. С наследованием такое "снятие способности в рантайме" потребовало бы паттернов вроде Strategy или State поверх и без того сложной иерархии; здесь это тривиальная операция над данными.

**Cache-friendliness — то, ради чего это реально используется в играх**

`ComponentStorage<T>` — sparse set: `dense_` хранит компоненты одного типа подряд в памяти (`std::vector<T>`). `movementSystem` бежит по `world.velocities.entities()`, и внутри цикла обращения к `Velocity` идут линейно по памяти — процессор эффективно предзагружает кэш-линии наперёд. Сравните с типичной альтернативой на виртуальном полиморфизме — массивом `std::vector<std::unique_ptr<GameObject>>`, где каждый `GameObject` выделен отдельно в куче в произвольном месте, а вызов виртуального метода `update()` — это ещё и непрямой переход через vtable. На больших количествах сущностей (тысячи-десятки тысяч в игре) эта разница в производительности не абстрактная, а прямо измеримая — это и есть суть "data-oriented design", с которого выросла вся идея ECS.

**Где это полезно шире игр**

Тот же принцип "сущность = произвольный набор независимо добавляемых/снимаемых атрибутов данных, обрабатываемых системами, которые ничего не знают о типе сущности целиком" переносится на любые предметные области, где объекты имеют динамически меняющийся, не укладывающийся в дерево наследования набор свойств: например, обработка заказов, где к заказу в рантайме может быть привязано произвольное подмножество данных (скидка, налоговые правила по региону, статус доставки, программа лояльности) — вместо `Order → DiscountedOrder → InternationalDiscountedOrder` с комбинаторным взрывом подклассов, можно хранить это как компоненты, привязанные к id заказа, и обрабатывать независимыми "системами" (расчёт налога, расчёт скидки, расчёт доставки), каждая из которых работает только с нужными ей данными.
