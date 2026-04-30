# Slay Text: Text-based Roguelike Card Game

> **COMP2113 Group 66 Project** • The University of Hong Kong <br>
> A terminal turn-based deck-building card game inspired by *Slay the Spire*, built purely with standard C++17 STL.

## Game Description
**Slay Text** is a single-player, roguelike deck-building card game that runs entirely in the terminal. Players select one of three unique heroes, each with distinct passives and combat styles, and battle through a randomized sequence of bosses. Using energy as a core resource, players cast attack, defense, status, and utility cards to defeat enemies while managing buffs, debuffs, and a cycling deck system. 

Players are able to choose whether they play single or multiple rounds. After each boss victory, players earn permanent buffs to strengthen their subsequent encounters. The game includes three difficulty levels and a complete **Save/Load system** that preserves hero health, upgrades, and campaign progress. Victory requires defeating all bosses in sequence; defeat occurs if hero health drops to 0.

---

## How to run

You should have a C++ compiler (`clang++` or `g++`) installed on your device. The program is built using standard C++17 libraries (STL). No external installations or non-standard libraries are required for Linux/macOS/WSL.

1. **Clone the repository**

   ```bash
   git clone https://github.com/IcyCrucifix/COMP2113_Group66.git
   cd COMP2113_Group66
   ```

2. **Compile the project**
  
   ```bash
   make all
   ```

3. **Start the program**

   ```bash
   ./slay_text
   ```

4. **Clean build files** (optional)

   ```bash
   make clean
   ```
------

## Gameplay Basics

### Core Game Loop

The game follows an escalating challenge loop. You can save or load your progress at any time.

```mermaid
%%{init: {'theme': 'default'}}%%
graph LR
    A[Choose Difficulty] --> B[Select Hero]
    B --> C((Battle Random Boss))
    C --> D[Turn-Based Battle]
    
    D -->|Round continues| C
    D -->|Hero HP ≤ 0| E[Game Over]
    D -->|Round Count > 12| E
    D -->|Boss HP ≤ 0| F[Get Buffs]
    
    F -->|Next Boss| C
    
    classDef default fill:#ffffff,stroke:#333333,color:#000000,stroke-width:2px;
    classDef boss fill:#fff0f0,stroke:#d32f2f,color:#000000,stroke-width:2px;
    classDef death fill:#ffebee,stroke:#b71c1c,color:#b71c1c,stroke-width:2px;
    
    class C boss;
    class E death;
```
### Game Rules

#### Round Flow

Each round against a boss strictly follows these 5 phases:

1. **Reset Energy** —— *Energy is fully refilled at the start*
2. **Draw Cards**
3. **Play Cards**
4. **Boss Acts**
5. **Status Effects Resolve**

#### Key Mechanics

- **Deck Cycle**: Auto-cycles from Draw Pile → Hand → Discard Pile.
- **Turn Limit**: Must defeat each boss within **`12` rounds**, otherwise the run fails.
- **Victory Condition**: **Boss HP ≤ `0`** triggers Game Victory.
- **Defeat Condition**: **Hero HP ≤ `0`** triggers Game Over.
- **Reward System**: Faster victories = **more permanent buff choices**.

----

## Required Coding Elements & Features

Here is how our project implements the required coding elements to support the core game features:

### 1. Generation of Random Events

