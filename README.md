# JoyStickOrtho

Проект с двумя реализациями джойстика на ESP32:

1. `XIAO-ESP32C6` с веб-интерфейсом (точка доступа + страница в браузере).
2. `ESP32-S3 + ПК`: desktop-приложение на C++ (SFML), которое читает JSON из `Serial`.

Прошивка для `ESP32-S3` сейчас отсутствует (утеряна), но формат данных и desktop-часть сохранены.

## Структура репозитория

```text
.
├── desktop/
│   └── joystick-visualizer/    # C++ приложение для ПК (SFML)
├── firmware/
│   ├── xiao-esp32c6-web/       # Прошивка XIAO-ESP32C6 с web UI
│   └── esp32-s3/               # Документация по утраченной S3-прошивке и JSON-протоколу
└── README.md
```

## Реализации

### 1) XIAO-ESP32C6 (web)

- Прошивка: `firmware/xiao-esp32c6-web/xiao_esp32c6_web.ino`
- Поднимает точку доступа Wi‑Fi и HTTP-сервер.
- Отдаёт JSON-данные джойстика через endpoint `/data`.

Подробности: `firmware/xiao-esp32c6-web/README.md`

### 2) ESP32-S3 + Desktop (Serial JSON)

- Desktop-приложение: `desktop/joystick-visualizer`
- Рисует траекторию движения джойстика в окне SFML.
- Читает строки JSON из последовательного порта.

Подробности: `desktop/joystick-visualizer/README.md` и `firmware/esp32-s3/README.md`

## Зачем так разделено

Репозиторий разделён по сценариям использования:

- `firmware/*` — прошивки и протоколы микроконтроллеров.
- `desktop/*` — приложения на ПК, которые работают с данными джойстика.

Это упрощает навигацию для ревью кода и потенциальных работодателей.
