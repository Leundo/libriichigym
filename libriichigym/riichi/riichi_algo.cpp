//
//  riichi_algo.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include "riichi_algo.hpp"


namespace riichi {

bool Hanfu::is_zero() noexcept {
    return yakuman == 0 && han == 0;
}

bool board_can_abort(const Board&) noexcept {
    return false;
}

bool board_can_tumo(const Board& board) noexcept {
    const uint32_t* value;
    if (hand_is_agari_form(board.chand(board.current_player), &value)) {
        Hanfu hanfu = board_agari_hanfu(board, value, board.current_player, AgariWay::TUMO);
        if (!hanfu.is_zero()) {
            return true;
        }
    }
    return false;
}

bool board_can_flower(const Board&) noexcept {
    return false;
}

bool board_can_sekan(const Board&) noexcept {
    return false;
}

bool board_can_riichi(const Board&) noexcept {
    return false;
}

ActionGroup board_calculate_loot_group(const Board&) noexcept {
    ActionGroup group = {};
    return group;
}

ActionGroup board_calculate_rong_group(const Board&) noexcept {
    ActionGroup group = {};
    return group;
}

ActionGroup board_calculate_kan_group(const Board& board) noexcept {
    ActionGroup group = {};
    if (!board.mountain.can_draw()) {
        return group;
    }
    for (Offset<uint8_t> i = 0; i < PLAYER_COUNT; i++) {
        if (i == static_cast<uint8_t>(board.current_player)) {
            continue;
        }
        if (board.hands[i].count_ignoring_level(board.drop_tile) >= 3) {
            group.set(i, board.drop_tile, ActionKind::OPEN_KAN);
            break;
        }
    }
    return group;
}

ActionGroup board_calculate_pon_group(const Board& board) noexcept {
    ActionGroup group = {};
    if (!board.mountain.can_draw()) {
        return group;
    }
    for (Offset<uint8_t> i = 0; i < PLAYER_COUNT; i++) {
        if (i == static_cast<uint8_t>(board.current_player)) {
            continue;
        }
        uint8_t target_count = board.hands[i].count_ignoring_level(board.drop_tile);
        uint8_t all_count = board.hands[i].sum();
        if (target_count >= 2 && all_count > target_count) {
            group.set(i, board.drop_tile, ActionKind::PON);
            break;
        }
    }
    return group;
}

/*
 P F C: Smart chi for left, mid, or right.
 u: Pass.
 TODO: more selection
 */
ActionGroup board_calculate_chi_group(const Board& board) noexcept {
    ActionGroup group = {};
    uint8_t digit = tile_digit(board.drop_tile);
    TileKind kind = tile_kind(board.drop_tile);
    if (!board.mountain.can_draw()) {
        return group;
    }
    if (!tilekind_is_suuhai(kind)) {
        return group;
    }
    Offset<uint8_t> player = static_cast<uint8_t>(player_cycle_next(board.current_player));
    const Hand& hand = board.chand(player);
    
    Tile p_tile = tile_cycle_prev(board.drop_tile);
    Tile pp_tile = tile_cycle_prev(p_tile);
    Tile ppp_tile = tile_cycle_prev(pp_tile);
    Tile n_tile = tile_cycle_next(board.drop_tile);
    Tile nn_tile = tile_cycle_next(n_tile);
    Tile nnn_tile = tile_cycle_next(nn_tile);
    
    uint8_t a_count = hand.sum();
    uint8_t d_count = hand.count_ignoring_level(board.drop_tile);
    uint8_t p_count = hand.count_ignoring_level(p_tile);
    uint8_t pp_count = hand.count_ignoring_level(pp_tile);
    uint8_t ppp_count = hand.count_ignoring_level(ppp_tile);
    uint8_t n_count = hand.count_ignoring_level(n_tile);
    uint8_t nn_count = hand.count_ignoring_level(nn_tile);
    uint8_t nnn_count = hand.count_ignoring_level(nnn_tile);
    
    bool can_chi = false;
    if (digit + 2 == 8 && n_count > 0 && nn_count > 0 && a_count > 2 + d_count) {
        can_chi = true;
        group.tip_insert(player, Tile::P);
    }
    if (digit + 2 <= 7 && n_count > 0 && nn_count > 0 && a_count > 2 + d_count + nnn_count) {
        can_chi = true;
        group.tip_insert(player, Tile::P);
    }
    if (digit + 1 <= 8 && digit >= 1 && n_count > 0 && p_count > 0 && a_count > 2 + d_count) {
        can_chi = true;
        group.tip_insert(player, Tile::F);
    }
    if (digit == 2 && p_count > 0 && pp_count > 0 && a_count > 2 + d_count) {
        can_chi = true;
        group.tip_insert(player, Tile::C);
    }
    if (digit >= 3 && p_count > 0 && pp_count > 0 && a_count > 2 + d_count + ppp_count) {
        can_chi = true;
        group.tip_insert(player, Tile::C);
    }
    
    if (can_chi) {
        group.set(player, board.drop_tile, ActionKind::CHI);
    }
    
    return group;
}


bool repick_tile_to_avoid_kuikae_after_exposing(const Board& board, Tile& tile) noexcept {
    const Exposed* exposed = board.shrine.last_cexposed(board.current_player);
    if (exposed == nullptr || board.move == 0 || exposed->move < board.move - 1 || tile_is_undefined(tile)) {
        return false;
    }
    
    if (exposed->naki == Naki::PON) {
        if (tile_downgrade(tile) == tile_downgrade(exposed->keytile)) {
            const Hand& hand = board.chand(board.current_player);
            Tile met_tiled = hand.first_tile_except(tile_downgrade(exposed->keytile), tile_upgrade(exposed->keytile));
            if (tile_is_undefined(met_tiled)) {
                return false;
            } else {
                tile = met_tiled;
                return true;
            }
        } else {
            return false;
        }
    } else if (exposed->naki == Naki::CHI) {
        Tile genbutu = tile_downgrade(exposed->trgtile);
        Tile suji = Tile::UND;
        if (genbutu == tile_downgrade(exposed->keytile)) {
            suji = tile_linear_next(tile_linear_next(tile_linear_next(exposed->keytile)));
        } else if (genbutu == tile_linear_next(tile_linear_next(exposed->keytile))) {
            suji = tile_linear_prev(exposed->keytile);
        }
        if (tile_is_undefined(tile) || tile == genbutu || tile == suji) {
            const Hand& hand = board.chand(board.current_player);
            Tile met_tiled = hand.first_tile_except(tile_downgrade(genbutu), tile_downgrade(suji), tile_upgrade(genbutu), tile_upgrade(suji));
            if (tile_is_undefined(met_tiled)) {
                return false;
            } else {
                tile = met_tiled;
                return true;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    return false;
}

void issue_permission_to_avoid_kuikae_before_discarding(Board& board) noexcept {
    const Exposed* exposed = board.shrine.last_cexposed(board.current_player);
    Tile except_0 = Tile::UND;
    Tile except_1 = Tile::UND;
    
    if (exposed == nullptr) {
        // Nothing
    } else if (board.move == 0 || exposed->move < board.move - 1) {
        // Nothing
    } else if (exposed->naki == Naki::PON) {
        except_0 = exposed->trgtile;
    } else if (exposed->naki == Naki::CHI) {
        except_0 = exposed->trgtile;
        if (exposed->trgtile == exposed->keytile) {
            except_1 = tile_linear_next(tile_linear_next(tile_linear_next(exposed->keytile)));
        } else if (exposed->trgtile == tile_linear_next(tile_linear_next(exposed->keytile))) {
            except_1 = tile_linear_prev(exposed->keytile);
        }
    }
    
    const Hand& hand = board.chand(board.current_player);
    for (uint8_t i = 0; i < hand.tile_counts.size(); i++) {
        if (hand.tile_counts[i] > 0 && i != tile_to(tile_upgrade(except_0)) && i != tile_to(tile_upgrade(except_1)) && i != tile_to(tile_downgrade(except_0)) && i != tile_to(tile_downgrade(except_1))) {
            board.action_tip.set(i);
        } else {
            board.action_tip.reset(i);
        }
    }
}


// MARK: - Combo
/*
 mentu_statu:
 0: koutu or shuntu. 0: koutu, 1: shuntu.
 1: is kan.
 2: is not closed.
 3: is closed kan.
 
 mode:
 0: normal.
 1: chiitoitu.
 2: chuuren.
 
 triggered:
 0 .. 3: mentu
 4: pair
 */
class Combo {
public:
    Tile mentu_tiles[4];
    uint8_t mentu_status[4];
    Tile pair_tile;
    uint8_t combinable_set;
    uint8_t mode;
    
    bool is_koutu(Offset<uint8_t> index) {
        return !uoptset_contain(mentu_status[index], 0);
    }
    
    static Combo make(uint32_t value, const Board& board, Player player, Tile trgtile) {
        Combo combo = {};
        
        if ((value & 0x01000000) != 0) {
            combo.mode = 1;
            uint8_t tile_index = 0;
            for (uint8_t tile = tile_to(TILE_FIRST); tile <= tile_to(TILE_LAST_IGNORING_LEVEL); tile++) {
                if (board.chand(player).count(tile) > 0) {
                    if (tile_index < 4) {
                        combo.mentu_tiles[tile_index] = tile_from(tile);
                    } else if (tile_index < 8) {
                        combo.mentu_status[tile_index] = tile;
                    }
                    tile_index += 1;
                }
            }
            return combo;
        } else if ((value & 0x02000000) != 0) {
            combo.mode = 2;
            Tile tile = tilekind_first_tile(tile_kind(board.chand(player).first_tile()));
            combo.mentu_tiles[0] = tile;
            
            if (board.chand(player).count(tile) > 3) {
                combo.mentu_tiles[1] = tile;
            } else if (board.chand(player).count(tile) > 3) {
                combo.mentu_tiles[1] = tile_add(tile, 8);
            } else {
                for (uint8_t i = 1; i < 8; i++) {
                    if (board.chand(player).count_ignoring_level(tile_add(tile, i)) > 1) {
                        combo.mentu_tiles[1] = tile_add(tile, i);
                        break;
                    }
                }
            }
            return combo;
        }
        
        memset(&combo.mentu_status, 0, 4);
        combo.mode = 0;
        
        for (uint8_t i = 0; i < 4; i++) {
            if (uoptset_contain(value, i + 4)) {
                uoptset_insert(&combo.mentu_status[i], 0);
            }
        }
        uint8_t pair_index = uoptset_extract(value, 4, 0);
        uint8_t mentu_pointer = 0;
        uint8_t mentu_index = uoptset_extract(value, 4, (mentu_pointer + 2) * 4);
        uint8_t appear_count = 0;
        
        for (uint8_t tile = tile_to(TILE_FIRST); tile <= tile_to(TILE_LAST_IGNORING_LEVEL); tile++) {
            if (board.chand(player).count_ignoring_level(tile) > 0) {
                if (pair_index == appear_count) {
                    combo.pair_tile = tile_from(tile);
                }
                while (mentu_index == appear_count) {
                    combo.mentu_tiles[mentu_pointer] = tile_from(tile);
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
            uint8_t mentu_index = 3 - i;
            combo.mentu_tiles[mentu_index] = exposed->keytile;
            switch (exposed->naki) {
                case Naki::PON: {
                    uoptset_insert(&combo.mentu_status[mentu_index], 2);
                    break;
                }
                case Naki::CHI: {
                    uoptset_insert(&combo.mentu_status[mentu_index], 0);
                    uoptset_insert(&combo.mentu_status[mentu_index], 2);
                    break;
                }
                case Naki::OPEN_KAN: {
                    uoptset_insert(&combo.mentu_status[mentu_index], 1);
                    uoptset_insert(&combo.mentu_status[mentu_index], 2);
                    break;
                }
                case Naki::ADD_KAN: {
                    uoptset_insert(&combo.mentu_status[mentu_index], 1);
                    uoptset_insert(&combo.mentu_status[mentu_index], 2);
                    break;
                }
                case Naki::CLOSED_KAN: {
                    uoptset_remove(&combo.mentu_status[mentu_index], 0);
                    uoptset_insert(&combo.mentu_status[mentu_index], 1);
                    uoptset_insert(&combo.mentu_status[mentu_index], 3);
                    break;
                }
                case Naki::UND: {
                    break;
                }
            }
        }
        
        combo.combinable_set = 0;
        trgtile = tile_downgrade(trgtile);
        for (uint8_t i = 0; i < 4; i++) {
            if (uoptset_contain(combo.mentu_status[i], 2) || uoptset_contain(combo.mentu_status[i], 3)) {
                continue;
            }
            if (combo.is_koutu(i) && trgtile == combo.mentu_tiles[i]) {
                uoptset_insert(&combo.combinable_set, i);
            } else if (!combo.is_koutu(i) && tile_to(trgtile) >= tile_to(combo.mentu_tiles[i]) && tile_to(trgtile) <= tile_to(combo.mentu_tiles[i]) + 2) {
                uoptset_insert(&combo.combinable_set, i);
            }
        }
        if (trgtile == combo.pair_tile) {
            uoptset_insert(&combo.combinable_set, 4);
        }
        
        return combo;
    }
};


// MARK: - Agari
static uint32_t calculate_hand_key(const Hand& hand) {
    uint32_t key = 0;
    int32_t position = -1;
    uint32_t state = 0; /* sleep, seq, cut */
    
    for (TileKind kind: TILEKIND_CASES) {
        uint8_t cycle = tilekind_cycle(kind);
        Tile first_tile = tilekind_first_tile(kind);
        bool is_jihai = tilekind_is_jihai(kind);
        
        for (uint8_t i = 0; i < cycle; i++) {
            Tile tile = tile_add(first_tile, i);
            uint8_t count = hand.count_ignoring_level(tile);
            if (count > 0) {
                state = 1;
                position += 1;
                if (count == 2) {
                    key |= 0b11 << position;
                    position += 2;
                } else if (count == 3) {
                    key |= 0b1111 << position;
                    position += 4;
                } else if (count == 4) {
                    key |= 0b111111 << position;
                    position += 6;
                }
                if (is_jihai) {
                    state = 2;
                    key |= 0b1 << position;
                    position += 1;
                }
            } else if (state == 1) {
                state = 2;
                key |= 0b1 << position;
                position += 1;
            }
        }
        if (state == 1) {
            state = 2;
            key |= 0b1 << position;
            position += 1;
        }
    }
    
    return key;
}

bool hand_is_agari_form(const Hand& hand, const uint32_t* values[]) {
    bool are_all_yaochuuhais = true;
    uint8_t kokushi_detective = 0;
    uint8_t sum = hand.sum();
    
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
    
    if (unlikely(kokushi_detective <= 1 && are_all_yaochuuhais && sum == 14)) {
        if (values != nullptr) {
            *values = nullptr;
        }
        return true;
    }
    
    uint32_t key = calculate_hand_key(hand);
    const uint32_t* found_values = find_values_from_patterns(key);
    if (values != nullptr) {
        *values = found_values;
    }
    
    return found_values != nullptr;
}


static Hanfu board_kokushi_hanfu(const Board& board, Tile trgtile, Player player, AgariWay agariway) {
    Hanfu hanfu = { .yakuman = 1 };
    if (board.chand(player).count(trgtile) == 2) {
        hanfu.yakuman += 1;
    }
    if (board.move < 4 && agariway == AgariWay::TUMO && board.shrine.last_cexposed(player) == nullptr) {
        hanfu.yakuman += 1;
    }
    return hanfu;
}

static Hanfu board_chuuren_hanfu(const Board& board, const Combo& combo, Tile trgtile, Player player, AgariWay agariway) {
    Hanfu hanfu = {.yakuman = 1};
    if (combo.mentu_tiles[1] == trgtile) {
        hanfu.yakuman += 1;
    }
    if (board.move < 4 && agariway == AgariWay::TUMO && board.shrine.last_cexposed(player) == nullptr) {
        hanfu.yakuman += 1;
    }
    return hanfu;
}

static Hanfu board_chiitoitu_hanfu(const Board& board, const Combo& combo, Tile trgtile, Player player, AgariWay agariway) {
    Hanfu hanfu = {};
    if (board.move < 4 && agariway == AgariWay::TUMO && board.shrine.last_cexposed(player) == nullptr) {
        hanfu.yakuman += 1;
        return hanfu;
    }
    return hanfu;
}

Hanfu board_agari_hanfu(const Board& board, const uint32_t* values, Player player, AgariWay agariway) {
    Hanfu hanfu = {};
    uint8_t value_index = 0;
    
    const Hand& hand = board.chand(player);
    Tile trgtile;
    switch (agariway) {
        case AgariWay::TUMO:
        case AgariWay::FLOWER: {
            trgtile = board.hold_tile;
            break;
        }
        case AgariWay::RONG:
        case AgariWay::LOOT: {
            trgtile = board.drop_tile;
            break;
        }
    }
    
    if (values == nullptr) {
        return board_kokushi_hanfu(board, trgtile, player, agariway);
    }
    
    for (uint8_t i = 0; (values[i] & 0x80000000) != 0 && (values[i] & 0x000000FF) != 0x000000FF; i++) {
        Combo combo = Combo::make(values[i], board, player, trgtile);
        if (combo.mode == 1) {
            return board_chiitoitu_hanfu(board, combo, trgtile, player, agariway);
        } else if (combo.mode == 2) {
            return board_chuuren_hanfu(board, combo, trgtile, player, agariway);
        }
    }
    
    return hanfu;
}

}
