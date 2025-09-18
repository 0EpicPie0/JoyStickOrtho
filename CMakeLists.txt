cmake_minimum_required(VERSION 3.16)
project(JoystickVisualizer)

set(CMAKE_CXX_STANDARD 20)

# Пути к Homebrew (обычно /opt/homebrew)
set(CMAKE_PREFIX_PATH "/opt/homebrew")

# Ищем SFML
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

set(SOURCES
    main.cpp
    src/SerialPort.cpp
    src/PacketParser.cpp
    src/PointNormalizer.cpp
    src/SplineRenderer.cpp
    src/AppConfig.cpp
    src/DeveloperPanel.cpp
)

add_executable(joystick ${SOURCES})

target_include_directories(joystick PRIVATE include)

target_link_libraries(joystick
    sfml-graphics
    sfml-window
    sfml-system
)

option(JOYSTICK_DEV_MODE "Enable developer control panel" OFF)
if (JOYSTICK_DEV_MODE)
    target_compile_definitions(joystick PRIVATE JOYSTICK_DEV_MODE)
endif()
