#pragma once

#ifdef JOYSTICK_DEV_MODE

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <array>
#include <string>

#include "PointNormalizer.hpp"

class DeveloperPanel
{
public:
    DeveloperPanel(PointNormalizer& normalizer, sf::Font& font);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update();
    void draw(sf::RenderTarget& target) const;

private:
    struct Slider
    {
        Slider(const std::string& label,
               float min,
               float max,
               float value,
               sf::Vector2f position,
               float width,
               float height,
               const sf::Font& font);

        std::string label;
        float min;
        float max;
        float value;
        sf::Vector2f position;
        float width;
        float height;
        sf::Text text;

        [[nodiscard]] sf::FloatRect trackRect() const;
        [[nodiscard]] sf::FloatRect hitRect() const;
        [[nodiscard]] float ratio() const;
        void setValue(float v);
    };

    void syncFromNormalizer();
    void applyToNormalizer();
    void updateVisuals();
    void handleMouseDrag(const sf::Vector2f& worldPos);

    PointNormalizer& normalizer_;
    sf::Font& font_;
    std::array<Slider, 3> sliders_;
    sf::RectangleShape background_;
    mutable sf::RectangleShape trackShape_;
    mutable sf::CircleShape knobShape_;
    std::size_t activeSlider_;
    bool dragging_;
};

#endif // JOYSTICK_DEV_MODE
