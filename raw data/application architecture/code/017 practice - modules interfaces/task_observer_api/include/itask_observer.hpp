// Контракт между host-приложением и плагинами-наблюдателями за задачами.
// Сигнатуры сознательно используют только POD-типы (int, const char*),
// а не std::string/std::vector - через границу dlopen/LoadLibrary лучше
// не пересекать STL-типы, чей layout зависит от версии/настроек
// стандартной библиотеки, с которой собран host, и может НЕ совпадать
// с той, с которой собран плагин (особенно если это разные компиляторы
// или версии компилятора). const char* - "наименьший общий знаменатель",
// который одинаково понимают любые двое.

#pragma once

constexpr int TASK_OBSERVER_API_VERSION{1};

class ITaskObserver {
public:
    virtual ~ITaskObserver() = default;
    virtual void onTaskAdded(int id, const char* title) = 0;
    virtual void onTaskRemoved(int id, const char* title) = 0;
    virtual void onTaskCompleted(int id) = 0;
    virtual void onTaskReopened(int id) = 0;
};

extern "C" {
    using CreateObserverFn = ITaskObserver* (*)();
    using DestroyObserverFn = void (*)(ITaskObserver*);
    using GetObserverApiVersionFn = int (*)();
}

#define OBSERVER_CREATE_FN_NAME "createTaskObserver"
#define OBSERVER_DESTROY_FN_NAME "destroyTaskObserver"
#define OBSERVER_API_VERSION_FN_NAME "taskObserverApiVersion"
