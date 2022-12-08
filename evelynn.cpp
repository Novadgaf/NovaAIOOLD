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
    #define GREEN (MAKE_COLOR ( 0,255,0, 255 )) 
    #define RED (MAKE_COLOR ( 255,0,0, 255 )) 

    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;

    script_spell* flash = nullptr;
    script_spell* smite = nullptr;

    TreeTab* main_tab = nullptr;

    const char* valid_w_monsters[1] = { "SRU_Gromp" };
    uint32_t eve_q_hash = 2887908017;
    float w_cast_tp = 0;
    bool timer_enabled = false;

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
        TreeEntry* w_mode_swap = nullptr;
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

    // combo
    bool q_logic();
    bool w_logic();
    bool e_logic();
    bool r_logic();
    bool rocketbelt_logic();

    // charge w logic
    bool w_timer_logic();
    bool check_w_charged();
    void startTimer();
    void resetTimer();
    float getTimerStatus();

    // farm logic
    bool isValidWMonster(game_object_script monster);
    bool validW(game_object_script monster);

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
        q->set_skillshot(0.25f, 60.0f, 2400.0f, { collisionable_objects::minions, collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);
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
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".comboUseW", "Use W", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                //combo::wait_for_charm = combo->add_checkbox(myhero->get_model() + ".comboWaitForCharm", "Wait for Charm", true);
                combo::wait_for_charm = combo->add_hotkey(myhero->get_model() + ".comboWaitForCharm", "Wait for Charm", TreeHotkeyMode::Toggle, 'K', true);
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

    
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        // killsteal with r
        if (r->is_ready() && killsteal::use_r->get_bool())
        {
            if (r_logic())
                return;
        }
       
        if (orbwalker->combo_mode())
        {
            // r should always kill if in combo mode so we declare it here in order to not write it 2 times
            if (r->is_ready() && combo::use_r->get_bool())
            {
                if (r_logic())
                    return;
            }

            if (combo::wait_for_charm->get_bool() && combo::use_w->get_bool())
            {
                if (w->is_ready())
                {
                    if (w_timer_logic())
                        return;
                }

                if (!(myhero->get_spell_state(w->get_slot()) & spell_state::NotLearned))
                    if (!check_w_charged())
                        return;
                        
                if (e->is_ready() && combo::use_e->get_bool())
                {
                    if (e_logic())
                        return;
                }

                if (q->is_ready() && combo::use_q->get_bool())
                {
                    if (q_logic())
                        return;
                }

            } 
            else
            {
                if (q->is_ready() && combo::use_q->get_bool())
                {
                    if (q_logic())
                        return;
                }

                if (combo::use_rocketbelt->get_bool())
                {
                    if (rocketbelt_logic())
                        return;
                }

                if (w->is_ready() && combo::use_w->get_bool())
                {
                    if (w_logic())
                        return;
                }

                if (e->is_ready() && combo::use_e->get_bool())
                {
                    if (e_logic())
                        return;
                }
            }
                
        }

        if (orbwalker->harass())
        {
            if (q->is_ready() && harass::use_q->get_bool())
            {
                if (q_logic())
                    return;
            }

            if (w->is_ready() && combo::use_w->get_bool())
            {
                if (w_logic())
                    return;
            }

            if (e->is_ready() && harass::use_e->get_bool())
            {
                if (e_logic())
                    return;
            }
        }

        // Checking if the user has selected flee_mode() (Default Z)
        if (orbwalker->flee_mode())
        {
            if (r->is_ready() && fleemode::use_r->get_bool())
            {
                    
                if (r->cast())
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
                    return !x->is_valid_target(q->range());
                }), lane_minions.end());

            // You can use this function to delete monsters that aren't in the specified range
            monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                {
                    return !x->is_valid_target(q->range());
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
                    if (q->cast(lane_minions[0]))
                        return;
                }
                if (e->is_ready() && laneclear::use_e->get_bool())
                {
                    if (e->cast(lane_minions[0]))
                    {
                        return;
                    }
                }
            }

            if (!monsters.empty())
            {
                // Logic responsible for monsters
                if (validW(monsters[0]))
                {
                    startTimer();
                    if (w->cast(monsters[0]))
                    {
                        return;
                    }
                }

                // wait for w charge
                if (!w->is_ready() && getTimerStatus() <= 2.5f && timer_enabled)
                {
                    return;
                }
                else if (getTimerStatus() >= 2.5f)
                    resetTimer();
                        

                if (q->is_ready() && jungleclear::use_q->get_bool())
                {
                    if (q->cast(monsters[0]))
                        return;
                }
                if (e->is_ready() && jungleclear::use_e->get_bool())
                {
                    if (e->cast(monsters[0]))
                        return;
                }
            }
        }
    }

    #pragma region q_logic
    bool q_logic()
    {
        auto target = target_selector->get_target(combo::q_range->get_int(), damage_type::magical);

        if (target == nullptr)
            return false;
        
        if (target->get_distance(myhero) <= q->range())
        {
            q->cast(target);
            return true;
        }
        return false;
    }
    #pragma endregion


    #pragma region w_logic
    bool w_logic()
    {
        auto target = target_selector->get_target(w->range(), damage_type::magical);

        if (target == nullptr)
            return false;

        if (target->get_distance(myhero) <= w->range())
        {
            w->cast(target);
            return true;
        }
        return false;
    }
    #pragma endregion


    #pragma region e_logic
    bool e_logic()
    {
        auto target = target_selector->get_target(e->range(), damage_type::magical);

        if (target == nullptr)
            return false;;

        if (target->get_distance(myhero) <= e->range())
        {
            e->cast(target);
            return true;
        }
        return false;
    }
    #pragma endregion

    #pragma region r_logic
    bool r_logic()
    {
        auto target = target_selector->get_target(r->range(), damage_type::magical);

        if (target == nullptr)
            return false;
        
        if (r->get_damage(target) > target->get_health())
        {
            if (target->get_distance(myhero) <= r->range())
            {
                r->cast(target);
                return true;
            }
        }
        return false;
    }
    #pragma endregion

    #pragma region protobelt_logic
    bool rocketbelt_logic()
    {
        const auto rocketbelt_slot = myhero->has_item(ItemId::Hextech_Rocketbelt);
        if (rocketbelt_slot == spellslot::invalid)
            return false;
        if (!myhero->is_item_ready(ItemId::Hextech_Rocketbelt))
            return false;
        const auto spell = myhero->get_item(rocketbelt_slot);
    
        auto target = target_selector->get_target(combo::q_range->get_int(), damage_type::magical);

        if (target == nullptr)
            return false;
    
        myhero->cast_spell(rocketbelt_slot, target->get_position());
        return true;
    }
    #pragma endregion


    // perfect w logic
    #pragma region w_timer_logic
    bool w_timer_logic()
    {
        auto target = target_selector->get_target(w->range(), damage_type::magical);

        if (target == nullptr)
            return false;

        if (target->get_distance(myhero) <= w->range())
        {
            w->cast(target);
            startTimer();
            return true;
        }
        return false;
    }
    #pragma endregion


    #pragma region check_w_state
    bool check_w_charged()
    {
        auto target = target_selector->get_target(w->range(), damage_type::magical);

        if (target == nullptr)
            return false;

        float distance = myhero->get_distance(target->get_position());
        float q_travel_time = distance / q->get_speed();
        float time_passed = getTimerStatus();
        float charged_time = q_travel_time + time_passed;
        
        if (charged_time >= 2.5f)
        {
            resetTimer();
            return true;
        }
        return false;
    }
    #pragma endregion


    #pragma region startTimer
    void startTimer()
    {
        timer_enabled = true;
        w_cast_tp = gametime->get_time();
    }
    #pragma endregion


    #pragma region resetTimer
    void resetTimer()
    {
        timer_enabled = false;
        w_cast_tp = 0;
    }
    #pragma endregion


    #pragma region getTimerStatus
    float getTimerStatus()
    {
        float current_tp = gametime->get_time();
        float time_passed = current_tp-w_cast_tp;
        return time_passed;
    }
    #pragma endregion


    // jungle logic
    #pragma region isValidWMonster()
    bool isValidWMonster(game_object_script monster)
    {
        if (monster->is_jungle_buff() || monster->is_epic_monster() || std::strcmp(monster->get_model_cstr(), "SRU_Gromp") == 0)
            return true;
        return false;
    }
    #pragma endregion


    #pragma region validW()
    bool validW(game_object_script monster)
    {
        if (monster == nullptr)
            return false;

        if (w->is_ready() && !q->is_ready() && !e->is_ready() && isValidWMonster(monster) && !myhero->has_buff(eve_q_hash))
        {
            return true;
        }
        console->print("else");
        return false;
    }
    #pragma endregion


    void on_before_attack(game_object_script target, bool* process)
    {
        *process = !timer_enabled;
    }


    void on_draw()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (draw_settings::draw_range_q->get_bool())
            draw_manager->add_circle(myhero->get_position(), q->range(), Q_DRAW_COLOR);

        if (w->is_ready() && draw_settings::draw_range_w->get_bool())
            draw_manager->add_circle(myhero->get_position(), w->range(), W_DRAW_COLOR);

        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), E_DRAW_COLOR);

        if (r->is_ready() && draw_settings::draw_range_r->get_bool())
            draw_manager->add_circle(myhero->get_position(), r->range(), R_DRAW_COLOR);

        auto pos = myhero->get_position();
        renderer->world_to_screen(pos, pos);
        if (combo::wait_for_charm->get_bool())
        {
            draw_manager->add_text_on_screen(pos + vector(-45, 30), GREEN, 15, "CHARGE W: [ON]");
        }
        else
        {
            draw_manager->add_text_on_screen(pos + vector(-45, 30), RED, 15, "CHARGE W: [OFF]");
        }
    }


    void on_buff_gain(game_object_script sender, buff_instance_script buff)
    {
        if (sender == myhero && buff->get_hash_name() == eve_q_hash)
            q->set_range(590);
    }


    void on_buff_lose(game_object_script sender, buff_instance_script buff)
    {
        if (sender == myhero && buff->get_hash_name() == eve_q_hash)
            q->set_range(800);
    }
}