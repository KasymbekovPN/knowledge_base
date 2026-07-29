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
        // системы по отдельности, не общей иерархии типов.
    }
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
