//
//  riichi_score.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/25.
//

#include "riichi_score.hpp"
#include "riichi_util.hpp"


namespace riichi {
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

uint8_t NormalArr::shuntu_count() const noexcept {
    return static_cast<uint8_t>(get_mentu_flag(0, IS_SHUNTU)) + static_cast<uint8_t>(get_mentu_flag(1, IS_SHUNTU)) + static_cast<uint8_t>(get_mentu_flag(2, IS_SHUNTU)) + static_cast<uint8_t>(get_mentu_flag(3, IS_SHUNTU));
}

uint8_t NormalArr::koutu_count() const noexcept {
    return 4 - shuntu_count();
}


uint8_t NormalArr::kantu_count() const noexcept {
    return static_cast<uint8_t>(get_mentu_flag(0, IS_KAN)) + static_cast<uint8_t>(get_mentu_flag(1, IS_KAN)) + static_cast<uint8_t>(get_mentu_flag(2, IS_KAN)) + static_cast<uint8_t>(get_mentu_flag(3, IS_KAN));
}

std::bitset<4> NormalArr::is_hidden_koutus() const noexcept {
    std::bitset<4> result = 0;
    bool could_avoid_koutu_combinded = get_pair_combinable();
    uint8_t draftee_index = 0;
    
    for (uint8_t i = 0; i < 4; i++) {
        if (get_mentu_flag(i, IS_SHUNTU) && get_mentu_combinable(i)) {
            could_avoid_koutu_combinded = true;
        } else if (!get_mentu_flag(i, IS_SHUNTU) && (!get_mentu_flag(i, IS_EXPOSED) || get_mentu_flag(i, IS_CLOSED_KAN))) {
            result.set(i);
            if (tile_downgrade(trgtile) == mentus[i]) {
                draftee_index = i;
            }
        }
    }
    if (!could_avoid_koutu_combinded) {
        result.reset(draftee_index);
    }
    return result;
}


bool NormalArr::is_pair_danki() const noexcept {
    return get_pair_combinable();
}

bool NormalArr::is_mentu_kanchan(Offset<uint8_t> index) const noexcept {
    return get_mentu_combinable(index) && get_mentu_flag(index, IS_SHUNTU) && tile_downgrade(trgtile) == tile_add(mentus[index], 1);
}

bool NormalArr::is_any_mentu_kanchan() const noexcept {
    return is_mentu_kanchan(0) || is_mentu_kanchan(1) || is_mentu_kanchan(2) || is_mentu_kanchan(3);
}

bool NormalArr::is_mentu_ryanmen(Offset<uint8_t> index) const noexcept {
    return get_mentu_combinable(index) && get_mentu_flag(index, IS_SHUNTU) && (tile_downgrade(trgtile) == mentus[index] || tile_downgrade(trgtile) == tile_add(mentus[index], 2));
}

bool NormalArr::is_any_mentu_ryanmen() const noexcept {
    return is_mentu_ryanmen(0) || is_mentu_ryanmen(1) || is_mentu_ryanmen(2) || is_mentu_ryanmen(3);
}

bool NormalArr::is_menzenchin() const noexcept {
    return (!get_mentu_flag(0, IS_EXPOSED) || get_mentu_flag(0, IS_CLOSED_KAN)) && (!get_mentu_flag(1, IS_EXPOSED) || get_mentu_flag(1, IS_CLOSED_KAN)) && (!get_mentu_flag(2, IS_EXPOSED) || get_mentu_flag(2, IS_CLOSED_KAN)) && (!get_mentu_flag(3, IS_EXPOSED) || get_mentu_flag(3, IS_CLOSED_KAN));
}

bool operator == (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept {
    return lhs.dealer_rong == rhs.dealer_rong;
}

bool operator < (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept {
    return lhs.dealer_rong < rhs.dealer_rong;;
}


PenaltyPoint YakuCombo::score() const noexcept {
    uint8_t yakuman_count = (yakus | bitset_make_containing<YAKUKIND_COUNT>(yakukind_to(YakuKind::KOUKUSHI_MUSOU), yakukind_to(YakuKind::PREFECT_KOUKUSHI_MUSOU), yakukind_to(YakuKind::SUUANKOU), yakukind_to(YakuKind::PREFECT_SUUANKOU), yakukind_to(YakuKind::DAISANGEN), yakukind_to(YakuKind::SHOUSUUSHII), yakukind_to(YakuKind::DAISUUSHII), yakukind_to(YakuKind::TSUUIISOU), yakukind_to(YakuKind::CHINROUTOU), yakukind_to(YakuKind::RYUUIISOU), yakukind_to(YakuKind::CHUUREN_POUTOU), yakukind_to(YakuKind::PREFECT_CHUUREN_POUTOU), yakukind_to(YakuKind::SUUANKOU), yakukind_to(YakuKind::TENHOU), yakukind_to(YakuKind::CHIIHOU))).count();
    if (yakuman_count > 0) {
        return {
            .dealer_rong = 48000 * yakuman_count,
            .dealer_tumo = 16000 * yakuman_count,
            .punter_rong = 32000 * yakuman_count,
            .punter_tumo_from_dealer = 16000 * yakuman_count,
            .punter_tumo_from_punter = 8000 * yakuman_count,
        };
    }
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
                if (!tile_is_yaochuu(tile)) {
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
static uint8_t board_dora_count(const Board& board, Player player, bool is_riichi) {
    uint8_t count = board.chand(player).count(Tile::M5R) + board.chand(player).count(Tile::S5R) + board.chand(player).count(Tile::P5R);
    
    for (uint8_t i = 0; i < board.mountain.dora_curr; i++) {
        Tile dora_tile = tile_cycle_next(board.mountain.outdora_tile(i));
        count += board.chand(player).count_ignoring_level(dora_tile);
    }
    if (is_riichi) {
        for (uint8_t i = 0; i < board.mountain.dora_curr; i++) {
            Tile dora_tile = tile_cycle_next(board.mountain.indora_tile(i));
            count += board.chand(player).count_ignoring_level(dora_tile);
        }
    }
    
    return count;
}

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
    
    arrangement.trgtile = trgtile;
    trgtile = tile_downgrade(trgtile);
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
    if (board.move < 4 && agarievent == AgariEvent::TUMO && board.shrine.is_empty()) {
        if (board.move == 0) {
            arrangement.yakus.set(yakukind_to(YakuKind::TENHOU));
        } else {
            arrangement.yakus.set(yakukind_to(YakuKind::CHIIHOU));
        }
    }
    
    yakucombo.is_nil = false;
    yakucombo.yakus = arrangement.yakus;
    yakucombo.is_menzen = true;
    yakucombo.fu = 0;
    yakucombo.dora_count = board_dora_count(board, player, false);
    return yakucombo;
}

static YakuCombo calculate_chuuren_yakucombo(const Board& board, ChuurenArr* arrangement, Player player, AgariEvent agarievent) {
    YakuCombo yakucombo;
    arrangement->yakus.set(yakukind_to(YakuKind::CHUUREN_POUTOU));
    
    if (arrangement->xtratile == arrangement->trgtile) {
        arrangement->yakus.set(yakukind_to(YakuKind::PREFECT_CHUUREN_POUTOU));
    }
    if (board.move < 4 && agarievent == AgariEvent::TUMO && board.shrine.is_empty()) {
        if (board.move == 0) {
            arrangement->yakus.set(yakukind_to(YakuKind::TENHOU));
        } else {
            arrangement->yakus.set(yakukind_to(YakuKind::CHIIHOU));
        }
    }
    
    yakucombo.is_nil = false;
    yakucombo.yakus = arrangement->yakus;
    yakucombo.is_menzen = true;
    yakucombo.fu = 0;
    yakucombo.dora_count = board_dora_count(board, player, false);
    return yakucombo;
}

static YakuCombo calculate_chiitoitsu_yakucombo(const Board& board, ChiitoitsuArr* arrangement, Player player, AgariEvent agarievent) {
    YakuCombo yakucombo;
    arrangement->yakus.set(yakukind_to(YakuKind::CHIITOITSU));
    
    if (board.move < 4 && agarievent == AgariEvent::TUMO && board.shrine.is_empty()) {
        if (board.move == 0) {
            arrangement->yakus.set(yakukind_to(YakuKind::TENHOU));
        } else {
            arrangement->yakus.set(yakukind_to(YakuKind::CHIIHOU));
        }
    }
    
    std::bitset<TILEKIND_COUNT> any_is_tilekinds = 0;
    bool any_is_yaochuuhai = false;
    bool all_are_yaochuuhai = true;
    
    
    for (TileKind kind: TILEKIND_CASES) {
        uint8_t cycle = tilekind_cycle(kind);
        Tile first_tile = tilekind_first_tile(kind);
        for (uint8_t i = 0; i < cycle; i++) {
            Tile tile = tile_add(first_tile, i);
            uint8_t count = board.chand(player).count_ignoring_level(tile);
            if (count <= 0) {
                continue;
            }
            
            if (tile_is_yaochuu(tile)) {
                any_is_yaochuuhai = true;
            } else {
                all_are_yaochuuhai = false;
            }
            any_is_tilekinds.set(tilekind_to(kind));
        }
    }

    
    if ((any_is_tilekinds & bitset_make_containing<TILEKIND_COUNT>(underlie(TileKind::MAN), underlie(TileKind::SOU), underlie(TileKind::PIN))) == 0) {
        arrangement->yakus.set(yakukind_to(YakuKind::TSUUIISOU));
    }
    if (all_are_yaochuuhai) {
        arrangement->yakus.set(yakukind_to(YakuKind::HONROUTOU));
    }
    if (!any_is_yaochuuhai) {
        arrangement->yakus.set(yakukind_to(YakuKind::TANYAO));
    }
    
    if ((any_is_tilekinds & bitset_make_containing<TILEKIND_COUNT>(underlie(TileKind::WIND), underlie(TileKind::SAN))) == 0 && any_is_tilekinds.count() == 1) {
        arrangement->yakus.set(yakukind_to(YakuKind::CHINITSU));
    } else if ((any_is_tilekinds & bitset_make_containing<TILEKIND_COUNT>(underlie(TileKind::MAN), underlie(TileKind::SOU), underlie(TileKind::PIN))).count() == 1) {
        arrangement->yakus.set(yakukind_to(YakuKind::HONITSU));
    }
    
    yakucombo.is_nil = false;
    yakucombo.yakus = arrangement->yakus;
    yakucombo.is_menzen = true;
    yakucombo.fu = 25;
    yakucombo.dora_count = board_dora_count(board, player, false);
    return yakucombo;
}

static YakuCombo calculate_normal_yakucombo(const Board& board, NormalArr* arrangement, Player player, AgariEvent agarievent, uint32_t value) {
    YakuCombo yakucombo;
    
    const bool is_menzen = arrangement->is_menzenchin();
    yakucombo.is_menzen = is_menzen;
    
    const uint8_t shuntu_count = arrangement->shuntu_count();
    const uint8_t koutu_count = arrangement->koutu_count();
    const uint8_t kantu_count = arrangement->kantu_count();
    const std::bitset<4> is_hidden_koutus = arrangement->is_hidden_koutus();
    const TileKind pair_tilekind = tile_kind(arrangement->pair);
    const Tile jikaze_tile = board.jikaze(player);
    const Tile bakaze_tile = board.bakaze();
    
    
    // 天和 | 地和
    if (board.move < 4 && agarievent == AgariEvent::TUMO && board.shrine.is_empty()) {
        if (board.move == 0) {
            arrangement->yakus.set(yakukind_to(YakuKind::TENHOU));
        } else {
            arrangement->yakus.set(yakukind_to(YakuKind::CHIIHOU));
        }
    }
    
    // 海底撈月 | 河底撈魚
    if (board.mountain.remaining_draw_count() == 0) {
        if (agarievent == AgariEvent::TUMO) {
            arrangement->yakus.set(yakukind_to(YakuKind::HAITEI_RAOYUE));
        } else if (agarievent == AgariEvent::RONG) {
            arrangement->yakus.set(yakukind_to(YakuKind::HOUTEI_RAOYUI));
        }
    }
    // 嶺上開花
    if (agarievent == AgariEvent::FLOWER) {
        arrangement->yakus.set(yakukind_to(YakuKind::RINSHAN_KAIHOU));
    }
    // 搶槓
    if (agarievent == AgariEvent::LOOT) {
        arrangement->yakus.set(yakukind_to(YakuKind::CHANKAN));
    }
    // 門前清自摸和
    if ((agarievent == AgariEvent::TUMO || agarievent == AgariEvent::FLOWER) && board.shrine.last_cexposed(player) == nullptr) {
        arrangement->yakus.set(yakukind_to(YakuKind::MENZENCHIN_TUMOHOU));
    }
    
    // 立直 | 両立直
    if (board.cache.cstatus(player)[Board::Cache::IS_IN_DOUBLE_RIICHI]) {
        arrangement->yakus.set(yakukind_to(YakuKind::DOUBLE_RIICHI));
    } else if (board.cache.cstatus(player)[Board::Cache::IS_IN_RIICHI]) {
        arrangement->yakus.set(yakukind_to(YakuKind::RIICHI));
    }
    // 一発
    if ((board.cache.cstatus(player) & bitset_make_containing<Board::Cache::STATUS_SIZE>(Board::Cache::IS_IN_DOUBLE_RIICHI, Board::Cache::IS_IN_RIICHI)) != 0) {
        auto* discarded = board.river.penultimate_cdiscarded(player);
        if (discarded == nullptr || !discarded->is_in_riichi) {
            arrangement->yakus.set(yakukind_to(YakuKind::IPPATSU));
        }
    }
    
    // 一盃口 | 二盃口
    if ((value & 0x20000000) == 0 && is_menzen) {
        arrangement->yakus.set(yakukind_to(YakuKind::RYANPEIKOU));
    } else if ((value & 0x10000000) == 0 && is_menzen) {
        arrangement->yakus.set(yakukind_to(YakuKind::IIPEIKOU));
    }
    
    if (shuntu_count == 4 && arrangement->is_any_mentu_ryanmen() && (tile_is_suu(arrangement->pair) || board.tile_is_kakukaze(player, arrangement->pair)) && is_menzen) {
        arrangement->yakus.set(yakukind_to(YakuKind::PINFU));
    }
    
    
    // Normal
    constexpr uint8_t INTERNAL_FLAG_WIDTH = 32;
    
    constexpr uint8_t ANY_IS_MAN = underlie(TileKind::MAN);
    constexpr uint8_t ANY_IS_SOU = underlie(TileKind::SOU);
    constexpr uint8_t ANY_IS_PIN = underlie(TileKind::PIN);
    constexpr uint8_t ANY_IS_WIND = underlie(TileKind::WIND);
    constexpr uint8_t ANY_IS_SAN = underlie(TileKind::SAN);
    
    constexpr uint8_t ANY_IS_YAOCHUU = 5;
    constexpr uint8_t ALL_ARE_YAOCHUU = 6;
    
    constexpr uint8_t ANY_IS_PAI = 7;
    constexpr uint8_t ANY_IS_FA = 8;
    constexpr uint8_t ANY_IS_CHUNG = 9;
    
    constexpr uint8_t ANY_IS_PAI_KOUKU = 10;
    constexpr uint8_t ANY_IS_FA_KOUKU = 11;
    constexpr uint8_t ANY_IS_CHUNG_KOUKU = 12;
    
    constexpr uint8_t ANY_IS_JIKAZE = 13;
    constexpr uint8_t ANY_IS_BAKAZE = 14;
    
    constexpr uint8_t ANY_IS_JIKAZE_KOUKU = 15;
    constexpr uint8_t ANY_IS_BAKAZE_KOUKU = 16;
    
    constexpr uint8_t ANY_IS_E_KOUKU = 17;
    constexpr uint8_t ANY_IS_S_KOUKU = 18;
    constexpr uint8_t ANY_IS_W_KOUKU = 19;
    constexpr uint8_t ANY_IS_N_KOUKU = 20;
    
    constexpr uint8_t ALL_ARE_JI = 21;
    constexpr uint8_t ALL_ARE_ROUTOU = 22;
    constexpr uint8_t ALL_ARE_GREEN = 23;
    
    constexpr uint8_t ALL_INCLUDING_YAOCHUU = 24;
    constexpr uint8_t ALL_INCLUDING_ROUTOU = 25;
    
    std::bitset<INTERNAL_FLAG_WIDTH> flags = bitset_make_containing<INTERNAL_FLAG_WIDTH>(ALL_ARE_YAOCHUU, ALL_ARE_JI, ALL_ARE_ROUTOU, ALL_ARE_GREEN, ALL_INCLUDING_YAOCHUU);
    
    for (uint8_t i = 0; i < 4; i++) {
        Tile key_tile = arrangement->mentus[i];
        TileKind tilekind = tile_kind(key_tile);
        uint8_t tiledigit = tile_digit(key_tile);
        
        flags |= 1 << underlie(tilekind);
        
        if (arrangement->get_mentu_flag(i, NormalArr::IS_SHUNTU)) {
            // Implicit tile is suu
            flags.reset(ALL_ARE_YAOCHUU);
            flags.reset(ALL_ARE_ROUTOU);
            flags.reset(ALL_ARE_GREEN);
            flags.reset(ALL_ARE_JI);
            if (tiledigit == 0 || tiledigit == 6) {
                flags.set(ANY_IS_YAOCHUU);
            } else if (tiledigit != 0 && tiledigit != 6) {
                flags.reset(ALL_INCLUDING_YAOCHUU);
                flags.reset(ALL_INCLUDING_ROUTOU);
            }
        } else {
            if (tile_is_yaochuu(key_tile)) {
                flags.set(ANY_IS_YAOCHUU);
            } else {
                flags.reset(ALL_ARE_YAOCHUU);
                flags.reset(ALL_INCLUDING_YAOCHUU);
            }
            
            if (!tile_is_routou(key_tile)) {
                flags.reset(ALL_ARE_ROUTOU);
                flags.reset(ALL_INCLUDING_ROUTOU);
            }
            if (!tile_is_green(key_tile)) {
                flags.reset(ALL_ARE_GREEN);
            }
            
            if (!tile_is_ji(key_tile)) {
                flags.reset(ALL_ARE_JI);
            } else {
                if (key_tile == Tile::P) {
                    flags.set(ANY_IS_PAI);
                    flags.set(ANY_IS_PAI_KOUKU);
                } else if (key_tile == Tile::F) {
                    flags.set(ANY_IS_FA);
                    flags.set(ANY_IS_FA_KOUKU);
                } else if (key_tile == Tile::C) {
                    flags.set(ANY_IS_CHUNG);
                    flags.set(ANY_IS_CHUNG_KOUKU);
                } else {
                    if (key_tile == Tile::E) {
                        flags.set(ANY_IS_E_KOUKU);
                    } else if (key_tile == Tile::S) {
                        flags.set(ANY_IS_S_KOUKU);
                    } else if (key_tile == Tile::W) {
                        flags.set(ANY_IS_W_KOUKU);
                    } else if (key_tile == Tile::N) {
                        flags.set(ANY_IS_N_KOUKU);
                    }
                    
                    if (jikaze_tile == key_tile) {
                        flags.set(ANY_IS_JIKAZE);
                        flags.set(ANY_IS_JIKAZE_KOUKU);
                    }
                    if (bakaze_tile == key_tile) {
                        flags.set(ANY_IS_BAKAZE);
                        flags.set(ANY_IS_BAKAZE_KOUKU);
                    }
                }
            }
        }
    }
    {
        Tile key_tile = arrangement->pair;
        TileKind tilekind = tile_kind(key_tile);
        
        flags |= 1 << underlie(tilekind);
        
        if (tile_is_yaochuu(key_tile)) {
            flags.set(ANY_IS_YAOCHUU);
        } else {
            flags.reset(ALL_ARE_YAOCHUU);
            flags.reset(ALL_INCLUDING_YAOCHUU);
        }
        
        if (!tile_is_routou(key_tile)) {
            flags.reset(ALL_ARE_ROUTOU);
            flags.reset(ALL_INCLUDING_ROUTOU);
        }
        if (!tile_is_green(key_tile)) {
            flags.reset(ALL_ARE_GREEN);
        }
        
        if (!tile_is_ji(key_tile)) {
            flags.reset(ALL_ARE_JI);
        } else {
            if (key_tile == Tile::P) {
                flags.set(ANY_IS_PAI);
            } else if (key_tile == Tile::F) {
                flags.set(ANY_IS_FA);
            } else if (key_tile == Tile::C) {
                flags.set(ANY_IS_CHUNG);
            } else {
                if (board.jikaze(player) == key_tile) {
                    flags.set(ANY_IS_JIKAZE);
                }
                if (board.bakaze() == key_tile) {
                    flags.set(ANY_IS_BAKAZE);
                }
            }
        }
    }
    
    // 断幺九
    if (!flags[ANY_IS_YAOCHUU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::TANYAO));
    }
    // 役牌
    if (flags[ANY_IS_PAI_KOUKU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::YAKUHAI_PAI));
    }
    if (flags[ANY_IS_FA_KOUKU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::YAKUHAI_FA));
    }
    if (flags[ANY_IS_CHUNG_KOUKU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::YAKUHAI_CHUNG));
    }
    if (flags[ANY_IS_JIKAZE_KOUKU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::YAKUHAI_JIKAZE));
    }
    if (flags[ANY_IS_BAKAZE_KOUKU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::YAKUHAI_BAKAZE));
    }
    
    // 純全帯么
    if (flags[ALL_INCLUDING_ROUTOU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::JUNCHAN_TAIYAO));
    }
    // 混老頭
    if (flags[ALL_ARE_YAOCHUU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::HONROUTOU));
    }
    // 全帯幺九
    if (!flags[ALL_INCLUDING_ROUTOU] && !flags[ALL_ARE_YAOCHUU] && flags[ALL_INCLUDING_YAOCHUU]) {
        arrangement->yakus.set(yakukind_to(YakuKind::CHANTAIYAO));
    }
    
    // 対々
    if (koutu_count == 4) {
        arrangement->yakus.set(yakukind_to(YakuKind::TOITOI));
    }
    // 三暗刻
    if (is_hidden_koutus.count() == 3) {
        arrangement->yakus.set(yakukind_to(YakuKind::SANANKOU));
    }
    // 小三元
    if ((flags & bitset_make_containing<INTERNAL_FLAG_WIDTH>(ANY_IS_PAI, ANY_IS_FA, ANY_IS_CHUNG)).count() >= 2 && pair_tilekind == TileKind::SAN) {
        arrangement->yakus.set(yakukind_to(YakuKind::SHOUSANGEN));
    }
    // 三槓子
    if (kantu_count == 3) {
        arrangement->yakus.set(yakukind_to(YakuKind::SANKANTSU));
    }
    // 清一色 | 混一色
    if ((flags & bitset_make_containing<INTERNAL_FLAG_WIDTH>(ANY_IS_MAN, ANY_IS_SOU, ANY_IS_PIN)).count() == 1) {
        if ((flags & bitset_make_containing<INTERNAL_FLAG_WIDTH>(ANY_IS_WIND, ANY_IS_SAN)).count() == 0) {
            arrangement->yakus.set(yakukind_to(YakuKind::CHINITSU));
        } else {
            arrangement->yakus.set(yakukind_to(YakuKind::HONITSU));
        }
    }
    // 四暗刻 | 四暗刻単騎
    if (is_hidden_koutus.count() == 4) {
        arrangement->yakus.set(yakukind_to(YakuKind::SUUANKOU));
        if (arrangement->trgtile == arrangement->pair) {
            arrangement->yakus.set(yakukind_to(YakuKind::PREFECT_SUUANKOU));
        }
    }
    // 大三元
    if ((flags & bitset_make_containing<INTERNAL_FLAG_WIDTH>(ANY_IS_PAI_KOUKU, ANY_IS_FA_KOUKU, ANY_IS_CHUNG_KOUKU)).count() == 3) {
        arrangement->yakus.set(yakukind_to(YakuKind::DAISANGEN));
    }
    // 小四喜 | 大四喜
    if ((flags & bitset_make_containing<INTERNAL_FLAG_WIDTH>(ANY_IS_E_KOUKU, ANY_IS_S_KOUKU, ANY_IS_W_KOUKU, ANY_IS_N_KOUKU)).count() == 3 || tile_is_kaze(arrangement->pair)) {
        arrangement->yakus.set(yakukind_to(YakuKind::SHOUSUUSHII));
    } else if ((flags & bitset_make_containing<INTERNAL_FLAG_WIDTH>(ANY_IS_E_KOUKU, ANY_IS_S_KOUKU, ANY_IS_W_KOUKU, ANY_IS_N_KOUKU)).count() == 3) {
        arrangement->yakus.set(yakukind_to(YakuKind::SHOUSUUSHII));
        arrangement->yakus.set(yakukind_to(YakuKind::DAISUUSHII));
    }
    // 字一色
    if (flags[ALL_ARE_JI]) {
        arrangement->yakus.set(yakukind_to(YakuKind::TSUUIISOU));
    }
    // 绿一色
    if (flags[ALL_ARE_GREEN]) {
        arrangement->yakus.set(yakukind_to(YakuKind::RYUUIISOU));
    }
    // 四槓子
    if (kantu_count == 4) {
        arrangement->yakus.set(yakukind_to(YakuKind::SUUKANTSU));
    }
    
    // 一気通貫
    if (shuntu_count >= 3) {
        for (TileKind tilekind: {TileKind::MAN, TileKind::SOU, TileKind::PIN}) {
            std::bitset<3> ittsu_detective = 0;
            for (uint8_t i = 0; i < 4; i++) {
                Tile key_tile = arrangement->mentus[i];
                if (arrangement->get_mentu_flag(i, NormalArr::IS_SHUNTU) && tile_kind(key_tile) == tilekind) {
                    uint8_t tiledigit = tile_digit(key_tile);
                    if (tiledigit % 3 == 0) {
                        ittsu_detective.set(tiledigit % 3);
                    }
                }
            }
            if (ittsu_detective == std::bitset<3>{ 0b111 }) {
                arrangement->yakus.set(yakukind_to(YakuKind::ITTSU));
                break;
            }
        }
    }
    
    
    // 三色同順 | 三色同刻
    if (shuntu_count >= 3 || shuntu_count <= 1) {
        bool should_find_shuntu = shuntu_count >= 3;
        std::array<std::array<uint8_t, 4>, 2> sanshoku_detective = {};
        for (uint8_t i = 0; i < 4; i++) {
            if (arrangement->get_mentu_flag(i, NormalArr::IS_SHUNTU) == should_find_shuntu) {
                Tile key_tile = arrangement->mentus[i];
                TileKind tilekind = tile_kind(key_tile);
                uint8_t tiledigit = tile_digit(key_tile);
                if (sanshoku_detective[0][0] == 0) {
                    sanshoku_detective[0][0] = tiledigit + 1;
                    sanshoku_detective[0][static_cast<uint8_t>(tilekind) + 1 - static_cast<uint8_t>(TileKind::MAN)] = 1;
                } else if (sanshoku_detective[0][0] == tiledigit + 1) {
                    sanshoku_detective[0][static_cast<uint8_t>(tilekind) + 1 - static_cast<uint8_t>(TileKind::MAN)] = 1;
                } else if (sanshoku_detective[1][0] == 0) {
                    sanshoku_detective[1][0] = tiledigit + 1;
                    sanshoku_detective[1][static_cast<uint8_t>(tilekind) + 1 - static_cast<uint8_t>(TileKind::MAN)] = 1;
                } else if (sanshoku_detective[1][0] == tiledigit + 1) {
                    sanshoku_detective[1][static_cast<uint8_t>(tilekind) + 1 - static_cast<uint8_t>(TileKind::MAN)] = 1;
                } else {
                    break;
                }
            }
        }
        if (std::any_of(sanshoku_detective.cbegin(), sanshoku_detective.cend(), [](const auto& group){
            return std::all_of(group.cbegin() + 1, group.cend(), [](uint8_t value) {
                return value != 0;
            });
        })) {
            if (should_find_shuntu) {
                arrangement->yakus.set(yakukind_to(YakuKind::SANSHOKU_DOUJUN));
            } else {
                arrangement->yakus.set(yakukind_to(YakuKind::SANSHOKU_DOUKOU));
            }
        }
    }
    
    yakucombo.fu = 20;
    for (uint8_t i = 0; i < 4; i++) {
        if (!arrangement->get_mentu_flag(i, NormalArr::IS_SHUNTU)) {
            uint8_t base = 2;
            if (arrangement->get_mentu_flag(i, NormalArr::IS_KAN)) {
                base = 8;
            }
            if (is_hidden_koutus[i]) {
                base *= 2;
            }
            if (tile_is_yaochuu(arrangement->mentus[i])) {
                base *= 2;
            }
        }
    }
    {
        if (tile_is_sangen(arrangement->pair)) {
            yakucombo.fu += 2;
        } else {
            if (jikaze_tile == board.jikaze(player)) {
                yakucombo.fu += 2;
            }
            if (bakaze_tile == board.bakaze()) {
                yakucombo.fu += 2;
            }
        }
    }
    // Yaku affecting the type of machi:
    // Pinfu, Sanankou
    if (arrangement->is_pair_danki() && arrangement->is_any_mentu_kanchan()) {
        yakucombo.fu += 2;
    }
    if (agarievent == AgariEvent::TUMO || agarievent == AgariEvent::FLOWER) {
        yakucombo.fu += !arrangement->yakus[yakukind_to(YakuKind::PINFU)] ? 2 : 0;
    } else if (is_menzen) {
        yakucombo.fu += 10;
    }
    yakucombo.fu = (yakucombo.fu + 9) / 10 * 10;
    
    yakucombo.is_nil = arrangement->yakus.none();
    yakucombo.yakus = arrangement->yakus;
    yakucombo.is_menzen = is_menzen;
    yakucombo.dora_count = board_dora_count(board, player, arrangement->yakus[yakukind_to(YakuKind::RIICHI)] || arrangement->yakus[yakukind_to(YakuKind::DOUBLE_RIICHI)]);
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
            auto new_yakucombo = calculate_normal_yakucombo(board, normal_arrangement, player, agarievent, patterngroup.values[i]);
            if (yakucombo.is_nil || yakucombo.score() < new_yakucombo.score()) {
                yakucombo = new_yakucombo;
            }
        }
    }
    
    return yakucombo;
}

}
