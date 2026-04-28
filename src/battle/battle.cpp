#include "battle.h"

#include "utils.h"

#include <algorithm>
#include <iostream>

namespace
{
const std::string kWarmAccent = "\033[38;2;196;147;123m";
const std::string kPaleAccent = "\033[38;2;217;217;172m";
const std::string kOffWhite = "\033[38;2;227;227;227m";
const std::string kSoftBrown = "\033[38;2;165;140;115m";
const std::string kReset = "\033[0m";

/* What it does: Formats a centered colored title line for the battle interface.
 * Inputs: text - title text to display.
 * Outputs: Centered ANSI-colored title string.
 */
std::string titleLine(const std::string &text)
{
    return utils::center(kPaleAccent + text + kReset, 60);
}
}

Battle::Battle(std::vector<CardDefinition> cards, std::vector<HeroTemplate> heroes, std::vector<BossTemplate> bosses, std::string savePath)
    : cards_(std::move(cards)),
      heroes_(std::move(heroes)),
      bosses_(std::move(bosses)),
      permanentBuffs_(buildPermanentBuffCatalog()),
      deck_(std::make_unique<Deck>()),
      savePath_(std::move(savePath))
{
}

void Battle::setupNewRun(Difficulty difficulty, const std::string &heroId, unsigned int seed, int targetBossCount)
{
    const HeroTemplate *heroTemplate = findHeroTemplate(heroId);
    if (heroTemplate == nullptr)
    {
        throw std::runtime_error("Unknown hero id: " + heroId);
    }

    difficulty_ = difficulty;
    seed_ = seed;
    targetBossCount_ = std::max(1, targetBossCount);
    playerQuit_ = false;
    utils::seedRng(seed_);
    hero_ = std::make_unique<Hero>(*heroTemplate);
    deck_ = std::make_unique<Deck>();
    deck_->buildStarterDeck(cards_, hero_->profile().heroClass);
    battleIndex_ = 0;
    lastRoundCount_ = 0;
    lastHeroCategory_ = CardCategory::Other;
    battleLog_.clear();
    buildBossSequence();
}

bool Battle::setupFromSave(const SaveGameData &saveGame)
{
    const HeroTemplate *heroTemplate = findHeroTemplate(saveGame.heroId);
    if (heroTemplate == nullptr)
    {
        return false;
    }

    difficulty_ = saveGame.difficulty;
    seed_ = saveGame.seed;
    utils::seedRng(seed_);
    hero_ = std::make_unique<Hero>(*heroTemplate);
    deck_ = std::make_unique<Deck>();
    deck_->buildStarterDeck(cards_, hero_->profile().heroClass);
    battleIndex_ = saveGame.battleIndex;
    bossSequence_ = saveGame.bossSequence;
    targetBossCount_ = std::max(1, static_cast<int>(bossSequence_.size()));
    playerQuit_ = false;

    for (const std::string &buffId : saveGame.permanentBuffIds)
    {
        const PermanentBuff *buff = findPermanentBuffById(permanentBuffs_, buffId);
        if (buff != nullptr)
        {
            hero_->addPermanentBuff(*buff);
        }
    }

    hero_->setCurrentHp(saveGame.currentHp);
    battleLog_.clear();
    lastHeroCategory_ = CardCategory::Other;

    if (saveGame.battleInProgress && !saveGame.currentBossId.empty())
    {
        const BossTemplate *bossTemplate = findBossTemplate(saveGame.currentBossId);
        if (bossTemplate == nullptr)
        {
            return false;
        }

        boss_ = std::make_unique<Boss>(*bossTemplate, saveGame.currentBossMaxHp, difficulty_);
        boss_->setCurrentHp(saveGame.currentBossHp);
        boss_->setRoundsStarted(saveGame.currentBossRoundsStarted);
        hero_->statuses() = deserializeStatuses(saveGame.heroStatuses);
        boss_->statuses() = deserializeStatuses(saveGame.bossStatuses);
        deck_->restoreBattleState(restoreCardsById(saveGame.drawPile), restoreCardsById(saveGame.discardPile), restoreCardsById(saveGame.hand));
        currentRound_ = std::max(1, saveGame.currentRound);
        energy_ = saveGame.energy;
        currentBattlePressure_ = static_cast<BattlePressure>(saveGame.currentBattlePressure);
        currentPlannedMove_.name = saveGame.plannedMoveName;
        currentPlannedMove_.category = static_cast<CardCategory>(saveGame.plannedMoveCategory);
        currentPlannedMove_.damageType = static_cast<DamageType>(saveGame.plannedMoveDamageType);
        currentPlannedMove_.damage = saveGame.plannedMoveDamage;
        currentPlannedMove_.burn = saveGame.plannedMoveBurn;
        currentPlannedMove_.poison = saveGame.plannedMovePoison;
        currentPlannedMove_.vulnerability = saveGame.plannedMoveVulnerability;
        currentPlannedMove_.heal = saveGame.plannedMoveHeal;
        currentPlannedMove_.shield = saveGame.plannedMoveShield;
        currentPlannedMove_.guardAll = saveGame.plannedMoveGuardAll;
        currentPlannedMove_.guardPhysical = saveGame.plannedMoveGuardPhysical;
        currentPlannedMove_.guardMagic = saveGame.plannedMoveGuardMagic;
        currentPlannedMove_.power = saveGame.plannedMovePower;
        currentPlannedMove_.drawPenalty = saveGame.plannedMoveDrawPenalty;
        currentPlannedMove_.flavor = saveGame.plannedMoveFlavor;
        battleStateLoaded_ = true;
        lastRoundCount_ = currentRound_ - 1;
    }

    return true;
}

