//
//  riichi_algo.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include <algorithm>
#include "riichi_algo.hpp"
#include "riichi_score.hpp"


namespace riichi {

bool board_can_abort(const Board&) noexcept {
    return false;
}

bool board_set_tumo_yakucombos(Board& board) noexcept {
    auto patterngroup = hand_patterngroup(board.chand(board.current_player));
    if (patterngroup.has_value()) {
        auto yakucombo = board_yakucombo(board, patterngroup.value(), board.current_player, AgariEvent::TUMO);
        board.combo(board.current_player) = yakucombo;
        if (yakucombo.has_value()) {
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

ActionGroup board_calculate_rong_group_and_set_yakucombos(Board& board) noexcept {
    ActionGroup group = {};
    for (Offset<uint8_t> i = 0; i < PLAYER_COUNT; i++) {
        if (i == static_cast<uint8_t>(board.current_player)) {
            continue;
        }
        board.hand(i).increase(board.drop_tile);
        if (auto patterngroup = hand_patterngroup(board.chand(board.current_player)); patterngroup.has_value()) {
            auto yakucombo = board_yakucombo(board, patterngroup.value(), board.current_player, AgariEvent::TUMO);
            board.combo(board.current_player) = yakucombo;
            if (yakucombo.has_value()) {
                group.set(i, board.drop_tile, ActionKind::AGARI);
            }
        }
        board.hand(i).decrease(board.drop_tile);
    }
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

void board_restart(Board& board) noexcept {
    switch (board.session % 4) {
        case 0: { board.current_player = Player::P0; break; }
        case 1: { board.current_player = Player::P1; break; }
        case 2: { board.current_player = Player::P2; break; }
        case 3: { board.current_player = Player::P3; break; }
    }
    
    board.mountain = {};
    board.river = {};
    board.shrine = {};
    board.hands = {};
    
    board.move = 0;
    board.life = 1 << static_cast<uint8_t>(Board::LifeIndex::RUN);
    board.hold_tile = Tile::UND;
    board.drop_tile = Tile::UND;
    board.action_permission = 0;
    board.action_tip = 0;
    board.cache = Board::Cache();
    
    board.shuffle(board.mountain, board.random_generator);

    for (uint8_t i = 0; i < PLAYER_COUNT; i++) {
        for (uint8_t j = 0; j < 13; j++) {
            Tile tile = board.mountain.draw(false);
            board.hands[i].increase(tile);
            board.cache.fogs[i].decrease(tile);
        }
        board.cache.fogs[i].decrease(board.mountain.outdora_tile(0));
    }
    
    board.stage = Board::Stage::DRAW;
}

void board_calculate_and_update_score(Board& board) noexcept {
    if ((board.life & bitset_make_containing<8>(underlie(Board::LifeIndex::P0_AGARI), underlie(Board::LifeIndex::P1_AGARI), underlie(Board::LifeIndex::P2_AGARI), underlie(Board::LifeIndex::P3_AGARI))).count() > 0) {
        if (board.life[underlie(board.current_player)]) {
            // Tumo
            auto point = board.combos[underlie(board.current_player)].transform([](const auto& combo) {
                return combo.score();
            }).value_or(PenaltyPoint {});
            if (board.dealer() == board.current_player) {
                auto punters = board.punters();
                board.scores[underlie(board.current_player)] += 3 * point.dealer_tumo + board.honba * 300 + static_cast<int32_t>(board.tribute) * 1000;
                board.scores[underlie(std::get<0>(punters))] -= point.dealer_tumo + board.honba * 100;
                board.scores[underlie(std::get<1>(punters))] -= point.dealer_tumo + board.honba * 100;
                board.scores[underlie(std::get<2>(punters))] -= point.dealer_tumo + board.honba * 100;
                board.honba += 1;
            } else {
                auto punters = board.punters_expect(board.current_player);
                board.scores[underlie(board.current_player)] += 2 * point.punter_tumo_from_punter + point.punter_tumo_from_dealer + board.honba * 300 + static_cast<int32_t>(board.tribute) * 1000;
                board.scores[underlie(board.dealer())] -= point.punter_tumo_from_dealer + board.honba * 100;
                board.scores[underlie(std::get<0>(punters))] -= point.punter_tumo_from_punter + board.honba * 100;
                board.scores[underlie(std::get<1>(punters))] -= point.punter_tumo_from_punter + board.honba * 100;
                board.honba = 0;
                board.session += 1;
            }
        } else {
            // Rong
            bool does_dealer_rong = false;
            for (uint8_t offset = 1; offset < PLAYER_COUNT; offset++) {
                uint8_t i = (underlie(board.current_player) + offset) % PLAYER_COUNT;
                if (board.life[i] && board.combos[i].has_value()) {
                    auto point = board.combos[i].value().score();
                    if (board.dealer() == static_cast<Player>(i)) {
                        board.scores[underlie(board.current_player)] -= point.dealer_rong + board.honba * 300 + static_cast<int32_t>(board.tribute) * 1000;
                        board.scores[i] += point.dealer_rong + board.honba * 300;
                        does_dealer_rong = true;
                    } else {
                        board.scores[underlie(board.current_player)] -= point.dealer_rong + board.honba * 300 + static_cast<int32_t>(board.tribute) * 1000;
                        board.scores[i] += point.punter_rong + board.honba * 300;
                    }
                    board.tribute = 0;
                }
            }
            if (does_dealer_rong) {
                board.honba += 1;
            } else {
                board.honba = 0;
                board.session += 1;
            }
        }
        board.tribute = 0;
    } else {
        board.honba += 1;
    }
}

bool board_clean_and_can_restart(Board& board) noexcept {
    bool can_restart = board.session < 4 && std::all_of(board.scores.cbegin(), board.scores.cend(), [](auto score) {
        return score > 0;
    });
    if (can_restart) {
        board_restart(board);
    }
    return can_restart;
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


uint32_t calculate_hand_key(const Hand& hand) noexcept {
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

}
