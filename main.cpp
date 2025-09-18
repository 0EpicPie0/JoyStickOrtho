#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include "AppConfig.hpp"
#include "PacketParser.hpp"
#include "PointNormalizer.hpp"
#include "SerialPort.hpp"
#include "SplineRenderer.hpp"
#include "TemplateStorage.hpp"
#ifdef JOYSTICK_DEV_MODE
#include "DeveloperPanel.hpp"
#endif

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
constexpr unsigned int WINDOW_WIDTH = 800;
constexpr unsigned int WINDOW_HEIGHT = 600;
constexpr float ADC_MAX = 4095.0f;
constexpr std::size_t MAX_POINTS = 4000;
constexpr const char* DEFAULT_SERIAL_DEVICE = "/dev/cu.usbserial-10";
constexpr const char* DEFAULT_CONFIG_FILE = "joystick_config.cfg";
constexpr const char* DEFAULT_TEMPLATE_FILE = "joystick_template.csv";

std::string resolveSerialDevice()
{
    if (const char* env = std::getenv("JOYSTICK_SERIAL_PORT"))
    {
        return env;
    }
    return DEFAULT_SERIAL_DEVICE;
}

std::filesystem::path resolveConfigPath()
{
    if (const char* env = std::getenv("JOYSTICK_CONFIG"))
    {
        if (*env != '\0')
        {
            return std::filesystem::path(env);
        }
    }

    return std::filesystem::path(DEFAULT_CONFIG_FILE);
}

std::filesystem::path resolveTemplatePath(const AppConfig& config, const std::filesystem::path& configPath)
{
    if (const char* env = std::getenv("JOYSTICK_TEMPLATE"))
    {
        if (*env != '\0')
        {
            return std::filesystem::path(env);
        }
    }

    std::filesystem::path candidate = config.templateFile.empty() ? std::filesystem::path(DEFAULT_TEMPLATE_FILE)
                                                                   : std::filesystem::path(config.templateFile);

    if (candidate.is_relative())
    {
        const std::filesystem::path base = configPath.parent_path();
        if (!base.empty())
        {
            candidate = base / candidate;
        }
    }

    return candidate;
}

bool loadFont(sf::Font& font)
{
    if (const char* env = std::getenv("JOYSTICK_FONT"))
    {
        if (font.openFromFile(env))
        {
            return true;
        }
    }

    const char* candidates[] = {
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/Supplemental/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };

    for (const char* path : candidates)
    {
        if (path != nullptr && font.openFromFile(path))
        {
            return true;
        }
    }

    return false;
}

std::string toFixed(float value, int precision)
{
    std::ostringstream stream;
    stream.setf(std::ios::fixed);
    stream << std::setprecision(precision) << value;
    return stream.str();
}

