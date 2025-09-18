#pragma once

#include <optional>
#include <string>

class SerialPort
{
public:
    SerialPort();
    explicit SerialPort(const std::string& device);
    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;
    SerialPort(SerialPort&&) = delete;
    SerialPort& operator=(SerialPort&&) = delete;
    ~SerialPort();

    bool open(const std::string& device);
    void close();
    [[nodiscard]] bool isOpen() const noexcept;
    std::optional<std::string> readLine();

private:
    std::size_t findLineBreak() const;
    void consumeLine(std::size_t endIndex);

    int fileDescriptor_;
    std::string buffer_;
};
