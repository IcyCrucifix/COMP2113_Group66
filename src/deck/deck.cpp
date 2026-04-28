#include "deck.h"

#include "utils.h"

#include <algorithm>
#include <stdexcept>

Deck::Deck() = default;

void Deck::buildStarterDeck(const std::vector<CardDefinition> &cardPool, HeroClass heroClass)
{
    starterDeckCapacity_ = 12;
    starterDeckSize_ = 0;
    starterDeck_ = std::make_unique<CardDefinition[]>(starterDeckCapacity_);

    if (heroClass == HeroClass::Offensive)
    {
        addStarterCardById(cardPool, "light_strike");
        addStarterCardById(cardPool, "light_strike");
        addStarterCardById(cardPool, "heavy_strike");
        addStarterCardById(cardPool, "heavy_strike");
        addStarterCardById(cardPool, "frailty_strike");
        addStarterCardById(cardPool, "lethal_blow");
        addStarterCardById(cardPool, "defend");
        addStarterCardById(cardPool, "power_blessing");
        addStarterCardById(cardPool, "first_aid");
        addStarterCardById(cardPool, "tactical_mind");
    }
    else if (heroClass == HeroClass::Defensive)
    {
        addStarterCardById(cardPool, "light_strike");
        addStarterCardById(cardPool, "revenge_strike");
        addStarterCardById(cardPool, "defend");
        addStarterCardById(cardPool, "defend");
        addStarterCardById(cardPool, "iron_shield");
        addStarterCardById(cardPool, "iron_shield");
        addStarterCardById(cardPool, "magic_shield");
        addStarterCardById(cardPool, "opportunism");
        addStarterCardById(cardPool, "thorn_armor");
        addStarterCardById(cardPool, "first_aid");
    }
    else
    {
        addStarterCardById(cardPool, "magic_blast");
        addStarterCardById(cardPool, "magic_blast");
        addStarterCardById(cardPool, "flame_blast");
        addStarterCardById(cardPool, "flame_blast");
        addStarterCardById(cardPool, "toxic_storm");
        addStarterCardById(cardPool, "toxic_storm");
        addStarterCardById(cardPool, "crush_defense");
        addStarterCardById(cardPool, "green_ecology");
        addStarterCardById(cardPool, "first_aid");
        addStarterCardById(cardPool, "tactical_mind");
    }
}

void Deck::resetForBattle()
{
    drawPile_.clear();
    discardPile_.clear();
    hand_.clear();

    for (std::size_t index = 0; index < starterDeckSize_; ++index)
    {
        drawPile_.push_back(starterDeck_[index]);
    }

    std::shuffle(drawPile_.begin(), drawPile_.end(), utils::rng());
}

void Deck::drawCards(int count)
{
    for (int drawn = 0; drawn < count; ++drawn)
    {
        refillDrawPileIfNeeded();
        if (drawPile_.empty())
        {
            return;
        }

        hand_.push_back(drawPile_.back());
        drawPile_.pop_back();
    }
}

void Deck::discardHand()
{
    discardPile_.insert(discardPile_.end(), hand_.begin(), hand_.end());
    hand_.clear();
}

bool Deck::playCard(std::size_t index, CardDefinition &cardOut)
{
    if (index >= hand_.size())
    {
        return false;
    }

    cardOut = hand_[index];
    discardPile_.push_back(hand_[index]);
    hand_.erase(hand_.begin() + static_cast<long>(index));
    return true;
}

void Deck::moveHighestCostCardToBottom()
{
    if (hand_.empty())
    {
        return;
    }

    auto best = std::max_element(hand_.begin(), hand_.end(), [](const CardDefinition &left, const CardDefinition &right) {
        return left.cost < right.cost;
    });

    drawPile_.insert(drawPile_.begin(), *best);
    hand_.erase(best);
}

const std::vector<CardDefinition> &Deck::hand() const
{
    return hand_;
}

bool Deck::handEmpty() const
{
    return hand_.empty();
}

std::size_t Deck::handSize() const
{
    return hand_.size();
}

std::vector<std::string> Deck::drawPileIds() const
{
    std::vector<std::string> ids;
    for (const CardDefinition &card : drawPile_)
    {
        ids.push_back(card.id);
    }
    return ids;
}

std::vector<std::string> Deck::discardPileIds() const
{
    std::vector<std::string> ids;
    for (const CardDefinition &card : discardPile_)
    {
        ids.push_back(card.id);
    }
    return ids;
}

std::vector<std::string> Deck::handIds() const
{
    std::vector<std::string> ids;
    for (const CardDefinition &card : hand_)
    {
        ids.push_back(card.id);
    }
    return ids;
}

void Deck::restoreBattleState(const std::vector<CardDefinition> &drawPile, const std::vector<CardDefinition> &discardPile, const std::vector<CardDefinition> &hand)
{
    drawPile_ = drawPile;
    discardPile_ = discardPile;
    hand_ = hand;
}

void Deck::addStarterCardById(const std::vector<CardDefinition> &cardPool, const std::string &cardId)
{
    const auto found = std::find_if(cardPool.begin(), cardPool.end(), [&](const CardDefinition &card) {
        return card.id == cardId;
    });

    if (found == cardPool.end())
    {
        throw std::runtime_error("Missing starter card: " + cardId);
    }

    if (starterDeckSize_ >= starterDeckCapacity_)
    {
        throw std::runtime_error("Starter deck capacity exceeded.");
    }

    starterDeck_[starterDeckSize_] = *found;
    ++starterDeckSize_;
}

void Deck::refillDrawPileIfNeeded()
{
    if (!drawPile_.empty())
    {
        return;
    }

    if (discardPile_.empty())
    {
        return;
    }

    drawPile_ = discardPile_;
    discardPile_.clear();
    std::shuffle(drawPile_.begin(), drawPile_.end(), utils::rng());
}
