//
//  main.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include <iostream>
#include "riichi_gym.h"


int main(int argc, const char * argv[]) {
    riichi_gym_board_t* board = riichi_gym_board_malloc();
    
    while(true) {
        auto group = riichi_gym_board_request(board);
        
    }
    
    return 0;
}
