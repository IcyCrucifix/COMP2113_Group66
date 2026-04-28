#include "save_load.h"

#include "utils.h"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace
{
/* What it does: Reads a text data file and returns all non-empty, non-comment lines.
 * Inputs: path - file path to read.
 * Outputs: Vector of usable data lines from the file.
 */
std::vector<std::string> loadDelimitedLines(const std::string &path)
{
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line))
    {
        if (!line.empty() && line[0] != '#')
        {
            lines.push_back(line);
        }
    }

    return lines;
}

/* What it does: Joins a list of strings into one comma-separated string for save-file storage.
 * Inputs: values - list of strings to combine.
 * Outputs: Single comma-delimited string.
 */
std::string joinList(const std::vector<std::string> &values)
{
    return utils::join(values, ",");
}
}

std::vector<CardDefinition> loadCardDefinitions(const std::string &path)
{
    std::vector<CardDefinition> cards;
    for (const std::string &line : loadDelimitedLines(path))
    {
        const std::vector<std::string> parts = utils::split(line, '|');
        if (parts.size() < 10)
        {
            continue;
        }

        CardDefinition card;
        card.id = parts[0];
        card.name = parts[1];
        card.cost = std::stoi(parts[2]);
        card.category = parseCardCategory(parts[3]);
        card.effect = parseCardEffect(parts[4]);
        card.damageType = parseDamageType(parts[5]);
        card.value1 = std::stoi(parts[6]);
        card.value2 = std::stoi(parts[7]);
        card.value3 = std::stoi(parts[8]);
        card.description = parts[9];
        card.notes = parts.size() > 10 ? parts[10] : "";
        cards.push_back(card);
    }
    return cards;
}

std::vector<HeroTemplate> loadHeroTemplates(const std::string &path)
{
    std::vector<HeroTemplate> heroes;
    for (const std::string &line : loadDelimitedLines(path))
    {
        const std::vector<std::string> parts = utils::split(line, '|');
        if (parts.size() < 8)
        {
            continue;
        }

        HeroTemplate hero;
        hero.id = parts[0];
        hero.name = parts[1];
        hero.heroClass = utils::parseHeroClass(parts[2]);
        hero.maxHp = std::stoi(parts[3]);
        hero.energyCap = std::stoi(parts[4]);
        hero.speed = std::stoi(parts[5]);
        hero.passiveText = parts[6];
        hero.playstyle = parts[7];
        heroes.push_back(hero);
    }
    return heroes;
}

std::vector<BossTemplate> loadBossTemplates(const std::string &path)
{
    std::vector<BossTemplate> bosses;
    for (const std::string &line : loadDelimitedLines(path))
    {
        const std::vector<std::string> parts = utils::split(line, '|');
        if (parts.size() < 8)
        {
            continue;
        }

        BossTemplate boss;
        boss.id = parts[0];
        boss.name = parts[1];
        boss.archetype = utils::parseBossArchetype(parts[2]);
        boss.minHp = std::stoi(parts[3]);
        boss.maxHp = std::stoi(parts[4]);
        boss.speed = std::stoi(parts[5]);
        boss.behaviorText = parts[6];
        boss.specialText = parts[7];
        bosses.push_back(boss);
    }
    return bosses;
}

