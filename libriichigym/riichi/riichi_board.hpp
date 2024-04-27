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
#include "riichi_yaku.hpp"


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
        END,
    };
    
    enum class LifeIndex: uint8_t {
        P0_AGARI = static_cast<uint8_t>(Player::P0),
        P1_AGARI = static_cast<uint8_t>(Player::P1),
        P2_AGARI = static_cast<uint8_t>(Player::P2),
        P3_AGARI = static_cast<uint8_t>(Player::P3),
        RUN = 4,
        ABORT = 5,
    };
    
    class Cache {
    public:
        Cache() noexcept = default;
        ~Cache() noexcept = default;
        
        constexpr static uint8_t STATUS_SIZE = 5;
        
        std::array<Hand, PLAYER_UPPER_COUNT> fogs = {};
        std::array<std::bitset<STATUS_SIZE>, PLAYER_UPPER_COUNT> statuses = {};
        
        constexpr static uint8_t IS_IN_RIICHI = 0;
        constexpr static uint8_t IS_IN_DOUBLE_RIICHI = 1;
        constexpr static uint8_t IS_IN_DOUJUN_FURITEN = 2;
        constexpr static uint8_t IS_IN_SUTEHAI_FURITEN = 3;
        constexpr static uint8_t IS_IN_RIICHI_FURITEN = 4;
        
        
        template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
        const std::bitset<STATUS_SIZE>& cstatus(AnyPlayer player) const noexcept {
            return statuses[static_cast<uint8_t>(player)];
        }
        
        template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
        std::bitset<STATUS_SIZE>& status(AnyPlayer player) noexcept {
            return statuses[static_cast<uint8_t>(player)];
        }
        
        
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
    std::array<std::optional<YakuCombo>, PLAYER_UPPER_COUNT> combos = {};
    
    Cache cache;
    
    Tile bakaze() const noexcept;
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Tile jikaze(AnyPlayer player) const noexcept {
        switch ((session + std::to_underlying(player)) % 4) {
            case 0: return Tile::E;
            case 1: return Tile::S;
            case 2: return Tile::W;
            case 3: return Tile::N;
        }
        __builtin_unreachable();
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    bool tile_is_none_of_bakaze_or_jikaze(AnyPlayer player, Tile tile) const noexcept {
        return tile != bakaze() && tile != jikaze(player);
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    bool tile_is_kakukaze(AnyPlayer player, Tile tile) const noexcept {
        return tile_is_kaze(tile) && tile_is_none_of_bakaze_or_jikaze(player, tile);
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    const Hand& chand(AnyPlayer player) const noexcept {
        return hands[static_cast<uint8_t>(player)];
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Hand& hand(AnyPlayer player) noexcept {
        return hands[static_cast<uint8_t>(player)];
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    const std::optional<YakuCombo>& ccombo(AnyPlayer player) const noexcept {
        return combos[static_cast<uint8_t>(player)];
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    std::optional<YakuCombo>& combo(AnyPlayer player) noexcept {
        return combos[static_cast<uint8_t>(player)];
    }
    
    Player dealer() const noexcept;
    std::tuple<Player, Player, Player> punters() const noexcept;
    std::pair<Player, Player> punters_expect(Player) const noexcept;

    ActionGroup request() noexcept;
    void response(const ActionGroup&) noexcept;
};

}

#endif /* riichi_board_hpp */
