#pragma once

#include <filesystem>

struct AppConfig
{
    float sensitivity{1.6f};
    float smoothing{0.25f};
    float deadZone{0.02f};
};

[[nodiscard]] bool loadAppConfig(const std::filesystem::path& filePath, AppConfig& outConfig);
[[nodiscard]] bool saveAppConfig(const std::filesystem::path& filePath, const AppConfig& config);
