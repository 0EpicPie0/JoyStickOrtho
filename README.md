# ESP32 Joystick Painter

A desktop visualizer for ESP32-based joystick input that draws a smooth Catmull–Rom spline in real time, compares the live stroke against a saved template, and provides developer tooling to tune signal processing.

## Features
- Reads JSON packets (`{"x":...,"y":...,"cmd":...}`) from a serial-connected ESP32 at 9600 baud.
- Normalizes 12-bit ADC coordinates (0–4095) into an 800×600 SFML 3 window.
- Renders the live stroke as a Catmull–Rom spline with adjustable thickness, smoothing, and dead zone.
- Loads optional templates (CSV of normalized points), overlays them semi-transparently, and highlights deviations beyond a configurable tolerance.
- Keyboard controls: `ESC` to exit, `C` to clear the canvas and reset calibration.
- Developer panel (compile with `JOYSTICK_DEV_MODE`) exposing sliders for sensitivity, smoothing, dead zone, line width, and tolerance, plus buttons to save settings and export the current stroke as a template.
- Persists settings in `joystick_config.cfg` and templates in `joystick_template.csv` (paths configurable via environment variables).

## Requirements
- C++20 compiler (`g++` or `clang++`).
- SFML 3 (graphics, window, system components).
- POSIX environment with access to the serial device exposed by the ESP32.

## Build
```bash
# Standard build
make

# Developer build with panel support
make dev
```

You can also use CMake:
```bash
cmake -B build -DJOYSTICK_DEV_MODE=ON
cmake --build build
```
*(Enable developer mode by setting the cache option `JOYSTICK_DEV_MODE=ON`.)*

## Configuration
Settings are loaded from `joystick_config.cfg` (created on first save). Key fields:
- `sensitivity`, `smoothing`, `dead_zone` — input conditioning parameters.
- `line_thickness` — spline stroke width in pixels.
- `deviation_threshold` — allowed distance (in pixels) before the live stroke is flagged red.
- `template_file` — relative or absolute path to the template CSV.

Environment overrides:
- `JOYSTICK_SERIAL_PORT` — custom serial device (default `/dev/cu.usbserial-10`).
- `JOYSTICK_CONFIG` — alternate configuration file path.
- `JOYSTICK_TEMPLATE` — template path override.
- `JOYSTICK_FONT` — font file path for UI text.

## Template Workflow
1. Launch the developer build (`make dev && ./joystick`).
2. Enable the panel sliders to achieve the desired feel.
3. Draw the reference gesture, then hit **Save template** to export it as normalized points (default `joystick_template.csv`).
4. Switch to the standard build (`make && ./joystick`): the saved template appears semi-transparent, and the live stroke turns red when deviating beyond the tolerance.

## Usage
Run the executable after connecting the ESP32:
```bash
./joystick
```
Watch the status box (bottom-left) for current parameters and template match state ("match" vs "mismatch").

## License
This project is distributed under the MIT License. See `LICENSE` (if present) for details.
