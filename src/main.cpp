#include "battle.h"
#include "save_load.h"
#include "utils.h"

#include <ctime>
#include <iostream>
#include <stdexcept>

/* What it does: Starts the program, loads game data, collects the player's setup choices, and runs a new or loaded campaign.
 * Inputs: None.
 * Outputs: Returns 0 when the program finishes normally, or 1 if a fatal error is caught.
 */
int main()
{
    try
    {
        const std::string cardsPath = utils::resolveFirstExistingPath({"./src/cards/cards.txt", "./cards.txt"});
        const std::string heroesPath = utils::resolveFirstExistingPath({"./src/hero/heroes.txt", "./heroes.txt"});
        const std::string bossesPath = utils::resolveFirstExistingPath({"./src/bosses/bosses.txt", "./bosses.txt"});
        const std::string savePath = "./savegame.txt";

        const std::vector<CardDefinition> cards = loadCardDefinitions(cardsPath);
        const std::vector<HeroTemplate> heroes = loadHeroTemplates(heroesPath);
        const std::vector<BossTemplate> bosses = loadBossTemplates(bossesPath);
        Battle battle(cards, heroes, bosses, savePath);

        utils::clearScreen();
        std::cout << "Welcome to Text Slay the Spire!\n";
        std::cout << "1. New Run\n";
        std::cout << "2. Load Save\n";
        const int menuChoice = utils::promptChoice("> ", 1, 2);

        if (menuChoice == 2)
        {
            SaveGameData saveGame;
            if (!loadGameFromFile(savePath, saveGame) || !battle.setupFromSave(saveGame))
            {
                std::cout << "No valid save file was found. Starting a new run instead.\n";
            }
            else
            {
                const bool victory = battle.playCampaign();
                if (battle.didPlayerQuit())
                {
                    std::cout << "\nGame saved. See you next time.\n";
                }
                else
                {
                    std::cout << (victory ? "\nYou cleared the campaign!\n" : "\nYou were defeated.\n");
                }
                return 0;
            }
        }

        utils::clearScreen();
        std::cout << "Choose difficulty:\n";
        std::cout << "1. Easy (Boss HP x0.75, Hero card values x1.12, Boss move values x0.88)\n";
        std::cout << "2. Normal (Boss HP x1.00, Hero card values x1.00, Boss move values x1.00)\n";
        std::cout << "3. Hard (Boss HP x1.20, Hero card values x0.92, Boss move values x1.15, Battle Pressure on rounds 3/6/9/12)\n";
        const Difficulty difficulty = static_cast<Difficulty>(utils::promptChoice("> ", 1, 3));

        std::cout << "\nChoose run length:\n";
        std::cout << "1. Single Boss\n";
        std::cout << "2. Multi-Boss Run (up to 3 bosses)\n";
        const int runLengthChoice = utils::promptChoice("> ", 1, 2);
        const int targetBossCount = runLengthChoice == 1 ? 1 : 3;

        std::cout << "\nChoose your hero:\n";
        for (std::size_t index = 0; index < heroes.size(); ++index)
        {
            std::cout << (index + 1) << ". " << heroes[index].name << " [" << heroClassName(heroes[index].heroClass) << "]"
                      << " HP " << heroes[index].maxHp
                      << " | Energy " << heroes[index].energyCap << "\n";
            std::cout << "   Passive: " << heroes[index].passiveText << "\n";
            std::cout << "   Playstyle: " << heroes[index].playstyle << "\n";
        }

        const int heroChoice = utils::promptChoice("> ", 1, static_cast<int>(heroes.size()));
        const unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
        battle.setupNewRun(difficulty, heroes[static_cast<std::size_t>(heroChoice - 1)].id, seed, targetBossCount);

        const bool victory = battle.playCampaign();
        if (battle.didPlayerQuit())
        {
            std::cout << "\nGame saved. See you next time.\n";
        }
        else
        {
            std::cout << (victory ? "\nYou cleared the run!\n" : "\nYou were defeated.\n");
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << "Fatal error: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
