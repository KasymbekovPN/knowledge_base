---
tags:
  - programming-language
  - docker
---
![[bind_mount_vs_named_volume.svg|697]]

## Проблема, которую решают Volumes

Как разобрали в прошлом сообщении — writable-слой контейнера эфемерный. **Volumes** — это механизм вынести данные за пределы жизненного цикла контейнера, в отдельное хранилище, которое переживает `docker rm`.

Есть два основных подхода — они видны на схеме.

## Bind mount — монтируем конкретную папку хоста

Ты сам указываешь путь на хосте, который становится видимым внутри контейнера.

```powershell
docker run -v C:/projects/mydata:/app/data alpine
```

**Характеристики:**

- Ты полностью контролируешь, где физически лежат данные
- Удобно для разработки — например, монтировать папку с исходным кодом, чтобы менять файлы на хосте и сразу видеть изменения в контейнере, без пересборки образа
- Docker не управляет этой папкой — можно случайно удалить/переместить её обычными средствами ОС

## Named volume — управляется Docker'ом

Ты создаёшь volume с именем, а где физически хранятся данные — решает сам Docker (обычно `/var/lib/docker/volumes/...` внутри VM Docker Desktop).

```powershell
docker volume create my_data
docker run -v my_data:/app/data alpine
```

**Характеристики:**

- Не нужно думать о конкретном пути на хосте
- Docker сам управляет жизненным циклом, есть команды для инспекции/бэкапа
- Стандартный выбор для продакшена — баз данных, персистентных данных приложений

## Управление volumes

```powershell
docker volume create my_data      # создать
docker volume ls                  # список всех volumes
docker volume inspect my_data     # где физически лежит, метаданные
docker volume rm my_data          # удалить (только если не используется контейнером)
docker volume prune               # удалить все неиспользуемые volumes
```

## Синтаксис `-v` vs `--mount`

Есть два способа задать монтирование:

```powershell
# -v — короткий синтаксис
docker run -v my_data:/app/data alpine
docker run -v C:/projects/data:/app/data alpine

# --mount — явный, многословный, рекомендуется Docker'ом для продакшена
docker run --mount source=my_data,target=/app/data alpine
docker run --mount type=bind,source=C:/projects/data,target=/app/data alpine
```

`--mount` требует явно указывать `type=volume` или `type=bind` — меньше шансов на опечатку или путаницу (у `-v` частая ошибка: перепутать порядок хост/контейнер, а Docker в некоторых случаях просто создаст новый volume с "странным" именем вместо ошибки).

## Практика — проверим, что данные переживают `docker rm`

```powershell
# Создаём volume и пишем в него
docker run -v my_data:/app/data alpine sh -c "echo hello > /app/data/test.txt"

# Удаляем контейнер (не volume!)
docker container prune -f

# Запускаем НОВЫЙ контейнер с тем же volume
docker run -v my_data:/app/data alpine cat /app/data/test.txt
```

Ожидаемо: `hello` — контейнер был удалён, а данные в volume остались, потому что volume существует независимо от конкретного контейнера.
