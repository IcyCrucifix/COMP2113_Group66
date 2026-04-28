#include "card.h"

#include "deck.h"
#include "hero.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
/* What it does: Scales a numeric card value by a multiplier and rounds it to an integer.
 * Inputs: value - base numeric value; scale - multiplier to apply; allowZero - whether zero is allowed as the minimum result.
 * Outputs: Scaled rounded integer value, clamped to at least 1 unless zero is allowed.
 */
int scaleValue(int value, double scale, bool allowZero = false)
{
    const int scaled = static_cast<int>(std::round(static_cast<double>(value) * scale));
    if (allowZero)
    {
        return std::max(0, scaled);
    }
    return std::max(1, scaled);
}

/* What it does: Scales a healing value and then applies the hero's healing bonus percentage.
 * Inputs: baseHealing - unscaled healing amount; hero - acting hero whose healing bonuses apply; scale - multiplier to apply before healing bonuses.
 * Outputs: Final non-negative healing amount after scaling and bonus healing are applied.
 */
int applyHealingScale(int baseHealing, const Hero &hero, double scale)
{
    const int scaled = scaleValue(baseHealing, scale);
    const double totalScale = 1.0 + (static_cast<double>(hero.healingBonusPercent()) / 100.0);
    return std::max(0, static_cast<int>(std::round(static_cast<double>(scaled) * totalScale)));
}
}

CardCategory parseCardCategory(const std::string &text)
{
    if (text == "Attack")
    {
        return CardCategory::Attack;
    }
    if (text == "Defense")
    {
        return CardCategory::Defense;
    }
    if (text == "Status")
    {
        return CardCategory::Status;
    }
    return CardCategory::Other;
}

DamageType parseDamageType(const std::string &text)
{
    if (text == "Physical")
    {
        return DamageType::Physical;
    }
    if (text == "Magic")
    {
        return DamageType::Magic;
    }
    return DamageType::Pure;
}

CardEffect parseCardEffect(const std::string &text)
{
    if (text == "DIRECT_DAMAGE")
    {
        return CardEffect::DirectDamage;
    }
    if (text == "DAMAGE_AND_BURN")
    {
        return CardEffect::DamageAndBurn;
    }
    if (text == "DAMAGE_AND_VULNERABILITY")
    {
        return CardEffect::DamageAndVulnerability;
    }
    if (text == "SELF_DAMAGE_THEN_DAMAGE")
    {
        return CardEffect::SelfDamageThenDamage;
    }
    if (text == "VULNERABILITY_BURST")
    {
        return CardEffect::VulnerabilityBurst;
    }
    if (text == "BLOCK_TO_DAMAGE")
    {
        return CardEffect::BlockToDamage;
    }
    if (text == "GUARD_ALL")
    {
        return CardEffect::GuardAll;
    }
    if (text == "GUARD_PHYSICAL_AND_BLOCK")
    {
        return CardEffect::GuardPhysicalAndBlock;
    }
    if (text == "GUARD_MAGIC_AND_BLOCK")
    {
        return CardEffect::GuardMagicAndBlock;
    }
    if (text == "STATUS_COUNTER_GUARD")
    {
        return CardEffect::StatusCounterGuard;
    }
    if (text == "THORN_GUARD")
    {
        return CardEffect::ThornGuard;
    }
    if (text == "GAIN_ENERGY_SAVING")
    {
        return CardEffect::GainEnergySaving;
    }
    if (text == "GAIN_POWER")
    {
        return CardEffect::GainPower;
    }
    if (text == "BLOCK_TO_POWER")
    {
        return CardEffect::BlockToPower;
    }
    if (text == "APPLY_POISON")
    {
        return CardEffect::ApplyPoison;
    }
    if (text == "APPLY_VULNERABILITY")
    {
        return CardEffect::ApplyVulnerability;
    }
    if (text == "SELF_RISK_BURN")
    {
        return CardEffect::SelfRiskBurn;
    }
    if (text == "HEAL")
    {
        return CardEffect::Heal;
    }
    if (text == "CONDITIONAL_HEAL")
    {
        return CardEffect::ConditionalHeal;
    }
    if (text == "FILTER_DRAW")
    {
        return CardEffect::FilterDraw;
    }
    if (text == "NEXT_TURN_ENERGY")
    {
        return CardEffect::NextTurnEnergy;
    }
    throw std::runtime_error("Unknown card effect token: " + text);
}

