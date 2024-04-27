//
//  riichi_yaku.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/27.
//

#ifndef riichi_yaku_hpp
#define riichi_yaku_hpp

#include <cstdint>
#include <bitset>


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

constexpr uint8_t YAKUKIND_COUNT = 45;

#define yakukind_to(yakukind)\
(static_cast<uint8_t>(yakukind))


struct PenaltyPoint {
    int32_t dealer_rong = 0;
    int32_t dealer_tumo = 0;
    int32_t punter_rong = 0;
    int32_t punter_tumo_from_dealer = 0;
    int32_t punter_tumo_from_punter = 0;
};

bool operator == (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept;
bool operator < (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept;


class YakuCombo {
public:
    YakuCombo() noexcept = default;
    ~YakuCombo() noexcept = default;
    
    std::bitset<YAKUKIND_COUNT> yakus;
    bool is_menzen;
    uint8_t fu;
    uint8_t dora_count;
        
    PenaltyPoint score() const noexcept;
};

}


#endif /* riichi_yaku_hpp */
