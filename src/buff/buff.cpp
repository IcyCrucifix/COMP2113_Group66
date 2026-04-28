#include "buff.h"

#include <algorithm>
#include <cmath>

namespace
{
/* What it does: Clamps an integer so it cannot go below zero.
 * Inputs: value - integer value to clamp.
 * Outputs: Zero if the input is negative; otherwise the original value.
 */
int clampNonNegative(int value)
{
    return std::max(0, value);
}
}

std::string difficultyName(Difficulty difficulty)
{
    switch (difficulty)
    {
    case Difficulty::Easy:
        return "Easy";
    case Difficulty::Normal:
        return "Normal";
    case Difficulty::Hard:
        return "Hard";
    }
    return "Unknown";
}

std::string damageTypeName(DamageType damageType)
{
    switch (damageType)
    {
    case DamageType::Physical:
        return "physical";
    case DamageType::Magic:
        return "magic";
    case DamageType::Pure:
        return "pure";
    }
    return "unknown";
}

std::string cardCategoryName(CardCategory category)
{
    switch (category)
    {
    case CardCategory::Attack:
        return "Attack";
    case CardCategory::Defense:
        return "Defense";
    case CardCategory::Status:
        return "Status";
    case CardCategory::Other:
        return "Other";
    }
    return "Unknown";
}

std::string heroClassName(HeroClass heroClass)
{
    switch (heroClass)
    {
    case HeroClass::Offensive:
        return "Offensive";
    case HeroClass::Defensive:
        return "Defensive";
    case HeroClass::Magic:
        return "Magic";
    }
    return "Unknown";
}

std::string bossArchetypeName(BossArchetype archetype)
{
    switch (archetype)
    {
    case BossArchetype::Offensive:
        return "Offensive";
    case BossArchetype::Defensive:
        return "Defensive";
    case BossArchetype::Magic:
        return "Magic";
    case BossArchetype::Hybrid:
        return "Hybrid";
    }
    return "Unknown";
}

std::vector<std::string> activeStatusTags(const StatusBlock &statuses)
{
    std::vector<std::string> tags;

    if (statuses.block > 0)
    {
        tags.push_back("[Block x" + std::to_string(statuses.block) + "]");
    }
    if (statuses.power > 0)
    {
        tags.push_back("[Power x" + std::to_string(statuses.power) + "]");
    }
    if (statuses.burn > 0)
    {
        tags.push_back("[Burn x" + std::to_string(statuses.burn) + "]");
    }
    if (statuses.vulnerability > 0)
    {
        tags.push_back("[Vulnerability x" + std::to_string(statuses.vulnerability) + "]");
    }
    if (statuses.poison > 0)
    {
        tags.push_back("[Poison x" + std::to_string(statuses.poison) + "]");
    }
    if (statuses.energySaving > 0)
    {
        tags.push_back("[Energy Saving x" + std::to_string(statuses.energySaving) + "]");
    }
    if (statuses.shield > 0)
    {
        tags.push_back("[Shield " + std::to_string(statuses.shield) + "]");
    }

    return tags;
}

int countDebuffs(const StatusBlock &statuses)
{
    int count = 0;
    if (statuses.burn > 0)
    {
        ++count;
    }
    if (statuses.vulnerability > 0)
    {
        ++count;
    }
    if (statuses.poison > 0)
    {
        ++count;
    }
    return count;
}

double matchupMultiplier(HeroClass heroClass, BossArchetype archetype)
{
    if (heroClass == HeroClass::Offensive && archetype == BossArchetype::Defensive)
    {
        return 0.75;
    }
    if (heroClass == HeroClass::Offensive && archetype == BossArchetype::Magic)
    {
        return 1.25;
    }
    if (heroClass == HeroClass::Defensive && archetype == BossArchetype::Offensive)
    {
        return 1.25;
    }
    if (heroClass == HeroClass::Defensive && archetype == BossArchetype::Magic)
    {
        return 0.75;
    }
    if (heroClass == HeroClass::Magic && archetype == BossArchetype::Offensive)
    {
        return 0.75;
    }
    if (heroClass == HeroClass::Magic && archetype == BossArchetype::Defensive)
    {
        return 1.25;
    }
    if (heroClass == HeroClass::Magic && archetype == BossArchetype::Hybrid)
    {
        return 1.25;
    }
    return 1.0;
}

