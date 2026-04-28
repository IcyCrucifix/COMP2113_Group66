#pragma once

#include "deck.h"
#include "save_load.h"

#include <memory>
#include <string>
#include <vector>

struct BossMove
{
    std::string name;
    CardCategory category = CardCategory::Attack;
    DamageType damageType = DamageType::Physical;
    int damage = 0;
    int burn = 0;
    int poison = 0;
    int vulnerability = 0;
    int heal = 0;
    int shield = 0;
    int guardAll = 0;
    int guardPhysical = 0;
    int guardMagic = 0;
    int power = 0;
    int drawPenalty = 0;
    std::string flavor;
};

enum class BattlePressure
{
    None,
    Ferocity,
    GuardBreak,
    ScorchPulse,
    ToxicPulse
};

class Battle
{
public:
    /* What it does: Constructs the battle controller with all loaded runtime data.
     * Inputs: cards - loaded card data; heroes - loaded hero templates; bosses - loaded boss templates; savePath - path for campaign save files.
     * Outputs: Ready battle controller.
     */
    Battle(std::vector<CardDefinition> cards, std::vector<HeroTemplate> heroes, std::vector<BossTemplate> bosses, std::string savePath);

    /* What it does: Starts a new campaign from player selections.
     * Inputs: difficulty - chosen difficulty; heroId - selected hero template id; seed - random seed for this run; targetBossCount - how many boss fights this run should contain.
     * Outputs: None.
     */
    void setupNewRun(Difficulty difficulty, const std::string &heroId, unsigned int seed, int targetBossCount);

    /* What it does: Restores a campaign from a save file snapshot.
     * Inputs: saveGame - loaded save snapshot.
     * Outputs: True if the save could be restored; otherwise false.
     */
    bool setupFromSave(const SaveGameData &saveGame);

    /* What it does: Plays the full campaign until victory or defeat.
     * Inputs: None.
     * Outputs: True if all bosses were defeated; otherwise false.
     */
    bool playCampaign();

    /* What it does: Exports the current campaign state for saving.
     * Inputs: None.
     * Outputs: SaveGameData snapshot.
     */
    SaveGameData exportSave() const;

    /* What it does: Returns whether the player quit the run from the in-battle menu.
     * Inputs: None.
     * Outputs: True if the player chose quit; otherwise false.
     */
    bool didPlayerQuit() const;

private:
    /* What it does: Finds a hero template by id.
     * Inputs: heroId - template identifier.
     * Outputs: Pointer to the hero template or nullptr if not found.
     */
    const HeroTemplate *findHeroTemplate(const std::string &heroId) const;

    /* What it does: Finds a boss template by id.
     * Inputs: bossId - template identifier.
     * Outputs: Pointer to the boss template or nullptr if not found.
     */
    const BossTemplate *findBossTemplate(const std::string &bossId) const;

    /* What it does: Builds a unique sequence of bosses for the campaign.
     * Inputs: None.
     * Outputs: None.
     */
    void buildBossSequence();

    /* What it does: Creates the boss object for the current campaign step.
     * Inputs: None.
     * Outputs: None.
     */
    void loadCurrentBoss();

    /* What it does: Plays one boss battle.
     * Inputs: None.
     * Outputs: True if the boss was defeated; otherwise false.
     */
    bool playSingleBattle();

    /* What it does: Runs the player's interactive turn loop.
     * Inputs: plannedMove - boss move prepared for this round.
     * Outputs: None.
     */
    void playerTurn(const BossMove &plannedMove);

    /* What it does: Executes the boss turn and appends the resulting log lines.
     * Inputs: move - planned boss move.
     * Outputs: None.
     */
    void bossTurn(const BossMove &move);

    /* What it does: Chooses the boss's planned move for the current round.
     * Inputs: None.
     * Outputs: BossMove definition.
     */
    BossMove chooseBossMove() const;

