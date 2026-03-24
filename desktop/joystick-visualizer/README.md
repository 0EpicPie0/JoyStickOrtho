# Joystick Visualizer (Desktop)

Приложение на C++/SFML для визуализации данных джойстика, получаемых по `Serial` в формате JSON.

## Возможности

- Чтение данных из последовательного порта.
- Парсинг JSON-пакетов формата `{"x":...,"y":...,"cmd":"..."}`.
- Отрисовка сглаженной траектории джойстика.
- Режим разработчика (`JOYSTICK_DEV_MODE`) для подстройки параметров.

## Требования

- C++20
- SFML 3
- POSIX serial API (`termios`) — Linux/macOS

## Сборка

### CMake

```bash
cd desktop/joystick-visualizer
cmake -S . -B build
cmake --build build
```

### Makefile

```bash
cd desktop/joystick-visualizer
make
```

## Запуск

```bash
cd desktop/joystick-visualizer
./joystick
```

Полезные переменные окружения:

- `JOYSTICK_SERIAL_PORT` — путь к serial-порту (по умолчанию: `/dev/cu.usbserial-10`)
- `JOYSTICK_CONFIG` — путь к config-файлу (по умолчанию: `joystick_config.cfg`)
- `JOYSTICK_FONT` — путь к TTF/OTF-файлу шрифта

## Конфиг

Файл: `joystick_config.cfg`

Параметры:

- `sensitivity`
- `smoothing`
- `dead_zone`

## Примечание по ESP32-S3

Оригинальная прошивка `ESP32-S3`, передававшая JSON, утеряна.  
Описание формата пакетов и требований к сериализации: `../../firmware/esp32-s3/README.md`.
