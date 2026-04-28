#pragma once

#include "buff.h"

#include <string>
#include <vector>

class Hero;
class Boss;
class Deck;

enum class CardEffect
{
    DirectDamage,
    DamageAndBurn,
    DamageAndVulnerability,
    SelfDamageThenDamage,
    VulnerabilityBurst,
    BlockToDamage,
    GuardAll,
    GuardPhysicalAndBlock,
    GuardMagicAndBlock,
    StatusCounterGuard,
    ThornGuard,
    GainEnergySaving,
    GainPower,
    BlockToPower,
    ApplyPoison,
    ApplyVulnerability,
    SelfRiskBurn,
    Heal,
    ConditionalHeal,
    FilterDraw,
    NextTurnEnergy
};

struct CardDefinition
{
    std::string id;
    std::string name;
    int cost = 0;
    CardCategory category = CardCategory::Other;
    CardEffect effect = CardEffect::DirectDamage;
    DamageType damageType = DamageType::Pure;
    int value1 = 0;
    int value2 = 0;
    int value3 = 0;
    std::string description;
    std::string notes;
};

struct CardContext
{
    double heroValueScale = 1.0;
    CardCategory bossPlannedCategory = CardCategory::Attack;
    Deck *deck = nullptr;
};

struct CardPlayResult
{
    bool success = false;
    int cardsToDraw = 0;
    std::vector<std::string> logLines;
};

/* What it does: Parses a card category token from file data.
 * Inputs: text - category token read from a file.
 * Outputs: Parsed card category enum.
 */
CardCategory parseCardCategory(const std::string &text);

/* What it does: Parses a damage type token from file data.
 * Inputs: text - damage type token read from a file.
 * Outputs: Parsed damage type enum.
 */
DamageType parseDamageType(const std::string &text);

/* What it does: Parses a card effect token from file data.
 * Inputs: text - effect token read from a file.
 * Outputs: Parsed card effect enum.
 */
CardEffect parseCardEffect(const std::string &text);

/* What it does: Computes the effective energy cost for a card after passive discounts.
 * Inputs: card - card being evaluated; hero - acting hero.
 * Outputs: Final energy cost.
 */
int effectiveCardCost(const CardDefinition &card, const Hero &hero);

/* What it does: Resolves a played card and applies its effects to the hero, boss, and deck.
 * Inputs: card - card being played; hero - acting hero; boss - target boss; context - extra round context such as planned boss category and deck pointer.
 * Outputs: CardPlayResult describing success, extra draws, and log messages.
 */
CardPlayResult resolveCardEffect(const CardDefinition &card, Hero &hero, Boss &boss, const CardContext &context);
