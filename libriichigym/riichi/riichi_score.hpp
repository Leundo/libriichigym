//
//  riichi_score.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/25.
//

#ifndef riichi_score_hpp
#define riichi_score_hpp

#include <variant>
#include "riichi_algo.hpp"


namespace riichi {

enum class YakuKind: uint8_t {
    // Upgradable
    RIICHI,
    DOUBLE_RIICHI,

    LIPEIKOU,
    RYANPEIKOU,

    HONITSU,
    CHINITSU,

    CHANTAIYAO,
    JUNCHAN_TAIYAO,
    HONROUTOU,

    // Perfectible
    KOUKUSHI_MUSOU,
    PREFECT_KOUKUSHI_MUSOU,

    SUUANKOU,
    PREFECT_SUUANKOU,

    CHUUREN_POUTOU,
    PREFECT_CHUUREN_POUTOU,

    // Other
    MENZENCHIN_TUMOHOU,
    IPPATSU,
    PINFU,

    HAITEI_RAOYUE,
    HOUTEI_RAOYUI,
    RINSHAN_KAIHOU,
    CHANKAN,
    TANYAO,
    YAKUHAI_JIKAZE,
    YAKUHAI_BAKAZE,
    YAKUHAI_PAI,
    YAKUHAI_FA,
    YAKUHAI_CHUNG,

    SANSHOKU_DOUJUN,
    ITTSU,
    TOITOI,
    SANANKOU,
    SANSHOKU_DOUKOU,
    SANKANTSU,
    CHIITOITSU,
    SHOUSANGEN,

    DAISANGEN,
    SHOUSUUSHII,
    TSUUIISOU,
    CHINROUTOU,
    RYUUIISOU,
    SUUKANTSU,
    TENHOU,
    CHIIHOU,
    NAGASHI_MANGAN,
};

uint8_t yakukind_to(YakuKind) noexcept;

enum class AgariEvent {
    TUMO,
    RONG,
    FLOWER,
    LOOT,
};


typedef struct PatternGroup {
    bool is_koukushi = false;
    const uint32_t* values = nullptr;
    
    bool is_nil() const noexcept;
    static bool is_value_valid(uint32_t) noexcept;
} PatternGroup;

typedef struct ChiitoitsuArr {
    std::bitset<64> yakus = {};
    Tile pairs[7];
    Tile trgtile;
} ChiitoitsuArr;

typedef struct KoukushiArr {
    std::bitset<64> yakus = {};
    Tile pair;
    Tile trgtile;
} KoukushiArr;

typedef struct ChuurenArr {
    std::bitset<64> yakus = {};
    Tile xtratile;
    Tile trgtile;
} ChuurenArr;

typedef struct NormalArr {
    std::bitset<64> yakus = {};
    std::bitset<21> status = {};
    Tile mentus[4];
    Tile pair;
    Tile trgtile;
    
    bool get_mentu_flag(Offset<uint8_t>, uint8_t) const noexcept;
    void set_mentu_flag(Offset<uint8_t>, uint8_t, bool) noexcept;
    bool get_mentu_combinable(Offset<uint8_t>) const noexcept;
    void set_mentu_combinable(Offset<uint8_t>, bool) noexcept;
    bool get_pair_combinable() const noexcept;
    void set_pair_combinable(bool) noexcept;
    
    constexpr static uint8_t IS_SHUNTU = 0;
    constexpr static uint8_t IS_KAN = 1;
    constexpr static uint8_t IS_EXPOSED = 2;
    constexpr static uint8_t IS_CLOSED_KAN = 3;
} NormalArr;

using Arrangement = std::variant<ChiitoitsuArr, KoukushiArr, ChuurenArr, NormalArr>;


struct PenaltyPoint {
    int32_t dealer_rong = 0;
    int32_t dealer_tumo = 0;
    int32_t punter_rong = 0;
    int32_t punter_tumo_from_punter = 0;
    int32_t punter_tumo_from_dealer = 0;
};

bool operator == (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept;
bool operator < (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept;


class YakuCombo {
public:
    YakuCombo() noexcept = default;
    ~YakuCombo() noexcept = default;
    
    bool is_nil = true;
    Arrangement arrangement;
    
    std::bitset<64> yakus() const noexcept;
    
    PenaltyPoint score() const noexcept;
};


PatternGroup hand_patterngroup(const Hand&);

YakuCombo board_yakucombo(const Board&, const PatternGroup&, Player, AgariEvent);


}


#endif /* riichi_score_hpp */
