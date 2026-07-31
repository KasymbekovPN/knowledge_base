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
        },        
        {            
	        "name": "release",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Release"  
            }  
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
        },  
        {            
	        "name": "release",  
            "configurePreset": "release"  
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
cmake_minimum_required(VERSION 3.40)  
project(abs_temp CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
// MVP vs MVVM на одном и том же примере (счётчик), чтобы структурная  
// разница была видна прямо в коде: кто на кого ссылается и кто кого зовёт.  
  
#include <functional>  
#include <iostream>  
#include <format>  
#include <string>  
  
// ---------------- Model - общий для обоих вариантов ----------------  
class CounterModel {  
public:  
    int value() const { return value_; }  
    void increment() { ++value_; }  
    void decrement() { if (value_ > 0) --value_; }  
  
private:  
    int value_{0};  
};  
  
// ============================================================  
// MVP: Presenter АКТИВНО вызывает методы View через интерфейс IView.  
// View пассивна - только отображает то, что ей явно сказали показать.  
// ============================================================  
namespace mvp {  
  
class IView {  
public:  
    virtual ~IView() = default;  
    virtual void showCount(int value) = 0;  
    virtual void showMessage(const std::string& msg) = 0;  
};  
  
// Presenter содержит ВСЮ логику "что и когда показывать" - ни Model,  
// ни View сами по себе этого не знают.  
class Presenter {  
public:  
    Presenter(CounterModel& model, IView& view): model_{model}, view_{view} {  
        view_.showCount(model_.value());  
    }  
    void onIncrementClicked() {  
        model_.increment();  
        view_.showCount(model_.value());  
    }  
    void onDecrementClicked() {  
        if (model_.value() == 0) {  
            view_.showMessage("Value is zero.");  
            return;  
        }        
        model_.decrement();  
        view_.showCount(model_.value());  
    }  
private:  
    CounterModel& model_;  
    IView& view_;  
};  
  
class ConsoleView: public IView {  
public:  
    void showCount(int value) override {  
        std::cout << std::format("  [MVP View] counter = {}\n", value);  
    }    
    void showMessage(const std::string& msg) override {  
        std::cout << std::format("  [MVP View] message: {}\n", msg);  
    }};  
  
// Fake для юнит-теста Presenter БЕЗ какого-либо реального UI  
class FakeView: public IView {  
public:  
    void showCount(const int value) override {  
        lastCount = value;  
        ++countCalls;  
    }    
    void showMessage(const std::string& msg) override {  
        lastMessage = msg;  
    }  
    int lastCount{-1};  
    int countCalls{0};  
    std::string lastMessage{""};  
};  
  
void demo() {  
    std::cout << "-- MVP: real View --\n";  
    CounterModel model;  
    ConsoleView view;  
    Presenter presenter{model, view};  
    presenter.onIncrementClicked();  
    presenter.onDecrementClicked();  
  
    std::cout << "\n--- MVP: unit0test Presenter without UI ---\n";  
    CounterModel testModel;  
    FakeView fakeView;  
    Presenter testPresenter{testModel, fakeView};  
    // проверяем бизнес-правило "не в минус"  
    testPresenter.onDecrementClicked();  
    std::cout << std::format("  countCalls = {}, lastMessage = '{}'\n", fakeView.countCalls, fakeView.lastMessage);  
}  
  
}  
  
// ============================================================  
// MVVM: ViewModel НЕ ЗНАЕТ о существовании View вообще - ни ссылки,  
// ни интерфейса. Вместо явных вызовов view.showX() - публикует изменения  
// через "точки привязки" (тот же принцип, что Signal/Slot), на которые  
// View подписывается сама.  
// ============================================================  
namespace mvvm {  
  
class CounterViewModel {  
public:  
    explicit CounterViewModel(CounterModel& model): model_{model} {}  
  
    void increment() {  
        model_.increment();  
        notify();  
    }  
    void decrement() {  
        if (model_.value() == 0) {  
            std::cout << std::format("value is zero");  
            return;  
        }        model_.decrement();  
        notify();  
    }  
    // Точки привязки (binding points) - View сама решает на них подписаться.  
    // ViewModel никогда не вызывает ничего у конкретного View - он даже    
    // не знает, что такое View, не то что какой у него тип.    
    std::function<void(int)> onCountChanged;  
    std::function<void(const std::string&)> onMessage;  
  
private:  
    void notify() {  
        if (onCountChanged) onCountChanged(model_.value());  
    }  
    CounterModel& model_;  
};  
  
// View сама устанавливает биндинг при создании - направление подписки  
// противоположно направлению вызова в MVP.  
class ConsoleView {  
public:  
    explicit ConsoleView(CounterViewModel& vm) {  
        vm.onCountChanged = [](int value) {  
            std::cout << std::format("  [MVVM View] counter = {}\n", value);  
        };        
        vm.onMessage = [](const std::string& msg) {  
            std::cout << std::format("  [MVVM View] message: {}\n", msg);  
        };    
    }
};  
  
void demo() {  
    std::cout << "\n--- MVVM --\n";  
    CounterModel model;  
    CounterViewModel vm{model};  
    // View подписывается на ViewModel, а не наоборот  
    const ConsoleView view{vm};  
    (void) view;  
  
    vm.increment();  
    vm.increment();  
    vm.decrement();  
}  
  
}  
  
int main() {  
    mvp::demo();  
    mvvm::demo();  
  
    return 0;  
}
```

**MVC — классическая, но неоднозначная точка отсчёта**

В оригинальной (Smalltalk) идее MVC View напрямую наблюдает за Model — почти как Observer из наших ранних примеров: Model меняется, оповещает подписанные View, каждая View сама решает, как перерисоваться. Controller отвечает только за **вход** — переводит действия пользователя в вызовы на Model, а иногда ещё и выбирает, какую View показать. Ключевая деталь, которую часто путают: в классическом MVC View знает про Model напрямую, Controller не обязан быть посредником для чтения данных — только для ввода.

То, что называют "MVC" в вебе (Rails, Django, ASP.NET MVC) — на самом деле другой паттерн под тем же именем: там Controller дёргает и Model, и View сам, View — это шаблон/сериализатор без какого-либо собственного состояния и подписок, потому что HTTP-ответ формируется один раз за запрос и выбрасывается — нет постоянно живущего View-объекта, который нужно синхронизировать с Model во времени. Стоит явно различать эти два значения одного термина — путаница между ними реально сбивает с толку людей, переходящих между desktop/game UI и web-бэкендом.

**MVP — View становится полностью пассивной**

`IView` в примере — чистый интерфейс без единой строчки логики: `showCount()`, `showMessage()`, ничего больше. Вся логика "что показать в ответ на что" переехала в `Presenter`, который держит ссылки и на `Model`, и на `IView`, и активно **вызывает** методы View. View про Model вообще ничего не знает — только про то, что Presenter ей явно скажет отобразить.

Главный практический выигрыш виден в тесте: `Presenter` протестирован с `FakeView` — вообще без единого реального UI-компонента, без запущенного окна, без event loop. Это прямое продолжение темы DI vs Service Locator: зависимость (`IView&`) передаётся явно через конструктор, значит её легко подменить на fake — та же логика, что была с `IPaymentGateway`.

**MVVM — ViewModel не знает о View вообще, даже через интерфейс**

Разница с MVP тонкая, но структурно важная: у `Presenter` есть `IView&` — он **обязан** знать хоть какой-то (пусть абстрактный) тип View. У `CounterViewModel` в примере нет вообще ничего похожего — только `std::function`-поля `onCountChanged`/`onMessage`, которые кто-то снаружи может подписать, а может и не подписывать. Направление противоположно MVP: не "ViewModel вызывает View", а "View сама подписывается на ViewModel" — ровно тот же принцип, что Signal/Slot из более раннего разбора (в реальных фреймворках — Qt `QML` + C++ `QObject`-свойства, WPF/XAML data binding — эта подписка вообще не пишется руками, а генерируется декларативным биндингом из разметки).

MVVM особенно хорош там, где есть готовый механизм привязки данных — без него MVVM на голом C++ (как в демо) по факту превращается в MVP с более гибкой, но менее очевидной для чтения связью через `std::function` вместо явного интерфейса.

**Где что уместно**

Десктопный GUI с полноценной поддержкой биндинга (Qt/QML, WPF, современный Avalonia) — естественная территория MVVM, инфраструктура сама делает то, что в демо пришлось бы писать руками. Более старые/менее декларативные тулкиты (голый Win32, старые Qt Widgets без активного использования property binding, wxWidgets) — там MVP надёжнее и понятнее: явный интерфейс `IView` проще отлаживать, чем сеть неявных подписок без biнding-фреймворка под капотом.

Игровой UI — обычно ни то, ни другое в чистом виде. Immediate-mode GUI (ImGui-стиль) вообще не создаёт постоянных View-объектов — интерфейс перерисовывается заново каждый кадр прямо из текущих данных, и вопрос "как синхронизировать View с Model во времени" просто не возникает, потому что нет долгоживущей View, которую нужно держать в актуальном состоянии. Более комплексный игровой UI иногда действительно использует MVVM-подобный биндинг (Unity UI Toolkit — UXML/USS с C#-биндингами, UMG в Unreal), но тяжёлые персистентные слои Presenter/ViewModel в реальном игровом цикле часто избыточны по производительности и плохо стыкуются с ECS-подходом, который разбирали раньше — там UI обычно проще смоделировать как ещё один набор компонентов и систему, читающую данные напрямую, а не как отдельную MVC-иерархию поверх геймплейных сущностей.

Сервер — здесь "MVC" (в веб-смысле) скорее организационный паттерн разделения ответственности (роутинг/валидация запроса в Controller, доступ к данным в Model, форматирование ответа в шаблоне/сериализаторе), а не паттерн синхронизации живого UI — сравнивать его с MVP/MVVM напрямую не совсем корректно, это разные по сути задачи под похожими именами.
