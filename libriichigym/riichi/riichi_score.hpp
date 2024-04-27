//
//  riichi_score.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/25.
//

#ifndef riichi_score_hpp
#define riichi_score_hpp

#include <variant>
#include <optional>
#include "riichi_algo.hpp"
#include "riichi_yaku.hpp"


namespace riichi {

typedef struct PatternGroup {
    const uint32_t* values = nullptr;
    bool is_koukushi = false;
    
    static bool is_value_valid(uint32_t) noexcept;
} PatternGroup;

enum class AgariEvent {
    TUMO,
    RONG,
    FLOWER,
    LOOT,
};

typedef struct ChiitoitsuArr {
    std::bitset<YAKUKIND_COUNT> yakus = {};
    Tile pairs[7];
    Tile trgtile;
} ChiitoitsuArr;

typedef struct KoukushiArr {
    std::bitset<YAKUKIND_COUNT> yakus = {};
    Tile pair;
    Tile trgtile;
} KoukushiArr;

typedef struct ChuurenArr {
    std::bitset<YAKUKIND_COUNT> yakus = {};
    Tile xtratile;
    Tile trgtile;
} ChuurenArr;

typedef struct NormalArr {
    std::bitset<YAKUKIND_COUNT> yakus = {};
    std::bitset<21> status = {};
    Tile mentus[4];
    Tile pair;
    Tile trgtile;
    
    enum class Machi {
        NIL,
        DANKI,
        KANCHAN,
        PENCHAN,
        RYANMEN,
        SOUPON,
    };
    
    bool get_mentu_flag(Offset<uint8_t>, uint8_t) const noexcept;
    void set_mentu_flag(Offset<uint8_t>, uint8_t, bool) noexcept;
    bool get_mentu_combinable(Offset<uint8_t>) const noexcept;
    void set_mentu_combinable(Offset<uint8_t>, bool) noexcept;
    bool get_pair_combinable() const noexcept;
    void set_pair_combinable(bool) noexcept;
    
    uint8_t shuntu_count() const noexcept;
    uint8_t koutu_count() const noexcept;
    uint8_t kantu_count() const noexcept;
    
    std::bitset<4> is_hidden_koutus() const noexcept;
    
    bool is_pair_danki() const noexcept;
    bool is_mentu_kanchan(Offset<uint8_t>) const noexcept;
    bool is_any_mentu_kanchan() const noexcept;
    
    bool is_mentu_ryanmen(Offset<uint8_t>) const noexcept;
    bool is_any_mentu_ryanmen() const noexcept;
    
    bool is_menzenchin() const noexcept;
    
    constexpr static uint8_t IS_SHUNTU = 0;
    constexpr static uint8_t IS_KAN = 1;
    constexpr static uint8_t IS_EXPOSED = 2;
    constexpr static uint8_t IS_CLOSED_KAN = 3;
} NormalArr;

using Arrangement = std::variant<ChiitoitsuArr, KoukushiArr, ChuurenArr, NormalArr>;


std::optional<PatternGroup> hand_patterngroup(const Hand&) noexcept;

std::optional<YakuCombo> board_yakucombo(const Board&, const PatternGroup&, Player, AgariEvent) noexcept;

}


#endif /* riichi_score_hpp */
