#include "../plugin_sdk/plugin_sdk.hpp"
#include "evelynn.h"
#include "iostream"

namespace evelynn
{
    // define colors in on_draw() using RGBA (102,255,255)
    #define Q_DRAW_COLOR (MAKE_COLOR ( 102,255,255, 255 )) 
    #define W_DRAW_COLOR (MAKE_COLOR ( 102,255,255, 255 ))  
    #define E_DRAW_COLOR (MAKE_COLOR ( 102,255,255, 255 ))  
    #define R_DRAW_COLOR (MAKE_COLOR ( 102,255,255, 255 ))   

    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;

    script_spell* flash = nullptr;
    script_spell* smite = nullptr;
    //script_spell* rocketbelt = nullptr;

    TreeTab* main_tab = nullptr;

    namespace draw_settings
    {
        TreeEntry* draw_range_q = nullptr;
        TreeEntry* draw_range_w = nullptr;
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* draw_flash_range = nullptr;
    }

    namespace combo
    {
        TreeEntry* q_seperator = nullptr;
        TreeEntry* use_q = nullptr;
        TreeEntry* q_range = nullptr;

        TreeEntry* w_seperator = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* wait_for_charm = nullptr;

        TreeEntry* e_seperator = nullptr;
        TreeEntry* use_e = nullptr;

        TreeEntry* r_seperator = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* chase_with_r = nullptr;

        TreeEntry* item_seperator = nullptr;
        TreeEntry* use_rocketbelt = nullptr;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_range = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* sheen_procc = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_r;
    }

    namespace killsteal
    {
        TreeEntry* use_r;
    }

    namespace misc
    {
        TreeEntry* use_r = nullptr;
        TreeEntry* hp_in_percent = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack(game_object_script target, bool* process);
    void on_buff_gain(game_object_script sender, buff_instance_script buff);
    void on_buff_lose(game_object_script sender, buff_instance_script buff);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void rocketbelt_logic();
    void w_logic();
    void e_logic();
    void r_logic();

    // Enum is used to define myhero region 
    enum Position
    {
        Line,
        Jungle
    };

