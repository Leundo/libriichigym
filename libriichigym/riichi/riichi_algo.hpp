//
//  riichi_algo.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#ifndef riichi_algo_hpp
#define riichi_algo_hpp

#include "riichi_board.hpp"


namespace riichi {

bool board_can_abort(const Board&) noexcept;
bool board_can_flower(const Board&) noexcept;
bool board_can_sekan(const Board&) noexcept;
bool board_can_riichi(const Board&) noexcept;

void board_set_tumo_yakucombos(Board*) noexcept;

ActionGroup board_calculate_loot_group(const Board&) noexcept;
ActionGroup board_calculate_rong_group(const Board&) noexcept;
ActionGroup board_calculate_kan_group(const Board&) noexcept;
ActionGroup board_calculate_pon_group(const Board&) noexcept;
ActionGroup board_calculate_chi_group(const Board&) noexcept;


bool repick_tile_to_avoid_kuikae_after_exposing(const Board&, Tile&) noexcept;
void issue_permission_to_avoid_kuikae_before_discarding(Board&) noexcept;


const uint32_t* find_values_from_patterns(uint32_t) noexcept;
uint32_t calculate_hand_key(const Hand&) noexcept;

}

#endif /* riichi_algo_hpp */
