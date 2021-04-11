# highloadcup2021

## Поддерживаемые сборки

- release - предназначен для отправления решения на проверку
- debug - запускается с gdbserver и предназначен для подключения из clion
- dev - включает санитайзеры и проверки
- dev-thread - включает санитайзер для многопоточности
- profile-cpu - профилирование CPU
- profile-memory - профилирование памяти
- dump - stack trace dump
- lock - профилирование на lock contention

## Синхронизация директории на удаленный хост

Из корня проекта необходимо запустить:

```
jwatch -path . -script ./sync.sh
```

## Сборка и запуск

Выполняются из каталога app.

```
./build.sh RELEASE_TYPE
```

Запуск:

```
./run.sh
```

## Профилирование cpu или memory

- нужно собрать образ в режиме profile-cpu или profile-memory
- запустить контейнер
- выполнить скрипт `profile.sh`

Рекомендуется профилирование делать на linux хосте. Во-первых, с текущим `run.sh` веб сервер, который запускается
визуализацию профиля не доступен (нужно убрать --network host). Во-вторых, профилирование на macos имеет мало смысла.

Скрипт profile.sh выведет строку вида:

```
Serving web UI on http://172.17.0.4:9000
```

В режиме --network host IP адрес уже корректен в случае linux. На макос это будет ip адрес не хоста, а виртуалки,
поэтому по порту с хоста pprof доступен не будет. Без network host на macos работать должно, а вот на linux IP нужен
заменить на публичный адрес хоста.

## Профилирование на lock contention

- нужно собрать образ в режиме lock
- запустить контейнер
- после остановки будет распечатана статистика по lock contention

## Stack trace dump

- собрать образ в режиме dump
- запустить контейнер
- выполнить скрипт `dump.sh`

## Как запустить сервер локально

- перейти в директорию stubserver
- выполнить:

```
docker build -t hlc21_stub_server . && docker run -i --rm -t -e SERVER_RUN_TIME_IN_SECONDS=60000000 -p 8000:8000 hlc21_stub_server
```

## Подключение дебагером из clion

### Локально

- собрать с профилем debug и запустить. В логах должно быть:

```
Process ./highloadcup2021 created; pid = 18
Listening on port 1234
```

- создать в clion debug конфигурацию типа debug gdb remote debug:
    - GDB: bundled GDB
    - 'target remote' args: 127.0.0.1:1234
    - запустить конфигурацию

### Удаленно

- собрать локально бинарник с профилем debug
- собрать с профилем debug и запустить на удаленном хосте. В логах должно быть:

```
Process ./highloadcup2021 created; pid = 15
Listening on port 1234
```

- запустить контейнер с помощью команды `run.sh` (требуется, чтобы скопировать либы в sysroot)
- локально запустить команду для копирования sysroot из контейнера (требуется единоразово, когда обновляется OS или
  системные библиотеки в контейнере)

```
mkdir -p sysroot ; docker cp -L highloadcup-bin:/lib sysroot
```

- создать в clion debug конфигурацию типа debug gdb remote debug:
    - GDB: bundled GDB
    - 'target remote' args: BUILD3:1234
    - symbol file: путь к файлу `app/out/highloadcup2021`
    - sysroot: путь к `app/sysroot`
    - запустить конфигурацию

## Полезные ссылки:

- https://blog.feabhas.com/2015/11/becoming-a-rule-of-zero-hero/
