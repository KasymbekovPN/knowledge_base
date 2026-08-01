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
            if (!bus_) return;
            bus_->unsubscribeRaw(type_, id_);
            bus_ = nullptr;
        }

    private:
        friend class EventBus;
        Connection(EventBus* bus, const std::type_index type, const SubscriptionId id):
            bus_{bus}, type_{type}, id_{id} {}

        EventBus* bus_{nullptr};
        std::type_index type_{typeid(void)};
        SubscriptionId id_{0};
    };

    template<typename EventT>
    Connection subscribe(std::function<void(const EventT&)> handler) {
        const auto TYPE{std::type_index(typeid(EventT))};
        const SubscriptionId ID{nextId_++};
        handlers_[TYPE].push_back({
            ID,
            [handler](const void* e) { handler(*static_cast<const EventT*>(e)); }
        });

        return {this, TYPE, ID};
    }

    template<typename EventT>
    void publish(const EventT& event) {
        const auto IT{handlers_.find(std::type_index(typeid(EventT)))};
        if (IT == handlers_.end()) return;
        auto listCopy = IT->second;
        for (const auto& fn: listCopy | std::views::values) fn(&event);
    }

private:
    void unsubscribeRaw(const std::type_index type, const SubscriptionId id) {
        const auto IT{handlers_.find(type)};
        if (IT == handlers_.end()) return;
        auto& list = IT->second;
        list.erase(
            std::ranges::remove_if(list, [id](const auto& entry) { return entry.first == id; }).begin(),
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
