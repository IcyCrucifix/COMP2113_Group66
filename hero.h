#pragma once

#include "buff.h"

#include <string>
#include <vector>

struct HeroTemplate
{
    std::string id;
    std::string name;
    HeroClass heroClass = HeroClass::Offensive;
    int maxHp = 0;
    int energyCap = 0;
    int speed = 0;
    std::string passiveText;
    std::string playstyle;
};

struct BossTemplate
{
    std::string id;
    std::string name;
    BossArchetype archetype = BossArchetype::Offensive;
    int minHp = 0;
    int maxHp = 0;
    int speed = 0;
    std::string behaviorText;
    std::string specialText;
};

class Hero
{
public:
    /* What it does: Constructs a hero runtime state from a hero template.
     * Inputs: heroTemplate - chosen template loaded from file.
     * Outputs: A ready-to-configure hero object.
     */
    explicit Hero(const HeroTemplate &heroTemplate);

    /* What it does: Returns the template/profile information for this hero.
     * Inputs: None.
     * Outputs: Constant reference to the hero template.
     */
    const HeroTemplate &profile() const;

    /* What it does: Returns the mutable status bundle for this hero.
     * Inputs: None.
     * Outputs: Reference to the hero's combat statuses.
     */
    StatusBlock &statuses();

    /* What it does: Returns the read-only status bundle for this hero.
     * Inputs: None.
     * Outputs: Constant reference to the hero's combat statuses.
     */
    const StatusBlock &statuses() const;

    /* What it does: Starts a fresh campaign state for the hero.
     * Inputs: None.
     * Outputs: None.
     */
    void resetCampaignState();

    /* What it does: Clears battle-only statuses for the next boss fight.
     * Inputs: None.
     * Outputs: None.
     */
    void resetForBattle();

    /* What it does: Applies per-round passives before the player acts.
     * Inputs: None.
     * Outputs: None.
     */
    void beginRound();

    /* What it does: Resolves end-of-round hero passives and appends log messages.
     * Inputs: logs - battle log lines to append to.
     * Outputs: None.
     */
    void endRound(std::vector<std::string> &logs);

    /* What it does: Heals the hero without exceeding max HP.
     * Inputs: amount - healing amount.
     * Outputs: None.
     */
    void heal(int amount);

    /* What it does: Deducts HP from the hero and clamps the result at zero.
     * Inputs: amount - HP damage to apply.
     * Outputs: None.
     */
    void takeDamage(int amount);

    /* What it does: Sets the hero's current HP directly while respecting valid bounds.
     * Inputs: amount - target HP value.
     * Outputs: None.
     */
    void setCurrentHp(int amount);

    /* What it does: Records damage dealt this round for burst passives.
     * Inputs: amount - damage dealt by the hero.
     * Outputs: None.
     */
    void noteDamageDealt(int amount);

    /* What it does: Applies a permanent buff to this hero.
     * Inputs: buff - permanent reward definition.
     * Outputs: None.
     */
    void addPermanentBuff(const PermanentBuff &buff);

    /* What it does: Returns all permanent reward ids already owned by the hero.
     * Inputs: None.
     * Outputs: Vector of buff identifiers.
     */
    std::vector<std::string> permanentBuffIds() const;

    /* What it does: Restores some HP between boss fights.
     * Inputs: amount - amount to recover.
     * Outputs: None.
     */
    void recoverBetweenBattles(int amount);

    /* What it does: Returns current HP.
     * Inputs: None.
     * Outputs: Current HP value.
     */
    int currentHp() const;

    /* What it does: Returns total max HP after permanent buffs.
     * Inputs: None.
     * Outputs: Effective max HP value.
     */
    int maxHp() const;

    /* What it does: Returns total energy cap after permanent buffs.
     * Inputs: None.
     * Outputs: Effective energy cap.
     */
    int energyCap() const;

    /* What it does: Returns how many cards should be drawn this round.
     * Inputs: enemyStatuses - enemy status bundle for passive checks.
     * Outputs: Number of cards to draw.
     */
    int cardsPerRound(const StatusBlock &enemyStatuses) const;

    /* What it does: Returns bonus percent damage from permanent buffs and hero passives.
     * Inputs: enemyBelowThirtyPercent - whether the boss is under 30% HP; isMagicDamage - whether the damage is magic.
     * Outputs: Extra outgoing damage percentage.
     */
    int damageBonusPercent(bool enemyBelowThirtyPercent, bool isMagicDamage) const;

    /* What it does: Returns the cost discount for Defense cards from passives.
     * Inputs: None.
     * Outputs: Integer discount applied to defense cards.
     */
    int defenseCardDiscount() const;

    /* What it does: Returns extra Burn stacks granted by passives and permanent buffs.
     * Inputs: None.
     * Outputs: Extra Burn stacks.
     */
    int burnBonusStacks() const;

