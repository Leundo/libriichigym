//
//  riichi_score.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/25.
//

#include "riichi_score.hpp"
#include "riichi_util.hpp"


namespace riichi {

uint8_t yakukind_to(YakuKind yakukind) noexcept {
    return static_cast<uint8_t>(yakukind);
}

bool PatternGroup::is_nil() const noexcept {
    return !is_koukushi && values == nullptr;
}

bool PatternGroup::is_value_valid(uint32_t value) noexcept {
    return (value & 0x80000000) != 0 && (value & 0x000000FF) != 0x000000FF;
}


bool NormalArr::get_mentu_flag(Offset<uint8_t> mentu_index, uint8_t flag_index) const noexcept {
    return status[mentu_index * 4 + flag_index];
}

void NormalArr::set_mentu_flag(Offset<uint8_t> mentu_index, uint8_t flag_index, bool flag) noexcept {
    status[mentu_index * 4 + flag_index] = flag;
}

bool NormalArr::get_mentu_combinable(Offset<uint8_t> mentu_index) const noexcept {
    return status[4 * 4 + mentu_index];
}

void NormalArr::set_mentu_combinable(Offset<uint8_t> mentu_index, bool flag) noexcept {
    status[4 * 4 + mentu_index] = flag;
}

bool NormalArr::get_pair_combinable() const noexcept {
    return status[4 * 4 + 4];
}

void NormalArr::set_pair_combinable(bool flag) noexcept {
    status[4 * 4 + 4] = flag;
}


bool operator == (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept {
    return lhs.dealer_rong == rhs.dealer_rong;
}

bool operator < (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept {
    return lhs.dealer_rong < rhs.dealer_rong;;
}


std::bitset<64> YakuCombo::yakus() const noexcept {
    return std::visit(Visitor {
        [](const auto& arr) { return arr.yakus; },
    }, arrangement);
}

PenaltyPoint YakuCombo::score() const noexcept {
    return {};
}


PatternGroup hand_patterngroup(const Hand& hand) {
    PatternGroup patterngroup = {};
    
    bool are_all_yaochuuhais = true;
    uint8_t kokushi_detective = 0;
    
    for (TileKind kind: TILEKIND_CASES) {
        uint8_t cycle = tilekind_cycle(kind);
        Tile first_tile = tilekind_first_tile(kind);
        
        for (uint8_t i = 0; i < cycle; i++) {
            Tile tile = tile_add(first_tile, i);
            uint8_t count = hand.count(tile);
            
            if (count > 0) {
                if (!tile_is_yaochuuhai(tile)) {
                    are_all_yaochuuhais = false;
                }
                kokushi_detective += count - 1;
            }
        }
    }
    
    if (unlikely(kokushi_detective <= 1 && are_all_yaochuuhais && hand.sum() == 14)) {
        patterngroup.is_koukushi = true;
    } else {
        uint32_t key = calculate_hand_key(hand);
        patterngroup.values = find_values_from_patterns(key);
    }
    
    return patterngroup;
}


// MARK: - Agari
static Arrangement arrangement_make(uint32_t value, const Board& board, Player player, Tile trgtile) {
    
    if ((value & 0x01000000) != 0) {
        auto arrangement = ChiitoitsuArr();
        arrangement.trgtile = trgtile;
        
        uint8_t pair_index = 0;
        for (uint8_t tile = tile_to(TILE_FIRST); tile <= tile_to(TILE_LAST_IGNORING_LEVEL); tile++) {
            if (board.chand(player).count(tile) > 0) {
                arrangement.pairs[pair_index] = tile_from(tile);
                pair_index += 1;
            }
        }
        arrangement.yakus.set(yakukind_to(YakuKind::CHIITOITSU));
        return arrangement;
    } else if ((value & 0x02000000) != 0) {
        auto arrangement = ChuurenArr();
        arrangement.trgtile = trgtile;
        
        Tile tile = tilekind_first_tile(tile_kind(board.chand(player).first_tile()));
        if (board.chand(player).count(tile) > 3) {
            arrangement.xtratile = tile;
        } else if (board.chand(player).count(tile) > 3) {
            arrangement.xtratile = tile_add(tile, 8);
        } else {
            for (uint8_t i = 1; i < 8; i++) {
                if (board.chand(player).count_ignoring_level(tile_add(tile, i)) > 1) {
                    arrangement.xtratile = tile_add(tile, i);
                    break;
                }
            }
        }
        arrangement.yakus.set(yakukind_to(YakuKind::CHUUREN_POUTOU));
        return arrangement;
    }
    
    auto arrangement = NormalArr();
    for (uint8_t i = 0; i < 4; i++) {
        if (uoptset_contain(value, i + 4)) {
            arrangement.set_mentu_flag(i, NormalArr::IS_SHUNTU, true);
        }
    }
    uint8_t pair_index = uoptset_extract(value, 4, 0);
    uint8_t mentu_pointer = 0;
    uint8_t mentu_index = uoptset_extract(value, 4, (mentu_pointer + 2) * 4);
    uint8_t appear_count = 0;
    
    for (uint8_t tile = tile_to(TILE_FIRST); tile <= tile_to(TILE_LAST_IGNORING_LEVEL); tile++) {
        if (board.chand(player).count_ignoring_level(tile) > 0) {
            if (pair_index == appear_count) {
                arrangement.pair = tile_from(tile);
            }
            while (mentu_index == appear_count) {
                arrangement.mentus[mentu_pointer] = tile_from(tile);
                mentu_pointer += 1;
                mentu_index = uoptset_extract(value, 4, (mentu_pointer + 2) * 4);
            }
            appear_count += 1;
        }
    }
    
    for (uint8_t i = 0; i < 4; i++) {
        const Exposed* exposed = board.shrine.cexposed(player, i);
        if (exposed == NULL) {
            break;
        }
        uint8_t mentu_pointer = 3 - i;
        arrangement.mentus[mentu_pointer] = exposed->keytile;
        switch (exposed->naki) {
            case Naki::PON: {
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_EXPOSED, true);
                break;
            }
            case Naki::CHI: {
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_SHUNTU, true);
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_EXPOSED, true);
                break;
            }
            case Naki::OPEN_KAN:
            case Naki::ADD_KAN: {
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_KAN, true);
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_EXPOSED, true);
                break;
            }
            case Naki::CLOSED_KAN: {
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_KAN, true);
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_EXPOSED, true);
                arrangement.set_mentu_flag(mentu_pointer, NormalArr::IS_CLOSED_KAN, true);
                break;
            }
            case Naki::UND: {
                break;
            }
        }
    }
    
    trgtile = tile_downgrade(trgtile);
    arrangement.trgtile = trgtile;
    for (uint8_t i = 0; i < 4; i++) {
        if (arrangement.get_mentu_flag(i, NormalArr::IS_EXPOSED)) {
            continue;
        }
        if (!arrangement.get_mentu_flag(i, NormalArr::IS_SHUNTU) && trgtile == arrangement.mentus[i]) {
            arrangement.set_mentu_combinable(i, true);
        } else if (arrangement.get_mentu_flag(i, NormalArr::IS_SHUNTU) && tile_to(trgtile) >= tile_to(arrangement.mentus[i]) && tile_to(trgtile) <= tile_to(arrangement.mentus[i]) + 2) {
            arrangement.set_mentu_combinable(i, true);
        }
    }
    if (trgtile == arrangement.pair) {
        arrangement.set_pair_combinable(true);
    }
    
    return arrangement;
}

