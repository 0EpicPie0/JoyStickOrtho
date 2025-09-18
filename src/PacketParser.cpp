#include "PacketParser.hpp"

#include <cctype>
#include <optional>
#include <string_view>

namespace
{
std::optional<std::string> extractToken(std::string_view line, std::string_view key)
{
    const std::string pattern = '"' + std::string(key) + '"';
    const std::size_t keyPos = line.find(pattern);
    if (keyPos == std::string::npos)
    {
        return std::nullopt;
    }

    const std::size_t colonPos = line.find(':', keyPos + pattern.size());
    if (colonPos == std::string::npos)
    {
        return std::nullopt;
    }

    std::size_t valueStart = colonPos + 1;
    while (valueStart < line.size() && std::isspace(static_cast<unsigned char>(line[valueStart])))
    {
        ++valueStart;
    }

    if (valueStart >= line.size())
    {
        return std::nullopt;
    }

    if (line[valueStart] == '"')
    {
        ++valueStart;
        const std::size_t valueEnd = line.find('"', valueStart);
        if (valueEnd == std::string::npos)
        {
            return std::nullopt;
        }
        return std::string(line.substr(valueStart, valueEnd - valueStart));
    }

    std::size_t valueEnd = valueStart;
    while (valueEnd < line.size() && (std::isdigit(static_cast<unsigned char>(line[valueEnd])) || line[valueEnd] == '-' || line[valueEnd] == '.'))
    {
        ++valueEnd;
    }

    return std::string(line.substr(valueStart, valueEnd - valueStart));
}
} // namespace

bool parsePacket(const std::string& line, SamplePoint& out)
{
    const auto xToken = extractToken(line, "x");
    const auto yToken = extractToken(line, "y");
    const auto cmdToken = extractToken(line, "cmd");

    if (!xToken || !yToken || !cmdToken)
    {
        return false;
    }

    try
    {
        out.x = std::stof(*xToken);
        out.y = std::stof(*yToken);
    }
    catch (...)
    {
        return false;
    }

    out.command = *cmdToken;
    return true;
}