bool saveGameToFile(const std::string &path, const SaveGameData &saveGame)
{
    std::ofstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    file << "hero_id=" << saveGame.heroId << '\n';
    file << "difficulty=" << static_cast<int>(saveGame.difficulty) << '\n';
    file << "current_hp=" << saveGame.currentHp << '\n';
    file << "battle_index=" << saveGame.battleIndex << '\n';
    file << "seed=" << saveGame.seed << '\n';
    file << "permanent_buffs=" << joinList(saveGame.permanentBuffIds) << '\n';
    file << "boss_sequence=" << joinList(saveGame.bossSequence) << '\n';
    file << "battle_in_progress=" << (saveGame.battleInProgress ? 1 : 0) << '\n';
    file << "current_boss_id=" << saveGame.currentBossId << '\n';
    file << "current_boss_hp=" << saveGame.currentBossHp << '\n';
    file << "current_boss_max_hp=" << saveGame.currentBossMaxHp << '\n';
    file << "current_boss_rounds_started=" << saveGame.currentBossRoundsStarted << '\n';
    file << "current_round=" << saveGame.currentRound << '\n';
    file << "energy=" << saveGame.energy << '\n';
    file << "current_battle_pressure=" << saveGame.currentBattlePressure << '\n';
    file << "planned_move_name=" << saveGame.plannedMoveName << '\n';
    file << "planned_move_category=" << saveGame.plannedMoveCategory << '\n';
    file << "planned_move_damage_type=" << saveGame.plannedMoveDamageType << '\n';
    file << "planned_move_damage=" << saveGame.plannedMoveDamage << '\n';
    file << "planned_move_burn=" << saveGame.plannedMoveBurn << '\n';
    file << "planned_move_poison=" << saveGame.plannedMovePoison << '\n';
    file << "planned_move_vulnerability=" << saveGame.plannedMoveVulnerability << '\n';
    file << "planned_move_heal=" << saveGame.plannedMoveHeal << '\n';
    file << "planned_move_shield=" << saveGame.plannedMoveShield << '\n';
    file << "planned_move_guard_all=" << saveGame.plannedMoveGuardAll << '\n';
    file << "planned_move_guard_physical=" << saveGame.plannedMoveGuardPhysical << '\n';
    file << "planned_move_guard_magic=" << saveGame.plannedMoveGuardMagic << '\n';
    file << "planned_move_power=" << saveGame.plannedMovePower << '\n';
    file << "planned_move_draw_penalty=" << saveGame.plannedMoveDrawPenalty << '\n';
    file << "planned_move_flavor=" << saveGame.plannedMoveFlavor << '\n';
    file << "hero_statuses=" << joinList([&]() {
        std::vector<std::string> values;
        for (int value : saveGame.heroStatuses)
        {
            values.push_back(std::to_string(value));
        }
        return values;
    }()) << '\n';
    file << "boss_statuses=" << joinList([&]() {
        std::vector<std::string> values;
        for (int value : saveGame.bossStatuses)
        {
            values.push_back(std::to_string(value));
        }
        return values;
    }()) << '\n';
    file << "draw_pile=" << joinList(saveGame.drawPile) << '\n';
    file << "discard_pile=" << joinList(saveGame.discardPile) << '\n';
    file << "hand=" << joinList(saveGame.hand) << '\n';
    return true;
}

