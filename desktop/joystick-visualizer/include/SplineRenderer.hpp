#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <vector>

void drawSpline(sf::RenderTarget& target,
                const std::vector<sf::Vector2f>& points,
                sf::Color color = sf::Color::Green,
                float step = 0.02f);
