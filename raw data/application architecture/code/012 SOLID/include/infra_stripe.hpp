// infra_stripe.hpp — НИЗКОУРОВНЕВЫЙ модуль (конкретная деталь: конкретный
// платёжный провайдер). Обратите внимание на направление #include:
// infra зависит от domain (реализует его интерфейс), а не наоборот.
// Именно это и называется "инверсией" в Dependency Inversion - стрелка
// зависимости идёт от низкоуровневой детали к высокоуровневой абстракции,
// а не как в "наивной" многослойной архитектуре, где верхний слой обычно
// зависит от нижнего.
#pragma once

#include "domain.hpp"

class StripeGateway: public IPaymentGateway {
public:
    bool charge(const double amount) override;
};
