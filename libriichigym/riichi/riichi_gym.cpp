//
//  riichi_gym.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include <cstring>
#include "riichi_gym.h"
#include "riichi_board.hpp"


static riichi::ActionGroup convert_to_cpp_actiongroup(const riichi_gym_actiongroup_t& group) {
    riichi::ActionGroup cpp_group = {};
    memcpy(&cpp_group.tiles, &group.tiles, 4);
    memcpy(&cpp_group.actionkinds, &group.actionkinds, 4);
    for (uint8_t i = 0; i < 4; i++) {
        cpp_group.tips[i] = group.tips[i];
    }
    return cpp_group;
}

static riichi_gym_actiongroup_t convert_to_c_actiongroup(const riichi::ActionGroup& group) {
    riichi_gym_actiongroup_t c_group = {};
    memcpy(&c_group.tiles, &group.tiles, 4);
    memcpy(&c_group.actionkinds, &group.actionkinds, 4);
    for (uint8_t i = 0; i < 4; i++) {
        c_group.tips[i] = group.tips[i].to_ullong();
    }
    return c_group;
}


// MARK: - Extern
riichi_gym_board_t* riichi_gym_board_malloc() {
    riichi::Board* board = new riichi::Board();
    return reinterpret_cast<riichi_gym_board_t*>(board);
}


riichi_gym_board_t* riichi_gym_board_malloc_with_seed(unsigned int seed) {
    riichi::Board* board = new riichi::Board(seed);
    return reinterpret_cast<riichi_gym_board_t*>(board);
}


void riichi_gym_board_free(riichi_gym_board_t* board) {
    if (board != nullptr) {
        delete reinterpret_cast<riichi::Board*>(board);
    }
}


riichi_gym_actiongroup_t riichi_gym_board_request(riichi_gym_board_t* board) {
    if (board != nullptr) {
        return convert_to_c_actiongroup(reinterpret_cast<riichi::Board*>(board)->request());
    } else {
        return {};
    }
}

void riichi_gym_board_respound(riichi_gym_board_t* board, riichi_gym_actiongroup_t actiongroup) {
    if (board != nullptr) {
        reinterpret_cast<riichi::Board*>(board)->response(convert_to_cpp_actiongroup(actiongroup));
    }
}
