#include "TemplateStorage.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
std::string trim(std::string value)
{
    const auto notSpace = [](unsigned char ch) { return !std::isspace(static_cast<unsigned char>(ch)); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

bool parseCoordinates(const std::string& line, float& outX, float& outY)
{
    std::istringstream stream(line);
    std::string token;
    if (!std::getline(stream, token, ','))
    {
        return false;
    }
    try
    {
        outX = std::stof(trim(token));
        if (!std::getline(stream, token))
        {
            return false;
        }
        outY = std::stof(trim(token));
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}
}

bool loadTemplateFile(const std::filesystem::path& filePath,
                      unsigned int width,
                      unsigned int height,
                      std::vector<sf::Vector2f>& outPoints)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::vector<sf::Vector2f> loaded;
    std::string line;

    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        float normX = 0.0f;
        float normY = 0.0f;
        if (!parseCoordinates(line, normX, normY))
        {
            continue;
        }

        normX = std::clamp(normX, 0.0f, 1.0f);
        normY = std::clamp(normY, 0.0f, 1.0f);
        loaded.emplace_back(normX * static_cast<float>(width),
                            normY * static_cast<float>(height));
    }

    if (loaded.size() < 4)
    {
        return false;
    }

    outPoints = std::move(loaded);
    return true;
}

bool saveTemplateFile(const std::filesystem::path& filePath,
                      unsigned int width,
                      unsigned int height,
                      const std::vector<sf::Vector2f>& points)
{
    if (points.size() < 4)
    {
        std::cerr << "Not enough points to save template.\n";
        return false;
    }

    std::error_code ec;
    const auto parent = filePath.parent_path();
    if (!parent.empty())
    {
        std::filesystem::create_directories(parent, ec);
    }

    std::ofstream file(filePath, std::ios::trunc);
    if (!file.is_open())
    {
        std::cerr << "Failed to open template file for writing: " << filePath << '\n';
        return false;
    }

    file << "# Normalized template points (x,y)\n";
    for (const auto& point : points)
    {
        const float normX = (width > 0) ? point.x / static_cast<float>(width) : 0.0f;
        const float normY = (height > 0) ? point.y / static_cast<float>(height) : 0.0f;
        file << normX << "," << normY << '\n';
    }

    return true;
}