bool Battle::playCampaign()
{
    while (hero_ != nullptr && hero_->isAlive() && battleIndex_ < static_cast<int>(bossSequence_.size()))
    {
        if (!battleStateLoaded_)
        {
            loadCurrentBoss();
        }
        if (!playSingleBattle())
        {
            if (playerQuit_)
            {
                return false;
            }
            removeSaveFile(savePath_);
            return false;
        }

        ++battleIndex_;
        if (battleIndex_ < static_cast<int>(bossSequence_.size()))
        {
            rewardScreen(lastRoundCount_);
            const int recovery = std::max(18, hero_->maxHp() / 5);
            hero_->recoverBetweenBattles(recovery);
            std::cout << "\nYou recovered " << recovery << " HP before the next boss.\n";
            saveGameToFile(savePath_, exportSave());
            utils::promptLine("Press Enter to continue to the next boss...");
        }
    }

    removeSaveFile(savePath_);
    return hero_ != nullptr && hero_->isAlive();
}

SaveGameData Battle::exportSave() const
{
    SaveGameData saveGame;
    saveGame.heroId = hero_->profile().id;
    saveGame.difficulty = difficulty_;
    saveGame.currentHp = hero_->currentHp();
    saveGame.battleIndex = battleIndex_;
    saveGame.seed = seed_;
    saveGame.permanentBuffIds = hero_->permanentBuffIds();
    saveGame.bossSequence = bossSequence_;
    saveGame.battleInProgress = boss_ != nullptr && boss_->isAlive() && !playerQuit_;
    if (saveGame.battleInProgress)
    {
        saveGame.currentBossId = boss_->profile().id;
        saveGame.currentBossHp = boss_->currentHp();
        saveGame.currentBossMaxHp = boss_->maxHp();
        saveGame.currentBossRoundsStarted = boss_->roundsStarted();
        saveGame.currentRound = currentRound_;
        saveGame.energy = energy_;
        saveGame.currentBattlePressure = static_cast<int>(currentBattlePressure_);
        saveGame.plannedMoveName = currentPlannedMove_.name;
        saveGame.plannedMoveCategory = static_cast<int>(currentPlannedMove_.category);
        saveGame.plannedMoveDamageType = static_cast<int>(currentPlannedMove_.damageType);
        saveGame.plannedMoveDamage = currentPlannedMove_.damage;
        saveGame.plannedMoveBurn = currentPlannedMove_.burn;
        saveGame.plannedMovePoison = currentPlannedMove_.poison;
        saveGame.plannedMoveVulnerability = currentPlannedMove_.vulnerability;
        saveGame.plannedMoveHeal = currentPlannedMove_.heal;
        saveGame.plannedMoveShield = currentPlannedMove_.shield;
        saveGame.plannedMoveGuardAll = currentPlannedMove_.guardAll;
        saveGame.plannedMoveGuardPhysical = currentPlannedMove_.guardPhysical;
        saveGame.plannedMoveGuardMagic = currentPlannedMove_.guardMagic;
        saveGame.plannedMovePower = currentPlannedMove_.power;
        saveGame.plannedMoveDrawPenalty = currentPlannedMove_.drawPenalty;
        saveGame.plannedMoveFlavor = currentPlannedMove_.flavor;
        saveGame.heroStatuses = serializeStatuses(hero_->statuses());
        saveGame.bossStatuses = serializeStatuses(boss_->statuses());
        saveGame.drawPile = deck_->drawPileIds();
        saveGame.discardPile = deck_->discardPileIds();
        saveGame.hand = deck_->handIds();
    }
    return saveGame;
}

