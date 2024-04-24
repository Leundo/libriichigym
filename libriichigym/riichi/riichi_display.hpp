//
//  riichi_display.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#ifndef riichi_display_hpp
#define riichi_display_hpp

#include <array>
#include <span>
#include <bitset>
#include "riichi_tile.hpp"
#include "riichi_config.hpp"
#include "riichi_util.hpp"


namespace riichi {

// MARK: - Player
enum class Player: uint8_t {
    P0 = 0,
    P1 = 1,
    P2 = 2,
    P3 = 3,
};

constexpr Player PLAYER_FIRST = Player::P0;
constexpr Player PLAYER_LAST = static_cast<Player>(static_cast<uint8_t>(Player::P0) + RIICHI_GYM_PLAYER_COUNT);
constexpr uint8_t PLAYER_UPPER_COUNT = 4;
constexpr uint8_t PLAYER_COUNT = RIICHI_GYM_PLAYER_COUNT;

Player player_cycle_next(Player) noexcept;
Player player_cycle_prev(Player) noexcept;


// MARK: - Naki
enum class Naki: uint8_t {
    PON = 0,
    CHI = 1,
    OPEN_KAN = 2,
    ADD_KAN = 3,
    CLOSED_KAN = 4,
    UND = UINT8_MAX,
};


// MARK: - Discarded
class Discarded {
public:
    enum class Kiri: uint8_t {
        UNK = 0,
        OLD = 1,
        NEW = 2,
    };
    
    Discarded() noexcept = default;
    ~Discarded() noexcept = default;
    
    Tile tile = {};
    uint8_t move = {};
    Player owner = {};
    bool is_onriichi = {};
    Kiri kiri = {};
    Naki naki = {};
    Offset<uint8_t> triggering = {};
};


// MARK: - Exposed
class Exposed {
public:
    Exposed() noexcept = default;
    ~Exposed() noexcept = default;
    
    Naki naki = {};
    Tile keytile = {}; // is allway downgraded
    Tile trgtile = {};
    bool is_upgraded = {};
    uint8_t move = {};
    Player owner = {};
    Player trigger = {};
};


// MARK: - Hand
class Hand {
public:
    Hand() noexcept = default;
    ~Hand() noexcept = default;
    
    std::array<uint8_t, TILE_COLOR> tile_counts = {};
    
    uint8_t sum() const noexcept;
    Tile first_tile() const noexcept;
    
    template <AnyMatching<Tile, Offset<uint8_t>> AnyTile>
    void increase(AnyTile tile) noexcept {
        tile_counts[static_cast<uint8_t>(tile)] += 1;
    }
    
    template <AnyMatching<Tile, Offset<uint8_t>> AnyTile>
    void decrease(AnyTile tile) noexcept {
        if (tile_counts[static_cast<uint8_t>(tile)] > 0) {
            tile_counts[static_cast<uint8_t>(tile)] -= 1;
        }
    }
    
    template <AnyMatching<Tile, Offset<uint8_t>> AnyTile>
    void clear(AnyTile tile) noexcept {
        tile_counts[static_cast<uint8_t>(tile)] = 0;
    }
    
    template <AnyMatching<Tile, Offset<uint8_t>> AnyTile>
    bool decrease_upgraded_prioritizely(AnyTile tile) noexcept {
        if (tile_is_gradeable(tile) && count(static_cast<Tile>(tile_upgrade(tile))) > 0) {
            tile_counts[static_cast<uint8_t>(tile)] -= 1;
            return true;
        } else {
            decrease(tile_downgrade(tile));
            return false;
        }
    }
    
    template <AnyMatching<Tile, Offset<uint8_t>> AnyTile>
    uint8_t count(AnyTile tile) const noexcept {
        return tile_counts[static_cast<uint8_t>(tile)];
    }
    
