---
tags:
  - programming-language
  - docker
---

# Кэширование слоёв в CI — продолжение прошлой темы

Как мы выяснили — раннер эфемерный, поэтому Docker-кэш, который мы получаем "бесплатно" локально, тут не работает сам по себе. Нужно явно сказать CI, **где** хранить кэш между запусками и **как** его переиспользовать.

## GitHub Actions — `type=gha`

Самый простой вариант, если используешь `docker/build-push-action` — встроенная интеграция с кэшем самого GitHub Actions:

```yaml
- name: Собрать и запушить consumer
  uses: docker/build-push-action@v6
  with:
    context: ./consumer
    push: true
    tags: pablo/consumer:latest,pablo/consumer:${{ github.sha }}
    cache-from: type=gha
    cache-to: type=gha,mode=max
```

- **`cache-from: type=gha`** — перед сборкой попытаться подтянуть кэш слоёв из хранилища GitHub Actions
- **`cache-to: type=gha,mode=max`** — после сборки сохранить туда **все** слои, включая промежуточные стадии multi-stage build (`mode=max`), а не только слои финального образа (`mode=min` — по умолчанию)

**Важный нюанс про `mode=max`:** раз у нас multi-stage build (`builder` → `final`), `mode=min` закэширует только слои финальной стадии. Если исходники не менялись, но ты хочешь, чтобы кэшировалась ещё и `builder`-стадия (компилятор, установленные зависимости) — обязательно нужен `mode=max`, иначе кэш `builder`-стадии просто не будет сохраняться между запусками.

## GitHub Actions — `type=registry` (альтернатива)

Вместо встроенного кэша GHA можно хранить кэш прямо в registry рядом с самим образом:

```yaml
- name: Собрать и запушить consumer
  uses: docker/build-push-action@v6
  with:
    context: ./consumer
    push: true
    tags: pablo/consumer:latest
    cache-from: type=registry,ref=pablo/consumer:buildcache
    cache-to: type=registry,ref=pablo/consumer:buildcache,mode=max
```

Тут в registry появляется дополнительный "образ" `pablo/consumer:buildcache` — по факту это не рабочий образ, а просто упакованные слои для переиспользования. Плюс такого подхода — кэш не привязан к конкретному GitHub-репозиторию/раннеру, можно шарить между разными CI-системами, если понадобится.

## GitLab CI — через registry кэш (аналог `type=registry`)

В GitLab CI, поскольку нет специального встроенного действия как `build-push-action`, кэш обычно реализуют вручную через `--cache-from`, подтягивая ранее запушенный образ:

```yaml
build-consumer:
  stage: build
  image: docker:24
  services:
    - docker:24-dind
  script:
    - echo "$CI_REGISTRY_PASSWORD" | docker login -u "$CI_REGISTRY_USER" --password-stdin $CI_REGISTRY
    # Пытаемся подтянуть предыдущий образ как источник кэша (если его ещё нет — просто игнорируем ошибку)
    - docker pull $CI_REGISTRY_IMAGE/consumer:latest || true
    - docker build --cache-from $CI_REGISTRY_IMAGE/consumer:latest -t $CI_REGISTRY_IMAGE/consumer:$CI_COMMIT_SHA ./consumer
    - docker push $CI_REGISTRY_IMAGE/consumer:$CI_COMMIT_SHA
```

Это работает по принципу: "скачай последнюю опубликованную версию образа, используй её слои как базу для сравнения — если инструкция Dockerfile и её входные данные совпадают с тем, что уже в этом образе, переиспользуй слой вместо пересборки".

**Ограничение:** обычный `--cache-from` в классическом (не-BuildKit) движке умеет матчить только слои **финальной** стадии — для multi-stage придётся указать `--cache-from` для каждой промежуточной стадии отдельно, либо переключиться на BuildKit-синтаксис:

```yaml
variables:
  DOCKER_BUILDKIT: "1"

build-consumer:
  stage: build
  script:
    - docker pull $CI_REGISTRY_IMAGE/consumer:buildcache || true
    - docker build \
        --build-arg BUILDKIT_INLINE_CACHE=1 \
        --cache-from $CI_REGISTRY_IMAGE/consumer:buildcache \
        -t $CI_REGISTRY_IMAGE/consumer:$CI_COMMIT_SHA \
        -t $CI_REGISTRY_IMAGE/consumer:buildcache \
        ./consumer
    - docker push $CI_REGISTRY_IMAGE/consumer:$CI_COMMIT_SHA
    - docker push $CI_REGISTRY_IMAGE/consumer:buildcache
```

## Практический эффект — что это реально экономит

Возвращаясь к нашему проекту: если в `consumer/Dockerfile` не менялся `apk add --no-cache build-base cmake boost-dev` (тяжёлая инструкция, качает и ставит десятки МБ пакетов) — с правильно настроенным кэшем CI **не будет** переустанавливать эти пакеты при каждом запуске пайплайна, если изменился только `main.cpp`. Без кэша — эта установка происходит заново на **каждом** push, что на CI-раннере (часто более медленном, чем локальная машина, да ещё с сетевым скачиванием пакетов) может добавлять минуты к каждой сборке.

## Итоговое правило

Тот же принцип, что и с обычным layer caching (стабильное — в начало, часто меняющееся — в конец Dockerfile), просто применённый ещё и на уровне CI: явно указать, **откуда** брать кэш при старте раннера и **куда** его сохранить по завершении, потому что раннер сам по себе ничего не помнит между запусками.
