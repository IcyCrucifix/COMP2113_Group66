#include "utils.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>

namespace
{
std::mt19937 g_rng{std::random_device{}()};
}

namespace utils
{
std::mt19937 &rng()
{
    return g_rng;
}

void seedRng(unsigned int seed)
{
    g_rng.seed(seed);
}

int randomInt(int low, int high)
{
    std::uniform_int_distribution<int> distribution(low, high);
    return distribution(g_rng);
}

bool chance(double probability)
{
    std::bernoulli_distribution distribution(probability);
    return distribution(g_rng);
}

std::vector<std::string> split(const std::string &text, char delimiter)
{
    std::vector<std::string> parts;
    std::stringstream stream(text);
    std::string item;
    while (std::getline(stream, item, delimiter))
    {
        parts.push_back(item);
    }
    return parts;
}

std::string join(const std::vector<std::string> &parts, const std::string &delimiter)
{
    std::ostringstream stream;
    for (std::size_t index = 0; index < parts.size(); ++index)
    {
        if (index > 0)
        {
            stream << delimiter;
        }
        stream << parts[index];
    }
    return stream.str();
}

void clearScreen()
{
    std::cout << "\033[2J\033[H";
}

int promptChoice(const std::string &prompt, int low, int high)
{
    while (true)
    {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line))
        {
            return low;
        }

        std::stringstream stream(line);
        int choice = 0;
        if (stream >> choice && choice >= low && choice <= high)
        {
            return choice;
        }
        std::cout << "Please enter a number between " << low << " and " << high << ".\n";
    }
}

std::string promptLine(const std::string &prompt)
{
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::string repeat(const std::string &token, int count)
{
    std::string result;
    for (int index = 0; index < count; ++index)
    {
        result += token;
    }
    return result;
}

std::string center(const std::string &text, std::size_t width)
{
    const std::size_t visible = stripAnsi(text).size();
    if (visible >= width)
    {
        return text;
    }
    const std::size_t leftPadding = (width - visible) / 2;
    const std::size_t rightPadding = width - visible - leftPadding;
    return std::string(leftPadding, ' ') + text + std::string(rightPadding, ' ');
}

std::string padVisible(const std::string &text, std::size_t width)
{
    const std::size_t visible = stripAnsi(text).size();
    if (visible >= width)
    {
        return text;
    }
    return text + std::string(width - visible, ' ');
}

std::string stripAnsi(const std::string &text)
{
    std::string stripped;
    bool inEscape = false;

    for (char character : text)
    {
        if (!inEscape && character == '\033')
        {
            inEscape = true;
            continue;
        }
        if (inEscape)
        {
            if (character == 'm')
            {
                inEscape = false;
            }
            continue;
        }
        stripped.push_back(character);
    }

    return stripped;
}

std::string color(const std::string &text, int r, int g, int b)
{
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m" + text + "\033[0m";
}

std::string bar(int current, int maximum, int width, const std::string &fillColor, const std::string &emptyColor)
{
    if (maximum <= 0)
    {
        maximum = 1;
    }

    const double ratio = std::clamp(static_cast<double>(current) / static_cast<double>(maximum), 0.0, 1.0);
    const int filled = static_cast<int>(std::round(ratio * static_cast<double>(width)));
    return fillColor + repeat("█", filled) + emptyColor + repeat("░", std::max(0, width - filled)) + "\033[0m";
}

bool fileExists(const std::string &path)
{
    return std::filesystem::exists(path);
}

std::string resolveFirstExistingPath(const std::vector<std::string> &candidates)
{
    for (const std::string &candidate : candidates)
    {
        if (fileExists(candidate))
        {
            return candidate;
        }
    }
    return candidates.empty() ? std::string() : candidates.front();
}

HeroClass parseHeroClass(const std::string &text)
{
    if (text == "Offensive")
    {
        return HeroClass::Offensive;
    }
    if (text == "Defensive")
    {
        return HeroClass::Defensive;
    }
    return HeroClass::Magic;
}

BossArchetype parseBossArchetype(const std::string &text)
{
    if (text == "Offensive")
    {
        return BossArchetype::Offensive;
    }
    if (text == "Defensive")
    {
        return BossArchetype::Defensive;
    }
    if (text == "Magic")
    {
        return BossArchetype::Magic;
    }
    return BossArchetype::Hybrid;
}
}