bool Battle::didPlayerQuit() const
{
    return playerQuit_;
}

const HeroTemplate *Battle::findHeroTemplate(const std::string &heroId) const
{
    const auto found = std::find_if(heroes_.begin(), heroes_.end(), [&](const HeroTemplate &hero) {
        return hero.id == heroId;
    });
    return found == heroes_.end() ? nullptr : &(*found);
}

const BossTemplate *Battle::findBossTemplate(const std::string &bossId) const
{
    const auto found = std::find_if(bosses_.begin(), bosses_.end(), [&](const BossTemplate &boss) {
        return boss.id == bossId;
    });
    return found == bosses_.end() ? nullptr : &(*found);
}

void Battle::buildBossSequence()
{
    bossSequence_.clear();
    std::vector<std::string> allIds;
    for (const BossTemplate &boss : bosses_)
    {
        allIds.push_back(boss.id);
    }
    std::shuffle(allIds.begin(), allIds.end(), utils::rng());

    const int fights = std::min(targetBossCount_, static_cast<int>(allIds.size()));
    for (int index = 0; index < fights; ++index)
    {
        bossSequence_.push_back(allIds[index]);
    }
}

void Battle::loadCurrentBoss()
{
    const BossTemplate *bossTemplate = findBossTemplate(bossSequence_[battleIndex_]);
    if (bossTemplate == nullptr)
    {
        throw std::runtime_error("Unknown boss id in sequence.");
    }

    const int minHp = bossTemplate->minHp;
    const int maxHp = bossTemplate->maxHp;
    const int count = maxHp - minHp + 1;
    const int lowerCount = count / 3;
    const int middleCount = count / 3;
    const int upperCount = count - lowerCount - middleCount;
    const int lowerEnd = minHp + lowerCount - 1;
    const int middleStart = lowerEnd + 1;
    const int middleEnd = middleStart + middleCount - 1;
    const int upperStart = maxHp - upperCount + 1;

    int rolledHp = minHp;
    if (difficulty_ == Difficulty::Easy)
    {
        rolledHp = utils::randomInt(minHp, lowerEnd);
    }
    else if (difficulty_ == Difficulty::Normal)
    {
        rolledHp = utils::randomInt(middleStart, middleEnd);
    }
    else
    {
        rolledHp = utils::randomInt(upperStart, maxHp);
    }

    boss_ = std::make_unique<Boss>(*bossTemplate, rolledHp, difficulty_);
    hero_->resetForBattle();
    deck_->resetForBattle();
    battleLog_.clear();
}