std::string buildInstructionText(const PointNormalizer& normalizer,
                                 bool templateLoaded,
                                 bool deviating,
                                 float lineThickness)
{
    std::string text = "ESC - exit | C - clear canvas | Thickness: " + toFixed(lineThickness, 2);
#ifdef JOYSTICK_DEV_MODE
    text += "\nSens: " + toFixed(normalizer.sensitivity(), 2);
    text += " | Smooth: " + toFixed(normalizer.smoothing(), 2);
    text += " | Dead: " + toFixed(normalizer.deadZone(), 3);
    text += " | Dev: adjust sliders, save settings/template";
#endif

    text += "\nTemplate: ";
    if (!templateLoaded)
    {
        text += "none";
    }
    else
    {
        text += deviating ? "mismatch" : "match";
    }

    return text;
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

bool hasDeviation(const std::vector<sf::Vector2f>& actual,
                  const std::vector<sf::Vector2f>& reference,
                  float threshold)
{
    if (actual.size() < 2 || reference.size() < 2 || threshold <= 0.0f)
    {
        return false;
    }

    const float thresholdSquared = threshold * threshold;
    const std::size_t lastActualIndex = actual.size() - 1;
    const std::size_t lastReferenceIndex = reference.size() - 1;

    for (std::size_t i = 0; i < actual.size(); ++i)
    {
        const float progress = lastActualIndex > 0 ? static_cast<float>(i) / static_cast<float>(lastActualIndex) : 0.0f;
        const float referencePos = progress * static_cast<float>(lastReferenceIndex);
        const std::size_t baseIndex = static_cast<std::size_t>(referencePos);
        const std::size_t nextIndex = std::min(baseIndex + 1, lastReferenceIndex);
        const float localT = referencePos - static_cast<float>(baseIndex);

        const sf::Vector2f interpolated = reference[baseIndex] + (reference[nextIndex] - reference[baseIndex]) * localT;
        const sf::Vector2f diff = actual[i] - interpolated;
        const float distSquared = diff.x * diff.x + diff.y * diff.y;
        if (distSquared > thresholdSquared)
        {
            return true;
        }
    }

    return false;
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

    const std::filesystem::path configPath = resolveConfigPath();
    AppConfig config;
    const bool configLoaded = loadAppConfig(configPath, config);
    if (!configLoaded)
    {
        std::cerr << "Using default settings; config file not found or invalid: " << configPath << '\n';
    }

    config.lineThickness = std::clamp(config.lineThickness, 0.5f, 30.0f);
    config.deviationThreshold = std::max(0.0f, config.deviationThreshold);
    if (config.templateFile.empty())
    {
        config.templateFile = DEFAULT_TEMPLATE_FILE;
    }

    const std::filesystem::path templatePath = resolveTemplatePath(config, configPath);
    std::vector<sf::Vector2f> templatePoints;
    bool templateLoaded = loadTemplateFile(templatePath, WINDOW_WIDTH, WINDOW_HEIGHT, templatePoints);
    if (!templateLoaded)
    {
        std::cerr << "Template not loaded: " << templatePath << '\n';
    }

    PointNormalizer normalizer(WINDOW_WIDTH, WINDOW_HEIGHT, ADC_MAX, config.sensitivity, config.smoothing, config.deadZone);
    std::vector<sf::Vector2f> points;
    points.reserve(1024);

    sf::Font uiFont;
    const bool fontLoaded = loadFont(uiFont);
    sf::Text instructions(uiFont);
    sf::RectangleShape instructionsBackground;

    if (fontLoaded)
    {
        instructions.setCharacterSize(16);
        instructions.setFillColor(sf::Color::White);
        instructionsBackground.setFillColor(sf::Color(0, 0, 0, 160));
        instructionsBackground.setOutlineThickness(1.0f);
        instructionsBackground.setOutlineColor(sf::Color(60, 180, 255, 150));
    }

#ifdef JOYSTICK_DEV_MODE
    DeveloperPanel developerPanel(
        normalizer,
        config,
        uiFont,
        [&config, configPath]() {
            if (!saveAppConfig(configPath, config))
            {
                std::cerr << "Failed to save config to " << configPath << '\n';
            }
        },
        [&points, &templatePoints, &templateLoaded, templatePath]() {
            if (points.size() < 4)
            {
                std::cerr << "Not enough points to save template." << '\n';
                return;
            }

            if (saveTemplateFile(templatePath, WINDOW_WIDTH, WINDOW_HEIGHT, points))
            {
                std::vector<sf::Vector2f> updated;
                if (loadTemplateFile(templatePath, WINDOW_WIDTH, WINDOW_HEIGHT, updated))
                {
                    templatePoints = std::move(updated);
                    templateLoaded = !templatePoints.empty();
                    std::cout << "Template saved to " << templatePath << '\n';
                }
                else
                {
                    templateLoaded = false;
                }
            }
        });
#else
    (void)configLoaded;
#endif

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
#ifdef JOYSTICK_DEV_MODE
            developerPanel.handleEvent(*event, window);
#endif
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

        const bool templateAvailable = templatePoints.size() >= 4;
        const bool deviating = templateAvailable ? hasDeviation(points, templatePoints, config.deviationThreshold) : false;

        window.clear(sf::Color::Black);

        if (templateAvailable)
        {
            drawSpline(window,
                       templatePoints,
                       sf::Color(0, 200, 255, 120),
                       std::max(1.0f, config.lineThickness));
        }

        if (!points.empty())
        {
            const sf::Color strokeColor = deviating ? sf::Color::Red : sf::Color::Green;
            drawSpline(window, points, strokeColor, std::max(1.0f, config.lineThickness));
        }
#ifdef JOYSTICK_DEV_MODE
        developerPanel.update();
        developerPanel.draw(window);
#endif

        if (fontLoaded)
        {
            instructions.setString(buildInstructionText(normalizer, templateAvailable, deviating, config.lineThickness));
            const sf::FloatRect bounds = instructions.getLocalBounds();
            const sf::Vector2f backgroundSize(bounds.size.x + 24.0f, bounds.size.y + 24.0f);
            const sf::Vector2f backgroundPos(10.0f, static_cast<float>(WINDOW_HEIGHT) - backgroundSize.y - 10.0f);
            instructionsBackground.setSize(backgroundSize);
            instructionsBackground.setPosition(backgroundPos);
            instructions.setPosition({backgroundPos.x + 12.0f, backgroundPos.y + 8.0f});

            window.draw(instructionsBackground);
            window.draw(instructions);
        }

        window.display();
        sf::sleep(sf::milliseconds(2));
    }

    return 0;
}