int computeDamage(int baseDamage, DamageType damageType, const StatusBlock &attacker, const StatusBlock &defender, int bonusPercent)
{
    double amount = static_cast<double>(baseDamage);
    amount *= 1.0 + (static_cast<double>(attacker.power) * 0.20);
    amount *= 1.0 + (static_cast<double>(attacker.tempOutgoingBonusPercent + bonusPercent) / 100.0);

    if (damageType == DamageType::Physical)
    {
        amount *= 1.0 + (static_cast<double>(defender.vulnerability) * 0.05);
    }
    else if (damageType == DamageType::Magic)
    {
        amount *= 1.0 + (static_cast<double>(defender.burn) * 0.07);
    }

    return clampNonNegative(static_cast<int>(std::round(amount)));
}

int mitigateDamage(int incomingDamage, DamageType damageType, StatusBlock &defender)
{
    double amount = static_cast<double>(incomingDamage);
    amount *= 1.0 + (static_cast<double>(defender.tempIncomingBonusPercent) / 100.0);

    double reduction = static_cast<double>(defender.tempDamageReductionPercent);
    if (damageType == DamageType::Physical)
    {
        reduction += static_cast<double>(defender.tempPhysicalReductionPercent);
    }
    else if (damageType == DamageType::Magic)
    {
        reduction += static_cast<double>(defender.tempMagicReductionPercent);
    }
    reduction = std::min(95.0, reduction);
    amount *= (100.0 - reduction) / 100.0;

    const int blockReduction = std::min(defender.block * 20, 80);
    amount *= (100.0 - static_cast<double>(blockReduction)) / 100.0;

    int reduced = clampNonNegative(static_cast<int>(std::round(amount)));
    if (defender.shield > 0)
    {
        const int absorbed = std::min(defender.shield, reduced);
        defender.shield -= absorbed;
        reduced -= absorbed;
    }
    return clampNonNegative(reduced);
}

int poisonTickDamage(const StatusBlock &statuses)
{
    return clampNonNegative(statuses.poison * 12);
}

void resetTurnBonuses(StatusBlock &statuses)
{
    statuses.tempDamageReductionPercent = 0;
    statuses.tempPhysicalReductionPercent = 0;
    statuses.tempMagicReductionPercent = 0;
    statuses.tempOutgoingBonusPercent = 0;
    statuses.tempIncomingBonusPercent = 0;
    statuses.opportunismReady = false;
    statuses.thornArmorReady = false;
}

void clearBattleStatuses(StatusBlock &statuses)
{
    statuses = StatusBlock{};
}

std::vector<PermanentBuff> buildPermanentBuffCatalog()
{
    return {
        {"mighty_strikes", "Mighty Strikes", "+20% to all damage you deal.", 20, 0, 0, 0, 0, 0, 0, 0, 0},
        {"iron_will", "Iron Will", "+15 maximum HP.", 0, 15, 0, 0, 0, 0, 0, 0, 0},
        {"energy_surge", "Energy Surge", "+1 Energy cap every round.", 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {"scholar_mind", "Scholar Mind", "Draw 1 extra card each round.", 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {"ember_mastery", "Ember Mastery", "All Burn applications gain +2 stacks.", 0, 0, 0, 0, 2, 0, 0, 0, 0},
        {"toxic_mastery", "Toxic Mastery", "All Poison applications gain +2 stacks.", 0, 0, 0, 0, 0, 2, 0, 0, 0},
        {"vital_echo", "Vital Echo", "+20% to healing effects.", 0, 0, 0, 0, 0, 0, 20, 0, 0},
        {"barrier_core", "Barrier Core", "+8 Shield each round and start battles with 1 Block.", 0, 0, 0, 0, 0, 0, 0, 8, 1}};
}

const PermanentBuff *findPermanentBuffById(const std::vector<PermanentBuff> &buffs, const std::string &id)
{
    for (const PermanentBuff &buff : buffs)
    {
        if (buff.id == id)
        {
            return &buff;
        }
    }
    return nullptr;
}
