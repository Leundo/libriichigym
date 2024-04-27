//
//  riichi_yaku.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/27.
//

#include "riichi_yaku.hpp"
#include "riichi_util.hpp"


namespace riichi {

bool operator == (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept {
    return lhs.dealer_rong == rhs.dealer_rong;
}

bool operator < (const PenaltyPoint& lhs, const PenaltyPoint& rhs) noexcept {
    return lhs.dealer_rong < rhs.dealer_rong;
}


PenaltyPoint YakuCombo::score() const noexcept {
    int32_t yakuman = static_cast<int32_t>((yakus & bitset_make_containing<YAKUKIND_COUNT>(yakukind_to(YakuKind::KOUKUSHI_MUSOU), yakukind_to(YakuKind::PREFECT_KOUKUSHI_MUSOU), yakukind_to(YakuKind::SUUANKOU), yakukind_to(YakuKind::PREFECT_SUUANKOU), yakukind_to(YakuKind::DAISANGEN), yakukind_to(YakuKind::SHOUSUUSHII), yakukind_to(YakuKind::DAISUUSHII), yakukind_to(YakuKind::TSUUIISOU), yakukind_to(YakuKind::CHINROUTOU), yakukind_to(YakuKind::RYUUIISOU), yakukind_to(YakuKind::CHUUREN_POUTOU), yakukind_to(YakuKind::PREFECT_CHUUREN_POUTOU), yakukind_to(YakuKind::SUUANKOU), yakukind_to(YakuKind::TENHOU), yakukind_to(YakuKind::CHIIHOU))).count());
    if (yakuman > 0) {
        return {
            .dealer_rong = 48000 * yakuman,
            .dealer_tumo = 16000 * yakuman,
            .punter_rong = 32000 * yakuman,
            .punter_tumo_from_dealer = 16000 * yakuman,
            .punter_tumo_from_punter = 8000 * yakuman,
        };
    }
    int32_t pan = 0;
    int32_t corrected_fu = fu;
    pan += (yakus & bitset_make_containing<YAKUKIND_COUNT>(yakukind_to(YakuKind::RIICHI), yakukind_to(YakuKind::MENZENCHIN_TUMOHOU), yakukind_to(YakuKind::IPPATSU), yakukind_to(YakuKind::PINFU), yakukind_to(YakuKind::IIPEIKOU), yakukind_to(YakuKind::HAITEI_RAOYUE), yakukind_to(YakuKind::HOUTEI_RAOYUI), yakukind_to(YakuKind::RINSHAN_KAIHOU), yakukind_to(YakuKind::CHANKAN), yakukind_to(YakuKind::TANYAO), yakukind_to(YakuKind::YAKUHAI_PAI), yakukind_to(YakuKind::YAKUHAI_FA), yakukind_to(YakuKind::YAKUHAI_CHUNG), yakukind_to(YakuKind::YAKUHAI_JIKAZE), yakukind_to(YakuKind::YAKUHAI_BAKAZE))).count();
    
    pan += (yakus & bitset_make_containing<YAKUKIND_COUNT>(yakukind_to(YakuKind::DOUBLE_RIICHI), yakukind_to(YakuKind::TOITOI), yakukind_to(YakuKind::SANANKOU), yakukind_to(YakuKind::SANSHOKU_DOUKOU), yakukind_to(YakuKind::SANKANTSU), yakukind_to(YakuKind::CHIITOITSU), yakukind_to(YakuKind::HONROUTOU), yakukind_to(YakuKind::SHOUSANGEN))).count() * 2;
    
    pan += (yakus & bitset_make_containing<YAKUKIND_COUNT>(yakukind_to(YakuKind::CHANTAIYAO), yakukind_to(YakuKind::SANSHOKU_DOUJUN), yakukind_to(YakuKind::ITTSU))).count() * (static_cast<int32_t>(is_menzen) + 1);
    
    pan += (yakus & bitset_make_containing<YAKUKIND_COUNT>(yakukind_to(YakuKind::HONITSU), yakukind_to(YakuKind::JUNCHAN_TAIYAO), yakukind_to(YakuKind::RYANPEIKOU))).count() * (static_cast<int32_t>(is_menzen) + 2);
    
    pan += (yakus & bitset_make_containing<YAKUKIND_COUNT>(yakukind_to(YakuKind::CHINITSU))).count() * (static_cast<int32_t>(is_menzen) + 5);
    
    if (pan <= 0) {
        return {};
    }
    pan += dora_count;
    if (pan == 1 && corrected_fu <= 20) {
        corrected_fu = 30;
    }
    if (pan > 13) {
        pan = 13;
    }
    if ((pan == 3 && corrected_fu >= 70) || (pan == 4 && corrected_fu >= 40)) {
        pan = 5;
    }
    if (pan >= 5) {
        int32_t base;
        if (pan <= 5) {
            base = 2000;
        } else if (pan == 6 || pan == 7) {
            base = 3000;
        } else if (pan >= 8 && pan <= 10) {
            base = 4000;
        } else if (pan == 11 || pan == 12) {
            base = 6000;
        } else {
            base = 8000;
        }
        return {
            .dealer_rong = 6 * base,
            .dealer_tumo = 2 * base,
            .punter_rong = 4 * base,
            .punter_tumo_from_dealer = 2 * base,
            .punter_tumo_from_punter = base,
        };
    }
    int32_t base = corrected_fu << (2 + pan);
    return {
        .dealer_rong = (6 * base + 99) / 100 * 100,
        .dealer_tumo = (2 * base + 99) / 100 * 100,
        .punter_rong = (4 * base + 99) / 100 * 100,
        .punter_tumo_from_dealer = (2 * base + 99) / 100 * 100,
        .punter_tumo_from_punter = (base + 99) / 100 * 100,
    };
}

}
