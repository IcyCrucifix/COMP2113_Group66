#include "hero.h"

#include "buff.h"

#include <algorithm>

Hero::Hero(const HeroTemplate &heroTemplate) : profile_(heroTemplate)
{
    resetCampaignState();
}

const HeroTemplate &Hero::profile() const
{
    return profile_;
}

StatusBlock &Hero::statuses()
{
    return statuses_;
}

const StatusBlock &Hero::statuses() const
{
    return statuses_;
}

void Hero::resetCampaignState()
{
    bonusMaxHp_ = 0;
    bonusEnergyCap_ = 0;
    bonusDraw_ = 0;
    bonusDamagePercent_ = 0;
    bonusBurn_ = 0;
    bonusPoison_ = 0;
    bonusHealingPercent_ = 0;
    bonusShieldPerRound_ = 0;
    bonusStartingBlock_ = 0;
    permanentBuffIds_.clear();
    currentHp_ = maxHp();
    resetForBattle();
}

void Hero::resetForBattle()
{
    clearBattleStatuses(statuses_);
    statuses_.block += bonusStartingBlock_;
    damageDealtThisRound_ = 0;
}

void Hero::beginRound()
{
    damageDealtThisRound_ = 0;
    resetTurnBonuses(statuses_);

    if (profile_.heroClass == HeroClass::Defensive)
    {
        statuses_.block += 1;
        statuses_.shield += 12;
    }

    statuses_.shield += bonusShieldPerRound_;
}

void Hero::endRound(std::vector<std::string> &logs)
{
    if (profile_.heroClass == HeroClass::Offensive && damageDealtThisRound_ >= 220)
    {
        statuses_.power += 1;
        logs.push_back(profile_.name + " gained 1 Power from the burst passive.");
    }
}

void Hero::heal(int amount)
{
    currentHp_ = std::min(maxHp(), currentHp_ + std::max(0, amount));
}

void Hero::takeDamage(int amount)
{
    currentHp_ = std::max(0, currentHp_ - std::max(0, amount));
}

void Hero::setCurrentHp(int amount)
{
    currentHp_ = std::max(0, std::min(maxHp(), amount));
}

void Hero::noteDamageDealt(int amount)
{
    damageDealtThisRound_ += std::max(0, amount);
}

void Hero::addPermanentBuff(const PermanentBuff &buff)
{
    bonusMaxHp_ += buff.maxHpBonus;
    bonusEnergyCap_ += buff.energyCapBonus;
    bonusDraw_ += buff.drawBonus;
    bonusDamagePercent_ += buff.damagePercent;
    bonusBurn_ += buff.burnBonus;
    bonusPoison_ += buff.poisonBonus;
    bonusHealingPercent_ += buff.healingBonusPercent;
    bonusShieldPerRound_ += buff.shieldPerTurnBonus;
    bonusStartingBlock_ += buff.startingBlock;
    permanentBuffIds_.push_back(buff.id);
    currentHp_ = std::min(maxHp(), currentHp_ + buff.maxHpBonus);
}

std::vector<std::string> Hero::permanentBuffIds() const
{
    return permanentBuffIds_;
}

void Hero::recoverBetweenBattles(int amount)
{
    heal(amount);
}

int Hero::currentHp() const
{
    return currentHp_;
}

int Hero::maxHp() const
{
    return profile_.maxHp + bonusMaxHp_;
}

int Hero::energyCap() const
{
    return profile_.energyCap + bonusEnergyCap_;
}

int Hero::cardsPerRound(const StatusBlock &enemyStatuses) const
{
    int cards = 4 + bonusDraw_;
    if (profile_.heroClass == HeroClass::Magic && countDebuffs(enemyStatuses) >= 2)
    {
        cards += 1;
    }
    return std::max(1, cards - statuses_.nextTurnDrawPenalty);
}

int Hero::damageBonusPercent(bool enemyBelowThirtyPercent, bool isMagicDamage) const
{
    int bonus = bonusDamagePercent_;
    if (profile_.heroClass == HeroClass::Offensive && enemyBelowThirtyPercent)
    {
        bonus += 3;
    }
    if (profile_.heroClass == HeroClass::Magic && isMagicDamage)
    {
        bonus += 15;
    }
    return bonus;
}

int Hero::defenseCardDiscount() const
{
    return profile_.heroClass == HeroClass::Defensive ? 1 : 0;
}

int Hero::burnBonusStacks() const
{
    const int classBonus = profile_.heroClass == HeroClass::Magic ? 1 : 0;
    return classBonus + bonusBurn_;
}

int Hero::poisonBonusStacks() const
{
    const int classBonus = profile_.heroClass == HeroClass::Magic ? 1 : 0;
    return classBonus + bonusPoison_;
}

int Hero::healingBonusPercent() const
{
    return bonusHealingPercent_;
}

int Hero::passiveReflectPercent() const
{
    return profile_.heroClass == HeroClass::Defensive ? 10 : 0;
}

int Hero::startingBlockBonus() const
{
    return bonusStartingBlock_;
}

int Hero::shieldPerRoundBonus() const
{
    return bonusShieldPerRound_;
}

bool Hero::isAlive() const
{
    return currentHp_ > 0;
}

Boss::Boss(const BossTemplate &bossTemplate, int rolledHp, Difficulty difficulty) : profile_(bossTemplate), currentHp_(rolledHp), maxHp_(rolledHp), difficulty_(difficulty)
{
}

const BossTemplate &Boss::profile() const
{
    return profile_;
}

StatusBlock &Boss::statuses()
{
    return statuses_;
}

const StatusBlock &Boss::statuses() const
{
    return statuses_;
}

void Boss::heal(int amount)
{
    currentHp_ = std::min(maxHp_, currentHp_ + std::max(0, amount));
}

void Boss::takeDamage(int amount)
{
    currentHp_ = std::max(0, currentHp_ - std::max(0, amount));
}

int Boss::currentHp() const
{
    return currentHp_;
}

int Boss::maxHp() const
{
    return maxHp_;
}

bool Boss::isAlive() const
{
    return currentHp_ > 0;
}

bool Boss::belowThirtyPercent() const
{
    return currentHp_ * 100 <= maxHp_ * 30;
}

bool Boss::belowFortyPercent() const
{
    return currentHp_ * 100 <= maxHp_ * 40;
}

void Boss::setCurrentHp(int amount)
{
    currentHp_ = std::max(0, std::min(maxHp_, amount));
}

void Boss::setRoundsStarted(int roundsStarted)
{
    roundsStarted_ = std::max(0, roundsStarted);
}

void Boss::beginRound()
{
    ++roundsStarted_;
    resetTurnBonuses(statuses_);
}

int Boss::roundsStarted() const
{
    return roundsStarted_;
}

int Boss::damageBonusPercent() const
{
    int bonus = 0;
    if (profile_.archetype == BossArchetype::Offensive && belowThirtyPercent())
    {
        bonus += 30;
    }
    return bonus;
}

Difficulty Boss::difficulty() const
{
    return difficulty_;
}
