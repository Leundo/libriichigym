//
//  riichi_gym.h
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#ifndef riichi_gym_h
#define riichi_gym_h

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef struct riichi_gym_board riichi_gym_board_t;

typedef struct riichi_gym_actiongroup {
    uint8_t tiles[4];
    uint8_t actionkinds[4];
    uint64_t tips[4];
} riichi_gym_actiongroup_t;


riichi_gym_board_t* riichi_gym_board_malloc();

riichi_gym_board_t* riichi_gym_board_malloc_with_seed(unsigned int);

void riichi_gym_board_free(riichi_gym_board_t*);


riichi_gym_actiongroup_t riichi_gym_board_request(riichi_gym_board_t*);

void riichi_gym_board_respound(riichi_gym_board_t*, riichi_gym_actiongroup_t);


#ifdef __cplusplus
}
#endif

#endif /* riichi_gym_h */
