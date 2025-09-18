#include "SplineRenderer.hpp"

#include <SFML/Graphics/PrimitiveType.hpp>

#include <algorithm>
#include <cmath>

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
                float thickness,
                float step)
{
    if (points.size() < 4 || step <= 0.0f || thickness <= 0.0f)
    {
        return;
    }

    const std::size_t segments = points.size() - 3;
    const std::size_t estimate = static_cast<std::size_t>(segments * (1.0f / step + 1.0f));

    std::vector<sf::Vector2f> samples;
    samples.reserve(estimate);

    for (std::size_t i = 1; i <= points.size() - 3; ++i)
    {
        const sf::Vector2f& p0 = points[i - 1];
        const sf::Vector2f& p1 = points[i];
        const sf::Vector2f& p2 = points[i + 1];
        const sf::Vector2f& p3 = points[i + 2];

        for (float t = 0.0f; t <= 1.0f; t += step)
        {
            const sf::Vector2f position = catmullRom(p0, p1, p2, p3, t);
            samples.push_back(position);
        }
    }

    if (samples.size() < 2)
    {
        return;
    }

    const float halfThickness = std::max(thickness * 0.5f, 0.5f);

    std::vector<sf::Vertex> vertices;
    vertices.reserve(samples.size() * 2);

    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        sf::Vector2f tangent;
        if (i == 0)
        {
            tangent = samples[i + 1] - samples[i];
        }
        else if (i == samples.size() - 1)
        {
            tangent = samples[i] - samples[i - 1];
        }
        else
        {
            tangent = samples[i + 1] - samples[i - 1];
        }

        const float length = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);
        sf::Vector2f normal{0.0f, 0.0f};
        if (length > 0.0001f)
        {
            normal = sf::Vector2f{-tangent.y / length, tangent.x / length};
        }

        const sf::Vector2f offset = normal * halfThickness;

        sf::Vertex left;
        left.position = samples[i] + offset;
        left.color = color;

        sf::Vertex right;
        right.position = samples[i] - offset;
        right.color = color;

        vertices.push_back(left);
        vertices.push_back(right);
    }

    if (!vertices.empty())
    {
        target.draw(vertices.data(), vertices.size(), sf::PrimitiveType::TriangleStrip);
    }
}
