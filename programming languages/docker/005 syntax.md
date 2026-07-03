---
tags:
  - programming-language
  - docker
---
# Синтаксис Dockerfile: основные инструкции

## `FROM` — базовый образ

Первая (обычно) инструкция в Dockerfile. Задаёт, на основе какого образа строится твой.

```dockerfile
FROM alpine:3.20
```

Можно использовать несколько `FROM` в одном файле — это называется **multi-stage build**. Каждый `FROM` начинает новый этап сборки.

```dockerfile
FROM golang:1.22 AS builder
# ... сборка
FROM alpine:3.20
# ... финальный образ
```

## `RUN` — выполнить команду во время сборки

Выполняется **один раз при сборке образа**, результат (изменения в файловой системе) фиксируется как новый слой.

```dockerfile
RUN apt-get update && apt-get install -y curl
```

**Важный практический момент** — каждый `RUN` создаёт отдельный слой. Если писать так:

```dockerfile
RUN apt-get update
RUN apt-get install -y curl
RUN apt-get clean
```

— получится 3 слоя, и мусор от `apt-get update` останется в предыдущем слое навсегда, даже после `clean`. Правильнее объединять через `&&`:

```dockerfile
RUN apt-get update && \
    apt-get install -y curl && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
```

Так это один слой, и итоговый образ меньше.

## `COPY` — скопировать файлы с хоста в образ

```dockerfile
COPY package.json /app/
COPY . /app/
```

Синтаксис: `COPY <откуда на хосте> <куда в образе>`. Путь на хосте — относительно **build context** (той папки, которую ты передаёшь в `docker build`, у тебя это была `"C:/projects/.../003 images and containers"`).

## `ADD` — то же самое, но с суперспособностями

`ADD` умеет всё, что `COPY`, плюс:

```dockerfile
# Автоматически распаковывает архивы
ADD app.tar.gz /app/

# Может скачивать по URL (но так лучше не делать)
ADD https://example.com/file.txt /app/
```

**Правило, которого стоит придерживаться:** используй `COPY` по умолчанию, `ADD` — только когда реально нужна автораспаковка архива. Причина: `ADD` ведёт себя менее предсказуемо (например, с URL не проверяет TLS-сертификаты и не удаляет файл при ошибке), и это официальная рекомендация из Docker-документации — "prefer COPY over ADD".

## `WORKDIR` — рабочая директория

Задаёт директорию, в которой будут выполняться все последующие `RUN`, `CMD`, `ENTRYPOINT`, `COPY`, `ADD`. Если директории нет — она создастся автоматически.

```dockerfile
WORKDIR /app
COPY . .
RUN npm install
CMD ["node", "index.js"]
```

Эквивалент `cd /app`, только персистентный для всех последующих инструкций (в отличие от `RUN cd /app`, который влияет только на саму эту строку).

## `CMD` — команда по умолчанию при старте контейнера

Выполняется **при запуске контейнера** (`docker run`), не при сборке.

```dockerfile
CMD ["node", "index.js"]
```

Два синтаксиса:

```dockerfile
CMD ["executable", "arg1", "arg2"]   # exec-форма — рекомендуется
CMD executable arg1 arg2             # shell-форма — оборачивается в /bin/sh -c
```

Разница важна: exec-форма не создаёт промежуточный shell-процесс, поэтому сигналы (`SIGTERM` при `docker stop`) доходят до приложения напрямую. В shell-форме сигнал сначала ловит `sh`, и не всегда правильно прокидывает дальше — из-за этого контейнер может не останавливаться мгновенно.

**Ключевая особенность:** `CMD` можно **переопределить** при запуске:

```dockerfile
CMD ["node", "index.js"]
```

```powershell
docker run myapp node other_script.js
# CMD полностью игнорируется, выполнится "node other_script.js"
```

## `ENTRYPOINT` — команда, которую нельзя просто так переопределить

Тоже задаёт команду при старте, но ведёт себя иначе.

```dockerfile
ENTRYPOINT ["node", "index.js"]
```

```powershell
docker run myapp --port=3000
# --port=3000 не заменяет ENTRYPOINT, а ДОБАВЛЯЕТСЯ к нему как аргумент:
# итоговая команда: node index.js --port=3000
```

Чтобы переопределить `ENTRYPOINT`, нужно явно передать флаг:

```powershell
docker run --entrypoint sh myapp
```

## `CMD` + `ENTRYPOINT` вместе — самый частый паттерн

Это классическая комбинация: `ENTRYPOINT` задаёт фиксированную программу, `CMD` — аргументы по умолчанию, которые легко переопределить.

