#include "PointNormalizer.hpp"

#include <algorithm>
#include <cmath>

PointNormalizer::PointNormalizer(unsigned int width,
                                 unsigned int height,
                                 float adcMax,
                                 float sensitivity,
                                 float smoothing,
                                 float deadZone)
    : width_(width)
    , height_(height)
    , adcMax_(adcMax)
    , sensitivity_(std::clamp(sensitivity, 1.0f, 4.0f))
    , smoothing_(std::clamp(smoothing, 0.0f, 0.95f))
    , deadZone_(std::max(deadZone, 0.0f))
    , calibrated_(false)
    , offsetX_(0.0f)
    , offsetY_(0.0f)
    , haveOutput_(false)
    , lastOutput_({0.0f, 0.0f})
{
}

sf::Vector2f PointNormalizer::normalize(float rawX, float rawY)
{
    if (adcMax_ <= 0.0f)
    {
        return {0.0f, 0.0f};
    }

    const float normX = std::clamp(rawX / adcMax_, 0.0f, 1.0f);
    const float normY = std::clamp(rawY / adcMax_, 0.0f, 1.0f);

    if (!calibrated_)
    {
        offsetX_ = 0.5f - normX;
        offsetY_ = 0.5f - normY;
        calibrated_ = true;
        haveOutput_ = false;
    }

    float adjustedX = std::clamp(normX + offsetX_, 0.0f, 1.0f);
    float adjustedY = std::clamp(normY + offsetY_, 0.0f, 1.0f);

    const float halfDeadZone = deadZone_ * 0.5f;
    if (std::fabs(adjustedX - 0.5f) < halfDeadZone)
    {
        adjustedX = 0.5f;
    }
    if (std::fabs(adjustedY - 0.5f) < halfDeadZone)
    {
        adjustedY = 0.5f;
    }

    auto applyResponseCurve = [this](float value) {
        const float centered = std::clamp(value - 0.5f, -0.5f, 0.5f);
        const float magnitude = std::pow(std::min(std::fabs(centered) * 2.0f, 1.0f), sensitivity_);
        const float scaled = (centered >= 0.0f ? 1.0f : -1.0f) * (magnitude * 0.5f);
        return std::clamp(scaled + 0.5f, 0.0f, 1.0f);
    };

    const float curvedX = applyResponseCurve(adjustedX);
    const float curvedY = applyResponseCurve(adjustedY);

    sf::Vector2f target{
        curvedX * static_cast<float>(width_),
        curvedY * static_cast<float>(height_)
    };

    if (!haveOutput_)
    {
        lastOutput_ = target;
        haveOutput_ = true;
        return target;
    }

    const sf::Vector2f smoothed{
        lastOutput_.x + smoothing_ * (target.x - lastOutput_.x),
        lastOutput_.y + smoothing_ * (target.y - lastOutput_.y)
    };

    lastOutput_ = smoothed;
    return smoothed;
}

void PointNormalizer::reset()
{
    calibrated_ = false;
    offsetX_ = 0.0f;
    offsetY_ = 0.0f;
    haveOutput_ = false;
    lastOutput_ = {0.0f, 0.0f};
}
