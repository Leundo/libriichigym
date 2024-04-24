//
//  riichi_tile.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#ifndef riichi_tile_hpp
#define riichi_tile_hpp

#include <cstdint>
#include <climits>
#include <algorithm>
#include <array>
#include <utility>


namespace riichi {

// MARK: - Tile
enum class Tile: uint8_t {
    M1 = 0,
    M2 = 1,
    M3 = 2,
    M4 = 3,
    M5 = 4,
    M6 = 5,
    M7 = 6,
    M8 = 7,
    M9 = 8,
    S1 = 9,
    S2 = 10,
    S3 = 11,
    S4 = 12,
    S5 = 13,
    S6 = 14,
    S7 = 15,
    S8 = 16,
    S9 = 17,
    P1 = 18,
    P2 = 19,
    P3 = 20,
    P4 = 21,
    P5 = 22,
    P6 = 23,
    P7 = 24,
    P8 = 25,
    P9 = 26,
    E = 27,
    S = 28,
    W = 29,
    N = 30,
    P = 31,
    F = 32,
    C = 33,
    M5R = 34,
    S5R = 35,
    P5R = 36,
    UND = UINT8_MAX,
};

constexpr Tile TILE_FIRST = Tile::M1;
constexpr Tile TILE_LAST_IGNORING_LEVEL = Tile::C;
constexpr Tile TILE_LAST = Tile::P5R;
constexpr uint8_t TILE_COLOR = 37;


bool tile_is_undefined(Tile) noexcept;
bool tile_is_equal(Tile, Tile) noexcept;
const char* tile_describe(Tile) noexcept;


bool tile_is_gradeable(Tile) noexcept;
Tile tile_downgrade(Tile) noexcept;
Tile tile_upgrade(Tile) noexcept;
uint8_t tile_digit(Tile) noexcept;


Tile tile_cycle_next(Tile) noexcept;
Tile tile_cycle_prev(Tile) noexcept;
Tile tile_linear_next(Tile) noexcept;
Tile tile_linear_prev(Tile) noexcept;


bool tile_is_jihai(Tile) noexcept;
bool tile_is_suuhai(Tile) noexcept;


template<class URBG>
void tiles_shuffle(Tile* tiles, uint8_t size, URBG& rng) noexcept {
    std::shuffle(tiles, tiles + size, rng);
}


// MARK: - TileKind
enum class TileKind: uint8_t {
    MAN = 0,
    SOU = 1,
    PIN = 2,
    WIND = 3,
    SAN = 4,
    UND = UINT8_MAX,
};

constexpr TileKind TILEKIND_FIRST = TileKind::MAN;
constexpr TileKind TILEKIND_LAST = TileKind::SAN;


TileKind tile_kind(Tile) noexcept;
Tile tile_make(TileKind, uint8_t) noexcept;


Tile tilekind_first_tile(TileKind) noexcept;
uint8_t tilekind_cycle(TileKind) noexcept;


bool tilekind_is_undefined(TileKind) noexcept;
bool tilekind_is_jihai(TileKind) noexcept;
bool tilekind_is_suuhai(TileKind) noexcept;


}

#endif /* riichi_tile_hpp */
