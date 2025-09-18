#pragma once

#include <SFML/System/Vector2.hpp>

#include <filesystem>
#include <vector>

[[nodiscard]] bool loadTemplateFile(const std::filesystem::path& filePath,
                                    unsigned int width,
                                    unsigned int height,
                                    std::vector<sf::Vector2f>& outPoints);

[[nodiscard]] bool saveTemplateFile(const std::filesystem::path& filePath,
                                    unsigned int width,
                                    unsigned int height,
                                    const std::vector<sf::Vector2f>& points);
