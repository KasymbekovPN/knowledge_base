---
tags:
  - programming-language
  - docker
---

# Базовые команды:

## `docker ps` — список контейнеров

Показывает **запущенные** контейнеры.

```powershell
docker ps
```

Вывод:

```
CONTAINER ID   IMAGE           COMMAND    STATUS          PORTS     NAMES
a1b2c3d4e5f6   practice:1.0    "sh"       Up 5 minutes              c1
```

**Полезные флаги:**

```powershell
docker ps -a          # показать ВСЕ контейнеры, включая остановленные
docker ps -q          # только ID (удобно комбинировать с другими командами)
docker ps -a -q       # ID всех контейнеров, включая остановленные
docker ps --filter "status=exited"   # только остановленные
```

Частый паттерн — удалить сразу все остановленные контейнеры:

```powershell
docker rm $(docker ps -a -q --filter "status=exited")
```

## `docker images` — список образов

Показывает все образы, скачанные или собранные локально.

```powershell
docker images
```

Вывод:

```
REPOSITORY   TAG       IMAGE ID       CREATED         SIZE
practice     1.0       f4a2b8c91d3e   2 hours ago     7.34MB
alpine       3.20      324bc02ae123   3 weeks ago     7.34MB
```

**Полезные флаги:**

```powershell
docker images -q                  # только ID образов
docker images -f "dangling=true"  # "висячие" образы без тега (мусор после пересборок)
```

Удалить весь такой мусор:

```powershell
docker image prune
```

## `docker logs` — логи контейнера

Показывает вывод (stdout/stderr), который контейнер напечатал с момента старта.

```powershell
docker logs c1
```

**Полезные флаги:**

```powershell
docker logs -f c1        # follow — стримить логи в реальном времени (как tail -f)
docker logs --tail 50 c1 # последние 50 строк
docker logs --since 10m c1  # логи только за последние 10 минут
docker logs -t c1        # с таймстампами
```

Это первое, что стоит проверять, если контейнер упал или ведёт себя не так, как ожидаешь.

## `docker exec` — выполнить команду внутри УЖЕ ЗАПУЩЕННОГО контейнера

Отличие от `docker run` принципиальное: `run` создаёт **новый** контейнер, `exec` заходит в **существующий работающий**.

```powershell
docker exec c1 cat /app/note.txt
```

Самый частый вариант — зайти внутрь интерактивно, как по SSH:

```powershell
docker exec -it c1 sh
```

- `-i` (interactive) — держать stdin открытым
- `-t` (tty) — выделить псевдотерминал, чтобы был нормальный интерактивный шелл

Если в образе есть bash — можно `docker exec -it c1 bash`, но в alpine-образах его обычно нет, только `sh`.

## `docker run` — создать и запустить новый контейнер

Это самая часто используемая команда. Она делает сразу три вещи: если образа нет локально — скачивает его (`pull`), создаёт новый контейнер из образа, запускает его.

```powershell
docker run practice:1.0
```

### Ключевые флаги

|Флаг|Что делает|
|---|---|
|`-it`|интерактивный режим + терминал (для shell'а внутри контейнера)|
|`-d`|detached — запустить в фоне, вернуть управление терминалу сразу|
|`--name c1`|дать контейнеру понятное имя вместо случайного|
|`--rm`|удалить контейнер автоматически после остановки|
|`-p 8080:80`|пробросить порт: `хост:контейнер`|
|`-e VAR=value`|задать переменную окружения|
|`-v host_path:container_path`|смонтировать volume/папку (разберём подробно на Этапе 3)|

### Примеры

```powershell
# Фоновый веб-сервер с проброшенным портом
docker run -d --name web -p 8080:80 nginx

# Одноразовый контейнер для теста — сам удалится после выхода
docker run -it --rm alpine sh

# С переменной окружения
docker run -e POSTGRES_PASSWORD=secret -d postgres:16
```

### Важный нюанс — `run` vs повторный запуск

Каждый вызов `docker run` создаёт **новый** контейнер, даже если образ тот же самый:

```powershell
docker run --name a1 practice:1.0
docker run --name a1 practice:1.0   # ОШИБКА: имя уже занято
```

Если контейнер с таким именем уже существует (даже остановленный) — получишь ошибку `Conflict. The container name "/a1" is already in use`. Чтобы просто **перезапустить существующий**, используется другая команда:

```powershell
docker start a1     # запустить существующий, не создавая новый
```

## `docker rm` — удалить контейнер

Удаляет контейнер **полностью**, вместе с его writable-слоем — то есть со всеми данными, которые в нём накопились и не были вынесены в volume.

```powershell
docker rm c1
```

**Важное ограничение:** нельзя удалить запущенный контейнер напрямую:

```powershell
docker rm c1
# Error response from daemon: cannot remove running container
```

Сначала останови, потом удали:

```powershell
docker stop c1
docker rm c1
```

Либо принудительно, одной командой:

```powershell
docker rm -f c1
```

(`-f` = force, эквивалент `stop` + `rm`)

### Массовое удаление

```powershell
# Удалить все остановленные контейнеры разом
docker container prune

# Удалить конкретный список
docker rm c1 c2 c3
```

## Как это соотносится с предыдущими командами

```powershell
docker run --name test -d nginx   # создать и запустить
docker ps                          # убедиться, что работает
docker logs test                   # посмотреть логи
docker exec -it test sh            # залезть внутрь
docker stop test                   # остановить
docker rm test                     # удалить насовсем
```

Или короче — если контейнер был одноразовым:

```powershell
docker rm -f test
```

## Как эти команды складываются в рабочий цикл

```powershell
docker ps                    # что вообще сейчас крутится?
docker logs -f my_container  # почему оно себя странно ведёт?
docker exec -it my_container sh   # залезть и посмотреть руками
docker images                # какие образы вообще есть на диске?
```

Это ровно тот набор, которым пользуешься каждый день при отладке — прежде чем разбирать `docker inspect` и более редкие команды.