    Position my_hero_region;

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 800);
        q->set_skillshot(0.3f, 60.0f, 2400.0f, { collisionable_objects::minions, collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);
        q->set_spell_lock(false);

        w = plugin_sdk->register_spell(spellslot::w, 1200);
        e = plugin_sdk->register_spell(spellslot::e, 280);
        r = plugin_sdk->register_spell(spellslot::r, 540);

        // Checking which slots the Summoner Flash and smite are on
        //
        if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
            flash = plugin_sdk->register_spell(spellslot::summoner1, 400.f);
        else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
            flash = plugin_sdk->register_spell(spellslot::summoner2, 400.f);
        if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerSmite"))
            smite = plugin_sdk->register_spell(spellslot::summoner1, 500);
        else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerSmite"))
            smite = plugin_sdk->register_spell(spellslot::summoner2, 500);


        


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("evelynn", "Evelynn");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo");
            {
                combo::q_seperator = combo->add_separator("settingsQ", "-- Q Settings --");
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".comboUseQ", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::q_range = combo->add_slider(myhero->get_model() + ".comboQRange", "Q range for combo", 750, 500, 800);
                combo::w_seperator = combo->add_separator("settingsW", "-- W Settings --");
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".comboUseW", "Use W in Team Fight", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::wait_for_charm = combo->add_checkbox(myhero->get_model() + ".combowaitForCharm", "Wait for Charm", true);
                combo::e_seperator = combo->add_separator("settingsE", "-- E Settings --");
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".comboUseE", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::r_seperator = combo->add_separator("settingsR", "-- R Settings --");
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".comboUseR", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                combo::chase_with_r = combo->add_checkbox(myhero->get_model() + ".comboChaseWithR", "Chase killable targets by using R backwards", true);
                combo::item_seperator = combo->add_separator("settingsItems", "-- Item Settings --");
                combo::use_rocketbelt = combo->add_checkbox(myhero->get_model() + ".comboUseRocketbelt", "Use Rocketbelt", true);
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harassUseQ", "Use Q", true);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_w = harass->add_checkbox(myhero->get_model() + ".harassUseW", "Use W (won't wait)", true);
                harass::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harassUseE", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "LaneClear");
            {
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseQ", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::sheen_procc = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseSheen", "Procc sheen on turret by using Q", true);
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseE", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "JungleClear");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclearUseQ", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_w = jungleclear->add_checkbox(myhero->get_model() + ".jungleclearUseW", "Use W", true);
                jungleclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclearrUseE", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee");
            {
                fleemode::use_r = fleemode->add_checkbox(myhero->get_model() + ".fleemodeUseR", "Use R", true);
                fleemode::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
            }

            auto killsteal = main_tab->add_tab(myhero->get_model() + ".killsteal", "KillSteal");
            {
                killsteal::use_r = killsteal->add_checkbox(myhero->get_model() + ".killstealUseR", "Use R", true);
                killsteal::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
            }

            auto misc = main_tab->add_tab(myhero->get_model() + ".misc", "Misc Settings");
            {
                misc::use_r = misc->add_checkbox(myhero->get_model() + ".miscUseR", "Use R if I fall low", true);
                misc::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                misc::hp_in_percent = misc->add_slider(myhero->get_model() + ".miscHpPercent", "^ Use if my HP are < ", 5, 0, 50);
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".drawings", "Drawings Settings");
            {
                draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".drawingQ", "Draw Q range", true);
                draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".drawingW", "Draw W range", true);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".drawingE", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".drawingR", "Draw R range", true);
                draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
            }
        }

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack);
        event_handler<events::on_buff_gain>::add_callback(on_buff_gain);
        event_handler<events::on_buff_lose>::add_callback(on_buff_lose);
    }

    void unload()
    {
        // Always remove all declared spells
        //
        plugin_sdk->remove_spell(q);
        plugin_sdk->remove_spell(w);
        plugin_sdk->remove_spell(e);
        plugin_sdk->remove_spell(r);

        if (flash)
            plugin_sdk->remove_spell(flash);
        if (smite)
            plugin_sdk->remove_spell(smite);



        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
        event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack);
        event_handler<events::on_buff_gain>::remove_handler(on_buff_gain);
        event_handler<events::on_buff_lose>::remove_handler(on_buff_lose);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }
        //rocketbelt = plugin_sdk->register_spell(myhero->has_item(3152), 1200);
        
        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (r->is_ready() && combo::use_r->get_bool())
                {
                    r_logic();
                }
                
                if (q->is_ready() && combo::use_q->get_bool())
                {
                    q_logic();
                }

                if (combo::use_rocketbelt->get_bool())
                {
                    rocketbelt_logic();
                }

                if (w->is_ready() && combo::use_w->get_bool())
                {
                    w_logic();
                }

                if (e->is_ready() && combo::use_e->get_bool())
                {
                    e_logic();
                }
            }

            //Checking if the user has selected harass() (Default C)
            if (orbwalker->harass())
            {
                if (q->is_ready() && harass::use_q->get_bool())
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(q->range(), damage_type::physical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr)
                    {
                        if (!myhero->has_buff(buff_hash("GarenE")))
                        {
                            if (!myhero->is_under_enemy_turret())
                            {
                                if (q->cast())
                                {
                                    return;
                                }
                            }
                        }

                        // Disabling e if q inflicts more damage than the enemy's health
                        else if (myhero->has_buff(buff_hash("GarenE")) && q->get_damage(target) > target->get_health())
                        {
                            if (e->cast())
                            {
                                if (q->cast())
                                {
                                    return;
                                }
                            }
                        }

                    }
                }
                if (!q->is_ready() && e->is_ready() && harass::use_e->get_bool())
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(e->range(), damage_type::physical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr)
                    {
                        if (!myhero->has_buff(buff_hash("GarenE")))
                        {
                            if (e->cast())
                            {
                                return;
                            }
                        }
                    }
                }

            }

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (q->is_ready() && fleemode::use_r->get_bool())
                {
                    if (q->cast())
                    {
                        return;
                    }
                }
            }

            // Checking if the user has selected lane_clear_mode() (Default V)
            if (orbwalker->lane_clear_mode())
            {

                // Gets enemy minions from the entitylist
                auto lane_minions = entitylist->get_enemy_minions();

                // Gets jugnle mobs from the entitylist
                auto monsters = entitylist->get_jugnle_mobs_minions();

                // You can use this function to delete minions that aren't in the specified range
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(e->range() + 300);
                    }), lane_minions.end());

                // You can use this function to delete monsters that aren't in the specified range
                monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(e->range() + 300);
                    }), monsters.end());

                //std::sort -> sort monsters by max helth
                std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
                    {
                        return a->get_max_health() > b->get_max_health();
                    });

                // Set the correct region where myhero is
                if (!lane_minions.empty())
                {
                    my_hero_region = Position::Line;
                }
                else if (!monsters.empty())
                {
                    my_hero_region = Position::Jungle;
                }

                if (!lane_minions.empty())
                {
                    // Logic responsible for lane minions
                    //
                    if (q->is_ready() && laneclear::use_q->get_bool())
                    {
                        if (myhero->is_under_enemy_turret())
                        {
                            if (myhero->count_enemies_in_range(q->range()) == 0)
                            {
                                if (q->cast())
                                {
                                    return;
                                }
                            }
                        }
                        if (!myhero->has_buff(buff_hash("GarenE")))
                        {
                            if (q->cast())
                                return;
                        }
                    }
                    if (q->cooldown_time() > 0 && e->is_ready() && laneclear::use_e->get_bool() && !myhero->has_buff(buff_hash("GarenE")))
                    {
                        if (myhero->is_under_enemy_turret())
                        {
                            if (myhero->count_enemies_in_range(e->range()) == 0)
                            {
                                if (e->cast())
                                {
                                    return;
                                }
                            }
                        }
                        else if (!myhero->has_buff(buff_hash("GarenE")))
                        {
                            if (e->cast())
                            {
                                return;
                            }
                        }
                    }
                }

                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (q->is_ready() && jungleclear::use_q->get_bool())
                    {
                        if (!myhero->has_buff(buff_hash("GarenE")))
                        {
                            if (q->cast())
                                return;
                        }
                    }
                    if (q->cooldown_time() > 0 && e->is_ready() && jungleclear::use_e->get_bool() && !myhero->has_buff(buff_hash("GarenQ")))
                    {
                        if (!myhero->has_buff(buff_hash("GarenE")))
                        {
                            if (e->cast())
                                return;
                        }
                    }
                }
            }
        }
    }

