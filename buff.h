#pragma once

#include <string>
#include <vector>

enum class Difficulty
{
    Easy = 1,
    Normal = 2,
    Hard = 3
};

enum class DamageType
{
    Physical,
    Magic,
    Pure
};

enum class CardCategory
{
    Attack,
    Defense,
    Status,
    Other
};

enum class HeroClass
{
    Offensive,
    Defensive,
    Magic
};

enum class BossArchetype
{
    Offensive,
    Defensive,
    Magic,
    Hybrid
};

enum class BasicAction
{
    Attack = 1,
    Defend = 2,
    Status = 3
};

struct StatusBlock
{
    int burn = 0;
    int vulnerability = 0;
    int block = 0;
    int power = 0;
    int poison = 0;
    int energySaving = 0;
    int shield = 0;
    int nextTurnEnergyBonus = 0;
    int nextTurnDrawPenalty = 0;
    int tempDamageReductionPercent = 0;
    int tempPhysicalReductionPercent = 0;
    int tempMagicReductionPercent = 0;
    int tempOutgoingBonusPercent = 0;
    int tempIncomingBonusPercent = 0;
    bool opportunismReady = false;
    bool thornArmorReady = false;
};

struct PermanentBuff
{
    std::string id;
    std::string name;
    std::string description;
    int damagePercent = 0;
    int maxHpBonus = 0;
    int energyCapBonus = 0;
    int drawBonus = 0;
    int burnBonus = 0;
    int poisonBonus = 0;
    int healingBonusPercent = 0;
    int shieldPerTurnBonus = 0;
    int startingBlock = 0;
};

/* What it does: Converts a difficulty enum to a user-facing name.
 * Inputs: difficulty - difficulty enum value.
 * Outputs: English label for the difficulty.
 */
std::string difficultyName(Difficulty difficulty);

/* What it does: Converts a damage type enum to a user-facing name.
 * Inputs: damageType - damage type enum value.
 * Outputs: English label for the damage type.
 */
std::string damageTypeName(DamageType damageType);

/* What it does: Converts a card category enum to a user-facing name.
 * Inputs: category - card category enum value.
 * Outputs: English label for the category.
 */
std::string cardCategoryName(CardCategory category);

/* What it does: Converts a hero class enum to a user-facing name.
 * Inputs: heroClass - hero class enum value.
 * Outputs: English label for the hero class.
 */
std::string heroClassName(HeroClass heroClass);

/* What it does: Converts a boss archetype enum to a user-facing name.
 * Inputs: archetype - boss archetype enum value.
 * Outputs: English label for the archetype.
 */
std::string bossArchetypeName(BossArchetype archetype);

/* What it does: Converts a basic action enum to a user-facing name.
 * Inputs: action - action enum value.
 * Outputs: English label for the action.
 */
std::string basicActionName(BasicAction action);

/* What it does: Lists all active statuses in printable tag form.
 * Inputs: statuses - status bundle to inspect.
 * Outputs: Vector of formatted status strings such as "[Burn x2]".
 */
std::vector<std::string> activeStatusTags(const StatusBlock &statuses);

/* What it does: Counts how many debuff stacks/categories are active.
 * Inputs: statuses - status bundle to inspect.
 * Outputs: Number of active debuff categories.
 */
int countDebuffs(const StatusBlock &statuses);

/* What it does: Returns the hero-vs-boss matchup multiplier from the design sheet.
 * Inputs: heroClass - chosen hero class; archetype - boss archetype.
 * Outputs: Coefficient applied to hero numeric values for the matchup.
 */
double matchupMultiplier(HeroClass heroClass, BossArchetype archetype);

/* What it does: Calculates outgoing damage after offensive and vulnerability/burn modifiers.
 * Inputs: baseDamage - base card or move damage; damageType - physical/magic/pure; attacker - attacker statuses; defender - defender statuses; bonusPercent - extra percent from passives/difficulty.
 * Outputs: Rounded damage before mitigation.
 */
int computeDamage(int baseDamage, DamageType damageType, const StatusBlock &attacker, const StatusBlock &defender, int bonusPercent);

/* What it does: Applies mitigation, shield absorption, and block reduction to incoming damage.
 * Inputs: incomingDamage - damage before mitigation; damageType - physical/magic/pure; defender - defender statuses (modified because shield is consumed).
 * Outputs: Final HP damage that should be deducted.
 */
int mitigateDamage(int incomingDamage, DamageType damageType, StatusBlock &defender);

/* What it does: Calculates poison tick damage for the end of the round.
 * Inputs: statuses - target statuses.
 * Outputs: Total poison damage before mitigation.
 */
int poisonTickDamage(const StatusBlock &statuses);

/* What it does: Clears one-round-only status flags and bonuses.
 * Inputs: statuses - status bundle to modify.
 * Outputs: None.
 */
void resetTurnBonuses(StatusBlock &statuses);

/* What it does: Clears all combat statuses for a new battle.
 * Inputs: statuses - status bundle to modify.
 * Outputs: None.
 */
void clearBattleStatuses(StatusBlock &statuses);

/* What it does: Builds the catalog of permanent rewards shown after boss fights.
 * Inputs: None.
 * Outputs: Vector of permanent buff definitions.
 */
std::vector<PermanentBuff> buildPermanentBuffCatalog();

/* What it does: Finds a permanent buff by id inside a catalog.
 * Inputs: buffs - buff catalog; id - buff identifier.
 * Outputs: Pointer to the matching buff or nullptr if not found.
 */
const PermanentBuff *findPermanentBuffById(const std::vector<PermanentBuff> &buffs, const std::string &id);