bool Battle::playSingleBattle()
{
    int round = battleStateLoaded_ ? currentRound_ : 1;
    bool resumeCurrentRound = battleStateLoaded_;

    while (round <= 12 && hero_->isAlive() && boss_->isAlive())
    {
        if (!resumeCurrentRound)
        {
            hero_->beginRound();
            boss_->beginRound();
            currentRound_ = round;
            currentBattlePressure_ = battlePressureForRound(round);
            if (currentBattlePressure_ != BattlePressure::None)
            {
                pushLog("Hard Mode - Battle Pressure: " + battlePressureName(currentBattlePressure_) + " is active this round.");
            }

            energy_ = hero_->energyCap() + hero_->statuses().nextTurnEnergyBonus;
            hero_->statuses().nextTurnEnergyBonus = 0;
            deck_->discardHand();
            deck_->drawCards(hero_->cardsPerRound(boss_->statuses()));
            hero_->statuses().nextTurnDrawPenalty = 0;
            currentPlannedMove_ = chooseBossMove();
        }

        playerTurn(currentPlannedMove_);
        if (playerQuit_ || !hero_->isAlive() || !boss_->isAlive())
        {
            break;
        }

        if (hero_->isAlive() && boss_->isAlive())
        {
            bossTurn(currentPlannedMove_);
            if (!hero_->isAlive() || !boss_->isAlive())
            {
                break;
            }
        }

        lastRoundCount_ = round;
        endRound();
        resumeCurrentRound = false;
        battleStateLoaded_ = false;
        ++round;
    }

    if (round > 12 && boss_->isAlive())
    {
        pushLog("The 12-round limit expired. The boss wins the battle.");
    }

    lastRoundCount_ = std::min(round, 12);
    if (playerQuit_)
    {
        return false;
    }
    return hero_->isAlive() && boss_->currentHp() <= 0;
}

std::vector<int> Battle::serializeStatuses(const StatusBlock &statuses) const
{
    return {
        statuses.burn,
        statuses.vulnerability,
        statuses.block,
        statuses.power,
        statuses.poison,
        statuses.energySaving,
        statuses.shield,
        statuses.nextTurnEnergyBonus,
        statuses.nextTurnDrawPenalty,
        statuses.tempDamageReductionPercent,
        statuses.tempPhysicalReductionPercent,
        statuses.tempMagicReductionPercent,
        statuses.tempOutgoingBonusPercent,
        statuses.tempIncomingBonusPercent,
        statuses.opportunismReady ? 1 : 0,
        statuses.thornArmorReady ? 1 : 0};
}

StatusBlock Battle::deserializeStatuses(const std::vector<int> &values) const
{
    StatusBlock statuses;
    if (values.size() >= 16)
    {
        statuses.burn = values[0];
        statuses.vulnerability = values[1];
        statuses.block = values[2];
        statuses.power = values[3];
        statuses.poison = values[4];
        statuses.energySaving = values[5];
        statuses.shield = values[6];
        statuses.nextTurnEnergyBonus = values[7];
        statuses.nextTurnDrawPenalty = values[8];
        statuses.tempDamageReductionPercent = values[9];
        statuses.tempPhysicalReductionPercent = values[10];
        statuses.tempMagicReductionPercent = values[11];
        statuses.tempOutgoingBonusPercent = values[12];
        statuses.tempIncomingBonusPercent = values[13];
        statuses.opportunismReady = values[14] != 0;
        statuses.thornArmorReady = values[15] != 0;
    }
    return statuses;
}

std::vector<CardDefinition> Battle::restoreCardsById(const std::vector<std::string> &cardIds) const
{
    std::vector<CardDefinition> cards;
    for (const std::string &cardId : cardIds)
    {
        const auto found = std::find_if(cards_.begin(), cards_.end(), [&](const CardDefinition &card) {
            return card.id == cardId;
        });
        if (found != cards_.end())
        {
            cards.push_back(*found);
        }
    }
    return cards;
}

void Battle::playerTurn(const BossMove &plannedMove)
{
    while (hero_->isAlive() && boss_->isAlive())
    {
        renderBattleScreen(lastRoundCount_ + 1);
        const int choice = utils::promptChoice("> ", 1, 3);

        if (choice == 1)
        {
            if (deck_->handEmpty())
            {
                pushLog("Your hand is empty.");
                continue;
            }

            const int cardChoice = utils::promptChoice("Choose a card number: ", 1, static_cast<int>(deck_->handSize()));
            CardDefinition card;
            const CardDefinition preview = deck_->hand()[static_cast<std::size_t>(cardChoice - 1)];
            const int cost = effectiveCardCost(preview, *hero_);

            if (cost > energy_)
            {
                pushLog("Not enough Energy for " + preview.name + ".");
                continue;
            }

            if (!deck_->playCard(static_cast<std::size_t>(cardChoice - 1), card))
            {
                pushLog("That card is no longer in your hand.");
                continue;
            }

            energy_ -= cost;
            lastHeroCategory_ = card.category;

            CardContext context;
            context.heroValueScale = heroValueScale();
            context.bossPlannedCategory = plannedMove.category;
            context.deck = deck_.get();

            const CardPlayResult result = resolveCardEffect(card, *hero_, *boss_, context);
            for (const std::string &line : result.logLines)
            {
                pushLog(line);
            }
        }
        else if (choice == 2)
        {
            pushLog("You ended the round.");
            break;
        }
        else
        {
            saveGameToFile(savePath_, exportSave());
            playerQuit_ = true;
            pushLog("Game saved. Quitting to desktop.");
            break;
        }
    }
}