bool loadGameFromFile(const std::string &path, SaveGameData &saveGame)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        const std::size_t equals = line.find('=');
        if (equals == std::string::npos)
        {
            continue;
        }

        const std::string key = line.substr(0, equals);
        const std::string value = line.substr(equals + 1);

        if (key == "hero_id")
        {
            saveGame.heroId = value;
        }
        else if (key == "difficulty")
        {
            saveGame.difficulty = static_cast<Difficulty>(std::stoi(value));
        }
        else if (key == "current_hp")
        {
            saveGame.currentHp = std::stoi(value);
        }
        else if (key == "battle_index")
        {
            saveGame.battleIndex = std::stoi(value);
        }
        else if (key == "seed")
        {
            saveGame.seed = static_cast<unsigned int>(std::stoul(value));
        }
        else if (key == "permanent_buffs")
        {
            saveGame.permanentBuffIds = value.empty() ? std::vector<std::string>{} : utils::split(value, ',');
        }
        else if (key == "boss_sequence")
        {
            saveGame.bossSequence = value.empty() ? std::vector<std::string>{} : utils::split(value, ',');
        }
        else if (key == "battle_in_progress")
        {
            saveGame.battleInProgress = std::stoi(value) != 0;
        }
        else if (key == "current_boss_id")
        {
            saveGame.currentBossId = value;
        }
        else if (key == "current_boss_hp")
        {
            saveGame.currentBossHp = std::stoi(value);
        }
        else if (key == "current_boss_max_hp")
        {
            saveGame.currentBossMaxHp = std::stoi(value);
        }
        else if (key == "current_boss_rounds_started")
        {
            saveGame.currentBossRoundsStarted = std::stoi(value);
        }
        else if (key == "current_round")
        {
            saveGame.currentRound = std::stoi(value);
        }
        else if (key == "energy")
        {
            saveGame.energy = std::stoi(value);
        }
        else if (key == "current_battle_pressure")
        {
            saveGame.currentBattlePressure = std::stoi(value);
        }
        else if (key == "planned_move_name")
        {
            saveGame.plannedMoveName = value;
        }
        else if (key == "planned_move_category")
        {
            saveGame.plannedMoveCategory = std::stoi(value);
        }
        else if (key == "planned_move_damage_type")
        {
            saveGame.plannedMoveDamageType = std::stoi(value);
        }
        else if (key == "planned_move_damage")
        {
            saveGame.plannedMoveDamage = std::stoi(value);
        }
        else if (key == "planned_move_burn")
        {
            saveGame.plannedMoveBurn = std::stoi(value);
        }
        else if (key == "planned_move_poison")
        {
            saveGame.plannedMovePoison = std::stoi(value);
        }
        else if (key == "planned_move_vulnerability")
        {
            saveGame.plannedMoveVulnerability = std::stoi(value);
        }
        else if (key == "planned_move_heal")
        {
            saveGame.plannedMoveHeal = std::stoi(value);
        }
        else if (key == "planned_move_shield")
        {
            saveGame.plannedMoveShield = std::stoi(value);
        }
        else if (key == "planned_move_guard_all")
        {
            saveGame.plannedMoveGuardAll = std::stoi(value);
        }
        else if (key == "planned_move_guard_physical")
        {
            saveGame.plannedMoveGuardPhysical = std::stoi(value);
        }
        else if (key == "planned_move_guard_magic")
        {
            saveGame.plannedMoveGuardMagic = std::stoi(value);
        }
        else if (key == "planned_move_power")
        {
            saveGame.plannedMovePower = std::stoi(value);
        }
        else if (key == "planned_move_draw_penalty")
        {
            saveGame.plannedMoveDrawPenalty = std::stoi(value);
        }
        else if (key == "planned_move_flavor")
        {
            saveGame.plannedMoveFlavor = value;
        }
        else if (key == "hero_statuses")
        {
            saveGame.heroStatuses.clear();
            for (const std::string &part : (value.empty() ? std::vector<std::string>{} : utils::split(value, ',')))
            {
                saveGame.heroStatuses.push_back(std::stoi(part));
            }
        }
        else if (key == "boss_statuses")
        {
            saveGame.bossStatuses.clear();
            for (const std::string &part : (value.empty() ? std::vector<std::string>{} : utils::split(value, ',')))
            {
                saveGame.bossStatuses.push_back(std::stoi(part));
            }
        }
        else if (key == "draw_pile")
        {
            saveGame.drawPile = value.empty() ? std::vector<std::string>{} : utils::split(value, ',');
        }
        else if (key == "discard_pile")
        {
            saveGame.discardPile = value.empty() ? std::vector<std::string>{} : utils::split(value, ',');
        }
        else if (key == "hand")
        {
            saveGame.hand = value.empty() ? std::vector<std::string>{} : utils::split(value, ',');
        }
    }

    return !saveGame.heroId.empty();
}

bool removeSaveFile(const std::string &path)
{
    if (!utils::fileExists(path))
    {
        return true;
    }
    return std::remove(path.c_str()) == 0;
}
