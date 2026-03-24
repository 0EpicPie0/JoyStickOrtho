#pragma once

#include <SFML/System/Vector2.hpp>

class PointNormalizer
{
public:
    PointNormalizer(unsigned int width,
                    unsigned int height,
                    float adcMax,
                    float sensitivity = 1.6f,
                    float smoothing = 0.25f,
                    float deadZone = 0.02f);

    sf::Vector2f normalize(float rawX, float rawY);
    void reset();
    void configure(float sensitivity, float smoothing, float deadZone);

    [[nodiscard]] float sensitivity() const noexcept;
    [[nodiscard]] float smoothing() const noexcept;
    [[nodiscard]] float deadZone() const noexcept;

private:
    unsigned int width_;
    unsigned int height_;
    float adcMax_;
    float sensitivity_;
    float smoothing_;
    float deadZone_;
    bool calibrated_;
    float offsetX_;
    float offsetY_;
    bool haveOutput_;
    sf::Vector2f lastOutput_;
};