    /* What it does: Returns extra Poison stacks granted by passives and permanent buffs.
     * Inputs: None.
     * Outputs: Extra Poison stacks.
     */
    int poisonBonusStacks() const;

    /* What it does: Returns bonus healing percentage from permanent buffs.
     * Inputs: None.
     * Outputs: Extra healing percentage.
     */
    int healingBonusPercent() const;

    /* What it does: Returns passive reflect percent from the hero kit.
     * Inputs: None.
     * Outputs: Damage reflection percentage.
     */
    int passiveReflectPercent() const;

    /* What it does: Returns starting Block gained from permanent buffs.
     * Inputs: None.
     * Outputs: Starting Block stacks.
     */
    int startingBlockBonus() const;

    /* What it does: Returns extra Shield generated each round from permanent buffs.
     * Inputs: None.
     * Outputs: Shield amount per round.
     */
    int shieldPerRoundBonus() const;

    /* What it does: Returns whether the hero is still alive.
     * Inputs: None.
     * Outputs: True if HP is above zero; otherwise false.
     */
    bool isAlive() const;

private:
    HeroTemplate profile_;
    int currentHp_ = 0;
    int bonusMaxHp_ = 0;
    int bonusEnergyCap_ = 0;
    int bonusDraw_ = 0;
    int bonusDamagePercent_ = 0;
    int bonusBurn_ = 0;
    int bonusPoison_ = 0;
    int bonusHealingPercent_ = 0;
    int bonusShieldPerRound_ = 0;
    int bonusStartingBlock_ = 0;
    int damageDealtThisRound_ = 0;
    std::vector<std::string> permanentBuffIds_;
    StatusBlock statuses_;
};

class Boss
{
public:
    /* What it does: Constructs a boss runtime state for one fight.
     * Inputs: bossTemplate - chosen boss template; rolledHp - HP picked for this fight; difficulty - selected difficulty.
     * Outputs: Ready-to-fight boss object.
     */
    Boss(const BossTemplate &bossTemplate, int rolledHp, Difficulty difficulty);

    /* What it does: Returns the boss profile information.
     * Inputs: None.
     * Outputs: Constant reference to the boss template.
     */
    const BossTemplate &profile() const;

    /* What it does: Returns the mutable status bundle for this boss.
     * Inputs: None.
     * Outputs: Reference to the boss's combat statuses.
     */
    StatusBlock &statuses();

    /* What it does: Returns the read-only status bundle for this boss.
     * Inputs: None.
     * Outputs: Constant reference to the boss's combat statuses.
     */
    const StatusBlock &statuses() const;

    /* What it does: Heals the boss without exceeding max HP.
     * Inputs: amount - healing amount.
     * Outputs: None.
     */
    void heal(int amount);

    /* What it does: Deducts HP from the boss and clamps the result at zero.
     * Inputs: amount - HP damage to apply.
     * Outputs: None.
     */
    void takeDamage(int amount);

    /* What it does: Returns current HP.
     * Inputs: None.
     * Outputs: Current HP value.
     */
    int currentHp() const;

    /* What it does: Returns max HP for this fight.
     * Inputs: None.
     * Outputs: Boss max HP value.
     */
    int maxHp() const;

    /* What it does: Returns whether the boss is alive.
     * Inputs: None.
     * Outputs: True if HP is above zero; otherwise false.
     */
    bool isAlive() const;

    /* What it does: Returns whether the boss is below 30% HP.
     * Inputs: None.
     * Outputs: True if the threshold has been crossed.
     */
    bool belowThirtyPercent() const;

    /* What it does: Returns whether the boss is below 40% HP.
     * Inputs: None.
     * Outputs: True if the threshold has been crossed.
     */
    bool belowFortyPercent() const;

    /* What it does: Sets the boss's current HP directly while respecting valid bounds.
     * Inputs: amount - target HP value.
     * Outputs: None.
     */
    void setCurrentHp(int amount);

    /* What it does: Restores the boss round counter when loading a save.
     * Inputs: roundsStarted - round counter value to restore.
     * Outputs: None.
     */
    void setRoundsStarted(int roundsStarted);

    /* What it does: Advances the boss round counter.
     * Inputs: None.
     * Outputs: None.
     */
    void beginRound();

    /* What it does: Returns how many rounds this boss has already started.
     * Inputs: None.
     * Outputs: Round counter for boss behavior logic.
     */
    int roundsStarted() const;

    /* What it does: Returns bonus outgoing damage from boss passives.
     * Inputs: None.
     * Outputs: Extra outgoing damage percentage.
     */
    int damageBonusPercent() const;

    /* What it does: Returns the selected difficulty for this boss.
     * Inputs: None.
     * Outputs: Difficulty enum.
     */
    Difficulty difficulty() const;

private:
    BossTemplate profile_;
    int currentHp_ = 0;
    int maxHp_ = 0;
    int roundsStarted_ = 0;
    Difficulty difficulty_ = Difficulty::Normal;
    StatusBlock statuses_;
};
