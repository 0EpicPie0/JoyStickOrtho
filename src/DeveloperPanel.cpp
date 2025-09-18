#include "DeveloperPanel.hpp"

#ifdef JOYSTICK_DEV_MODE

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#include <SFML/Window/Mouse.hpp>

namespace
{
constexpr float PANEL_PADDING = 12.0f;
constexpr float SLIDER_SPACING = 54.0f;
constexpr float TRACK_HEIGHT = 8.0f;
constexpr float TRACK_WIDTH = 220.0f;
constexpr float HIT_HEIGHT = 28.0f;
constexpr float KNOB_RADIUS = 10.0f;
} // namespace

DeveloperPanel::DeveloperPanel(PointNormalizer& normalizer, sf::Font& font)
    : normalizer_(normalizer)
    , font_(font)
    , sliders_{
          Slider{"Sensitivity", 0.2f, 4.0f, normalizer.sensitivity(), {PANEL_PADDING, PANEL_PADDING + 24.0f}, TRACK_WIDTH, TRACK_HEIGHT, font_},
          Slider{"Smoothing", 0.0f, 0.95f, normalizer.smoothing(), {PANEL_PADDING, PANEL_PADDING + 24.0f + SLIDER_SPACING}, TRACK_WIDTH, TRACK_HEIGHT, font_},
          Slider{"Dead zone", 0.0f, 0.3f, normalizer.deadZone(), {PANEL_PADDING, PANEL_PADDING + 24.0f + 2.0f * SLIDER_SPACING}, TRACK_WIDTH, TRACK_HEIGHT, font_}}
    , background_()
    , trackShape_(sf::Vector2f(TRACK_WIDTH, TRACK_HEIGHT))
    , knobShape_(KNOB_RADIUS)
    , activeSlider_(static_cast<std::size_t>(-1))
    , dragging_(false)
{
    background_.setSize({TRACK_WIDTH + PANEL_PADDING * 2.0f, PANEL_PADDING * 2.0f + SLIDER_SPACING * 3.0f});
    background_.setFillColor(sf::Color(0, 0, 0, 160));
    background_.setOutlineThickness(1.0f);
    background_.setOutlineColor(sf::Color(60, 180, 255, 200));

    trackShape_.setFillColor(sf::Color(80, 80, 80));
    knobShape_.setFillColor(sf::Color(60, 180, 255));
    knobShape_.setOrigin({KNOB_RADIUS, KNOB_RADIUS});

    updateVisuals();
}

void DeveloperPanel::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
{
    const auto mousePixel = sf::Mouse::getPosition(window);
    const sf::Vector2f mousePos = window.mapPixelToCoords(mousePixel);

    if (event.is<sf::Event::MouseButtonPressed>())
    {
        const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>();
        if (mouseButton->button == sf::Mouse::Button::Left)
        {
            for (std::size_t i = 0; i < sliders_.size(); ++i)
            {
                if (sliders_[i].hitRect().contains(mousePos))
                {
                    activeSlider_ = i;
                    dragging_ = true;
                    handleMouseDrag(mousePos);
                    break;
                }
            }
        }
    }
    else if (event.is<sf::Event::MouseButtonReleased>())
    {
        const auto* mouseButton = event.getIf<sf::Event::MouseButtonReleased>();
        if (mouseButton->button == sf::Mouse::Button::Left)
        {
            dragging_ = false;
            activeSlider_ = static_cast<std::size_t>(-1);
        }
    }
    else if (event.is<sf::Event::MouseMoved>())
    {
        if (dragging_)
        {
            handleMouseDrag(mousePos);
        }
    }
}

void DeveloperPanel::update()
{
    if (!dragging_)
    {
        syncFromNormalizer();
    }
    updateVisuals();
}

void DeveloperPanel::draw(sf::RenderTarget& target) const
{
    target.draw(background_);

    for (const auto& slider : sliders_)
    {
        const sf::FloatRect trackRect = slider.trackRect();
        trackShape_.setPosition(trackRect.position);
        target.draw(trackShape_);

        const float knobX = trackRect.position.x + slider.ratio() * trackRect.size.x;
        const float knobY = trackRect.position.y + trackRect.size.y * 0.5f;
        knobShape_.setPosition({knobX, knobY});
        target.draw(knobShape_);

        target.draw(slider.text);
    }
}

sf::FloatRect DeveloperPanel::Slider::trackRect() const
{
    return {{position.x, position.y}, {width, height}};
}

sf::FloatRect DeveloperPanel::Slider::hitRect() const
{
    const float paddingY = (HIT_HEIGHT - height) * 0.5f;
    return {{position.x, position.y - paddingY}, {width, HIT_HEIGHT}};
}

float DeveloperPanel::Slider::ratio() const
{
    if (max - min <= std::numeric_limits<float>::epsilon())
    {
        return 0.0f;
    }
    return (value - min) / (max - min);
}

void DeveloperPanel::Slider::setValue(float v)
{
    value = std::clamp(v, min, max);
}

void DeveloperPanel::syncFromNormalizer()
{
    sliders_[0].setValue(normalizer_.sensitivity());
    sliders_[1].setValue(normalizer_.smoothing());
    sliders_[2].setValue(normalizer_.deadZone());
}

void DeveloperPanel::applyToNormalizer()
{
    normalizer_.configure(sliders_[0].value, sliders_[1].value, sliders_[2].value);
}

void DeveloperPanel::updateVisuals()
{
    for (auto& slider : sliders_)
    {
        slider.text.setPosition({slider.position.x, slider.position.y - 22.0f});
        std::ostringstream stream;
        stream.setf(std::ios::fixed);
        const int precision = (slider.max > 1.5f) ? 2 : 3;
        stream << std::setprecision(precision) << slider.value;
        slider.text.setString(slider.label + ": " + stream.str());
    }
}

void DeveloperPanel::handleMouseDrag(const sf::Vector2f& worldPos)
{
    if (activeSlider_ >= sliders_.size())
    {
        return;
    }

    auto& slider = sliders_[activeSlider_];
    const sf::FloatRect track = slider.trackRect();
    const float clampedX = std::clamp(worldPos.x, track.position.x, track.position.x + track.size.x);
    const float ratio = (clampedX - track.position.x) / track.size.x;
    slider.setValue(slider.min + ratio * (slider.max - slider.min));
    applyToNormalizer();
    updateVisuals();
}

DeveloperPanel::Slider::Slider(const std::string& labelValue,
                               float minValue,
                               float maxValue,
                               float initialValue,
                               sf::Vector2f positionValue,
                               float widthValue,
                               float heightValue,
                               const sf::Font& font)
    : label(labelValue)
    , min(minValue)
    , max(maxValue)
    , value(initialValue)
    , position(positionValue)
    , width(widthValue)
    , height(heightValue)
    , text(font)
{
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::White);
}

#endif // JOYSTICK_DEV_MODE
