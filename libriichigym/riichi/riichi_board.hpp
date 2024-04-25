//
//  riichi_board.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#ifndef riichi_board_hpp
#define riichi_board_hpp

#include <bitset>
#include <optional>
#include <random>
#include <functional>
#include "riichi_display.hpp"

namespace riichi {

class Board {
public:
    enum class Stage: uint8_t {
        PREPARE,
        DRAW,
        ABORT,
        TUMO,
        SEKAN,
        LOOT,
        FLOWER,
        RIICHI,
        MAIN_DISCARD,
        AUX_DISCARD,
        RONG,
        PON,
        CHI,
        KAN,
        SCORE,
        CLEAN,
    };
    
    enum class LifeIndex: uint8_t {
        EAST_AGARI = static_cast<uint8_t>(Player::P0),
        SOUTH_AGARI = static_cast<uint8_t>(Player::P1),
        WEST_AGARI = static_cast<uint8_t>(Player::P2),
        NORTH_AGARI = static_cast<uint8_t>(Player::P3),
        RUN = 4,
        ABORT = 5,
    };
    
    class Cache {
    public:
        Cache() noexcept;
        ~Cache() noexcept = default;
        std::array<Hand, PLAYER_UPPER_COUNT> fogs;
        
        template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
        Hand& fog(AnyPlayer player) noexcept {
            return fogs[static_cast<uint8_t>(player)];
        }
        
        template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer, AnyMatching<Tile, Offset<uint8_t>> AnyTile>
        void fogs_clear_expect(AnyPlayer player, AnyTile tile) noexcept {
            for (uint8_t i = 0; i < PLAYER_COUNT; i++) {
                if (i == static_cast<uint8_t>(player)) {
                    continue;
                }
                fogs[i].clear(static_cast<Tile>(tile));
            }
        }
        
        template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer, AnyMatching<Tile, Offset<uint8_t>> AnyTile>
        void fogs_decrease_expect(AnyPlayer player, AnyTile tile) noexcept {
            for (uint8_t i = 0; i < PLAYER_COUNT; i++) {
                if (i == static_cast<uint8_t>(player)) {
                    continue;
                }
                fogs[i].decrease(static_cast<Tile>(tile));
            }
        }
    };
    
    Board(std::optional<unsigned int> seed = {}, std::optional<std::function<void(Mountain&, std::mt19937&)>> shuffle = {}) noexcept;
    ~Board() noexcept = default;
    
    unsigned int seed;
    std::mt19937 random_generator;
    std::function<void(Mountain&, std::mt19937&)> shuffle;
    
    
    uint8_t session;
    uint8_t honba;
    uint8_t move;
    uint8_t tribute;
    std::array<int32_t, PLAYER_UPPER_COUNT> scores;
    
    Mountain mountain = {};
    River river = {};
    Shrine shrine = {};
    std::array<Hand, PLAYER_UPPER_COUNT> hands = {};
    
    Player current_player;
    Stage stage;
    std::bitset<8> life;
    Tile hold_tile;
    Tile drop_tile;
    
    std::bitset<PLAYER_UPPER_COUNT> action_permission;
    std::bitset<TILE_COLOR> action_tip;
    
    Cache cache;
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    const Hand& chand(AnyPlayer player) const noexcept {
        return hands[static_cast<uint8_t>(player)];
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Hand& hand(AnyPlayer player) noexcept {
        return hands[static_cast<uint8_t>(player)];
    }

    void restart(bool) noexcept;
    ActionGroup request() noexcept;
    void response(const ActionGroup&) noexcept;
};

}

#endif /* riichi_board_hpp */