static YakuCombo calculate_kokushi_yakucombo(const Board& board, Tile trgtile, Player player, AgariEvent agarievent) {
    YakuCombo yakucombo;
    auto arrangement = KoukushiArr();
    arrangement.trgtile = trgtile;
    arrangement.yakus.set(yakukind_to(YakuKind::KOUKUSHI_MUSOU));
    
    if (board.chand(player).count(trgtile) == 2) {
        arrangement.yakus.set(yakukind_to(YakuKind::PREFECT_KOUKUSHI_MUSOU));
    }
    if (board.move < 4 && agarievent == AgariEvent::TUMO && board.shrine.last_cexposed(player) == nullptr) {
        if (board.move == 0) {
            arrangement.yakus.set(yakukind_to(YakuKind::TENHOU));
        } else {
            arrangement.yakus.set(yakukind_to(YakuKind::CHIIHOU));
        }
    }
    
    yakucombo.is_nil = false;
    yakucombo.arrangement = arrangement;
    return yakucombo;
}

static YakuCombo calculate_chuuren_yakucombo(const Board& board, ChuurenArr* arrangement, Player player, AgariEvent agarievent) {
    YakuCombo yakucombo;
    arrangement->yakus.set(yakukind_to(YakuKind::CHUUREN_POUTOU));
    
    if (arrangement->xtratile == arrangement->trgtile) {
        arrangement->yakus.set(yakukind_to(YakuKind::PREFECT_CHUUREN_POUTOU));
    }
    if (board.move < 4 && agarievent == AgariEvent::TUMO && board.shrine.last_cexposed(player) == nullptr) {
        if (board.move == 0) {
            arrangement->yakus.set(yakukind_to(YakuKind::TENHOU));
        } else {
            arrangement->yakus.set(yakukind_to(YakuKind::CHIIHOU));
        }
    }
    
    yakucombo.is_nil = false;
    yakucombo.arrangement = *arrangement;
    return yakucombo;
}

