---
tags:
  - programming-language
  - docker
---
# Docker в GitHub Actions и GitLab CI

## Общая идея — три составляющих

Независимо от платформы, типичный CI-пайплайн с Docker делает три вещи: **собирает** образ, опционально **тестирует** (например, здесь — реально поднимает consumer/producer и проверяет обмен сообщениями), и **публикует** в registry.

## GitHub Actions

### Базовый вариант — сборка через встроенный Docker

GitHub-раннеры уже имеют Docker предустановленным, поэтому простейший пайплайн:

```yaml
# .github/workflows/docker-build.yml
name: Build and test Docker images

on:
  push:
    branches: [main]
  pull_request:

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout код
        uses: actions/checkout@v4

      - name: Собрать consumer
        run: docker build -t consumer:${{ github.sha }} ./consumer

      - name: Собрать producer
        run: docker build -t producer:${{ github.sha }} ./producer

      - name: Создать тестовую сеть
        run: docker network create mynet

      - name: Запустить consumer
        run: docker run -d --name consumer --network mynet consumer:${{ github.sha }}

      - name: Проверить обмен сообщениями
        run: |
          OUTPUT=$(docker run --rm --network mynet producer:${{ github.sha }} consumer "ci-test")
          echo "$OUTPUT"
          echo "$OUTPUT" | grep -q "ECHO: ci-test" || (echo "Тест провален!" && exit 1)

      - name: Логи consumer (для отладки в случае падения)
        if: failure()
        run: docker logs consumer
```

Это уже полноценный CI: на каждый push или PR сборка обоих образов и **реальная проверка**, что они действительно общаются друг с другом через user-defined bridge — то же самое, что мы проверяли руками, только автоматически.

### Более "правильный" вариант — `docker/build-push-action`

Официальный action удобнее для публикации в registry и работы с multi-platform сборками:

```yaml
name: Build, test and push

on:
  push:
    branches: [main]

jobs:
  build-and-push:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Настроить Buildx
        uses: docker/setup-buildx-action@v3

      - name: Войти в Docker Hub
        uses: docker/login-action@v3
        with:
          username: ${{ secrets.DOCKERHUB_USERNAME }}
          password: ${{ secrets.DOCKERHUB_TOKEN }}

      - name: Собрать и запушить consumer
        uses: docker/build-push-action@v6
        with:
          context: ./consumer
          push: true
          tags: pablo/consumer:latest,pablo/consumer:${{ github.sha }}

      - name: Собрать и запушить producer
        uses: docker/build-push-action@v6
        with:
          context: ./producer
          push: true
          tags: pablo/producer:latest,pablo/producer:${{ github.sha }}
```

**Важный момент про секреты:** `DOCKERHUB_TOKEN` — это Personal Access Token (который мы разбирали в теме про `docker login`), а не пароль от аккаунта. Хранится в **Settings → Secrets and variables → Actions** репозитория на GitHub, никогда не пишется в сам YAML открытым текстом.

## GitLab CI

GitLab устроен чуть иначе — по умолчанию у раннера нет доступа к Docker daemon хоста, поэтому нужен **Docker-in-Docker (dind)**:

```yaml
# .gitlab-ci.yml
stages:
  - build
  - test
  - push

variables:
  DOCKER_HOST: tcp://docker:2376
  DOCKER_TLS_CERTDIR: "/certs"

build-consumer:
  stage: build
  image: docker:24
  services:
    - docker:24-dind
  script:
    - docker build -t consumer:$CI_COMMIT_SHA ./consumer
    - docker save consumer:$CI_COMMIT_SHA -o consumer-image.tar
  artifacts:
    paths:
      - consumer-image.tar

build-producer:
  stage: build
  image: docker:24
  services:
    - docker:24-dind
  script:
    - docker build -t producer:$CI_COMMIT_SHA ./producer
    - docker save producer:$CI_COMMIT_SHA -o producer-image.tar
  artifacts:
    paths:
      - producer-image.tar

integration-test:
  stage: test
  image: docker:24
  services:
    - docker:24-dind
  needs: ["build-consumer", "build-producer"]
  script:
    - docker load -i consumer-image.tar
    - docker load -i producer-image.tar
    - docker network create mynet
    - docker run -d --name consumer --network mynet consumer:$CI_COMMIT_SHA
    - sleep 2
    - |
      OUTPUT=$(docker run --rm --network mynet producer:$CI_COMMIT_SHA consumer "gitlab-ci-test")
      echo "$OUTPUT" | grep -q "ECHO: gitlab-ci-test"

push-images:
  stage: push
  image: docker:24
  services:
    - docker:24-dind
  needs: ["integration-test"]
  only:
    - main
  script:
    - echo "$CI_REGISTRY_PASSWORD" | docker login -u "$CI_REGISTRY_USER" --password-stdin $CI_REGISTRY
    - docker load -i consumer-image.tar
    - docker load -i producer-image.tar
    - docker tag consumer:$CI_COMMIT_SHA $CI_REGISTRY_IMAGE/consumer:latest
    - docker tag producer:$CI_COMMIT_SHA $CI_REGISTRY_IMAGE/producer:latest
    - docker push $CI_REGISTRY_IMAGE/consumer:latest
    - docker push $CI_REGISTRY_IMAGE/producer:latest
```

**Что тут нового по сравнению с GitHub Actions:**

- **`docker:24-dind`** — сервис "Docker внутри Docker", нужен раннеру, чтобы вообще иметь доступ к демону (GitHub-раннеры дают это "из коробки", GitLab — нет)
- **`$CI_REGISTRY_IMAGE`** — переменная, которую GitLab подставляет автоматически, если используешь встроенный **GitLab Container Registry** (не обязательно Docker Hub) — плюс не нужно заводить отдельные секреты, `$CI_REGISTRY_USER`/`$CI_REGISTRY_PASSWORD` GitLab предоставляет сам для встроенного registry
- **`artifacts`** — передача собранного образа между стадиями через `docker save`/`docker load`, потому что каждая стадия в GitLab CI по умолчанию — новый изолированный контейнер, не видящий образы, собранные в предыдущей стадии

## Ключевое отличие двух платформ

| |GitHub Actions|GitLab CI|
|---|---|---|
|Доступ к Docker daemon|сразу есть|нужен `dind` сервис|
|Встроенный registry|нет (нужен внешний Docker Hub/GHCR)|есть свой (`$CI_REGISTRY_IMAGE`)|
|Официальный build action|`docker/build-push-action`|нет аналога, обычно чистый `docker build`|

## Пока не рассматривали — кэш между запусками

Обрати внимание: в обоих примерах выше **каждый запуск CI пересобирает всё с нуля** — раннер эфемерный, локального кэша между запусками нет. Именно эту проблему решает следующая тема — кэш-стратегии в CI (`cache-from`/`cache-to`, `type=gha`, `type=registry`).
