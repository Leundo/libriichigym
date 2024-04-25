//
//  riichi_board.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include <cstring>
#include "riichi_board.hpp"
#include "riichi_algo.hpp"


namespace riichi {

Board::Cache::Cache() noexcept {
    fogs = {};
}


// MARK: - Board
Board::Board(std::optional<unsigned int> seed, std::optional<std::function<void(Mountain&, std::mt19937&)>> shuffle) noexcept {
    this->seed = seed.value_or(std::random_device{}());
    random_generator = std::mt19937(this->seed);
    this->shuffle = shuffle.value_or([](Mountain& mountain, std::mt19937& generator) noexcept -> void {
//        mountain
    });
    
    session = 0;
    honba = 0;
    move = 0;
    tribute = 0;
    scores.fill(RIICHI_GYM_INITIAL_SCORE);
    
//    mountain = Mountain();
//    river = River();
//    shrine = Shrine();
//    hands.fill(Hand());
    
    current_player = Player::P0;
    stage = Stage::PREPARE;
    life = 1 << static_cast<uint8_t>(LifeIndex::RUN);
    hold_tile = Tile::UND;
    drop_tile = Tile::UND;
    action_permission = 0;
    action_tip = 0;
    cache = Cache();
}


ActionGroup Board::request() noexcept {
    ActionGroup group = {};
    
    while (true) {
        switch (stage) {
            case Stage::PREPARE: {
                switch (session % 4) {
                    case 0: { current_player = Player::P0; break; }
                    case 1: { current_player = Player::P1; break; }
                    case 2: { current_player = Player::P2; break; }
                    case 3: { current_player = Player::P3; break; }
                }
                
                life = 1 << static_cast<uint8_t>(LifeIndex::RUN);
                move = 0;
                cache = Cache();
                
                tiles_shuffle(mountain.tiles.begin(), mountain.tiles.size(), random_generator);

                
                for (uint8_t i = 0; i < PLAYER_COUNT; i++) {
                    for (uint8_t j = 0; j < 13; j++) {
                        Tile tile = mountain.draw(false);
                        hands[i].increase(tile);
                        cache.fogs[i].decrease(tile);
                    }
                    cache.fogs[i].decrease(mountain.outdora_tile(0));
                }
                
                stage = Stage::DRAW;
                break;
            }
            case Stage::DRAW: {
                if (mountain.can_draw()) {
                    hold_tile = mountain.draw(false);
                    hand(current_player).increase(hold_tile);
                    cache.fog(current_player).decrease(hold_tile);
                    stage = Stage::ABORT;
                } else {
                    life = 1 << static_cast<uint8_t>(LifeIndex::ABORT);
                    stage = Stage::SCORE;
                }
                break;
            }
            case Stage::ABORT: {
                if (board_can_abort(*this)) {
                    group.set(current_player, hold_tile, ActionKind::OPEN_KAN);
                    return group;
                } else {
                    stage = Stage::TUMO;
                }
                break;
            }
            case Stage::TUMO: {
                if (board_can_tumo(*this)) {
                    group.set(current_player, hold_tile, ActionKind::AGARI);
                    return group;
                } else {
                    stage = Stage::SEKAN;
                }
                break;
            }
            case Stage::SEKAN: {
                if (board_can_sekan(*this)) {
                    group.set(current_player, hold_tile, ActionKind::SELF_KAN);
                    return group;
                } else {
                    stage = Stage::RIICHI;
                }
                break;
            }
            case Stage::FLOWER: {
                if (board_can_flower(*this)) {
                    group.set(current_player, hold_tile, ActionKind::AGARI);
                    return group;
                } else {
                    stage = Stage::RIICHI;
                }
                break;
            }
            case Stage::RIICHI: {
                if (board_can_riichi(*this)) {
                    group.set(current_player, hold_tile, ActionKind::RIICHI);
                    return group;
                } else {
                    stage = Stage::MAIN_DISCARD;
                }
                break;
            }
            case Stage::MAIN_DISCARD:
            case Stage::AUX_DISCARD: {
                group.set(current_player, hold_tile, ActionKind::DISCARD);
                issue_permission_to_avoid_kuikae_before_discarding(*this);
                return group;
            }
            case Stage::LOOT: {
                group = board_calculate_loot_group(*this);
                action_permission = group.actionkinds_nonundefineds();
                if (action_permission.any()) {
                    return group;
                } else {
                    stage = Stage::RIICHI;
                }
                break;
            }
            case Stage::RONG: {
                group = board_calculate_rong_group(*this);
                action_permission = group.actionkinds_nonundefineds();
                if (action_permission.any()) {
                    return group;
                } else {
                    stage = Stage::KAN;
                }
                break;
            }
            case Stage::KAN: {
                group = board_calculate_kan_group(*this);
                action_permission = group.actionkinds_nonundefineds();
                if (action_permission.any()) {
                    return group;
                } else {
                    stage = Stage::PON;
                }
                break;
            }
            case Stage::PON: {
                group = board_calculate_pon_group(*this);
                action_permission = group.actionkinds_nonundefineds();
                if (action_permission.any()) {
                    return group;
                } else {
                    stage = Stage::CHI;
                }
                break;
            }
            case Stage::CHI: {
                group = board_calculate_chi_group(*this);
                action_permission = group.actionkinds_nonundefineds();
                action_tip = group.tip_get(current_player);
                if (action_permission.any()) {
                    return group;
                } else {
                    current_player = player_cycle_next(current_player);
                    stage = Stage::DRAW;
                }
                break;
            }
            case Stage::SCORE: {
                break;
            }
            case Stage::CLEAN: {
                break;
            }
        }
    }
    
    return group;
}

void Board::response(const ActionGroup& group) noexcept {
    switch (stage) {
        case Stage::ABORT: {
            if (tile_is_undefined(group.get_tile(current_player))) {
                stage = Stage::TUMO;
            } else {
                life = 1 << static_cast<uint8_t>(LifeIndex::ABORT);
                stage = Stage::SCORE;
            }
        }
        case Stage::TUMO: {
            if (tile_is_undefined(group.get_tile(current_player))) {
                stage = Stage::SEKAN;
            } else {
                life = 1 << static_cast<uint8_t>(current_player);
                stage = Stage::SCORE;
            }
            break;
        }
        case Stage::SEKAN: {
            if (tile_is_undefined(group.get_tile(current_player))) {
                stage = Stage::RIICHI;
            } else {
                // TODO:
            }
            break;
        }
        case Stage::FLOWER: {
            if (tile_is_undefined(group.get_tile(current_player))) {
                stage = Stage::RIICHI;
            } else {
                life = 1 << static_cast<uint8_t>(current_player);
                stage = Stage::SCORE;
            }
            break;
        }
        case Stage::RIICHI: {
            if (tile_is_undefined(group.get_tile(current_player))) {
                stage = Stage::MAIN_DISCARD;
            } else {
                // TODO:
            }
            break;
        }
        case Stage::LOOT: {
            auto actables = group.tiles_nonundefineds();
            actables &= action_permission;
            if (actables.any()) {
                stage = Stage::MAIN_DISCARD;
            } else {
                // TODO:
            }
            break;
        }
        case Stage::RONG: {
            auto actables = group.tiles_nonundefineds();
            actables &= action_permission;
            if (actables.any()) {
                stage = Stage::KAN;
            } else {
                // TODO:
            }
            break;
        }
        case Stage::KAN: {
            auto actables = group.tiles_nonundefineds();
            actables &= action_permission;
            if (actables.any()) {
                stage = Stage::PON;
                break;
            }
            Player trigger = current_player;
            Player actor = static_cast<Player>(ffsl(actables.to_ulong()));
            if (tile_downgrade(group.get_tile(actor)) != tile_downgrade(drop_tile)) {
                stage = Stage::PON;
                break;
            }
            
            // Change Shrine
            Exposed* exposed = shrine.append(actor);
            exposed->naki = Naki::OPEN_KAN;
            exposed->keytile = tile_downgrade(drop_tile);
            exposed->trgtile = drop_tile;
            exposed->is_upgraded = tile_is_gradeable(drop_tile);
            exposed->move = river.last_cdiscarded(trigger)->move;
            exposed->owner = actor;
            exposed->trigger = trigger;
            
            // Change Hand
            hand(actor).clear(tile_upgrade(drop_tile));
            hand(actor).clear(tile_downgrade(drop_tile));
            cache.fogs_clear_expect(actor, tile_upgrade(drop_tile));
            cache.fogs_clear_expect(actor, tile_downgrade(drop_tile));
            
            hold_tile = mountain.draw(true);
            hand(actor).increase(hold_tile);
            cache.fog(actor).decrease(hold_tile);
            
            current_player = actor;
            stage = Stage::FLOWER;
            break;
        }
        case Stage::PON: {
            auto actables = group.tiles_nonundefineds();
            actables &= action_permission;
            if (actables.any()) {
                stage = Stage::CHI;
                break;
            }
            Player trigger = current_player;
            Player actor = static_cast<Player>(ffsl(actables.to_ulong()));
            Tile action_tile = group.get_tile(actor);
            if (tile_downgrade(action_tile) != tile_downgrade(drop_tile)) {
                stage = Stage::CHI;
                break;
            }
            
            // Change Shrine
            bool is_upgraded = ({
                bool is_upgraded = false;
                Tile upgraded = tile_upgrade(drop_tile);
                Tile downgraded = tile_downgrade(drop_tile);
                if (upgraded != downgraded) {
                    if (drop_tile == upgraded) {
                        is_upgraded = true;
                    } else if (action_tile == upgraded) {
                        is_upgraded = true;
                    } else if (hand(actor).count(downgraded) < 2) {
                        is_upgraded = true;
                    }
                }
                is_upgraded;
            });
            Exposed* exposed = shrine.append(actor);
            exposed->naki = Naki::PON;
            exposed->keytile = tile_downgrade(drop_tile);
            exposed->trgtile = drop_tile;
            exposed->is_upgraded = is_upgraded;
            exposed->move = river.last_cdiscarded(trigger)->move;
            exposed->owner = actor;
            exposed->trigger = trigger;
            
            // Change Hand
            if (is_upgraded && drop_tile != tile_upgrade(drop_tile)) {
                hand(actor).decrease(tile_upgrade(action_tile));
                hand(actor).decrease(tile_downgrade(action_tile));
                
                cache.fogs_decrease_expect(actor, tile_upgrade(action_tile));
                cache.fogs_decrease_expect(actor, tile_downgrade(action_tile));
            } else {
                hand(actor).decrease(tile_downgrade(action_tile));
                hand(actor).decrease(tile_downgrade(action_tile));
                
                cache.fogs_decrease_expect(actor, tile_downgrade(action_tile));
                cache.fogs_decrease_expect(actor, tile_downgrade(action_tile));
            }
            
            current_player = actor;
            hold_tile = Tile::UND;
            stage = Stage::AUX_DISCARD;
            break;
        }
        case Stage::CHI: {
            auto actables = group.tiles_nonundefineds();
            actables &= action_permission;
            if (actables.any()) {
                current_player = player_cycle_next(current_player);
                stage = Stage::DRAW;
                break;
            }
            Player trigger = current_player;
            Player actor = static_cast<Player>(ffsl(actables.to_ulong()));
            Tile action_tile = group.get_tile(actor);
            if (tile_is_undefined(action_tile) || action_tip[static_cast<uint8_t>(action_tile)] == false) {
                current_player = player_cycle_next(current_player);
                stage = Stage::DRAW;
                break;
            }
            if (action_tile != Tile::P && action_tile != Tile::F && action_tile != Tile::C) {
                current_player = player_cycle_next(current_player);
                stage = Stage::DRAW;
                __builtin_unreachable();
                break;
            }
            
            Exposed* exposed = shrine.append(actor);
            Tile keytile = Tile::UND;
            bool is_upgraded = false;
            
            Tile neighbor_tile_0;
            Tile neighbor_tile_1;
            // No double check
            if (action_tile == Tile::P) {
                neighbor_tile_0 = tile_cycle_next(drop_tile);
                neighbor_tile_1 = tile_cycle_next(neighbor_tile_0);
                keytile = tile_downgrade(drop_tile);
            } else if (action_tile == Tile::F) {
                neighbor_tile_0 = tile_cycle_next(drop_tile);
                neighbor_tile_1 = tile_cycle_prev(drop_tile);
                keytile = tile_downgrade(neighbor_tile_1);
            } else if (action_tile == Tile::C) {
                neighbor_tile_0 = tile_cycle_prev(drop_tile);
                neighbor_tile_1 = tile_cycle_prev(neighbor_tile_0);
                keytile = tile_downgrade(neighbor_tile_1);
            } else {
                __builtin_unreachable();
            }
            
            for (auto neighbor_tile: {neighbor_tile_0, neighbor_tile_1}) {
                if (hand(actor).decrease_upgraded_prioritizely(neighbor_tile)) {
                    is_upgraded = true;
                    cache.fogs_decrease_expect(actor, tile_upgrade(neighbor_tile));
                } else {
                    cache.fogs_decrease_expect(actor, tile_downgrade(neighbor_tile));
                }
            }
            
            exposed->naki = Naki::CHI;
            exposed->keytile = keytile;
            exposed->trgtile = drop_tile;
            exposed->is_upgraded = is_upgraded;
            exposed->move = river.last_cdiscarded(trigger)->move;
            exposed->owner = actor;
            exposed->trigger = trigger;
            
            current_player = actor;
            hold_tile = Tile::UND;
            stage = Stage::AUX_DISCARD;
            break;
        }
        case Stage::MAIN_DISCARD:
        case Stage::AUX_DISCARD: {
            Tile action_tile = group.get_tile(current_player);
            Tile discarded_tile = action_tile;
            Discarded::Kiri kiri;
            
            // If hold_tile is UND_TILE, then AUX_DISCARD
            // else MAIN_DISCARD or OPEN_KAN AUX_DISCARD
            if (tile_is_undefined(hold_tile)) {
                if (tile_is_undefined(discarded_tile) || chand(current_player).count(discarded_tile) <= 0) {
                    discarded_tile = chand(current_player).first_tile();
                }
                repick_tile_to_avoid_kuikae_after_exposing(*this, discarded_tile);
                kiri = Discarded::Kiri::OLD;
            } else {
                if (tile_is_undefined(discarded_tile)) {
                    discarded_tile = hold_tile;
                } else if (chand(current_player).count(discarded_tile) <= 0) {
                    action_tile = Tile::UND;
                    discarded_tile = hold_tile;
                }
                
                if (tile_is_undefined(action_tile)) {
                    kiri = Discarded::Kiri::NEW;
                } else if (action_tile == discarded_tile && chand(current_player).count(discarded_tile) > 1) {
                    kiri = Discarded::Kiri::OLD;
                } else {
                    kiri = Discarded::Kiri::NEW;
                }
            }
            
            const Discarded* last_discarded = river.last_cdiscarded(current_player);
            Discarded* discarded = river.append(current_player);
            discarded->tile = discarded_tile;
            discarded->move = move;
            discarded->owner = current_player;
            discarded->is_onriichi = last_discarded == NULL ? false : last_discarded->is_onriichi;
            discarded->kiri = kiri;
            discarded->naki = Naki::UND;
            
            hand(current_player).decrease(discarded_tile);
            cache.fogs_decrease_expect(current_player, discarded_tile);
            
            drop_tile = discarded_tile;
            move += 1;
            stage = Stage::RONG;
            break;
        }
        case Stage::PREPARE:
        case Stage::DRAW:
        case Stage::SCORE:
        case Stage::CLEAN:{
            break;
        }
    }
}

}
