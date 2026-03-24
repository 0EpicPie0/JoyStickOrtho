#include "AppConfig.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
constexpr const char* COMMENT_PREFIX = "#";

std::string trim(std::string value)
{
    const auto notSpace = [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

bool parseFloat(const std::string& input, float& out)
{
    auto trimmed = trim(input);
    if (trimmed.empty())
    {
        return false;
    }

    try
    {
        out = std::stof(trimmed);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}
}

bool loadAppConfig(const std::filesystem::path& filePath, AppConfig& outConfig)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    AppConfig loaded = outConfig;

    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line.rfind(COMMENT_PREFIX, 0) == 0)
        {
            continue;
        }

        const auto equalsPos = line.find('=');
        if (equalsPos == std::string::npos)
        {
            continue;
        }

        const std::string key = trim(line.substr(0, equalsPos));
        const std::string value = line.substr(equalsPos + 1);

        if (key == "sensitivity")
        {
            parseFloat(value, loaded.sensitivity);
        }
        else if (key == "smoothing")
        {
            parseFloat(value, loaded.smoothing);
        }
        else if (key == "dead_zone")
        {
            parseFloat(value, loaded.deadZone);
        }
    }

    outConfig = loaded;
    return true;
}

bool saveAppConfig(const std::filesystem::path& filePath, const AppConfig& config)
{
    std::error_code ec;
    const auto parent = filePath.parent_path();
    if (!parent.empty())
    {
        std::filesystem::create_directories(parent, ec);
    }

    std::ofstream file(filePath, std::ios::trunc);
    if (!file.is_open())
    {
        std::cerr << "Failed to open config file for writing: " << filePath << '\n';
        return false;
    }

    file << "# Joystick painter settings\n";
    file << "sensitivity=" << config.sensitivity << '\n';
    file << "smoothing=" << config.smoothing << '\n';
    file << "dead_zone=" << config.deadZone << '\n';
    return true;
}
