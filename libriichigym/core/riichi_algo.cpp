//
//  riichi_algo.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include "riichi_algo.hpp"


namespace riichi {

bool board_can_abort(const Board&) noexcept {
    return false;
}

bool board_can_tumo(const Board&) noexcept {
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
    
    if (exposed == NULL) {
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
        if (hand.tile_counts[i] > 0 && i != static_cast<uint8_t>(tile_upgrade(except_0)) && i != static_cast<uint8_t>(tile_upgrade(except_1)) && i != static_cast<uint8_t>(tile_downgrade(except_0)) && i != static_cast<uint8_t>(tile_downgrade(except_1))) {
            board.action_tip.set(i);
        } else {
            board.action_tip.reset(i);
        }
    }
}

}
