//
//  riichi_ui.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/24.
//

#include <cstring>
#include "riichi_ui.h"
#include "riichi_config.hpp"
#include "riichi_util.hpp"
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

constexpr std::size_t buffer_size = 2048;
static char buffer[buffer_size];
static std::size_t buffer_offset = 0;

static void buffer_clear() {
    buffer[0] = 0;
    buffer_offset = 0;
}

template <typename ... Args>
static void buffer_write(Args ... args) {
    buffer_offset += snprintf(buffer + buffer_offset, buffer_size - buffer_offset, args...);
}


// MARK: - Extern
const char* riichi_gym_render_cli_ui(const riichi_gym_board_t* c_board, riichi_gym_actiongroup_t c_group) {
    buffer_clear();
    if (c_board == nullptr) { return buffer; }
    auto group = convert_to_cpp_actiongroup(c_group);
    const auto* board = reinterpret_cast<const riichi::Board*>(c_board);
    
    // Info
    buffer_write("Seed: 0x%016X | ", board->seed);
    buffer_write("Ba: ");
    switch ((board->session / 4) % 4) {
        case 0: { buffer_write("E%d_%d | ", board->session % 4 + 1, board->honba); break;}
        case 1: { buffer_write("S%d_%d | ", board->session % 4 + 1, board->honba); break;}
        case 2: { buffer_write("W%d_%d | ", board->session % 4 + 1, board->honba); break;}
        case 3: { buffer_write("N%d_%d | ", board->session % 4 + 1, board->honba); break;}
    }
    buffer_write("Score: ");
    buffer_write("%6d %6d %6d %6d\n", board->scores[0], board->scores[1], board->scores[2], board->scores[3]);
    
    
    buffer_write("Player: P%d | ", board->current_player);
    buffer_write("Yama: %2d | ", board->mountain.remaining_draw_count());
    
    buffer_write("Hint:");
    for (uint8_t i = 0; i < board->mountain.dora_visible_count(); i++) {
        buffer_write("%2s ", riichi::tile_describe(board->mountain.outdora_tile(i)));
    }
    buffer_write("\nRivier:\n");
    
    constexpr uint8_t river_tile_count_per_line = 5;
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t line_head_index = i * river_tile_count_per_line;
        
        for (uint8_t j = 0; j < RIICHI_GYM_PLAYER_COUNT; j++) {
            if (j > 0) {
                buffer_write("     ");
            }
            for (uint8_t k = 0; k < river_tile_count_per_line; k++) {
                const auto* discarded = board->river.cdiscarded(j, line_head_index + k);
                if (discarded == nullptr) {
                    buffer_write("   ");
                } else {
                    buffer_write("%2s ", riichi::tile_describe(discarded->tile));
                }
            }
            if (j == RIICHI_GYM_PLAYER_COUNT - 1) {
                buffer_write("\n");
            }
        }
    }
    
    // Shrine
    buffer_write("Shrine:\n");
    for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
        for (uint8_t j = 0; j < 4; j++) {
            const auto* exposed = board->shrine.cexposed(i, j);
            if (exposed == nullptr) {
                buffer_write("    ");
                continue;
            }
            switch (exposed->naki) {
                case riichi::Naki::PON: {
                    buffer_write("%2sP ", riichi::tile_describe(exposed->keytile));
                    break;
                }
                case riichi::Naki::CHI: {
                    buffer_write("%2sC ", riichi::tile_describe(exposed->keytile));
                    break;
                }
                case riichi::Naki::OPEN_KAN: {
                    buffer_write("%2sK ", riichi::tile_describe(exposed->keytile));
                    break;
                }
                case riichi::Naki::ADD_KAN: {
                    buffer_write("%2s+ ", riichi::tile_describe(exposed->keytile));
                    break;
                }
                case riichi::Naki::CLOSED_KAN: {
                    buffer_write("%2sA ", riichi::tile_describe(exposed->keytile));
                    break;
                }
                default: {
                    buffer_write("    ");
                    break;
                }
            }
        }
        
        if (i == RIICHI_GYM_PLAYER_COUNT - 1) {
            buffer_write("\n");
        } else {
            buffer_write("    ");
        }
    }
    
    // Hands
    buffer_write("Hands:\nE S W N P F C 8 9   E S W N P F C 8 9   E S W N P F C 8 9   E S W N P F C 8 9\n");
    const auto hands = board->hands;
    
    for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
        if (i > 0) {
            buffer_write("   ");
        }
        buffer_write("%X %X %X %X %X %X %X    ", hands[i].count(riichi::Tile::E), hands[i].count(riichi::Tile::S), hands[i].count(riichi::Tile::W), hands[i].count(riichi::Tile::N), hands[i].count(riichi::Tile::P), hands[i].count(riichi::Tile::F), hands[i].count(riichi::Tile::C));
        if (i == RIICHI_GYM_PLAYER_COUNT - 1) {
            buffer_write("\n");
        }
    }
    for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
        if (i > 0) {
            buffer_write("M  ");
        }
        buffer_write("%X %X %X %X %X %X %X %X %X", hands[i].count(riichi::Tile::M1), hands[i].count(riichi::Tile::M2), hands[i].count(riichi::Tile::M3), hands[i].count(riichi::Tile::M4), hands[i].count(riichi::Tile::M5) + 10 * hands[i].count(riichi::Tile::M5R), hands[i].count(riichi::Tile::M6), hands[i].count(riichi::Tile::M7), hands[i].count(riichi::Tile::M8), hands[i].count(riichi::Tile::M9));
        if (i == RIICHI_GYM_PLAYER_COUNT - 1) {
            buffer_write("M\n");
        }
    }
    for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
        if (i > 0) {
            buffer_write("S  ");
        }
        buffer_write("%X %X %X %X %X %X %X %X %X", hands[i].count(riichi::Tile::S1), hands[i].count(riichi::Tile::S2), hands[i].count(riichi::Tile::S3), hands[i].count(riichi::Tile::S4), hands[i].count(riichi::Tile::S5) + 10 * hands[i].count(riichi::Tile::S5R), hands[i].count(riichi::Tile::S6), hands[i].count(riichi::Tile::S7), hands[i].count(riichi::Tile::S8), hands[i].count(riichi::Tile::S9));
        if (i == RIICHI_GYM_PLAYER_COUNT - 1) {
            buffer_write("S\n");
        }
    }
    for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
        if (i > 0) {
            buffer_write("P  ");
        }
        buffer_write("%X %X %X %X %X %X %X %X %X", hands[i].count(riichi::Tile::P1), hands[i].count(riichi::Tile::P2), hands[i].count(riichi::Tile::P3), hands[i].count(riichi::Tile::P4), hands[i].count(riichi::Tile::P5) + 10 * hands[i].count(riichi::Tile::P5R), hands[i].count(riichi::Tile::P6), hands[i].count(riichi::Tile::P7), hands[i].count(riichi::Tile::P8), hands[i].count(riichi::Tile::P9));
        if (i == RIICHI_GYM_PLAYER_COUNT - 1) {
            buffer_write("P\n");
        }
    }
    
    // Space
    buffer_write("\nSpace:\n");
    for (uint8_t tile = static_cast<uint8_t>(riichi::Tile::E); tile <= static_cast<uint8_t>(riichi::Tile::C); tile++) {
        if (board->action_tip[tile] == false) {
            buffer_write("- ");
        } else {
            buffer_write("%c ", riichi::tile_describe(static_cast<riichi::Tile>(tile))[0] - 'a' + 'A');
        }
    }
    buffer_write("      ");
    for (uint8_t tile = static_cast<uint8_t>(riichi::Tile::M1); tile <= static_cast<uint8_t>(riichi::Tile::P9); tile++) {
        uint8_t digit = tile % 9;
        if (digit == 4) {
            if (board->action_tip[tile] == false && board->action_tip[static_cast<uint8_t>(riichi::tile_upgrade(static_cast<riichi::Tile>(tile)))] == false) {
                buffer_write("- ");
            } else if (board->action_tip[tile] == true && board->action_tip[static_cast<uint8_t>(riichi::tile_upgrade(static_cast<riichi::Tile>(tile)))] == false) {
                buffer_write("5 ");
            } else if (board->action_tip[tile] == false && board->action_tip[static_cast<uint8_t>(riichi::tile_upgrade(static_cast<riichi::Tile>(tile)))] == true) {
                buffer_write("r ");
            } else {
                buffer_write("R ");
            }
        } else if (board->action_tip[tile] == false) {
            buffer_write("- ");
        } else {
            buffer_write("%d ", digit + 1);
        }
        
        if (digit == 8) {
            buffer_write("  ");
        }
    }
    buffer_write("\n");
    
    for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
        riichi::Tile tile = group.tiles[i];
        riichi::ActionKind actionkind = group.actionkinds[i];
        switch (actionkind) {
            case riichi::ActionKind::DISCARD: {
                buffer_write("DIS - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::RIICHI: {
                buffer_write("RII - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::CHI: {
                buffer_write("CHI - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::PON: {
                buffer_write("PON - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::OPEN_KAN: {
                buffer_write("KAN - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::SELF_KAN: {
                buffer_write("SEK - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::AGARI: {
                buffer_write("AGA - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::ABORT: {
                buffer_write("ABT - %2s", tile_describe(tile));
                break;
            }
            case riichi::ActionKind::UND: {
                buffer_write("UND - %2s", tile_describe(tile));
                break;
            }
        }
        if (i < RIICHI_GYM_PLAYER_COUNT - 1) {
            buffer_write("            ");
        } else {
            buffer_write("\n");
        }
    }
    
    return buffer;
}


void riichi_gym_wait_actiongroup_input(riichi_gym_actiongroup_t* group) {
    static char buffer[128];
    const char delimiter[] = " \n";
    
    for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
        group->tips[i] = static_cast<uint8_t>(riichi::Tile::UND);
    }
    
    uint8_t player = 0;
    uint8_t input_tile = static_cast<uint8_t>(riichi::Tile::UND);
    char* token;
    fgets(buffer, sizeof(buffer), stdin);
    token = strtok(buffer, delimiter);
    
    while(token != NULL && player < RIICHI_GYM_PLAYER_COUNT) {
        for (uint8_t tile = static_cast<uint8_t>(riichi::TILE_FIRST); tile <= static_cast<uint8_t>(riichi::TILE_LAST); tile++) {
            if (riichi::strncmpci(token, riichi::tile_describe(static_cast<riichi::Tile>(tile)), 2) == 0) {
                group->tiles[player] = tile;
                input_tile = tile;
                goto next_token_parse;
            }
        }
        if (riichi::strncmpci(token, riichi::tile_describe(riichi::Tile::UND), 2) == 0) {
            group->tiles[player] = static_cast<uint8_t>(riichi::Tile::UND);
            input_tile = static_cast<uint8_t>(riichi::Tile::UND);
            goto next_token_parse;
        }
        break;
    next_token_parse:
        player += 1;
        token = strtok(NULL, delimiter);
    }
    
    if (player == 1) {
        player = 0;
        uint8_t actable_count = 0;
        for (uint8_t i = 0; i < RIICHI_GYM_PLAYER_COUNT; i++) {
            if (group->actionkinds[i] != static_cast<uint8_t>(riichi::ActionKind::UND)) {
                actable_count += 1;
                player = i;
            }
        }
        
        if (actable_count == 1) {
            group->tiles[0] = static_cast<uint8_t>(riichi::Tile::UND);
            group->tiles[player] = input_tile;
        }
    }
}