static YakuCombo calculate_chiitoitsu_yakucombo(const Board& board, ChiitoitsuArr* arrangement, Player player, AgariEvent agarievent) {
    YakuCombo yakucombo;
    arrangement->yakus.set(yakukind_to(YakuKind::CHIITOITSU));
    
    
    yakucombo.is_nil = false;
    yakucombo.arrangement = *arrangement;
    return yakucombo;
}

static YakuCombo calculate_normal_yakucombo(const Board& board, NormalArr* arrangement, Player player, AgariEvent agarievent) {
    YakuCombo yakucombo;
    
    yakucombo.is_nil = false;
    yakucombo.arrangement = *arrangement;
    return yakucombo;
}


YakuCombo board_yakucombo(const Board& board, const PatternGroup& patterngroup, Player player, AgariEvent agarievent) {
    YakuCombo yakucombo;
    
    Tile trgtile;
    switch (agarievent) {
        case AgariEvent::TUMO:
        case AgariEvent::FLOWER: {
            trgtile = board.hold_tile;
            break;
        }
        case AgariEvent::RONG:
        case AgariEvent::LOOT: {
            trgtile = board.drop_tile;
            break;
        }
    }
    
    if (patterngroup.is_koukushi) {
        return calculate_kokushi_yakucombo(board, trgtile, player, agarievent);
    }
    
    for (uint8_t i = 0; PatternGroup::is_value_valid(patterngroup.values[i]); i++) {
        auto arrangement = arrangement_make(patterngroup.values[i], board, player, trgtile);
        
        if (auto* chiitoitsu_arrangement = std::get_if<ChiitoitsuArr>(&arrangement)) {
            return calculate_chiitoitsu_yakucombo(board, chiitoitsu_arrangement, player, agarievent);
        } else if (auto* koukushi_arrangement = std::get_if<KoukushiArr>(&arrangement)) {
            __builtin_unreachable();
        } else if (auto* chuuren_arrangement = std::get_if<ChuurenArr>(&arrangement)) {
            return calculate_chuuren_yakucombo(board, chuuren_arrangement, player, agarievent);
        } else if (auto* normal_arrangement = std::get_if<NormalArr>(&arrangement)) {
            auto new_yakucombo = calculate_normal_yakucombo(board, normal_arrangement, player, agarievent);
            if (yakucombo.is_nil || yakucombo.score() < new_yakucombo.score()) {
                yakucombo = new_yakucombo;
            }
        }
    }
    
    return yakucombo;
}

}