    template <AnyMatching<Tile, Offset<uint8_t>> AnyTile>
    uint8_t count_ignoring_level(AnyTile tile) const noexcept {
        if (static_cast<Tile>(tile) == Tile::M5 || static_cast<Tile>(tile) == Tile::M5R) {
            return count(Tile::M5) + count(Tile::M5R);
        } else if (static_cast<Tile>(tile) == Tile::S5 || static_cast<Tile>(tile) == Tile::S5R) {
            return count(Tile::S5) + count(Tile::S5R);
        } else if (static_cast<Tile>(tile) == Tile::P5 || static_cast<Tile>(tile) == Tile::P5R) {
            return count(Tile::P5) + count(Tile::P5R);
        } else {
            return count(tile);
        }
    }
    
    template <AllMatching<Tile>... Args>
    Tile first_tile_except(Args... args) const noexcept {
        std::array<Tile, sizeof...(args)> tiles = {args...};
        for (uint8_t i = 0; i < tile_counts.size(); i++) {
            if (tile_counts[i] > 0) {
                for (auto tile: tiles) {
                    if (static_cast<uint8_t>(tile) == i) {
                        goto outer_countinue;
                    }
                }
                return static_cast<Tile>(i);
            }
outer_countinue:
        }
        return Tile::UND;
    }
};


// MARK: - Mountain
constexpr uint8_t MOUNTAIN_TILE_COUNT = 136;

class Mountain {
public:
    Mountain() noexcept;
    ~Mountain() noexcept = default;
    
    std::array<Tile, MOUNTAIN_TILE_COUNT> tiles;
    std::span<Tile> draw_span;
    std::span<Tile> rinshan_span;
    std::span<Tile> outdora_span;
    std::span<Tile> indora_span;
    Offset<uint8_t> draw_curr;
    Offset<uint8_t> rinshan_curr;
    Offset<uint8_t> dora_curr;
    
    bool can_draw() const noexcept;
    Tile outdora_tile(Offset<uint8_t> i) const noexcept;
    Tile indora_tile(Offset<uint8_t> i) const noexcept;
    
    uint8_t remaining_draw_count() const noexcept;
    uint8_t dora_visible_count() const noexcept;
    
    Tile draw(bool is_from_rinshan) noexcept;
};


// MARK: - River
// Warning: capacity not enough if player count is 3
constexpr uint8_t RIVER_PLAYER_TILE_UPPER_COUNT = MOUNTAIN_TILE_COUNT / PLAYER_UPPER_COUNT + 4;

class River {
public:
    River() noexcept = default;
    ~River() noexcept = default;
    
