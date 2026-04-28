#pragma once

#include "card.h"
#include "hero.h"

#include <cstddef>
#include <memory>
#include <vector>

class Deck
{
public:
    /* What it does: Constructs an empty deck object.
     * Inputs: None.
     * Outputs: Empty deck ready for starter deck construction.
     */
    Deck();

    /* What it does: Builds the permanent starter deck for the selected hero class.
     * Inputs: cardPool - all loaded cards; heroClass - chosen hero archetype.
     * Outputs: None.
     */
    void buildStarterDeck(const std::vector<CardDefinition> &cardPool, HeroClass heroClass);

    /* What it does: Resets draw, discard, and hand piles for a new battle.
     * Inputs: None.
     * Outputs: None.
     */
    void resetForBattle();

    /* What it does: Draws cards from the deck into the player's hand.
     * Inputs: count - number of cards requested.
     * Outputs: None.
     */
    void drawCards(int count);

    /* What it does: Discards every card currently in the hand.
     * Inputs: None.
     * Outputs: None.
     */
    void discardHand();

    /* What it does: Plays a card from the hand by index.
     * Inputs: index - zero-based hand index; cardOut - output card that was played.
     * Outputs: True if a card was played; otherwise false.
     */
    bool playCard(std::size_t index, CardDefinition &cardOut);

    /* What it does: Moves the highest-cost card remaining in hand to the bottom of the draw pile.
     * Inputs: None.
     * Outputs: None.
     */
    void moveHighestCostCardToBottom();

    /* What it does: Returns the current hand.
     * Inputs: None.
     * Outputs: Constant reference to hand cards.
     */
    const std::vector<CardDefinition> &hand() const;

    /* What it does: Returns whether the hand is empty.
     * Inputs: None.
     * Outputs: True if the hand contains no cards; otherwise false.
     */
    bool handEmpty() const;

    /* What it does: Returns the number of cards in the hand.
     * Inputs: None.
     * Outputs: Hand size.
     */
    std::size_t handSize() const;

    /* What it does: Exports the current draw pile as card ids.
     * Inputs: None.
     * Outputs: Card id list in current draw order.
     */
    std::vector<std::string> drawPileIds() const;

    /* What it does: Exports the current discard pile as card ids.
     * Inputs: None.
     * Outputs: Card id list in current discard order.
     */
    std::vector<std::string> discardPileIds() const;

    /* What it does: Exports the current hand as card ids.
     * Inputs: None.
     * Outputs: Card id list in current hand order.
     */
    std::vector<std::string> handIds() const;

    /* What it does: Restores draw, discard, and hand piles from card lists.
     * Inputs: drawPile - cards to restore into the draw pile; discardPile - cards to restore into the discard pile; hand - cards to restore into the hand.
     * Outputs: None.
     */
    void restoreBattleState(const std::vector<CardDefinition> &drawPile, const std::vector<CardDefinition> &discardPile, const std::vector<CardDefinition> &hand);

private:
    /* What it does: Adds one card by id to the dynamically allocated starter deck.
     * Inputs: cardPool - all loaded cards; cardId - id to copy into the starter deck.
     * Outputs: None.
     */
    void addStarterCardById(const std::vector<CardDefinition> &cardPool, const std::string &cardId);

    /* What it does: Refills the draw pile from the discard pile and shuffles it.
     * Inputs: None.
     * Outputs: None.
     */
    void refillDrawPileIfNeeded();

    std::unique_ptr<CardDefinition[]> starterDeck_;
    std::size_t starterDeckSize_ = 0;
    std::size_t starterDeckCapacity_ = 0;
    std::vector<CardDefinition> drawPile_;
    std::vector<CardDefinition> discardPile_;
    std::vector<CardDefinition> hand_;
};
