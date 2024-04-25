//
//  riichi_display.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include "riichi_display.hpp"


namespace riichi {

// MARK: - Player
Player player_cycle_next(Player player) noexcept {
    if (static_cast<uint8_t>(player) + 1 < PLAYER_COUNT) {
        return static_cast<Player>(static_cast<uint8_t>(player) + 1);
    }
    return static_cast<Player>(0);
}

Player player_cycle_prev(Player player) noexcept {
    if (static_cast<uint8_t>(player) > 0) {
        return static_cast<Player>(static_cast<uint8_t>(player) - 1);
    }
    return static_cast<Player>(PLAYER_COUNT - 1);
}

// MARK: - Hand
uint8_t Hand::sum() const noexcept {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < tile_counts.size(); i++) {
        sum += tile_counts[i];
    }
    return sum;
}


Tile Hand::first_tile() const noexcept {
    for (uint8_t i = 0; i < tile_counts.size(); i++) {
        if (tile_counts[i] > 0) {
            return static_cast<Tile>(i);
        }
    }
    return Tile::UND;
}


// MARK: - Mountain
Mountain::Mountain() noexcept {
    tiles = {
        Tile::M1, Tile::M1, Tile::M1, Tile::M1,
        Tile::M2, Tile::M2, Tile::M2, Tile::M2,
        Tile::M3, Tile::M3, Tile::M3, Tile::M3,
        Tile::M4, Tile::M4, Tile::M4, Tile::M4,
        Tile::M5R, Tile::M5, Tile::M5, Tile::M5,
        Tile::M6, Tile::M6, Tile::M6, Tile::M6,
        Tile::M7, Tile::M7, Tile::M7, Tile::M7,
        Tile::M8, Tile::M8, Tile::M8, Tile::M8,
        Tile::M9, Tile::M9, Tile::M9, Tile::M9,
        
        Tile::S1, Tile::S1, Tile::S1, Tile::S1,
        Tile::S2, Tile::S2, Tile::S2, Tile::S2,
        Tile::S3, Tile::S3, Tile::S3, Tile::S3,
        Tile::S4, Tile::S4, Tile::S4, Tile::S4,
        Tile::S5R, Tile::S5, Tile::S5, Tile::S5,
        Tile::S6, Tile::S6, Tile::S6, Tile::S6,
        Tile::S7, Tile::S7, Tile::S7, Tile::S7,
        Tile::S8, Tile::S8, Tile::S8, Tile::S8,
        Tile::S9, Tile::S9, Tile::S9, Tile::S9,
        
        Tile::P1, Tile::P1, Tile::P1, Tile::P1,
        Tile::P2, Tile::P2, Tile::P2, Tile::P2,
        Tile::P3, Tile::P3, Tile::P3, Tile::P3,
        Tile::P4, Tile::P4, Tile::P4, Tile::P4,
        Tile::P5R, Tile::P5, Tile::P5, Tile::P5,
        Tile::P6, Tile::P6, Tile::P6, Tile::P6,
        Tile::P7, Tile::P7, Tile::P7, Tile::P7,
        Tile::P8, Tile::P8, Tile::P8, Tile::P8,
        Tile::P9, Tile::P9, Tile::P9, Tile::P9,
        
        Tile::E, Tile::E, Tile::E, Tile::E,
        Tile::S, Tile::S, Tile::S, Tile::S,
        Tile::W, Tile::W, Tile::W, Tile::W,
        Tile::N, Tile::N, Tile::N, Tile::N,
        
        Tile::P, Tile::P, Tile::P, Tile::P,
        Tile::F, Tile::F, Tile::F, Tile::F,
        Tile::C, Tile::C, Tile::C, Tile::C,
    };
    
    draw_curr = 0;
    rinshan_curr = 0;
    dora_curr = 1;
}

Tile Mountain::draw(bool is_from_rinshan) noexcept {
    Tile tile;
    if (!is_from_rinshan) {
        tile = tiles[draw_curr + Mountain::draw_begin];
        draw_curr += 1;
    } else {
        tile = tiles[rinshan_curr + Mountain::rinshan_begin];
        rinshan_curr += 1;
    }
    return tile;
}

bool Mountain::can_draw() const noexcept {
    return draw_curr < (Mountain::draw_end - Mountain::draw_begin);
};

Tile Mountain::outdora_tile(Offset<uint8_t> i) const noexcept {
    return tiles[i + Mountain::outdora_begin];
}

Tile Mountain::indora_tile(Offset<uint8_t> i) const noexcept {
    return tiles[i + Mountain::indora_begin];
}

uint8_t Mountain::remaining_draw_count() const noexcept {
    return Mountain::draw_end - Mountain::draw_begin - draw_curr;
}

uint8_t Mountain::dora_visible_count() const noexcept {
    return dora_curr;
}


// MARK: - Shrine




// MARK: - ActionGroup
bool actionKind_is_undefined(ActionKind actionkind) noexcept {
    return static_cast<uint8_t>(actionkind) > static_cast<uint8_t>(ActionKind::ABORT);
}


ActionGroup::ActionGroup() noexcept {
    tiles.fill(Tile::UND);
    actionkinds.fill(ActionKind::UND);
    tips = {};
}


std::bitset<PLAYER_UPPER_COUNT> ActionGroup::actionkinds_nonundefineds() const noexcept {
    std::bitset<PLAYER_UPPER_COUNT> nonundefineds = {};
    for (uint8_t i = 0; i < PLAYER_UPPER_COUNT; i++) {
        if (!actionKind_is_undefined(actionkinds[i])) {
            nonundefineds.set(i);
        }
    }
    return nonundefineds;
}

std::bitset<PLAYER_UPPER_COUNT> ActionGroup::tiles_nonundefineds() const noexcept {
    std::bitset<PLAYER_UPPER_COUNT> nonundefineds = {};
    for (uint8_t i = 0; i < PLAYER_UPPER_COUNT; i++) {
        if (!tile_is_undefined(tiles[i])) {
            nonundefineds.set(i);
        }
    }
    return nonundefineds;
}

}
