#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include "PacketParser.hpp"
#include "PointNormalizer.hpp"
#include "SerialPort.hpp"
#include "SplineRenderer.hpp"

#include <cstdlib>
#include <string>
#include <vector>

namespace
{
constexpr unsigned int WINDOW_WIDTH = 800;
constexpr unsigned int WINDOW_HEIGHT = 600;
constexpr float ADC_MAX = 4095.0f;
constexpr std::size_t MAX_POINTS = 4000;
constexpr const char* DEFAULT_SERIAL_DEVICE = "/dev/cu.usbserial-10";

std::string resolveSerialDevice()
{
    if (const char* env = std::getenv("JOYSTICK_SERIAL_PORT"))
    {
        return env;
    }
    return DEFAULT_SERIAL_DEVICE;
}

void trimPointCount(std::vector<sf::Vector2f>& points)
{
    if (points.size() <= MAX_POINTS)
    {
        return;
    }

    const std::size_t overflow = points.size() - MAX_POINTS;
    points.erase(points.begin(), points.begin() + static_cast<std::ptrdiff_t>(overflow));
}
}

int main()
{
    sf::VideoMode mode({WINDOW_WIDTH, WINDOW_HEIGHT});
    sf::RenderWindow window(mode, "ESP32 Joystick Painter", sf::State::Windowed);

    SerialPort serial(resolveSerialDevice());
    if (!serial.isOpen())
    {
        return 1;
    }

    PointNormalizer normalizer(WINDOW_WIDTH, WINDOW_HEIGHT, ADC_MAX);
    std::vector<sf::Vector2f> points;
    points.reserve(1024);

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                continue;
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
                else if (keyPressed->code == sf::Keyboard::Key::C)
                {
                    points.clear();
                    normalizer.reset();
                }
            }
        }

        while (true)
        {
            auto line = serial.readLine();
            if (!line)
            {
                break;
            }

            SamplePoint sample;
            if (!parsePacket(*line, sample))
            {
                continue;
            }

            const sf::Vector2f normalized = normalizer.normalize(sample.x, sample.y);
            points.push_back(normalized);
            trimPointCount(points);
        }

        window.clear(sf::Color::Black);
        drawSpline(window, points);
        window.display();

        sf::sleep(sf::milliseconds(2));
    }

    return 0;
}