void Battle::bossTurn(const BossMove &move)
{
    if (!hero_->isAlive() || !boss_->isAlive())
    {
        return;
    }

    pushLog("Boss used [" + move.name + "]!");

    boss_->statuses().power += move.power;
    boss_->statuses().shield += move.shield;
    boss_->statuses().tempDamageReductionPercent = std::max(boss_->statuses().tempDamageReductionPercent, static_cast<int>(move.guardAll * bossValueScale()));
    boss_->statuses().tempPhysicalReductionPercent = std::max(boss_->statuses().tempPhysicalReductionPercent, static_cast<int>(move.guardPhysical * bossValueScale()));
    boss_->statuses().tempMagicReductionPercent = std::max(boss_->statuses().tempMagicReductionPercent, static_cast<int>(move.guardMagic * bossValueScale()));
    if (move.heal > 0)
    {
        boss_->heal(static_cast<int>(move.heal * bossValueScale()));
    }

    int pressureDamageBonus = 0;
    if (currentBattlePressure_ == BattlePressure::Ferocity)
    {
        pressureDamageBonus = 18;
    }

    if (move.damage > 0)
    {
        const int outgoing = computeDamage(static_cast<int>((move.damage + pressureDamageBonus) * bossValueScale()), move.damageType, boss_->statuses(), hero_->statuses(), boss_->damageBonusPercent());
        const int finalDamage = mitigateDamage(outgoing, move.damageType, hero_->statuses());
        hero_->takeDamage(finalDamage);
        pushLog("You suffered " + std::to_string(finalDamage) + " damage.");

        if (hero_->statuses().thornArmorReady && move.category == CardCategory::Attack)
        {
            const int thornDamage = finalDamage * 2;
            boss_->takeDamage(thornDamage);
            pushLog("Thorn Armor reflected " + std::to_string(thornDamage) + " damage.");
        }

        const int passiveReflect = hero_->passiveReflectPercent();
        if (passiveReflect > 0 && finalDamage > 0)
        {
            const int reflected = std::max(1, finalDamage * passiveReflect / 100);
            boss_->takeDamage(reflected);
            pushLog("Bulwark reflection dealt " + std::to_string(reflected) + " damage back.");
        }
    }

    if (move.burn > 0)
    {
        hero_->statuses().burn += static_cast<int>(move.burn * bossValueScale());
        pushLog("You gained Burn.");
    }
    if (move.poison > 0)
    {
        hero_->statuses().poison += static_cast<int>(move.poison * bossValueScale());
        pushLog("You were poisoned.");
    }
    if (move.vulnerability > 0)
    {
        hero_->statuses().vulnerability += static_cast<int>(move.vulnerability * bossValueScale());
        pushLog("You became Vulnerable.");
    }
    if (currentBattlePressure_ == BattlePressure::GuardBreak && move.damage > 0)
    {
        hero_->statuses().vulnerability += 2;
        pushLog("Battle Pressure applied 2 extra Vulnerability.");
    }
    if (currentBattlePressure_ == BattlePressure::ScorchPulse)
    {
        hero_->statuses().burn += 2;
        pushLog("Battle Pressure scorched you for 2 Burn.");
    }
    if (currentBattlePressure_ == BattlePressure::ToxicPulse)
    {
        hero_->statuses().poison += 2;
        pushLog("Battle Pressure poisoned you for 2 Poison.");
    }
    if (move.drawPenalty > 0)
    {
        hero_->statuses().nextTurnDrawPenalty += move.drawPenalty;
        pushLog("Your next draw was reduced.");
    }

    if (hero_->statuses().opportunismReady && move.category == CardCategory::Status)
    {
        boss_->takeDamage(70);
        hero_->noteDamageDealt(70);
        pushLog("Opportunism punished the boss for using a status move.");
    }
}

