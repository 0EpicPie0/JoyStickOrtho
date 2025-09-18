# Contract / Контракт

## English

### Purpose
Develop a desktop visualization tool that reads joystick coordinates from a serial-connected ESP32 and renders a smooth Catmull–Rom spline in real time, allowing engineers and operators to verify gesture accuracy against saved motion templates.

### Functional Scope
- Create an 800×600 SFML 3 window titled "ESP32 Joystick Painter".
- Read newline-delimited JSON packets from the configured serial port at 9600 baud. Each packet contains `x`, `y`, and `cmd` fields; incomplete or malformed packets are ignored.
- Normalize joystick coordinates (0–4095) to window coordinates and keep a rolling buffer of recent points.
- Render the live path as a Catmull–Rom spline with configurable thickness and smoothing.
- When a template file is available, render it as a translucent reference path and compare the live stroke against it using the configurable tolerance; highlight mismatches in red.
- Provide keyboard shortcuts: `ESC` to quit, `C` to clear current points and reset calibration.
- Expose a developer panel (enabled through the `JOYSTICK_DEV_MODE` define) with adjustable sliders for sensitivity, smoothing, dead zone, line width, and tolerance, plus buttons to save settings and export the current drawing as a template.
- Persist settings (including template path) in `joystick_config.cfg`; persist templates as normalized CSV sequences.

### Non-Goals
- Cross-platform mobile support or APK packaging.
- Automated installation of serial drivers or detection of the ESP32 port.
- High-frequency data logging or analytics beyond rendering and basic deviation feedback.

### Success Criteria
- Program builds with `g++ -std=c++20` linking against SFML 3 graphics/window/system libraries.
- Normal mode (`make`) displays live data with template overlay and deviation feedback.
- Developer mode (`make dev`) loads the panel, allows parameter adjustments, and successfully saves both configuration and templates.

## Русский

### Назначение
Создать настольный инструмент визуализации, который считывает координаты джойстика с ESP32 по последовательному порту и в реальном времени рисует сглаженную кривую Catmull–Rom, чтобы инженеры и операторы могли проверять точность движений по сохранённым шаблонам.

### Объём работ
- Открывать окно SFML 3 размером 800×600 с заголовком "ESP32 Joystick Painter".
- Читать JSON-пакеты из указанного последовательного порта (9600 бод). Каждый пакет содержит поля `x`, `y`, `cmd`; некорректные строки игнорируются.
- Нормализовать координаты джойстика (0–4095) в координаты окна и хранить скользящий буфер последних точек.
- Отрисовывать текущую траекторию в виде Catmull–Rom сплайна с настраиваемой толщиной и сглаживанием.
- При наличии файла шаблона выводить его полупрозрачной линией и сравнивать текущий штрих с шаблоном, подсвечивая отклонения красным цветом согласно допустимому порогу.
- Поддерживать горячие клавиши: `ESC` — выход, `C` — очистка точек и сброс калибровки.
- Предоставить панель разработчика (активируется флагом `JOYSTICK_DEV_MODE`) со слайдерами чувствительности, сглаживания, мёртвой зоны, толщины линии и допуска, а также кнопками сохранения настроек и экспорта текущего рисунка в шаблон.
- Сохранять настройки (включая путь к шаблону) в `joystick_config.cfg`; сохранять шаблоны как CSV с нормализованными точками.

### Что не входит
- Поддержка мобильных платформ и выпуск APK.
- Автоматическая установка драйверов или поиск порта ESP32.
- Сбор и анализ данных за пределами отрисовки и базовой индикации отклонений.

### Критерии приёмки
- Проект собирается с помощью `g++ -std=c++20`, используя библиотеки SFML 3 graphics/window/system.
- В обычном режиме (`make`) отображаются текущие данные, шаблон и статусы совпадения.
- В режиме разработчика (`make dev`) активна панель, параметры меняются на лету, настройки и шаблоны успешно сохраняются.
