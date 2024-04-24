//
//  main.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include <iostream>
#include "riichi_gym.h"
#include "riichi_ui.h"


int main(int argc, const char * argv[]) {
    riichi_gym_board_t* board = riichi_gym_board_malloc();
    
    while(true) {
        riichi_gym_actiongroup_t group = riichi_gym_board_request(board);
        printf("%s", riichi_gym_render_cli_ui(board, group));
        riichi_gym_wait_actiongroup_input(&group);
        riichi_gym_board_respound(board, group);
    }
    
    return 0;
}
