//
//  riichi_ui.h
//  libriichigym
//
//  Created by Leundo on 2024/04/24.
//

#ifndef riichi_ui_h
#define riichi_ui_h

#include "riichi_gym.h"


#ifdef __cplusplus
extern "C" {
#endif


const char* riichi_gym_render_cli_ui(const riichi_gym_board_t*, riichi_gym_actiongroup_t);

void riichi_gym_wait_actiongroup_input(riichi_gym_actiongroup_t*);


#ifdef __cplusplus
}
#endif


#endif /* riichi_ui_h */
