---
tags:
  - programming-language
  - docker
---
# Синтаксис `docker-compose.yml`

## Базовая структура

```yaml
services:
  service_name:
    # конфигурация контейнера

networks:
  network_name:

volumes:
  volume_name:
```

Три верхнеуровневых секции: **services** (обязательная) — что запускать, **networks** и **volumes** (опциональные) — какая инфраструктура для этого нужна.

## `services` — сердце файла

Каждый сервис описывает один "тип" контейнера (можно масштабировать в несколько экземпляров).

### `build` — собрать образ из Dockerfile

```yaml
services:
  consumer:
    build: ./consumer
```

Это эквивалент `docker build -t ... ./consumer`, только Compose сам придумывает имя образа (обычно `<имя_проекта>-consumer`).

Развёрнутая форма, если нужен нестандартный Dockerfile или build-аргументы:

```yaml
services:
  consumer:
    build:
      context: ./consumer
      dockerfile: Dockerfile.prod
      args:
        BUILD_MODE: release
```

### `image` — использовать готовый образ вместо сборки

```yaml
services:
  db:
    image: postgres:16
```

Можно комбинировать с `build` — тогда `image` задаёт имя/тег для собранного образа:

```yaml
services:
  consumer:
    build: ./consumer
    image: myregistry.io/consumer:1.0
```

### `ports` — проброс портов (аналог `-p`)

```yaml
services:
  consumer:
    ports:
      - "5000:5000"     # хост:контейнер
      - "8080:80"
```

### `environment` — переменные окружения (аналог `-e`)

```yaml
services:
  db:
    environment:
      POSTGRES_PASSWORD: secret
      POSTGRES_DB: myapp
    # или через файл:
    env_file:
      - .env
```

### `depends_on` — порядок запуска

```yaml
services:
  producer:
    depends_on:
      - consumer
```

**Важный нюанс:** по умолчанию `depends_on` гарантирует только порядок **старта** контейнера, а не то, что приложение внутри уже готово принимать соединения. Если `consumer` запустился, но его `main()` ещё не дошёл до `acceptor.async_accept`, `producer` может попытаться подключиться слишком рано и получить ошибку.

Более надёжный вариант — с проверкой готовности через healthcheck:

```yaml
services:
  consumer:
    build: ./consumer
    healthcheck:
      test: ["CMD", "nc", "-z", "localhost", "5000"]
      interval: 2s
      timeout: 2s
      retries: 5

  producer:
    build: ./producer
    depends_on:
      consumer:
        condition: service_healthy
```

Тут `producer` реально дождётся, пока `consumer` пройдёт healthcheck (порт 5000 действительно слушается), а не просто "контейнер запущен".

### `networks` — какие сети подключить

```yaml
services:
  consumer:
    networks:
      - mynet
```

Если секцию `networks` у сервиса вообще не указывать — Compose **автоматически** создаёт для всего проекта одну общую сеть по умолчанию, и все сервисы в неё попадают. Явно объявлять сеть нужно только если хочешь несколько изолированных сетей или специфичные настройки.

### `volumes` — монтирование (аналог `-v`)

```yaml
services:
  db:
    volumes:
      - db_data:/var/lib/postgresql/data      # named volume
      - ./config:/etc/myapp/config:ro          # bind mount, ro = read-only
```

### `restart` — политика перезапуска

```yaml
services:
  consumer:
    restart: unless-stopped
    # варианты: no (по умолчанию), always, on-failure, unless-stopped
```

## Секция верхнего уровня `networks` и `volumes` — именование

```yaml
networks:
  mynet:
    driver: bridge   # по умолчанию и так bridge, можно не указывать явно

volumes:
  db_data:           # просто объявление — Docker создаст named volume при первом использовании
```

## Основные команды

```powershell
docker compose up              # создать сеть, собрать образы (если надо), запустить всё
docker compose up -d           # то же самое, но в фоне
docker compose up --build      # принудительно пересобрать образы перед запуском

docker compose down            # остановить и удалить контейнеры + сеть (volumes останутся!)
docker compose down -v         # то же самое, но удалить ещё и volumes

docker compose logs            # логи всех сервисов сразу
docker compose logs -f consumer   # логи конкретного сервиса, в реальном времени

docker compose build           # только собрать образы, не запускать
docker compose ps              # список контейнеров проекта (аналог docker ps, но только для этого compose-проекта)

docker compose restart consumer   # перезапустить один сервис
docker compose exec consumer sh   # зайти в работающий контейнер (аналог docker exec)
```
