#pragma once

#include <string>

struct SamplePoint
{
    float x{0.0f};
    float y{0.0f};
    std::string command;
};

bool parsePacket(const std::string& line, SamplePoint& out);
