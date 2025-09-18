#include "DeveloperPanel.hpp"

#ifdef JOYSTICK_DEV_MODE

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

#include <SFML/Window/Mouse.hpp>

namespace
{
constexpr float PANEL_PADDING = 12.0f;
constexpr float SLIDER_SPACING = 54.0f;
constexpr float TRACK_HEIGHT = 8.0f;
constexpr float TRACK_WIDTH = 220.0f;
constexpr float HIT_HEIGHT = 28.0f;
constexpr float KNOB_RADIUS = 10.0f;
constexpr float BUTTON_HEIGHT = 34.0f;
constexpr float BUTTON_MARGIN_TOP = 18.0f;
constexpr float BUTTON_GAP = 10.0f;
constexpr std::size_t SLIDER_COUNT = 4;
const sf::Color BUTTON_COLOR(60, 180, 255, 160);
const sf::Color BUTTON_COLOR_HOVER(90, 210, 255, 200);
} // namespace

DeveloperPanel::DeveloperPanel(PointNormalizer& normalizer,
                               AppConfig& config,
                               sf::Font& font,
                               SaveCallback onSave,
                               SaveTemplateCallback onSaveTemplate)
    : normalizer_(normalizer)
    , config_(config)
    , font_(font)
    , sliders_{
          Slider{"Sensitivity", 0.2f, 4.0f, normalizer.sensitivity(), {PANEL_PADDING, PANEL_PADDING + 24.0f}, TRACK_WIDTH, TRACK_HEIGHT, font_},
          Slider{"Smoothing", 0.0f, 0.95f, normalizer.smoothing(), {PANEL_PADDING, PANEL_PADDING + 24.0f + SLIDER_SPACING}, TRACK_WIDTH, TRACK_HEIGHT, font_},
          Slider{"Dead zone", 0.0f, 0.3f, normalizer.deadZone(), {PANEL_PADDING, PANEL_PADDING + 24.0f + 2.0f * SLIDER_SPACING}, TRACK_WIDTH, TRACK_HEIGHT, font_},
          Slider{"Line width", 1.0f, 12.0f, config.lineThickness, {PANEL_PADDING, PANEL_PADDING + 24.0f + 3.0f * SLIDER_SPACING}, TRACK_WIDTH, TRACK_HEIGHT, font_}}
    , background_()
    , trackShape_(sf::Vector2f(TRACK_WIDTH, TRACK_HEIGHT))
    , knobShape_(KNOB_RADIUS)
    , saveButton_(sf::Vector2f(TRACK_WIDTH, BUTTON_HEIGHT))
    , saveButtonText_(font_)
    , templateButton_(sf::Vector2f(TRACK_WIDTH, BUTTON_HEIGHT))
    , templateButtonText_(font_)
    , saveCallback_(std::move(onSave))
    , saveTemplateCallback_(std::move(onSaveTemplate))
    , activeSlider_(static_cast<std::size_t>(-1))
    , dragging_(false)
    , saveHover_(false)
    , templateHover_(false)
{
    const float totalButtonHeight = BUTTON_HEIGHT * 2.0f + BUTTON_GAP;
    const float panelHeight = PANEL_PADDING * 2.0f + SLIDER_SPACING * static_cast<float>(SLIDER_COUNT) + BUTTON_MARGIN_TOP + totalButtonHeight;
    background_.setSize({TRACK_WIDTH + PANEL_PADDING * 2.0f, panelHeight});
    background_.setFillColor(sf::Color(0, 0, 0, 160));
    background_.setOutlineThickness(1.0f);
    background_.setOutlineColor(sf::Color(60, 180, 255, 200));

    trackShape_.setFillColor(sf::Color(80, 80, 80));
    knobShape_.setFillColor(sf::Color(60, 180, 255));
    knobShape_.setOrigin({KNOB_RADIUS, KNOB_RADIUS});

    const float firstButtonY = PANEL_PADDING + 24.0f + SLIDER_SPACING * static_cast<float>(SLIDER_COUNT) + BUTTON_MARGIN_TOP;

    saveButton_.setPosition({PANEL_PADDING, firstButtonY});
    saveButton_.setFillColor(BUTTON_COLOR);
    saveButton_.setOutlineThickness(1.0f);
    saveButton_.setOutlineColor(sf::Color(20, 120, 200, 200));

    saveButtonText_.setCharacterSize(16);
    saveButtonText_.setFillColor(sf::Color::White);
    saveButtonText_.setString("Save settings");
    const sf::FloatRect textBounds = saveButtonText_.getLocalBounds();
    const sf::Vector2f textPos{
        saveButton_.getPosition().x + (saveButton_.getSize().x - textBounds.size.x) * 0.5f - textBounds.position.x,
        saveButton_.getPosition().y + (saveButton_.getSize().y - textBounds.size.y) * 0.5f - textBounds.position.y};
    saveButtonText_.setPosition(textPos);

    templateButton_.setPosition({PANEL_PADDING, firstButtonY + BUTTON_HEIGHT + BUTTON_GAP});
    templateButton_.setFillColor(BUTTON_COLOR);
    templateButton_.setOutlineThickness(1.0f);
    templateButton_.setOutlineColor(sf::Color(20, 120, 200, 200));

    templateButtonText_.setCharacterSize(16);
    templateButtonText_.setFillColor(sf::Color::White);
    templateButtonText_.setString("Save template");
    const sf::FloatRect templateBounds = templateButtonText_.getLocalBounds();
    const sf::Vector2f templatePos{
        templateButton_.getPosition().x + (templateButton_.getSize().x - templateBounds.size.x) * 0.5f - templateBounds.position.x,
        templateButton_.getPosition().y + (templateButton_.getSize().y - templateBounds.size.y) * 0.5f - templateBounds.position.y};
    templateButtonText_.setPosition(templatePos);

    updateVisuals();
    updateButtonVisual(saveButton_, false);
    updateButtonVisual(templateButton_, false);
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
            if (saveButton_.getGlobalBounds().contains(mousePos))
            {
                handleSaveClick();
            }
            else if (templateButton_.getGlobalBounds().contains(mousePos))
            {
                handleTemplateSaveClick();
            }
            dragging_ = false;
            activeSlider_ = static_cast<std::size_t>(-1);
        }
    }
    else if (event.is<sf::Event::MouseMoved>())
    {
        const bool overSaveButton = saveButton_.getGlobalBounds().contains(mousePos);
        if (overSaveButton != saveHover_)
        {
            saveHover_ = overSaveButton;
            updateButtonVisual(saveButton_, saveHover_);
        }

        const bool overTemplateButton = templateButton_.getGlobalBounds().contains(mousePos);
        if (overTemplateButton != templateHover_)
        {
            templateHover_ = overTemplateButton;
            updateButtonVisual(templateButton_, templateHover_);
        }

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

    target.draw(saveButton_);
    target.draw(saveButtonText_);
    target.draw(templateButton_);
    target.draw(templateButtonText_);
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
    sliders_[3].setValue(config_.lineThickness);
}

void DeveloperPanel::applyToNormalizer()
{
    config_.sensitivity = sliders_[0].value;
    config_.smoothing = sliders_[1].value;
    config_.deadZone = sliders_[2].value;
    config_.lineThickness = std::clamp(sliders_[3].value, 0.5f, 30.0f);
    normalizer_.configure(config_.sensitivity, config_.smoothing, config_.deadZone);
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

void DeveloperPanel::updateButtonVisual(sf::RectangleShape& button, bool hover)
{
    button.setFillColor(hover ? BUTTON_COLOR_HOVER : BUTTON_COLOR);
}

void DeveloperPanel::handleSaveClick()
{
    if (saveCallback_)
    {
        saveCallback_();
    }
}

void DeveloperPanel::handleTemplateSaveClick()
{
    if (saveTemplateCallback_)
    {
        saveTemplateCallback_();
    }
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
