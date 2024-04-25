//
//  riichi_tile.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include <utility>
#include <array>
#include "riichi_tile.hpp"
#include "riichi_util.hpp"


namespace riichi {

static const char tile_descriptions[] = "u\0\0" "1m\0" "2m\0" "3m\0" "4m\0" "5m\0" "6m\0" "7m\0" "8m\0" "9m\0" "1s\0" "2s\0" "3s\0" "4s\0" "5s\0" "6s\0" "7s\0" "8s\0" "9s\0" "1p\0" "2p\0" "3p\0" "4p\0" "5p\0" "6p\0" "7p\0" "8p\0" "9p\0" "e\0\0" "s\0\0" "w\0\0" "n\0\0" "p\0\0" "f\0\0" "c\0\0" "mr\0" "sr\0" "pr";


// MARK: - Tile
bool tile_is_undefined(Tile tile) noexcept {
    return tile > TILE_LAST;
}

bool tile_is_equal(Tile lhs, Tile rhs) noexcept {
    if (tile_is_undefined(lhs) || tile_is_undefined(rhs)) {
        if (tile_is_undefined(lhs) && tile_is_undefined(rhs)) {
            return true;
        } else {
            return false;
        }
    } else {
        return lhs == rhs;
    }
};

const char* tile_describe(Tile tile) noexcept {
    if (tile_is_undefined(tile)) {
        return tile_descriptions;
    }
    return &tile_descriptions[(static_cast<uint8_t>(tile) + 1) * 3];
}


bool tile_is_gradeable(Tile tile) noexcept {
    switch (tile) {
        case Tile::M5:
        case Tile::S5:
        case Tile::P5:
        case Tile::M5R:
        case Tile::S5R:
        case Tile::P5R: return true;
        default: return false;
    }
}

Tile tile_downgrade(Tile tile) noexcept {
    switch (tile) {
        case Tile::M5R: return Tile::M5;
        case Tile::S5R: return Tile::S5;
        case Tile::P5R: return Tile::P5;
        default: return tile;
    }
}

Tile tile_upgrade(Tile tile) noexcept {
    switch (tile) {
        case Tile::M5: return Tile::M5R;
        case Tile::S5: return Tile::S5R;
        case Tile::P5: return Tile::P5R;
        default: return tile;
    }
}


TileKind tile_kind(Tile tile) noexcept {
    if ((tile >= Tile::M1 && tile <= Tile::M9) || tile == Tile::M5R) {
        return TileKind::MAN;
    } else if ((tile >= Tile::S1 && tile <= Tile::S9) || tile == Tile::S5R) {
        return TileKind::SOU;
    } else if ((tile >= Tile::P1 && tile <= Tile::P9) || tile == Tile::P5R) {
        return TileKind::PIN;
    } else if (tile >= Tile::E && tile <= Tile::N) {
        return TileKind::WIND;
    } else if (tile >= Tile::P && tile <= Tile::C) {
        return TileKind::SAN;
    }
    return TileKind::UND;
}

uint8_t tile_digit(Tile tile) noexcept {
    tile = tile_downgrade(tile);
    TileKind tilekind = tile_kind(tile);
    return static_cast<uint8_t>(tile) - static_cast<uint8_t>(tilekind_first_tile(tilekind));
}


Tile tile_make(TileKind kind, uint8_t digit) noexcept {
    if (tilekind_is_undefined(kind)) { return Tile::UND; }
    if (digit >= tilekind_cycle(kind)) { return Tile::UND; }
    return static_cast<Tile>(static_cast<uint8_t>(tilekind_first_tile(kind)) + digit);
}


Tile tile_add(Tile tile, uint8_t num) noexcept {
    return static_cast<Tile>(static_cast<uint8_t>(tile) + num);
}

Tile tile_sub(Tile tile, uint8_t num) noexcept {
    return static_cast<Tile>(static_cast<uint8_t>(tile) - num);
}

uint8_t tile_to(Tile tile) noexcept {
    return static_cast<uint8_t>(tile);
}

Tile tile_from(uint8_t tile) noexcept {
    return static_cast<Tile>(tile);
}


Tile tile_cycle_next(Tile tile) noexcept {
    TileKind kind = tile_kind(tile);
    uint8_t digit = tile_digit(tile) + 1;
    uint8_t cycle = tilekind_cycle(kind);
    if (unlikely(digit >= cycle)) { digit = 0; }
    return tile_make(kind, digit);
}

Tile tile_cycle_prev(Tile tile) noexcept {
    TileKind kind = tile_kind(tile);
    uint8_t digit = tile_digit(tile);
    uint8_t cycle = tilekind_cycle(kind);
    if (likely(digit > 0)) { digit -= 1; }
    else { digit = cycle - 1; }
    return tile_make(kind, digit);
}

Tile tile_linear_next(Tile tile) noexcept {
    TileKind kind = tile_kind(tile);
    uint8_t digit = tile_digit(tile) + 1;
    uint8_t cycle = tilekind_cycle(kind);
    if (unlikely(digit >= cycle)) { return Tile::UND; }
    return tile_make(kind, digit);
}

Tile tile_linear_prev(Tile tile) noexcept {
    TileKind kind = tile_kind(tile);
    uint8_t digit = tile_digit(tile);
    if (likely(digit > 0)) { digit -= 1; }
    else { return Tile::UND; }
    return tile_make(kind, digit);
}


bool tile_is_jihai(Tile tile) noexcept {
    return static_cast<uint8_t>(tile) >= static_cast<uint8_t>(Tile::E) && static_cast<uint8_t>(tile) <= static_cast<uint8_t>(Tile::C);
}

bool tile_is_suuhai(Tile tile) noexcept {
    return !tile_is_jihai(tile) && !tile_is_undefined(tile);
}

bool tile_is_yaochuuhai(Tile tile) noexcept {
    return tile == Tile::E || tile == Tile::S || tile == Tile::W || tile == Tile::N || tile == Tile::P || tile == Tile::F || tile == Tile::C || tile == Tile::M1 || tile == Tile::M9 || tile == Tile::S1 || tile == Tile::S9 || tile == Tile::P1 || tile == Tile::P9;
}


// MARK: - Tilekind
Tile tilekind_first_tile(TileKind tilekind) noexcept {
    switch (tilekind) {
        case TileKind::MAN: { return Tile::M1; }
        case TileKind::SOU: { return Tile::S1; }
        case TileKind::PIN: { return Tile::P1; }
        case TileKind::WIND: { return Tile::E; }
        case TileKind::SAN: { return Tile::P; }
        default: { return Tile::UND; }
    }
}

uint8_t tilekind_cycle(TileKind tilekind) noexcept {
    switch (tilekind) {
        case TileKind::MAN:
        case TileKind::SOU:
        case TileKind::PIN: { return 9; }
        case TileKind::WIND: { return 4; }
        case TileKind::SAN: { return 3; }
        default: { return 0; }
    }
}


bool tilekind_is_undefined(TileKind tilekind) noexcept {
    return tilekind > TILEKIND_LAST;
}

bool tilekind_is_jihai(TileKind tilekind) noexcept {
    return (tilekind == TileKind::WIND || tilekind == TileKind::SAN);
}

bool tilekind_is_suuhai(TileKind tilekind) noexcept {
    return (static_cast<uint8_t>(tilekind) >= static_cast<uint8_t>(TileKind::MAN) || static_cast<uint8_t>(tilekind) <= static_cast<uint8_t>(TileKind::PIN));
}


}
