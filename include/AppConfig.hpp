#pragma once

#include <filesystem>
#include <string>

struct AppConfig
{
    float sensitivity{1.6f};
    float smoothing{0.25f};
    float deadZone{0.02f};
    float lineThickness{2.0f};
    float deviationThreshold{20.0f};
    std::string templateFile{"joystick_template.csv"};
};

[[nodiscard]] bool loadAppConfig(const std::filesystem::path& filePath, AppConfig& outConfig);
[[nodiscard]] bool saveAppConfig(const std::filesystem::path& filePath, const AppConfig& config);
