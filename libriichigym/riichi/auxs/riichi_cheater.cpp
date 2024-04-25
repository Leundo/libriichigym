//
//  riichi_cheater.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/25.
//

#include "riichi_cheater.h"
#include "riichi_board.hpp"


constexpr riichi::Tile M1 = riichi::Tile::M1;
constexpr riichi::Tile M2 = riichi::Tile::M2;
constexpr riichi::Tile M3 = riichi::Tile::M3;
constexpr riichi::Tile M4 = riichi::Tile::M4;
constexpr riichi::Tile M5 = riichi::Tile::M5;
constexpr riichi::Tile M6 = riichi::Tile::M6;
constexpr riichi::Tile M7 = riichi::Tile::M7;
constexpr riichi::Tile M8 = riichi::Tile::M8;
constexpr riichi::Tile M9 = riichi::Tile::M9;
constexpr riichi::Tile S1 = riichi::Tile::S1;
constexpr riichi::Tile S2 = riichi::Tile::S2;
constexpr riichi::Tile S3 = riichi::Tile::S3;
constexpr riichi::Tile S4 = riichi::Tile::S4;
constexpr riichi::Tile S5 = riichi::Tile::S5;
constexpr riichi::Tile S6 = riichi::Tile::S6;
constexpr riichi::Tile S7 = riichi::Tile::S7;
constexpr riichi::Tile S8 = riichi::Tile::S8;
constexpr riichi::Tile S9 = riichi::Tile::S9;
constexpr riichi::Tile P1 = riichi::Tile::P1;
constexpr riichi::Tile P2 = riichi::Tile::P2;
constexpr riichi::Tile P3 = riichi::Tile::P3;
constexpr riichi::Tile P4 = riichi::Tile::P4;
constexpr riichi::Tile P5 = riichi::Tile::P5;
constexpr riichi::Tile P6 = riichi::Tile::P6;
constexpr riichi::Tile P7 = riichi::Tile::P7;
constexpr riichi::Tile P8 = riichi::Tile::P8;
constexpr riichi::Tile P9 = riichi::Tile::P9;
constexpr riichi::Tile EE = riichi::Tile::E;
constexpr riichi::Tile SS = riichi::Tile::S;
constexpr riichi::Tile WW = riichi::Tile::W;
constexpr riichi::Tile NN = riichi::Tile::N;
constexpr riichi::Tile PP = riichi::Tile::P;
constexpr riichi::Tile FF = riichi::Tile::F;
constexpr riichi::Tile CC = riichi::Tile::C;
constexpr riichi::Tile MR = riichi::Tile::M5R;
constexpr riichi::Tile SR = riichi::Tile::S5R;
constexpr riichi::Tile PR = riichi::Tile::P5R;


#define SEQUENTIAL_TILES { \
M1, M1, M1, M1, M2, M2, M2, M2, M3, M3, M3, M3, M4,\
M4, M4, M4, MR, M5, M5, M5, M6, M6, M6, M6, M7, M7,\
M7, M7, M8, M8, M8, M8, M9, M9, M9, M9, S1, S1, S1,\
S1, S2, S2, S2, S2, S3, S3, S3, S3, S4, S4, S4, S4,\
SR, S5, S5, S5, S6, S6, S6, S6, S7, S7, S7, S7, S8, S8, S8, S8,\
S9, S9, S9, S9, P1, P1, P1, P1, P2, P2, P2, P2, P3, P3, P3, P3,\
P4, P4, P4, P4, PR, P5, P5, P5, P6, P6, P6, P6, P7, P7, P7, P7,\
P8, P8, P8, P8, P9, P9, P9, P9, EE, EE, EE, EE, SS, SS, SS, SS,\
WW, WW, WW, WW, NN, NN, NN, NN, PP, PP, PP, PP, FF, FF, FF, FF,\
CC, CC, CC, CC,\
}

#define FIRST_TURN_OPEN_KAN_TILES { \
M1, M1, M1, M1, M2, M2, M2, M2, M3, M3, M3, M3, M4,\
M4, M4, M4, SR, M5, M5, M5, M6, M6, M6, M6, M7, M7,\
M7, M7, M8, M8, M8, M8, M9, M9, M9, M9, S1, S1, S1,\
S1, S2, S2, S2, S2, S3, S3, S3, S3, S4, S4, S4, S4,\
MR, S5, S5, S5, S6, S6, S6, S6, S7, S7, S7, S7, S8, S8, S8, S8,\
S9, S9, S9, S9, P1, P1, P1, P1, P2, P2, P2, P2, P3, P3, P3, P3,\
P4, P4, P4, P4, PR, P5, P5, P5, P6, P6, P6, P6, P7, P7, P7, P7,\
P8, P8, P8, P8, P9, P9, P9, P9, EE, EE, EE, EE, SS, SS, SS, SS,\
WW, WW, WW, WW, NN, NN, NN, NN, PP, PP, PP, PP, FF, FF, FF, FF,\
CC, CC, CC, CC,\
}


void riichi_gym_board_false_shuffle(riichi_gym_board_t* c_board, int mode) {
    auto* board = reinterpret_cast<riichi::Board*>(c_board);
    
    switch (mode) {
        case 0: {
            board->shuffle = [](riichi::Mountain& mountain, std::mt19937&) noexcept -> void { mountain.tiles = SEQUENTIAL_TILES; };
            break;
        }
        case 1: {
            board->shuffle = [](riichi::Mountain& mountain, std::mt19937&) noexcept -> void { mountain.tiles = FIRST_TURN_OPEN_KAN_TILES; };
            break;
        }
        default: {
            break;
        }
    }
}