#pragma region q_logic
    void q_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(combo::q_range->get_int(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target == nullptr)
            return;
        // Check if the distance between myhero and enemy is smaller than q range
        
        if (target->get_distance(myhero) <= q->range())
        {
            q->cast(target);
            //rocketbelt->fast_cast(target->get_position());
        }
            
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(w->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target == nullptr)
            return;

        if (target->get_distance(myhero) <= w->range())
        {
            if (w->cast(target))
                return;
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(e->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target == nullptr)
            return;

        if (target->get_distance(myhero) <= e->range())
        {
            if (e->cast(target))
                return;
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::magical);

        // Always check an object is not a nullptr!
        if (target == nullptr)
            return;
        
        // Checking if the target will die from r damage
        if (r->get_damage(target) > target->get_health())
        {
            if (target->get_distance(myhero) <= r->range())
            {
                if (r->cast(target))
                    return;
            }
        }
    }
#pragma endregion

#pragma region protobelt_logic
    void rocketbelt_logic()
    {
        
        
        const auto rocketbelt_slot = myhero->has_item(ItemId::Hextech_Rocketbelt);
        if (rocketbelt_slot == spellslot::invalid)
            return;
        if (!myhero->is_item_ready(ItemId::Hextech_Rocketbelt))
            return;
        const auto spell = myhero->get_item(rocketbelt_slot);
    
        auto target = target_selector->get_target(combo::q_range->get_int(), damage_type::magical);

        if (target == nullptr)
            return;
    
        myhero->cast_spell(rocketbelt_slot, target->get_position());


    }
#pragma endregion

    void on_before_attack(game_object_script target, bool* process)
    {
        //if (q->is_ready())
        //{
        //    // Using q before autoattack on enemies
        //    if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
        //    {
        //        if (q->cast())
        //        {
        //            return;
        //        }
        //    }
        //}
    }

    void on_draw()
    {

        if (myhero->is_dead())
        {
            return;
        }

        // Draw Q range
        if (draw_settings::draw_range_q->get_bool())
            draw_manager->add_circle(myhero->get_position(), q->range(), Q_DRAW_COLOR);

        // Draw W range
        if (w->is_ready() && draw_settings::draw_range_w->get_bool())
            draw_manager->add_circle(myhero->get_position(), w->range(), W_DRAW_COLOR);

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), E_DRAW_COLOR);

        // Draw R range
        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), R_DRAW_COLOR);
    }

    void on_buff_gain(game_object_script sender, buff_instance_script buff)
    {
        if (sender != myhero)
            return;
            
        
        if (strcmp(buff->get_name_cstr(), "EvelynnQ2") == 0)
            q->set_range(500);
    }

    void on_buff_lose(game_object_script sender, buff_instance_script buff)
    {
        if (sender != myhero)
            return;


        if (strcmp(buff->get_name_cstr(), "EvelynnQ2") == 0)
            q->set_range(800);
    }
}