int effectiveCardCost(const CardDefinition &card, const Hero &hero)
{
    int cost = card.cost - hero.statuses().energySaving;
    if (card.category == CardCategory::Defense)
    {
        cost -= hero.defenseCardDiscount();
    }
    return std::max(0, cost);
}

CardPlayResult resolveCardEffect(const CardDefinition &card, Hero &hero, Boss &boss, const CardContext &context)
{
    CardPlayResult result;
    result.success = true;

    auto damageBoss = [&](int baseDamage, DamageType damageType) {
        const int bonus = hero.damageBonusPercent(boss.belowThirtyPercent(), damageType == DamageType::Magic);
        const int outgoing = computeDamage(scaleValue(baseDamage, context.heroValueScale), damageType, hero.statuses(), boss.statuses(), bonus);
        const int finalDamage = mitigateDamage(outgoing, damageType, boss.statuses());
        boss.takeDamage(finalDamage);
        hero.noteDamageDealt(finalDamage);
        result.logLines.push_back(card.name + " dealt " + std::to_string(finalDamage) + " " + damageTypeName(damageType) + " damage.");
        return finalDamage;
    };

    switch (card.effect)
    {
    case CardEffect::DirectDamage:
        damageBoss(card.value1, card.damageType);
        break;
    case CardEffect::DamageAndBurn:
        damageBoss(card.value1, card.damageType);
        boss.statuses().burn += scaleValue(card.value2, context.heroValueScale) + hero.burnBonusStacks();
        result.logLines.push_back(boss.profile().name + " gained Burn.");
        break;
    case CardEffect::DamageAndVulnerability:
        damageBoss(card.value1, card.damageType);
        boss.statuses().vulnerability += scaleValue(card.value2, context.heroValueScale) + (hero.profile().heroClass == HeroClass::Offensive ? 1 : 0);
        result.logLines.push_back(boss.profile().name + " became Vulnerable.");
        break;
    case CardEffect::SelfDamageThenDamage:
    {
        const int selfDamage = mitigateDamage(scaleValue(card.value1, context.heroValueScale), DamageType::Physical, hero.statuses());
        hero.takeDamage(selfDamage);
        result.logLines.push_back(card.name + " hit you for " + std::to_string(selfDamage) + " recoil damage.");
        damageBoss(card.value2, card.damageType);
        break;
    }
    case CardEffect::VulnerabilityBurst:
    {
        hero.statuses().tempIncomingBonusPercent += 100;
        const int burstBase = card.value1 + (boss.statuses().vulnerability * card.value2);
        damageBoss(burstBase, card.damageType);
        result.logLines.push_back("You will take double damage for the rest of this round.");
        break;
    }
    case CardEffect::BlockToDamage:
    {
        const int storedBlock = hero.statuses().block;
        hero.statuses().block = 0;
        if (storedBlock <= 0)
        {
            result.logLines.push_back("No Block was available, so Revenge Strike fizzled.");
        }
        else
        {
            damageBoss(storedBlock * card.value1, card.damageType);
        }
        break;
    }
    case CardEffect::GuardAll:
        hero.statuses().tempDamageReductionPercent = std::max(hero.statuses().tempDamageReductionPercent, scaleValue(card.value1, context.heroValueScale));
        result.logLines.push_back("You braced for all incoming damage.");
        break;
    case CardEffect::GuardPhysicalAndBlock:
        hero.statuses().tempPhysicalReductionPercent = std::max(hero.statuses().tempPhysicalReductionPercent, scaleValue(card.value1, context.heroValueScale));
        hero.statuses().block += scaleValue(card.value2, context.heroValueScale);
        result.logLines.push_back("You raised an Iron Shield.");
        break;
    case CardEffect::GuardMagicAndBlock:
        hero.statuses().tempMagicReductionPercent = std::max(hero.statuses().tempMagicReductionPercent, scaleValue(card.value1, context.heroValueScale));
        hero.statuses().block += scaleValue(card.value2, context.heroValueScale);
        result.logLines.push_back("You raised a Magic Shield.");
        break;
    case CardEffect::StatusCounterGuard:
        hero.statuses().tempDamageReductionPercent = std::max(hero.statuses().tempDamageReductionPercent, scaleValue(card.value1, context.heroValueScale));
        hero.statuses().opportunismReady = true;
        if (context.bossPlannedCategory == CardCategory::Status)
        {
            damageBoss(card.value2, DamageType::Pure);
            result.logLines.push_back("Opportunism anticipated a status move.");
        }
        break;
    case CardEffect::ThornGuard:
        hero.statuses().tempDamageReductionPercent = std::max(hero.statuses().tempDamageReductionPercent, scaleValue(card.value1, context.heroValueScale));
        hero.statuses().thornArmorReady = true;
        result.logLines.push_back("Thorn Armor is active for this round.");
        break;
    case CardEffect::GainEnergySaving:
        hero.statuses().energySaving += scaleValue(card.value1, context.heroValueScale);
        result.logLines.push_back("Energy Saving increased your efficiency.");
        break;
    case CardEffect::GainPower:
        hero.statuses().power += scaleValue(card.value1, context.heroValueScale);
        result.logLines.push_back("Power surged through your attacks.");
        break;
    case CardEffect::BlockToPower:
    {
        const int storedBlock = hero.statuses().block;
        hero.statuses().block = 0;
        hero.statuses().power += storedBlock;
        result.logLines.push_back("Metamorphosis converted " + std::to_string(storedBlock) + " Block into Power.");
        break;
    }
    case CardEffect::ApplyPoison:
        boss.statuses().poison += scaleValue(card.value1, context.heroValueScale) + hero.poisonBonusStacks();
        result.logLines.push_back(boss.profile().name + " was poisoned.");
        break;
    case CardEffect::ApplyVulnerability:
        boss.statuses().vulnerability += scaleValue(card.value1, context.heroValueScale) + (hero.profile().heroClass == HeroClass::Offensive ? 1 : 0);
        result.logLines.push_back(boss.profile().name + " became Vulnerable.");
        break;
    case CardEffect::SelfRiskBurn:
        hero.statuses().tempIncomingBonusPercent += scaleValue(card.value1, context.heroValueScale, true);
        boss.statuses().burn += scaleValue(card.value2, context.heroValueScale) + hero.burnBonusStacks();
        result.logLines.push_back("Inferno Blaze made you fragile but scorched the boss.");
        break;
    case CardEffect::Heal:
        hero.heal(applyHealingScale(card.value1, hero, context.heroValueScale));
        result.logLines.push_back(card.name + " restored your health.");
        break;
    case CardEffect::ConditionalHeal:
    {
        int healing = applyHealingScale(card.value1, hero, context.heroValueScale);
        if (context.bossPlannedCategory == CardCategory::Defense)
        {
            healing += applyHealingScale(card.value2, hero, context.heroValueScale);
        }
        hero.heal(healing);
        result.logLines.push_back(card.name + " restored " + std::to_string(healing) + " HP.");
        break;
    }
    case CardEffect::FilterDraw:
        if (context.deck != nullptr)
        {
            context.deck->moveHighestCostCardToBottom();
            context.deck->drawCards(card.value1);
            result.logLines.push_back("Tactical Mind cycled your hand and drew new cards.");
        }
        else
        {
            result.logLines.push_back("Tactical Mind could not find the deck context.");
        }
        break;
    case CardEffect::NextTurnEnergy:
        hero.statuses().nextTurnEnergyBonus += scaleValue(card.value1, context.heroValueScale);
        result.logLines.push_back("Endure Hardship stored bonus Energy for next round.");
        break;
    }

    return result;
}
