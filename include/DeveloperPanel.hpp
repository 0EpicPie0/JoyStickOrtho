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
#include <functional>
#include <string>

#include "AppConfig.hpp"
#include "PointNormalizer.hpp"

class DeveloperPanel
{
public:
    using SaveCallback = std::function<void()>;
    using SaveTemplateCallback = std::function<void()>;

    DeveloperPanel(PointNormalizer& normalizer,
                   AppConfig& config,
                   sf::Font& font,
                   SaveCallback onSave,
                   SaveTemplateCallback onSaveTemplate);

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
    void updateButtonVisual(sf::RectangleShape& button, bool hover);
    void handleSaveClick();
    void handleTemplateSaveClick();

    PointNormalizer& normalizer_;
    AppConfig& config_;
    sf::Font& font_;
    std::array<Slider, 4> sliders_;
    sf::RectangleShape background_;
    mutable sf::RectangleShape trackShape_;
    mutable sf::CircleShape knobShape_;
    sf::RectangleShape saveButton_;
    sf::Text saveButtonText_;
    sf::RectangleShape templateButton_;
    sf::Text templateButtonText_;
    SaveCallback saveCallback_;
    SaveTemplateCallback saveTemplateCallback_;
    std::size_t activeSlider_;
    bool dragging_;
    bool saveHover_;
    bool templateHover_;
};

#endif // JOYSTICK_DEV_MODE
