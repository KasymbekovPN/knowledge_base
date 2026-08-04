---
tags:
  - programming-language
  - architecture
---
[[raw data/application architecture/_|<=]]


6 отдельных CMake-библиотек + один исполняемый файл, композиционный корень — `app/main.cpp`.

- __event_bus__ (header-only, зависимостей нет)
- __config__ (данные — app_title, priority_labels)
- __task_domain__ → event_bus          # Model, единственный источник истины
- __commands__ → task_domain           # ICommand + Add/Remove/Complete/Reopen + CommandHistory (undo/redo) + parser текста
- __event_loop__ → commands, task_domain   # читает строки, парсит в ICommand, крутит CommandHistory
- __view__ (header-only) → task_domain, config, event_bus   # подписывается на события Model сама
- __app/main.cpp__ → event_loop + view     # единственное место, где они встречаются

Ключевое: `event_loop` и `view` никогда не ссылаются друг на друга — обе стороны знают только про `task_domain` (Model). Model публикует события через `EventBus`, View сама подписывается в конструкторе, EventLoop мутирует Model только через `ICommand`. 

![[pet_project_component_diagram.svg|700]]