BossMove Battle::chooseBossMove() const
{
    BossMove move;
    const BossArchetype archetype = boss_->profile().archetype;
    const auto buildMove = [](const std::string &name,
                              CardCategory category,
                              DamageType damageType,
                              int damage,
                              int burn,
                              int poison,
                              int vulnerability,
                              int heal,
                              int shield,
                              int guardAll,
                              int guardPhysical,
                              int guardMagic,
                              int power,
                              int drawPenalty,
                              const std::string &flavor) {
        BossMove built;
        built.name = name;
        built.category = category;
        built.damageType = damageType;
        built.damage = damage;
        built.burn = burn;
        built.poison = poison;
        built.vulnerability = vulnerability;
        built.heal = heal;
        built.shield = shield;
        built.guardAll = guardAll;
        built.guardPhysical = guardPhysical;
        built.guardMagic = guardMagic;
        built.power = power;
        built.drawPenalty = drawPenalty;
        built.flavor = flavor;
        return built;
    };

    if (archetype == BossArchetype::Offensive)
    {
        const int roll = utils::randomInt(1, 100);
        if (boss_->belowThirtyPercent() || roll > 75)
        {
            move = buildMove("Meteor Crash", CardCategory::Attack, DamageType::Physical, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "A heavy finisher.");
        }
        else if (roll > 50)
        {
            move = buildMove("Rending Claw", CardCategory::Attack, DamageType::Physical, 62, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "Fast physical pressure.");
        }
        else
        {
            move = buildMove("Blood Howl", CardCategory::Status, DamageType::Pure, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, "Builds rage before striking.");
        }
    }
    else if (archetype == BossArchetype::Defensive)
    {
        if (boss_->roundsStarted() % 3 == 0)
        {
            move = buildMove("Sanctuary Pulse", CardCategory::Defense, DamageType::Pure, 0, 0, 0, 0, 42, 18, 55, 0, 0, 0, 0, "A steady recovery pulse.");
        }
        else
        {
            const int roll = utils::randomInt(1, 100);
            if (roll > 55)
            {
                move = buildMove("Stone Bastion", CardCategory::Defense, DamageType::Pure, 0, 0, 0, 0, 0, 22, 65, 20, 20, 0, 0, "An ironclad guard stance.");
            }
            else
            {
                move = buildMove("Punishing Slam", CardCategory::Attack, DamageType::Physical, 48, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, "A counter-heavy slam.");
            }
        }
    }
    else if (archetype == BossArchetype::Magic)
    {
        const bool heroDebuffed = countDebuffs(hero_->statuses()) >= 2;
        if (heroDebuffed && utils::chance(0.35))
        {
            move = buildMove("Moonwell Recovery", CardCategory::Defense, DamageType::Pure, 0, 0, 0, 0, 36, 0, 45, 0, 25, 0, 0, "Recovers while the curse sticks.");
        }
        else
        {
            const int roll = utils::randomInt(1, 100);
            if (roll > 66)
            {
                move = buildMove("Ashen Breath", CardCategory::Attack, DamageType::Magic, 38, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Fire and pressure.");
            }
            else if (roll > 33)
            {
                move = buildMove("Venom Chant", CardCategory::Status, DamageType::Pure, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, "Poison magic.");
            }
            else
            {
                move = buildMove("Hex of Frailty", CardCategory::Status, DamageType::Pure, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, "Weakens physical defense.");
            }
        }
    }
    else
    {
        if (boss_->belowFortyPercent() && utils::chance(0.4))
        {
            move = buildMove("Mind Fog", CardCategory::Status, DamageType::Pure, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, "A disruptive adaptive curse.");
        }
        else if (lastHeroCategory_ == CardCategory::Attack)
        {
            move = buildMove("All-knowing Ward", CardCategory::Defense, DamageType::Pure, 0, 0, 0, 0, 0, 16, 50, 20, 20, 0, 0, "Answers aggression with defense.");
        }
        else if (lastHeroCategory_ == CardCategory::Defense)
        {
            move = buildMove("Adaptive Hex", CardCategory::Status, DamageType::Pure, 0, 2, 2, 1, 0, 0, 0, 0, 0, 0, boss_->belowFortyPercent() ? 1 : 0, "Punishes passive turns.");
        }
        else
        {
            move = buildMove("Adaptive Strike", CardCategory::Attack, DamageType::Physical, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "A measured, efficient strike.");
        }
    }

    return move;
}

BattlePressure Battle::battlePressureForRound(int roundNumber) const
{
    if (difficulty_ != Difficulty::Hard || roundNumber % 3 != 0)
    {
        return BattlePressure::None;
    }

    const int roll = utils::randomInt(0, 3);
    switch (roll)
    {
    case 0:
        return BattlePressure::Ferocity;
    case 1:
        return BattlePressure::GuardBreak;
    case 2:
        return BattlePressure::ScorchPulse;
    default:
        return BattlePressure::ToxicPulse;
    }
}

std::string Battle::battlePressureName(BattlePressure pressure) const
{
    switch (pressure)
    {
    case BattlePressure::Ferocity:
        return "Ferocity (+18 move damage)";
    case BattlePressure::GuardBreak:
        return "Guard Break (+2 Vulnerability on hit)";
    case BattlePressure::ScorchPulse:
        return "Scorch Pulse (+2 Burn after acting)";
    case BattlePressure::ToxicPulse:
        return "Toxic Pulse (+2 Poison after acting)";
    case BattlePressure::None:
    default:
        return "None";
    }
}

void Battle::endRound()
{
    const int bossPoison = poisonTickDamage(boss_->statuses());
    if (bossPoison > 0)
    {
        boss_->takeDamage(bossPoison);
        hero_->noteDamageDealt(bossPoison);
        pushLog(boss_->profile().name + " took " + std::to_string(bossPoison) + " poison damage.");
    }

    const int heroPoison = poisonTickDamage(hero_->statuses());
    if (heroPoison > 0)
    {
        hero_->takeDamage(heroPoison);
        pushLog("You took " + std::to_string(heroPoison) + " poison damage.");
    }

    hero_->endRound(battleLog_);

    if (hero_->statuses().tempIncomingBonusPercent > 0)
    {
        pushLog("Your self-risk effect has faded.");
    }

    resetTurnBonuses(hero_->statuses());
    resetTurnBonuses(boss_->statuses());
}

void Battle::rewardScreen(int roundsUsed)
{
    utils::clearScreen();
    const int choiceCount = rewardChoiceCount(roundsUsed);
    const std::vector<PermanentBuff> rewards = rollRewards(choiceCount);

    std::cout << kPaleAccent << "*** Boss Defeated! ***" << kReset << "\n";
    std::cout << "You defeated " << boss_->profile().name << " in " << roundsUsed << " rounds and earned " << choiceCount << " permanent buff choices.\n\n";
    std::cout << "Choose one permanent buff:\n";
    for (std::size_t index = 0; index < rewards.size(); ++index)
    {
        std::cout << (index + 1) << ". " << rewards[index].name << " (" << rewards[index].description << ")\n";
    }
    const int choice = utils::promptChoice("> ", 1, static_cast<int>(rewards.size()));
    hero_->addPermanentBuff(rewards[static_cast<std::size_t>(choice - 1)]);
    std::cout << "You obtained [" << rewards[static_cast<std::size_t>(choice - 1)].name << "]!\n";
}

std::vector<PermanentBuff> Battle::rollRewards(int choiceCount) const
{
    std::vector<PermanentBuff> pool = permanentBuffs_;
    std::shuffle(pool.begin(), pool.end(), utils::rng());

    std::vector<PermanentBuff> rewards;
    for (const PermanentBuff &buff : pool)
    {
        const auto owned = hero_->permanentBuffIds();
        if (std::find(owned.begin(), owned.end(), buff.id) == owned.end())
        {
            rewards.push_back(buff);
        }
        if (static_cast<int>(rewards.size()) == choiceCount)
        {
            break;
        }
    }

    if (rewards.empty())
    {
        rewards.push_back(permanentBuffs_.front());
    }
    return rewards;
}

int Battle::rewardChoiceCount(int roundsUsed) const
{
    if (roundsUsed <= 5)
    {
        return 5;
    }
    if (roundsUsed <= 7)
    {
        return 4;
    }
    if (roundsUsed <= 10)
    {
        return 3;
    }
    return 2;
}

double Battle::heroValueScale() const
{
    double scale = matchupMultiplier(hero_->profile().heroClass, boss_->profile().archetype);
    if (difficulty_ == Difficulty::Easy)
    {
        scale *= 1.12;
    }
    else if (difficulty_ == Difficulty::Hard)
    {
        scale *= 0.92;
    }
    return scale;
}

double Battle::bossValueScale() const
{
    if (difficulty_ == Difficulty::Easy)
    {
        return 0.88;
    }
    if (difficulty_ == Difficulty::Hard)
    {
        return 1.15;
    }
    return 1.0;
}

void Battle::renderBattleScreen(int roundNumber) const
{
    utils::clearScreen();

    std::cout << kWarmAccent << utils::repeat("=", 60) << kReset << "\n";
    std::cout << titleLine("[Battle - " + difficultyName(difficulty_) + "] Round " + std::to_string(roundNumber)) << "\n";
    std::cout << kWarmAccent << utils::repeat("=", 60) << kReset << "\n\n";

    const std::string heroTitle = utils::padVisible(kPaleAccent + std::string("[Player Hero] ") + hero_->profile().name + kReset, 34);
    const std::string bossTitle = kPaleAccent + std::string("[Boss] ") + boss_->profile().name + kReset;
    std::cout << heroTitle << "  " << bossTitle << "\n";

    const std::string heroHp = "HP:    " + utils::bar(hero_->currentHp(), hero_->maxHp(), 15, kOffWhite, kSoftBrown) + "  " + std::to_string(hero_->currentHp()) + "/" + std::to_string(hero_->maxHp());
    const std::string bossHp = "HP:    " + utils::bar(boss_->currentHp(), boss_->maxHp(), 15, kOffWhite, kSoftBrown) + "  " + std::to_string(boss_->currentHp()) + "/" + std::to_string(boss_->maxHp());
    std::cout << utils::padVisible(heroHp, 34) << "  " << bossHp << "\n";

    const std::string heroEnergy = "Energy:" + utils::bar(energy_, std::max(energy_, hero_->energyCap()), 15, kWarmAccent, kSoftBrown) + "  " + std::to_string(energy_) + "/" + std::to_string(hero_->energyCap());
    std::cout << utils::padVisible(heroEnergy, 34) << "  " << "Turn Order: Player First" << "\n";

    std::cout << utils::padVisible("Battle Type: Card Combat", 34) << "  " << "Archetype: " << bossArchetypeName(boss_->profile().archetype) << "\n";
    std::cout << utils::padVisible("Status: " + formatStatuses(hero_->statuses()), 34) << "  " << "Status: " << formatStatuses(boss_->statuses()) << "\n\n";

    std::cout << kPaleAccent << utils::repeat("-", 60) << kReset << "\n";
    std::cout << "Your hand (" << deck_->handSize() << " cards / Energy cap " << hero_->energyCap() << "):\n";
    if (deck_->handEmpty())
    {
        std::cout << "No cards in hand.\n";
    }
    else
    {
        for (std::size_t index = 0; index < deck_->hand().size(); ++index)
        {
            const CardDefinition &card = deck_->hand()[index];
            std::cout << "[" << (index + 1) << "] "
                      << utils::padVisible(card.name, 18)
                      << std::max(0, effectiveCardCost(card, *hero_)) << " EN   "
                      << card.description << "\n";
        }
    }

    std::cout << "\n" << kPaleAccent << utils::repeat("-", 60) << kReset << "\n";
    for (const std::string &line : battleLog_)
    {
        std::cout << line << "\n";
    }
    std::cout << "\nChoose an action:\n";
    std::cout << "1. Play a card (choose a card number)\n";
    std::cout << "2. End this round\n";
    std::cout << "3. Save and quit\n";
}

void Battle::pushLog(const std::string &message)
{
    battleLog_.push_back(message);
    if (battleLog_.size() > 6)
    {
        battleLog_.erase(battleLog_.begin());
    }
}

std::string Battle::formatStatuses(const StatusBlock &statuses) const
{
    const std::vector<std::string> tags = activeStatusTags(statuses);
    if (tags.empty())
    {
        return "None";
    }
    return utils::join(tags, " ");
}