    /* What it does: Resolves the Hard-only Battle Pressure modifier for the given round.
     * Inputs: roundNumber - current round number.
     * Outputs: One-round difficulty modifier to apply this round.
     */
    BattlePressure battlePressureForRound(int roundNumber) const;

    /* What it does: Returns the user-facing name of a Battle Pressure modifier.
     * Inputs: pressure - modifier to describe.
     * Outputs: Printable label.
     */
    std::string battlePressureName(BattlePressure pressure) const;

    /* What it does: Converts the current save snapshot into battle-only status values.
     * Inputs: statuses - status block to serialize.
     * Outputs: Integer field vector for save storage.
     */
    std::vector<int> serializeStatuses(const StatusBlock &statuses) const;

    /* What it does: Restores a status block from saved integer values.
     * Inputs: values - serialized status vector.
     * Outputs: Reconstructed status block.
     */
    StatusBlock deserializeStatuses(const std::vector<int> &values) const;

    /* What it does: Looks up cards by id and preserves their order.
     * Inputs: cardIds - ordered card ids to restore.
     * Outputs: Ordered card definitions.
     */
    std::vector<CardDefinition> restoreCardsById(const std::vector<std::string> &cardIds) const;

    /* What it does: Applies end-of-round poison damage and cleanup.
     * Inputs: None.
     * Outputs: None.
     */
    void endRound();

    /* What it does: Grants a random permanent reward after a boss victory.
     * Inputs: roundsUsed - total rounds spent in the battle.
     * Outputs: None.
     */
    void rewardScreen(int roundsUsed);

    /* What it does: Rolls the reward choices shown to the player.
     * Inputs: choiceCount - how many reward cards to show.
     * Outputs: Vector of sampled permanent buffs.
     */
    std::vector<PermanentBuff> rollRewards(int choiceCount) const;

    /* What it does: Calculates how many reward choices the player earns from performance.
     * Inputs: roundsUsed - total rounds spent in the battle.
     * Outputs: Number of reward choices.
     */
    int rewardChoiceCount(int roundsUsed) const;

    /* What it does: Calculates the combined scaling factor for hero values this fight.
     * Inputs: None.
     * Outputs: Multiplicative hero scale.
     */
    double heroValueScale() const;

    /* What it does: Calculates the combined scaling factor for boss values this fight.
     * Inputs: None.
     * Outputs: Multiplicative boss scale.
     */
    double bossValueScale() const;

    /* What it does: Renders the whole battle screen with the current state and log.
     * Inputs: roundNumber - current round number.
     * Outputs: None.
     */
    void renderBattleScreen(int roundNumber) const;

    /* What it does: Appends a battle log line while keeping the log short.
     * Inputs: message - line to append.
     * Outputs: None.
     */
    void pushLog(const std::string &message);

    /* What it does: Formats a list of active statuses for one side.
     * Inputs: statuses - status bundle to render.
     * Outputs: Single printable line.
     */
    std::string formatStatuses(const StatusBlock &statuses) const;

    std::vector<CardDefinition> cards_;
    std::vector<HeroTemplate> heroes_;
    std::vector<BossTemplate> bosses_;
    std::vector<PermanentBuff> permanentBuffs_;
    std::unique_ptr<Hero> hero_;
    std::unique_ptr<Deck> deck_;
    std::unique_ptr<Boss> boss_;
    Difficulty difficulty_ = Difficulty::Normal;
    unsigned int seed_ = 0;
    int battleIndex_ = 0;
    int energy_ = 0;
    int actionPoints_ = 0;
    int lastRoundCount_ = 0;
    int currentRound_ = 0;
    int targetBossCount_ = 3;
    bool playerQuit_ = false;
    CardCategory lastHeroCategory_ = CardCategory::Other;
    BattlePressure currentBattlePressure_ = BattlePressure::None;
    BossMove currentPlannedMove_;
    bool battleStateLoaded_ = false;
    std::vector<std::string> bossSequence_;
    std::vector<std::string> battleLog_;
    std::string savePath_;
};