    std::array<Discarded, MOUNTAIN_TILE_COUNT> discardeds = {};
    std::array<std::array<Offset<uint8_t>, RIVER_PLAYER_TILE_UPPER_COUNT>, PLAYER_UPPER_COUNT> routes = {};
    Offset<uint8_t> pub_curr = 0;
    std::array<Offset<uint8_t>, PLAYER_UPPER_COUNT> pvt_currs = {};
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    const Discarded* cdiscarded(AnyPlayer player, Offset<uint8_t> index) const noexcept {
        uint8_t curr = pvt_currs[static_cast<uint8_t>(player)];
        if (index >= curr) {
            return nullptr;
        } else {
            return &discardeds[routes[static_cast<uint8_t>(player)][index]];
        }
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    const Discarded* last_cdiscarded(AnyPlayer player) const noexcept {
        uint8_t curr = pvt_currs[static_cast<uint8_t>(player)];
        if (curr <= 0) {
            return nullptr;
        } else {
            return &discardeds[routes[static_cast<uint8_t>(player)][curr]];
        }
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Discarded* discarded(AnyPlayer player, Offset<uint8_t> index) noexcept {
        uint8_t curr = pvt_currs[static_cast<uint8_t>(player)];
        if (index >= curr) {
            return nullptr;
        } else {
            return &discardeds[routes[static_cast<uint8_t>(player)][index]];
        }
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Discarded* last_discarded(AnyPlayer player) noexcept {
        uint8_t curr = pvt_currs[static_cast<uint8_t>(player)];
        if (curr <= 0) {
            return nullptr;
        } else {
            return &discardeds[routes[static_cast<uint8_t>(player)][curr]];
        }
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Discarded* append(AnyPlayer player) noexcept {
        Discarded* discarded = &discardeds[pub_curr];
        routes[static_cast<uint8_t>(player)][pvt_currs[static_cast<uint8_t>(player)]] = pub_curr;
        pub_curr += 1;
        pvt_currs[static_cast<uint8_t>(player)] += 1;
        return discarded;
    }
};


// MARK: - Shrine
class Shrine {
public:
    Shrine() noexcept = default;
    ~Shrine() noexcept = default;
    
    std::array<Exposed, PLAYER_UPPER_COUNT * 4> exposeds = {};
    std::array<std::array<Offset<uint8_t>, 4>, PLAYER_UPPER_COUNT> routes = {};
    Offset<uint8_t> pub_curr = 0;
    std::array<Offset<uint8_t>, PLAYER_UPPER_COUNT> pvt_currs = {};
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    const Exposed* cexposed(AnyPlayer player, Offset<uint8_t> index) const noexcept {
        uint8_t curr = pvt_currs[static_cast<uint8_t>(player)];
        if (index >= curr) {
            return nullptr;
        } else {
            return &exposeds[routes[static_cast<uint8_t>(player)][index]];
        }
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    const Exposed* last_cexposed(AnyPlayer player) const noexcept {
        uint8_t curr = pvt_currs[static_cast<uint8_t>(player)];
        if (curr <= 0) {
            return nullptr;
        } else {
            return &exposeds[routes[static_cast<uint8_t>(player)][curr]];
        }
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Exposed* append(AnyPlayer player) noexcept {
        Exposed* exposed = &exposeds[pub_curr];
        routes[static_cast<uint8_t>(player)][pvt_currs[static_cast<uint8_t>(player)]] = pub_curr;
        pub_curr += 1;
        pvt_currs[static_cast<uint8_t>(player)] += 1;
        return exposed;
    }
};


// MARK: - Action
enum class ActionKind: uint8_t {
    DISCARD,
    RIICHI,
    PON,
    CHI,
    OPEN_KAN,
    SELF_KAN,
    AGARI,
    ABORT,
    UND,
};

bool actionKind_is_undefined(ActionKind actionkind) noexcept;


class ActionGroup {
public:
    ActionGroup() noexcept;
    ~ActionGroup() noexcept = default;
    
    std::array<Tile, PLAYER_UPPER_COUNT> tiles = {};
    std::array<ActionKind, PLAYER_UPPER_COUNT> actionkinds = {};
    std::array<std::bitset<TILE_COLOR>, PLAYER_UPPER_COUNT> tips = {};
    
    std::bitset<PLAYER_UPPER_COUNT> actionkinds_nonundefineds() const noexcept;
    std::bitset<PLAYER_UPPER_COUNT> tiles_nonundefineds() const noexcept;
    
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    void set(AnyPlayer player, Tile tile, ActionKind actionkind) noexcept {
        tiles[static_cast<uint8_t>(player)] = tile;
        actionkinds[static_cast<uint8_t>(player)] = actionkind;
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    Tile get_tile(AnyPlayer player) const noexcept {
        return tiles[static_cast<uint8_t>(player)];
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    void tip_insert(AnyPlayer player, Tile tile) noexcept {
        tips[static_cast<uint8_t>(player)].set(static_cast<uint8_t>(tile));
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    void tip_remove(AnyPlayer player, Tile tile) noexcept {
        tips[static_cast<uint8_t>(player)].reset(static_cast<uint8_t>(tile));
    }
    
    template <AnyMatching<Player, Offset<uint8_t>> AnyPlayer>
    std::bitset<TILE_COLOR> tip_get(AnyPlayer player) const noexcept {
        return tips[static_cast<uint8_t>(player)];
    }
};

}

#endif /* riichi_display_hpp */
