#pragma once

#include "buff.h"
#include "card.h"
#include "hero.h"

#include <string>
#include <vector>

struct SaveGameData
{
    std::string heroId;
    Difficulty difficulty = Difficulty::Normal;
    int currentHp = 0;
    int battleIndex = 0;
    unsigned int seed = 0;
    std::vector<std::string> permanentBuffIds;
    std::vector<std::string> bossSequence;
    bool battleInProgress = false;
    std::string currentBossId;
    int currentBossHp = 0;
    int currentBossMaxHp = 0;
    int currentBossRoundsStarted = 0;
    int currentRound = 1;
    int energy = 0;
    int currentBattlePressure = 0;
    std::string plannedMoveName;
    int plannedMoveCategory = 0;
    int plannedMoveDamageType = 0;
    int plannedMoveDamage = 0;
    int plannedMoveBurn = 0;
    int plannedMovePoison = 0;
    int plannedMoveVulnerability = 0;
    int plannedMoveHeal = 0;
    int plannedMoveShield = 0;
    int plannedMoveGuardAll = 0;
    int plannedMoveGuardPhysical = 0;
    int plannedMoveGuardMagic = 0;
    int plannedMovePower = 0;
    int plannedMoveDrawPenalty = 0;
    std::string plannedMoveFlavor;
    std::vector<int> heroStatuses;
    std::vector<int> bossStatuses;
    std::vector<std::string> drawPile;
    std::vector<std::string> discardPile;
    std::vector<std::string> hand;
};

/* What it does: Loads card definitions from a delimited data file.
 * Inputs: path - file path to the card data file.
 * Outputs: Vector of loaded card definitions.
 */
std::vector<CardDefinition> loadCardDefinitions(const std::string &path);

/* What it does: Loads hero templates from a delimited data file.
 * Inputs: path - file path to the hero data file.
 * Outputs: Vector of loaded hero templates.
 */
std::vector<HeroTemplate> loadHeroTemplates(const std::string &path);

/* What it does: Loads boss templates from a delimited data file.
 * Inputs: path - file path to the boss data file.
 * Outputs: Vector of loaded boss templates.
 */
std::vector<BossTemplate> loadBossTemplates(const std::string &path);

/* What it does: Writes the save game snapshot to disk.
 * Inputs: path - save file path; saveGame - serializable campaign state.
 * Outputs: True if the file was written successfully; otherwise false.
 */
bool saveGameToFile(const std::string &path, const SaveGameData &saveGame);

/* What it does: Reads the save game snapshot from disk.
 * Inputs: path - save file path; saveGame - output snapshot object.
 * Outputs: True if the file was read successfully; otherwise false.
 */
bool loadGameFromFile(const std::string &path, SaveGameData &saveGame);

/* What it does: Removes a save file if it exists.
 * Inputs: path - save file path.
 * Outputs: True if the file was removed or did not exist; otherwise false.
 */
bool removeSaveFile(const std::string &path);