```dockerfile
ENTRYPOINT ["python", "app.py"]
CMD ["--mode=production"]
```

```powershell
docker run myapp
# python app.py --mode=production

docker run myapp --mode=debug
# python app.py --mode=debug   (CMD полностью заменился)
```

Именно так устроены многие официальные образы — например, у `postgres` `ENTRYPOINT` запускает сервер, а `CMD` задаёт дефолтные флаги, которые можно подменить при `docker run`.

## Сводная таблица

|Инструкция|Когда выполняется|Можно переопределить при `docker run`|
|---|---|---|
|`RUN`|при сборке образа|нет (не применимо)|
|`CMD`|при старте контейнера|да, целиком|
|`ENTRYPOINT`|при старте контейнера|только через `--entrypoint`|
|`COPY` / `ADD`|при сборке образа|нет|
|`WORKDIR`|и при сборке, и при старте|нет|
|`FROM`|при сборке (задаёт базу)|нет|

## Практический пример

## Шаг 1 — структура проекта

Создай новую папку рядом с предыдущей (по аналогии с твоей нумерацией), например `004 cmd entrypoint`, и в ней два файла:

### cmd/greet.sh / entrypoint/greet.sh
```sh
#!/bin/sh
echo "Hello, $1!"
```

### cmd/Dockerfile

```dockerfile
FROM alpine:3.20  
  
WORKDIR /app  
  
copy greet.sh .  
RUN chmod +x greet.sh  
  
CMD ["./greet.sh", "world"]
```

## Шаг 2 — собери и запусти

```powershell
docker build -t greet:1.0 .
docker run greet:1.0
```

Ожидаемо:

```
Hello, world!
```

## Шаг 3 — переопредели `CMD`

```powershell
docker run greet:1.0 ./greet.sh Pablo
```

Результат:

```
Привет, Pablo!
```

`CMD` полностью заменился тем, что ты передал после имени образа. Docker даже не пытался ничего "сложить" — просто взял новую команду целиком.

## Шаг 4 — переделай на `ENTRYPOINT`

### entrypoint/Dockerfile
```dockerfile
FROM alpine:3.20

WORKDIR /app

COPY greet.sh .
RUN chmod +x greet.sh

ENTRYPOINT ["./greet.sh"]
CMD ["world"]
```

Пересобери под новым тегом:

```powershell
docker build -t greet:2.0 .
```

## Шаг 5 — запусти без аргументов

```powershell
docker run greet:2.0
```

```
Hello, world!
```

Работает так же — `ENTRYPOINT` выполнился, `CMD` подставился как аргумент по умолчанию.

## Шаг 6 — теперь попробуй "переопределить" так же, как раньше

```powershell
docker run greet:2.0 Pablo
```

```
Hello, Pablo!
```

Выглядит похоже, но механизм другой: тут `Pablo` не заменил `ENTRYPOINT`, а заменил `CMD` (аргумент). Реальная выполненная команда — `./greet.sh Pablo`, потому что `ENTRYPOINT` **всегда** остаётся `./greet.sh`, а всё, что передашь после имени образа, добавляется как аргументы к нему.

## Шаг 7 — момент, где разница становится видна явно

Попробуй сделать то же, что в Шаге 3, но теперь на `greet:2.0`:

```powershell
docker run greet:2.0 ./greet.sh Pablo
```

Результат будет **сломан**:

```
Hello, ./greet.sh!
```

Потому что реальная команда стала: `./greet.sh ./greet.sh Pablo` — `ENTRYPOINT` никуда не делся, а всё, что ты написал, ушло в `$1` и дальше как лишние аргументы. Скрипт получил `./greet.sh` в качестве имени, а `Pablo` просто проигнорировал (в скрипте используется только `$1`).

## Шаг 8 — как реально зайти внутрь контейнера с `ENTRYPOINT`

Раз `ENTRYPOINT` нельзя просто "отменить" аргументом — для этого есть отдельный флаг:

```powershell
docker run --entrypoint sh -it greet:2.0
```

Это единственный способ полностью заменить `ENTRYPOINT` на что-то другое (например, чтобы залезть в контейнер интерактивным shell'ом для отладки).

## Итог, который должен стать понятен на практике

| |`CMD`|`ENTRYPOINT`|
|---|---|---|
|Что происходит при `docker run image arg1 arg2`|`arg1 arg2` **заменяет** всю команду|`arg1 arg2` **добавляется** к `ENTRYPOINT`|
|Как заменить полностью|просто передать новую команду|нужен флаг `--entrypoint`|
|Типичное применение|значения по умолчанию, которые ожидаемо меняют|"жёстко прибитая" программа контейнера|
