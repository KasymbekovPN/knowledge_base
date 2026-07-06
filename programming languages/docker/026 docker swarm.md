---
tags:
  - programming-language
  - docker
---
![[docker_swarm_architecture.svg|697]]

## Что такое Docker Swarm

Swarm — это встроенный в сам Docker режим кластеризации. В отличие от Kubernetes, который нужно ставить и настраивать отдельно (или через minikube для локального теста), Swarm уже есть в каждой установке Docker — его просто нужно **включить**.

```powershell
docker swarm init
```

Эта одна команда превращает текущую машину в **Manager Node** — и у тебя уже есть работающий (пока из одного узла) кластер.

## Ключевые понятия Swarm

- **Manager Node** — принимает решения об оркестрации: куда ставить задачи, следит за состоянием кластера. Если Manager Node несколько (для отказоустойчивости самого control plane) — они синхронизируются через **Raft consensus** — алгоритм, гарантирующий, что все manager'ы согласны о текущем состоянии кластера, даже если один из них временно недоступен.
- **Worker Node** — выполняет реальную работу, то есть запускает контейнеры.
- **Service** — декларативное описание "какой образ, сколько реплик, какие порты" — концептуально прямой аналог `services:` в `docker-compose.yml`.
- **Task** — конкретный экземпляр контейнера, порождённый сервисом. Если у сервиса `replicas: 2` — будет 2 задачи, размазанные по доступным Worker Node (как видно на схеме: `consumer.1` на одном узле, `consumer.2` на другом).

## Самое приятное — синтаксис почти идентичен Compose

Это ключевое практическое преимущество Swarm перед Kubernetes для тебя прямо сейчас: **тот же `docker-compose.yml`, который мы уже написали**, можно (с минимальными правками) развернуть в Swarm:

```yaml
services:
  consumer:
    image: pablo/consumer:1.0
    deploy:
      replicas: 2
      restart_policy:
        condition: on-failure
    networks:
      - mynet

  producer:
    image: pablo/producer:1.0
    deploy:
      replicas: 1
    networks:
      - mynet

networks:
  mynet:
    driver: overlay
```

Единственное принципиально новое — секция `deploy` (игнорируется обычным `docker compose up`, но используется в Swarm-режиме) и `driver: overlay` для сети — это как раз тот **overlay network**, о котором мы говорили в контексте "могут ли контейнеры на разных хостах общаться через user-defined bridge" — не могут, а через overlay в Swarm уже могут.

## Разворачивание в Swarm

```powershell
docker swarm init
docker stack deploy -c docker-compose.yml myapp
```

```powershell
docker service ls          # список сервисов, аналог docker compose ps
docker service ps consumer # список задач конкретного сервиса, на каких узлах
docker service scale consumer=5   # масштабировать "на лету"
docker stack rm myapp      # снести весь стек
```

## Добавление worker-узлов в кластер

```powershell
# на manager-узле получить токен
docker swarm join-token worker

# на другой машине (реальной или виртуалке)
docker swarm join --token SWMTKN-... 192.168.1.10:2377
```

После этого Swarm начинает распределять задачи между всеми присоединёнными узлами автоматически — вот тут и происходит переход от "один хост" к "кластер", о котором мы говорили в позапрошлой теме.

## Swarm vs Kubernetes — теперь, когда видел оба

| |Docker Swarm|Kubernetes|
|---|---|---|
|Порог входа|низкий — `docker swarm init` и всё|высокий — множество концепций и компонентов|
|Формат конфигурации|Compose-файл + `deploy`|отдельный YAML-манифест (Deployment/Service/...)|
|Гибкость и возможности|базовые: реплики, health check, rolling update|огромная экосистема: autoscaling по метрикам, RBAC, CRD, service mesh, множество storage-провайдеров|
|Индустриальное распространение|нишевое, снижается|де-факто стандарт оркестрации|
|Встроен в Docker|да|нет, отдельный продукт|

## Практический вывод

Swarm отлично подходит, если тебе нужно **быстро** получить базовую отказоустойчивость и масштабирование для проекта, который уже описан через Compose, без изучения отдельной экосистемы — и твой накопленный опыт с `docker-compose.yml` почти полностью переносится. Kubernetes — выбор, если проект вырастет до уровня, где нужны более тонкие возможности (autoscaling, сложные stateful-сервисы, multi-tenant окружения), или если это требование индустрии/работодателя.
