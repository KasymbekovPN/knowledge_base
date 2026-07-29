// eventbus_lib - отдельная переиспользуемая библиотека. Публичный API -
// ровно этот заголовок, ничего больше. Не знает НИЧЕГО про задачи,
// плагины или что угодно предметно-специфичное - универсальная типобезопасная
// шина, которую можно унести в любой другой проект без изменений.

#pragma once

#include <algorithm>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <ranges>

class EventBus {
public:
    using SubscriptionId = std::size_t;

    class Connection {
    public:
        Connection() = default;
        Connection(Connection&& other) noexcept { *this = std::move(other); }
        Connection& operator=(Connection&& other) noexcept {
            if (this != &other) {
                disconnect();
                bus_ = other.bus_;
                type_ = other.type_;
                id_ = other.id_;
                other.bus_ = nullptr;
            }
            return *this;
        }
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        ~Connection() { disconnect(); }
        void disconnect() {
            if (bus_) {
                bus_->unsubscribeRaw(type_, id_);
                bus_ = nullptr;
            }
        }
    private:
        friend class EventBus;
        Connection(EventBus* bus, std::type_index type, SubscriptionId id)
            : bus_(bus), type_(type), id_(id) {}
        EventBus* bus_ = nullptr;
        std::type_index type_ = typeid(void);
        SubscriptionId id_ = 0;
    };

    template <typename EventT>
    Connection subscribe(std::function<void(const EventT&)> handler) {
        auto type = std::type_index(typeid(EventT));
        SubscriptionId id = nextId_++;
        handlers_[type].push_back(
            {id, [handler](const void* e) { handler(*static_cast<const EventT*>(e)); }});
        return {this, type, id};
    }

    template <typename EventT>
    void publish(const EventT& event) {
        auto it = handlers_.find(std::type_index(typeid(EventT)));
        if (it == handlers_.end()) return;
        auto listCopy = it->second;
        for (auto& [id, fn] : listCopy) fn(&event);
    }
private:
    void unsubscribeRaw(const std::type_index type, const SubscriptionId id) {
        const auto it{handlers_.find(type)};
        if (it == handlers_.end()) return;
        const auto& list = it->second;
        list.erase(
            std::ranges::remove_if(list, [id](const auto& entry){ return entry.first == id; }),
            list.end());
    }

    std::unordered_map<
        std::type_index,
        std::vector<std::pair<
            SubscriptionId,
            std::function<void(const void*)>>>
        > handlers_;
    SubscriptionId nextId_{1};
};
