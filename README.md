# highloadcup2021

## Поддерживаемые сборки

- release - предназначен для отправления решения на проверку
- debug - включает санитайзеры и проверки
- debug-thread - включает санитайзер для многопоточности
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

## Профилирование на lock contention

- нужно собрать образ в режиме lock
- запустить контейнер
- после остановки будет распечатана статистика по lock contention