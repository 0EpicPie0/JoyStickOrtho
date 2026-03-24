CXX = g++
BREW_PREFIX ?= /opt/homebrew
CXXFLAGS = -std=c++20 -Wall -I$(BREW_PREFIX)/include -Iinclude
LDFLAGS = -L$(BREW_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system

SRC = main.cpp \
      src/SerialPort.cpp \
      src/PacketParser.cpp \
      src/PointNormalizer.cpp \
      src/SplineRenderer.cpp \
      src/AppConfig.cpp \
      src/DeveloperPanel.cpp
TARGET = joystick

DEVFLAGS = -DJOYSTICK_DEV_MODE

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

dev:
	$(CXX) $(CXXFLAGS) $(DEVFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
