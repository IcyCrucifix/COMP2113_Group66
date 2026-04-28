#pragma once

#include "buff.h"

#include <random>
#include <string>
#include <vector>

namespace utils
{
/* What it does: Returns the shared random engine used by the game.
 * Inputs: None.
 * Outputs: Reference to the process-wide Mersenne Twister engine.
 */
std::mt19937 &rng();

/* What it does: Seeds the shared random engine.
 * Inputs: seed - seed value for the engine.
 * Outputs: None.
 */
void seedRng(unsigned int seed);

/* What it does: Returns a random integer inside an inclusive range.
 * Inputs: low - lower bound; high - upper bound.
 * Outputs: Random integer in [low, high].
 */
int randomInt(int low, int high);

/* What it does: Returns true with the requested probability.
 * Inputs: probability - chance between 0.0 and 1.0.
 * Outputs: True on success; otherwise false.
 */
bool chance(double probability);

/* What it does: Splits a string by one delimiter character.
 * Inputs: text - input string; delimiter - split delimiter.
 * Outputs: Vector of split tokens.
 */
std::vector<std::string> split(const std::string &text, char delimiter);

/* What it does: Joins a list of strings with one delimiter string.
 * Inputs: parts - pieces to join; delimiter - delimiter string.
 * Outputs: Combined string.
 */
std::string join(const std::vector<std::string> &parts, const std::string &delimiter);

/* What it does: Clears the terminal screen when supported.
 * Inputs: None.
 * Outputs: None.
 */
void clearScreen();

/* What it does: Reads a bounded integer choice from standard input.
 * Inputs: prompt - prompt line; low - smallest accepted value; high - largest accepted value.
 * Outputs: Valid integer chosen by the player.
 */
int promptChoice(const std::string &prompt, int low, int high);

/* What it does: Reads a full input line from standard input.
 * Inputs: prompt - prompt line.
 * Outputs: Trimmed input line.
 */
std::string promptLine(const std::string &prompt);

/* What it does: Repeats a string token several times.
 * Inputs: token - token to repeat; count - repeat count.
 * Outputs: Concatenated string.
 */
std::string repeat(const std::string &token, int count);

/* What it does: Centers text inside a fixed width.
 * Inputs: text - visible text; width - target width.
 * Outputs: Center-padded string.
 */
std::string center(const std::string &text, std::size_t width);

/* What it does: Pads text on the right while ignoring ANSI escape length.
 * Inputs: text - input string; width - target visible width.
 * Outputs: Right-padded string.
 */
std::string padVisible(const std::string &text, std::size_t width);

/* What it does: Removes ANSI escape sequences from a string.
 * Inputs: text - possibly colored string.
 * Outputs: Visible-text-only string.
 */
std::string stripAnsi(const std::string &text);

/* What it does: Returns a colored string using truecolor ANSI.
 * Inputs: text - text to color; r/g/b - RGB values.
 * Outputs: ANSI-colored string.
 */
std::string color(const std::string &text, int r, int g, int b);

/* What it does: Builds a block-style bar used for HP or Energy.
 * Inputs: current - current value; maximum - max value; width - bar width; fillColor - ANSI color for filled cells; emptyColor - ANSI color for empty cells.
 * Outputs: Colored bar string.
 */
std::string bar(int current, int maximum, int width, const std::string &fillColor, const std::string &emptyColor);

/* What it does: Checks whether a file exists.
 * Inputs: path - file path to check.
 * Outputs: True if the file exists; otherwise false.
 */
bool fileExists(const std::string &path);

/* What it does: Parses a hero class token from text.
 * Inputs: text - token to parse.
 * Outputs: Parsed hero class enum.
 */
HeroClass parseHeroClass(const std::string &text);

/* What it does: Parses a boss archetype token from text.
 * Inputs: text - token to parse.
 * Outputs: Parsed boss archetype enum.
 */
BossArchetype parseBossArchetype(const std::string &text);
}
