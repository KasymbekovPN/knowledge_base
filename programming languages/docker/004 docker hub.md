---
tags:
  - programming-language
  - docker
---

# Docker Hub — публичный registry

Docker Hub — это официальный публичный registry от Docker Inc., куда по умолчанию идут все команды `docker pull` / `docker push`, если явно не указан другой registry. Это как npm registry для Node.js или PyPI для Python, только для образов контейнеров.

## Структура имени образа

```
[registry/][namespace/]repository[:tag]
```

Примеры:

|Образ|Что это|
|---|---|
|`alpine`|сокращение от `docker.io/library/alpine:latest` — **официальный** образ|
|`alpine:3.20`|тот же официальный образ, конкретная версия (tag)|
|`pablo/my-app:1.0`|образ пользователя `pablo` в его личном namespace|
|`ghcr.io/pablo/my-app:1.0`|тот же образ, но уже не на Docker Hub, а на GitHub Container Registry|

Если `registry` не указан — Docker подставляет `docker.io` (Docker Hub). Если `namespace` не указан — подставляет `library` (это как раз официальные образы вроде `alpine`, `ubuntu`, `nginx`, `postgres`).

## Официальные vs community-образы

- **Official Images** (`library/*`) — курируются Docker и вендорами (например, `postgres` поддерживается совместно с PostgreSQL-командой). Прошли проверку безопасности, регулярно обновляются.
- **Verified Publisher** — образы от проверенных компаний (например `bitnami/*`), но не входят в `library`.
- **Обычные community-образы** — любой может запушить что угодно под своим namespace. Тут нужна осторожность: перед использованием чужого образа стоит глянуть Dockerfile и количество pull'ов/звёзд.

## Поиск и скачивание

```powershell
# Поиск образов (можно и на hub.docker.com — там удобнее)
docker search postgres

# Скачать без запуска
docker pull postgres:16

# Скачать и сразу запустить
docker run postgres:16
```

## Публикация своего образа

**1. Зарегистрироваться** на hub.docker.com — бесплатно.

**2. Авторизоваться в CLI:**

```powershell
docker login
```

**3. Образ должен быть назван с твоим username в начале** (иначе Docker Hub не поймёт, куда пушить):

```powershell
docker build -t pablo/practice:1.0 .
docker push pablo/practice:1.0
```

Если образ уже собран под другим именем — можно просто переприсвоить тег:

```powershell
docker tag practice:1.0 pablo/practice:1.0
docker push pablo/practice:1.0
```

## Важный практический момент — лимиты на anonymous pull

У Docker Hub есть **rate limit на скачивание** для неавторизованных и бесплатных аккаунтов (действует по IP или по аккаунту). Если в течение дня активно тянешь много образов (например, в CI) и вдруг начинаешь получать `toomanyrequests: You have reached your pull rate limit` — решение простое: `docker login` даже с бесплатным аккаунтом уже поднимает лимит заметно выше анонимного.

## Приватные репозитории

Бесплатный аккаунт Docker Hub даёт один приватный репозиторий. Для приватного образа команды `pull`/`push` те же самые — просто нужен `docker login`, и у образа должен быть доступ только у тебя (или тех, кому ты явно открыл).
