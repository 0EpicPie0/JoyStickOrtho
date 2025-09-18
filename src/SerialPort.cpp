#include "SerialPort.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

SerialPort::SerialPort() : fileDescriptor_(-1)
{
}

SerialPort::SerialPort(const std::string& device) : SerialPort()
{
    open(device);
}

SerialPort::~SerialPort()
{
    close();
}

bool SerialPort::open(const std::string& device)
{
    close();

    const int fd = ::open(device.c_str(), O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        std::cerr << "Не могу открыть порт " << device << ": " << std::strerror(errno) << '\n';
        return false;
    }

    termios options{};
    if (tcgetattr(fd, &options) != 0)
    {
        std::cerr << "tcgetattr() failed: " << std::strerror(errno) << '\n';
        ::close(fd);
        return false;
    }

    cfmakeraw(&options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~(CSTOPB | PARENB | CRTSCTS);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        std::cerr << "tcsetattr() failed: " << std::strerror(errno) << '\n';
        ::close(fd);
        return false;
    }

    fileDescriptor_ = fd;
    buffer_.clear();
    return true;
}

void SerialPort::close()
{
    if (fileDescriptor_ >= 0)
    {
        ::close(fileDescriptor_);
        fileDescriptor_ = -1;
    }
    buffer_.clear();
}

bool SerialPort::isOpen() const noexcept
{
    return fileDescriptor_ >= 0;
}

std::optional<std::string> SerialPort::readLine()
{
    if (!isOpen())
    {
        return std::nullopt;
    }

    char chunk[256];
    while (true)
    {
        const ssize_t bytesRead = ::read(fileDescriptor_, chunk, sizeof(chunk));
        if (bytesRead > 0)
        {
            buffer_.append(chunk, static_cast<std::size_t>(bytesRead));
            continue;
        }

        if (bytesRead == 0)
        {
            break;
        }

        if (bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            break;
        }

        if (bytesRead < 0)
        {
            std::cerr << "Ошибка чтения из Serial: " << std::strerror(errno) << '\n';
            break;
        }
    }

    const std::size_t newlinePos = findLineBreak();
    if (newlinePos == std::string::npos)
    {
        return std::nullopt;
    }

    std::string line = buffer_.substr(0, newlinePos);
    consumeLine(newlinePos);
    return line;
}

std::size_t SerialPort::findLineBreak() const
{
    for (std::size_t i = 0; i < buffer_.size(); ++i)
    {
        const char ch = buffer_[i];
        if (ch == '\n' || ch == '\r')
        {
            return i;
        }
    }
    return std::string::npos;
}

void SerialPort::consumeLine(std::size_t endIndex)
{
    std::size_t eraseCount = endIndex;
    if (eraseCount < buffer_.size())
    {
        ++eraseCount;
        while (eraseCount < buffer_.size() && (buffer_[eraseCount] == '\n' || buffer_[eraseCount] == '\r'))
        {
            ++eraseCount;
        }
    }

    buffer_.erase(0, eraseCount);
}