- **Supported Features:** Randomized card drawing, randomized boss selection and sequence, fluctuating boss HP limits, and buff reward drops.
- **Implementation:** We utilize the `<random>` library, specifically `std::mt19937` with a seed based on the current system time (`std::time(nullptr)`) in `main.cpp`, to ensure unpredictable and dynamic gameplay per run. [Example](https://github.com/IcyCrucifix/COMP2113_Group66/blob/main/src/main.cpp)

### 2. Data Structures for Storing Data

- **Supported Features:** Managing Hero attributes, Card properties, Boss state, dynamically cycling Decks, Hand cards, and active Buffs.
- **Implementation:** Extensively uses `std::vector` to hold collections of cards and templates. We implemented custom structs and classes (`Hero`, `Card`, `Boss`, `Deck`, `Battle`) to encapsulate game entity logic cleanly. [Example: hero.h](https://github.com/IcyCrucifix/COMP2113_Group66/blob/main/src/hero/hero.h), [Example: card.h](https://github.com/IcyCrucifix/COMP2113_Group66/blob/main/src/cards/card.h)

### 3. Dynamic Memory Management

- **Supported Features:** Safe runtime object creation for combat instances, cards being drawn/discarded, and preventing memory leaks during long runs.
- **Implementation:** Employs `std::unique_ptr` and standard STL container auto-management for dynamic array sizing. Resources are correctly freed after a battle ends.

### 4. File Input / Output

- **Supported Features:** Loading static game data (Cards, Heroes, Bosses) from text files and implementing a robust Save/Load system for campaign progress.
- **Implementation:** Uses `<fstream>` (`std::ifstream`, `std::ofstream`) to parse `cards.txt`, `heroes.txt`, and `bosses.txt` dynamically at launch, and to write/read `savegame.txt` when users save or load their progress. [Example](https://github.com/IcyCrucifix/COMP2113_Group66/blob/main/src/save_load/save_load.cpp)

### 5. Program Codes in Multiple Files

- **Supported Features:** Organizing the large codebase into modular subsystems for maintainability.

- **Implementation:** Divided into logical `.cpp` and `.h` pairs:

  ```text
  ├── main.cpp                # Game entry point and main menu logic
  ├── hero.cpp / hero.h       # Hero logic, passive skills, stats
  ├── card.cpp / card.h       # Card classes and execution effects
  ├── deck.cpp / deck.h       # Draw pile, discard pile, and hand management
  ├── battle.cpp / battle.h   # Combat engine and turn system
  ├── buff.cpp / buff.h       # Status conditions and permanent buffs
  ├── save_load.cpp / .h      # Persistence system for save/load
  ├── utils.cpp / utils.h     # Input validation, text parsing
  └── Makefile                # Build automation
  ```

### 6. Multiple Difficulty Levels

- **Supported Features:** Easy, Normal, and Hard difficulties altering the gameplay mechanics.
    - **Easy**: the selected boss’s rolled HP and boss move are reduced to `75%` of its normal value, the hero’s card effects are scaled up by `12%`, so attacks, healing, and status effects are all more favorable to the player.
    - **Normal**: no extra scaling is applied and all values stay at their base settings.
    - **Hard**:
        - **Basic hard scale**:
            - boss HP `1.20x`
            - hero card values `0.92x`
        - **Random hard scale**: every 3 rounds, the boss gains one random one-round Hard-only modifier before acting. Only one of these triggers in that round:
            - Ferocity: the boss’s move gains `+18` extra damage;
            - Guard Break: if the boss’s move deals damage, it also applies `+2` **Vulnerability**;
            - Scorch Pulse: after the boss acts, the player gains `+2` **Burn**;
            - Toxic Pulse: after the boss acts, the player gains `+2` **Poison**.
- **Implementation:** Applied via a global en56um in `main.cpp` that modifies boss HP scaling, attack power, boss intent visibility (UI), and triggers multi-phase mechanics on Hard.
- **Supported Features:** Roguelike Buff System. [Example](https://github.com/IcyCrucifix/COMP2113_Group66/blob/main/src/buff/buff.h)
    - If the player decides to play multiple rounds, the player chooses **one permanent buff** from a random reward selection after each boss victory. These buffs are run-persistent upgrades and are the main roguelike progression layer.
    - **Permanent buff list**
        - **Mighty Strikes**: `+20%` to all damage dealt.
        - **Iron Will**: `+15` maximum HP.
        - **Energy Surge**: `+1` Energy cap every round.
        - **Swift Step**: `+5` Speed.
        - **Scholar Mind**: Draw `1` extra card each round.
        - **Ember Mastery**: All Burn applications gain `+2` stacks.
        - **Toxic Mastery**: All Poison applications gain `+2` stacks.
        - **Vital Echo**: `+20%` to healing effects.
        - **Barrier Core**: Gain `+8 Shield` each round and start battles with `1 Block`.
    - **How permanent buffs are earned**
        <br>Reward count depends on battle performance:
        - `<= 5` rounds: `5` choices
        - `<= 7` rounds: `4` choices
        - `<= 10` rounds: `3` choices
        - otherwise: `2` choices
      <br>This means faster victories give better reward selection quality, reinforcing the roguelike snowball loop.
- **Implementation**: Permanent buffs are represented by the `PermanentBuff` struct in `slay_text/buff.h`, containing fields such as `damagePercent`, `maxHpBonus`, `startingBlock`,etc. The permanent buff catalog is built in `buildPermanentBuffCatalog()` in `slay_text/buff.cpp`.

------

## Core Game Systems

### Hero Classes

Users can choose their hero before each encounter. There are 3 heroes to choose from.

|      Hero Name      | Class Type |  HP  | Energy | Passives                                                     | Suggested Playing Style |
| :-----------------: | :--------: | :--: | :----: | ------------------------------------------------------------ | ----------------------- |
| Shadowblade Strider | Offensive  | `200`  |   `5`    | Gain Power if damage ≥`220`; <br>Vulnerability stacks `+1`; <br>`+3%` damage if enemy <`30%` HP | High-risk burst damage  |
|   Bulwark Knight    | Defensive  | `280`  |   `3`    | Gain Block + shield each turn; <br>Defense cost `-1`; <br>Reflect `10%` damage | Defensive counterattack |
| Wraithflame Sprite  |   Magic    | `220`  |   `4`    | Burn/Poison boosted; <br>Magic damage `+15%`; <br>Draw `+1` if 2 debuffs active | Damage-over-time        |

### Card System 

There are 23 kinds of cards in total, categorized into 4 types. Different types are related to different playing styles and actions.

- **Attack**
  - There are 8 kinds of **Attack Card** in total. Effects include deal damage, apply **Vulnerability** or **Burn**, burst damage, self-recoil effects.
  - **Sample Card**:
    - **Heavy Strike**: Deal `70` physical damage.
    - **Magic Blast**: Deal `80` magic damage.
- **Defense**
  - There are 5 kinds of **Defense Card** in total. Effects include damage reduction, apply **Block** and conditional counterattacks. 
  - **Sample Card**
    - **Iron Shield**: Reduce physical damage taken by `80%`, gain `2` **Block**.
    - **Thorn Armor**: Reduce damage taken by `50%`, if the enemy uses an **Attack Card**, the enemy takes double the damage you receive.
- **Status**
  - There are 8 kinds of **Status Cards** in total. They apply 7 different statuses, which are explained in the Status Effects section below.
- **Other**
    - **Tactical Mind**: Put one other card from your hand to the bottom of your deck, then draw 2 cards from the top.

### Status Effects 

During the battle, various status effects can be inflicted on both the player and the boss. These buffs and debuffs dynamically change combat performance, including attack power, defense capability and continuous damage. 7 statuses can be listed as follows:

- **Burn**: Each stack increases magic damage taken by `7%`.
  - **Sample Card**: Flame Blast
- **Vulnerability**: Each stack increases physical damage taken by `5%`.
  - **Sample Card**: Frailty Strike
- **Power**: Each stack increases damage dealt by `20%`.
  - **Sample Card**: Power Blasting
- **Block**: Each stack reduces lethal damage taken by `20%`.
  - **Sample Card**: Iron Shield, Revenge Strike
- **Poison**: At end of turn, deal `12` damage per stack.
  - **Sample Card**: Toxic Storm
- **Shield**: Absorbs incoming damage.
  - **Sample**: Defense
- **Energy Saving**: Reduces all card energy costs.
  - **Sample Card**: Green Ecology

### Boss System

| Boss Type |               &emsp;&emsp;Names&emsp;&emsp;               | HP Range | Core Behavior Logic                                          | Special Mechanics                                            |
| :-------: | :-------------------------------------------------------: | :------: | ------------------------------------------------------------ | ------------------------------------------------------------ |
| Offensive |    Dark Demon;<br>Lord of Blood;<br>Starscourge Radahn    | `280-400`  | High physical damage attacks with varying probabilities      | `+30%` damage and more frequent strong attacks if below `30%` HP |
| Defensive |     Sanctuary Guardian;<br>Void Giant;<br>Abyss Beast     | `380-520`  | Damage reduction, reflection, counterattacks                 | Heals every 3 turns; <br>stronger reflect below `30%` HP       |
|   Magic   | Chaos Witch; <br>Queen of the Full Moon; <br>Death Priest | `260-380`  | Applies Burn, Poison, Vulnerability instead of direct damage | Heals if player has debuffs; <br>stronger debuffs below `30%` HP |
|  Hybrid   |              Unknown; <br>Sir of All-knowing              | `300-440`  | Adapts to player actions dynamically                         | Phase change below `40%` HP; reduces player draw               |

### Hero-Boss Restrained Relationship

To add depth and replayability, each hero has clear advantages and disadvantages against different boss types. This creates a rock-paper-scissors style strategy where choosing the right hero can significantly change the battle outcome.

| Hero                          | vs Attack Boss | vs Defense Boss | vs Magic Boss | vs Hybrid Boss |
|:-------------------------------:|:----------------:|:-----------------:|:---------------:|:----------------:|
| Shadowblade Strider | Neutral `×1.0`  | Disadvantage `×0.75` | Advantage `×1.25` | Neutral `×1.0`   |
| Bulwark Knight  | Advantage `×1.25` | Neutral `×1.0`   | Disadvantage `×0.75` | Neutral `×1.0`   |
| Wraithflame Sprite     | Disadvantage `×0.75` | Advantage `×1.25` | Neutral `×1.0`  | Advantage `×1.25` |

**Matchup Rules**  
- **Advantage**: All damage dealt by the player is multiplied by **`1.25(+25%)`**.  
- **Disadvantage**: All damage dealt by the player is multiplied by **`0.75(-25%)`**.  
- **Neutral**: No multiplier (`×1.00`).  

This system is calculated automatically in every battle and affects all cards, reflections, and status effects.

### Save / Load System

The game saves your progress directly to the **current working directory**. This ensures you can safely exit the game and resume your run with complete consistency. 

#### Stored Data
The save file securely stores the following information:
* **Hero Status**: Your chosen Hero ID and current HP.
* **Progression**: A complete list of all acquired permanent buffs.
* **Encounter State**: The generated boss battle sequence and your current index within it.
* **RNG State**: The random seed, ensuring run consistency and preventing save-scumming of randomized events.

```mermaid
%%{init: {'theme': 'default'}}%%
graph LR
    A[Current Game State] -->|Saves to| B[(Save File)]
    B --> C[Hero ID & HP]
    B --> D[Acquired Buffs]
    B --> E[Boss Sequence & Index]
    B --> F[Random Seed]

    classDef file fill:#e0f7fa,stroke:#006064,color:#000000,stroke-width:2px;
    classDef data fill:#ffffff,stroke:#757575,color:#000000,stroke-width:1px;

    class B file;
    class C,D,E,F data;
```

------

## Acknowledgement

Thank you for taking the time to explore our project! This game was independently designed and developed by our team, inspired by classic roguelike deck-builder frameworks. We would like to thank our course coordinator and teaching assistants for the learning materials and support throughout the COMP2113 course.

### Team Members and Contributions


*Clarification: Team member [Wang Huanyu](https://github.com/eEthY) commited changes of renaming the paths of ample files, causing the accumulation of additions and deletions extremely high. The original contribution of Wang is approximately 700 lines of addition and deletion in total.*

*We apologize for any inconvenience that might occur duing calculation of the percentage of contributions of each member. Thank you.*

- **[Chen Zhiyu](https://github.com/francescachen777-web)**: Responsible for boss and hero design, testing and partially coding.
- **[Fu Yitong](https://github.com/Lena070112)**: Responsible for 
- **[Long Zeyan](https://github.com/Chris-LongZeyan)**: Responsible for README writing, testing and partially coding.
- **[Wang Huanyu](https://github.com/eEthY)**: Responsible for 
- **[Xu Jingfeng](https://github.com/IcyCrucifix)**: Responsible for the majority of coding.
- **[Zhang Yikun](https://github.com/pzdmmsd)**: Responsible for README writing, testing and partially coding.
