CXX = g++
CXXFLAGS = -std=c++20 -Wall -I/opt/homebrew/include -Iinclude
LDFLAGS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system

SRC = main.cpp \
      src/SerialPort.cpp \
      src/PacketParser.cpp \
      src/PointNormalizer.cpp \
      src/SplineRenderer.cpp
TARGET = joystick

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
