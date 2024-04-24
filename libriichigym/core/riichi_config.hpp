//
//  riichi_config.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#ifndef riichi_config_hpp
#define riichi_config_hpp

#ifdef __cplusplus
#include <climits>
#else
#include <limits.h>
#endif


// MARK: - Rule
#define RIICHI_GYM_MAJSOUL_4_RULE


#ifdef RIICHI_GYM_MAJSOUL_4_RULE

#define RIICHI_GYM_DORA_UPPER_LIMIT 5
#define RIICHI_GYM_KAN_UPPER_LIMIT 4
#define RIICHI_GYM_PLAYER_COUNT 4
#define RIICHI_GYM_INITIAL_SCORE 25000

#else
#error
#endif


// MARK: - Assert
#if ULONG_LONG_MAX >= UINT64_MAX
#else
#error
#endif

#endif /* riichi_config_hpp */
