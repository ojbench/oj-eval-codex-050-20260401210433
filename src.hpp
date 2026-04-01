// Implementation for ACMOJ Problem 3028 (Problem 050)
// Provides GameState and related interfaces expected by the judge.

#pragma once

#include <exception>
#include <optional>

class InvalidOperation : public std::exception {
public:
    const char* what() const noexcept override { return "invalid operation"; }
};

struct PlayInfo {
    int dummyCount = 0;
    int magnifierCount = 0;
    int converterCount = 0;
    int cageCount = 0;
};

class GameState {
public:
    enum class BulletType { Live, Blank };
    enum class ItemType { Dummy, Magnifier, Converter, Cage };

    GameState()
        : currentPlayer_(0),
          liveCount_(0),
          blankCount_(0),
          hp_{5, 5},
          nextKnown_(),
          usedCageThisTurn_{false, false},
          activeCage_{false, false} {}

    void fireAtOpponent(BulletType topBulletBeforeAction) {
        // Consume the bullet
        consumeTopBullet_(topBulletBeforeAction);

        // Apply effect
        if (topBulletBeforeAction == BulletType::Live) {
            hp_[opponent_()] -= 1;
        }

        // If game ends, do not process turn switching/cage
        if (winnerId() != -1) return;

        // Firing at opponent always requests end of turn
        requestEndTurn_();
    }

    void fireAtSelf(BulletType topBulletBeforeAction) {
        // Consume the bullet
        consumeTopBullet_(topBulletBeforeAction);

        if (topBulletBeforeAction == BulletType::Live) {
            hp_[currentPlayer_] -= 1;
            if (winnerId() != -1) return;
            // Live on self ends the turn (subject to cage)
            requestEndTurn_();
        } else {
            // Blank on self continues the turn (no end-turn)
        }
    }

    void useDummy(BulletType topBulletBeforeUse) {
        auto& me = info_[currentPlayer_];
        if (me.dummyCount <= 0) throw InvalidOperation();
        me.dummyCount -= 1;

        // Consume the top bullet and reveal its type (param given)
        consumeTopBullet_(topBulletBeforeUse);
        // Does not end the turn
    }

    void useMagnifier(BulletType topBulletBeforeUse) {
        auto& me = info_[currentPlayer_];
        if (me.magnifierCount <= 0) throw InvalidOperation();
        me.magnifierCount -= 1;

        // Reveal but do not consume
        nextKnown_ = topBulletBeforeUse;
        // Does not end the turn
    }

    void useConverter(BulletType topBulletBeforeUse) {
        auto& me = info_[currentPlayer_];
        if (me.converterCount <= 0) throw InvalidOperation();
        me.converterCount -= 1;

        // Flip the type of the next bullet in place and reveal it
        if (topBulletBeforeUse == BulletType::Live) {
            // Convert Live -> Blank
            --liveCount_;
            ++blankCount_;
            nextKnown_ = BulletType::Blank;
        } else {
            // Convert Blank -> Live
            --blankCount_;
            ++liveCount_;
            nextKnown_ = BulletType::Live;
        }
        // Does not end the turn
    }

    void useCage() {
        auto& me = info_[currentPlayer_];
        if (me.cageCount <= 0) throw InvalidOperation();
        if (usedCageThisTurn_[currentPlayer_]) throw InvalidOperation();

        me.cageCount -= 1;
        usedCageThisTurn_[currentPlayer_] = true;
        activeCage_[currentPlayer_] = true;
    }

    void reloadBullets(int liveCount, int blankCount) {
        liveCount_ = liveCount;
        blankCount_ = blankCount;
        // Knowledge about top bullet is reset after reloading
        nextKnown_.reset();
    }

    void reloadItem(int playerId, ItemType item) {
        auto& p = info_[playerId];
        switch (item) {
            case ItemType::Dummy: p.dummyCount += 1; break;
            case ItemType::Magnifier: p.magnifierCount += 1; break;
            case ItemType::Converter: p.converterCount += 1; break;
            case ItemType::Cage: p.cageCount += 1; break;
        }
    }

    double nextLiveBulletProbability() const {
        if (nextKnown_.has_value()) {
            return (*nextKnown_ == BulletType::Live) ? 1.0 : 0.0;
        }
        const int total = liveCount_ + blankCount_;
        if (total <= 0) return 0.0;
        return static_cast<double>(liveCount_) / static_cast<double>(total);
    }

    double nextBlankBulletProbability() const {
        if (nextKnown_.has_value()) {
            return (*nextKnown_ == BulletType::Blank) ? 1.0 : 0.0;
        }
        const int total = liveCount_ + blankCount_;
        if (total <= 0) return 0.0;
        return static_cast<double>(blankCount_) / static_cast<double>(total);
    }

    int winnerId() const {
        if (hp_[0] <= 0 && hp_[1] <= 0) {
            // Should not happen per problem setting, but prefer current player wins tie? Not specified.
            // Data guarantees no other invalid cases; pick opponent as loser if any.
            return (hp_[0] <= 0 && hp_[1] > 0) ? 1 : (hp_[1] <= 0 && hp_[0] > 0 ? 0 : -1);
        }
        if (hp_[0] <= 0) return 1;
        if (hp_[1] <= 0) return 0;
        return -1;
    }

private:
    int opponent_() const { return currentPlayer_ ^ 1; }

    void consumeTopBullet_(BulletType t) {
        // Upon consuming the top bullet, knowledge about it is cleared
        if (t == BulletType::Live) {
            --liveCount_;
        } else {
            --blankCount_;
        }
        nextKnown_.reset();
    }

    void requestEndTurn_() {
        if (activeCage_[currentPlayer_]) {
            // Prevent end-of-turn once and clear the effect
            activeCage_[currentPlayer_] = false;
            return;
        }
        // Switch turn
        currentPlayer_ ^= 1;
        // Reset per-turn state for the new current player
        usedCageThisTurn_[currentPlayer_] = false;
        activeCage_[currentPlayer_] = false;
    }

    int currentPlayer_;
    int liveCount_;
    int blankCount_;
    int hp_[2];
    PlayInfo info_[2];
    std::optional<BulletType> nextKnown_;
    bool usedCageThisTurn_[2];
    bool activeCage_[2];
};

