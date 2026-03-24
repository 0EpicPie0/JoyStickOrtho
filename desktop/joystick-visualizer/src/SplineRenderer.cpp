#include "SplineRenderer.hpp"

#include <SFML/Graphics/PrimitiveType.hpp>

namespace
{
sf::Vector2f catmullRom(const sf::Vector2f& p0,
                         const sf::Vector2f& p1,
                         const sf::Vector2f& p2,
                         const sf::Vector2f& p3,
                         float t)
{
    const float t2 = t * t;
    const float t3 = t2 * t;

    const float x = 0.5f * ((2.0f * p1.x) +
                            (-p0.x + p2.x) * t +
                            (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
                            (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);

    const float y = 0.5f * ((2.0f * p1.y) +
                            (-p0.y + p2.y) * t +
                            (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
                            (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);

    return {x, y};
}
}

void drawSpline(sf::RenderTarget& target,
                const std::vector<sf::Vector2f>& points,
                sf::Color color,
                float step)
{
    if (points.size() < 4 || step <= 0.0f)
    {
        return;
    }

    const std::size_t segments = points.size() - 3;
    const std::size_t estimate = static_cast<std::size_t>(segments * (1.0f / step + 1.0f));

    std::vector<sf::Vertex> vertices;
    vertices.reserve(estimate);

    for (std::size_t i = 1; i <= points.size() - 3; ++i)
    {
        const sf::Vector2f& p0 = points[i - 1];
        const sf::Vector2f& p1 = points[i];
        const sf::Vector2f& p2 = points[i + 1];
        const sf::Vector2f& p3 = points[i + 2];

        for (float t = 0.0f; t <= 1.0f; t += step)
        {
            const sf::Vector2f position = catmullRom(p0, p1, p2, p3, t);
            sf::Vertex vertex;
            vertex.position = position;
            vertex.color = color;
            vertices.push_back(vertex);
        }
    }

    if (!vertices.empty())
    {
        target.draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
    }
}
