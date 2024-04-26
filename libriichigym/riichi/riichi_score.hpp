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
    RIICHI = 0,
    DOUBLE_RIICHI = 1,

    IIPEIKOU = 2,
    RYANPEIKOU = 3,

    HONITSU = 4,
    CHINITSU = 5,

    CHANTAIYAO = 6,
    JUNCHAN_TAIYAO = 7,
    HONROUTOU = 8,

    // Perfectible
    KOUKUSHI_MUSOU = 9,
    PREFECT_KOUKUSHI_MUSOU = 10,

    SUUANKOU = 11,
    PREFECT_SUUANKOU = 12,

    CHUUREN_POUTOU = 13,
    PREFECT_CHUUREN_POUTOU = 14,

    // Other
    MENZENCHIN_TUMOHOU = 15,
    IPPATSU = 16,
    PINFU = 17,

    HAITEI_RAOYUE = 18,
    HOUTEI_RAOYUI = 19,
    RINSHAN_KAIHOU = 20,
    CHANKAN = 21,
    TANYAO = 22,
    YAKUHAI_JIKAZE = 23,
    YAKUHAI_BAKAZE = 24,
    YAKUHAI_PAI = 25,
    YAKUHAI_FA = 26,
    YAKUHAI_CHUNG = 27,

    SANSHOKU_DOUJUN = 28,
    ITTSU = 29,
    TOITOI = 30,
    SANANKOU = 31,
    SANSHOKU_DOUKOU = 32,
    SANKANTSU = 33,
    CHIITOITSU = 34,
    SHOUSANGEN = 35,

    DAISANGEN = 36,
    SHOUSUUSHII = 37,
    DAISUUSHII = 38,
    TSUUIISOU = 39,
    CHINROUTOU = 40,
    RYUUIISOU = 41,
    SUUKANTSU = 42,
    TENHOU = 43,
    CHIIHOU = 44,
};

#define yakukind_to(yakukind)\
(static_cast<uint8_t>(yakukind))


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
    uint8_t hidden_koutu_count() const noexcept;
    uint8_t kantu_count() const noexcept;
    
    bool is_mentu_ryanmen(Offset<uint8_t>) const noexcept;
    bool is_any_mentu_ryanmen() const noexcept;
    
    bool is_menzenchin() const noexcept;
    
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
    
    std::bitset<64> yakus;
    bool is_nil;
    bool is_menzen;
    uint8_t dora_count;
        
    PenaltyPoint score() const noexcept;
};


PatternGroup hand_patterngroup(const Hand&);

YakuCombo board_yakucombo(const Board&, const PatternGroup&, Player, AgariEvent);


}


#endif /* riichi_score_hpp */
