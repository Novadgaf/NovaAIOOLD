#include "../plugin_sdk/plugin_sdk.hpp"
#include "garen.h"

namespace garen
{
    // Define the colors that will be used in on_draw()
    //
#define Q_DRAW_COLOR (MAKE_COLOR ( 62, 129, 237, 255 ))  //Red Green Blue Alpha
#define W_DRAW_COLOR (MAKE_COLOR ( 227, 203, 20, 255 ))  //Red Green Blue Alpha
#define E_DRAW_COLOR (MAKE_COLOR ( 235, 12, 223, 255 ))  //Red Green Blue Alpha
#define R_DRAW_COLOR (MAKE_COLOR ( 224, 77, 13, 255 ))   //Red Green Blue Alpha

// To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;

    script_spell* flash = nullptr;

    // Declaration of menu objects
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
        TreeEntry* use_q = nullptr;

        TreeEntry* use_w = nullptr;
        TreeEntry* w_enemy_range = nullptr;
        TreeEntry* w_myhero_hp_under = nullptr;

        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_q_on_turret = nullptr;

        TreeEntry* use_e = nullptr;
        TreeEntry* auto_disable_e = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* auto_disable_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_q;
    }

    namespace misc
    {
        TreeEntry* use_w = nullptr;
        TreeEntry* time = nullptr;
        TreeEntry* over_my_hp_in_percent = nullptr;
    }

    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack(game_object_script target, bool* process);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
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
        q = plugin_sdk->register_spell(spellslot::q, 175);
        w = plugin_sdk->register_spell(spellslot::w, 0);
        e = plugin_sdk->register_spell(spellslot::e, 330);
        r = plugin_sdk->register_spell(spellslot::r, 400);

        // Checking which slot the Summoner Flash is on 
        //
        if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
            flash = plugin_sdk->register_spell(spellslot::summoner1, 400.f);
        else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
            flash = plugin_sdk->register_spell(spellslot::summoner2, 400.f);

        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("garen", "Garen");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".comboUseQ", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".comboUseW", "Use W in Team Fight", true);
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = combo->add_tab(myhero->get_model() + ".comboWConfig", "W Config");
                {
                    combo::w_enemy_range = w_config->add_slider(myhero->get_model() + ".comboUseWWhenEnemyInRange", "Use W when over enemy in range", 2, 0, 5);
                    combo::w_myhero_hp_under = w_config->add_slider(myhero->get_model() + ".comboMyheroHpUnder", "Myhero HP is under (in %)", 70, 0, 100);
                }
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".comboUseE", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".comboUseR", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_q = harass->add_checkbox(myhero->get_model() + ".harassUseQ", "Use Q", true);
                harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harassUseE", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::use_q = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseQ", "Use Q", true);
                laneclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_q_on_turret = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseQOnTurret", "Use Q On Turret", true);
                laneclear::use_q_on_turret->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseE", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseQ", "Use Q", true);
                laneclear::auto_disable_e = laneclear->add_checkbox(myhero->get_model() + ".laneclearAutoDisableE", "Auto disable E if no more minion", true);
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclearUseQ", "Use Q", true);
                jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclearrUseE", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                jungleclear::auto_disable_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleAutoDisableE", "Auto disable E if no more monster", true);
            }

            auto misc = main_tab->add_tab(myhero->get_model() + ".misc", "Misc Settings");
            {
                misc::use_w = misc->add_checkbox(myhero->get_model() + ".miscUseW", "Use W", true);
                misc::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto w_config = misc->add_tab(myhero->get_model() + ".miscWConfig", "W Config");
                {
                    misc::time = w_config->add_slider(myhero->get_model() + ".miscWConfigTime", "Set coming damage time (in ms)", 150, 0, 1000);
                    misc::over_my_hp_in_percent = w_config->add_slider(myhero->get_model() + ".miscWConfigOverMyHpInPercent", "Coming damage is over my HP (in %)", 20, 0, 100);
                }
            }
            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
            {
                fleemode::use_q = fleemode->add_checkbox(myhero->get_model() + ".fleemodeUseQ", "Use Q to ran away", true);
                fleemode::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
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


        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
        event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (!myhero->has_buff(buff_hash("GarenW")) && misc::use_w->get_bool() && w->is_ready())
        {
            if ((health_prediction->get_incoming_damage(myhero, misc::time->get_int() / 1000.f, true) * 100.f) /
                myhero->get_max_health() > myhero->get_health_percent() * (misc::over_my_hp_in_percent->get_int() / 100.f))
            {
                w->cast();
            }
        }

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
                if (q->is_ready() && fleemode::use_q->get_bool())
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

                // Automatically disable e when there are no more minions around myhero 
                else if (lane_minions.empty() && laneclear::auto_disable_e->get_bool() && my_hero_region == Position::Line)
                {
                    if (myhero->has_buff(buff_hash("GarenE")) && myhero->count_enemies_in_range(e->range() + 500) == 0)
                    {
                        if (e->cast())
                        {
                            return;
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

                // Automatically disable e when there are no more mobs around myhero 
                else if (monsters.empty() && jungleclear::auto_disable_e->get_bool() && my_hero_region == Position::Jungle)
                {
                    if (myhero->has_buff(buff_hash("GarenE")) && myhero->count_enemies_in_range(e->range() + 500) == 0)
                    {
                        if (e->cast())
                        {
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
        auto target = target_selector->get_target(500, damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            // Check if the distance between myhero and enemy is smaller than q range
            if (target->get_distance(myhero) <= q->range())
            {

                // Checking if the target will die from q damage
                if (q->get_damage(target) >= target->get_health())
                {
                    if (myhero->has_buff(buff_hash("GarenE")))
                    {
                        // Deactivation e
                        if (e->cast())
                        {
                            if (q->cast())
                                return;
                        }
                    }
                    else
                    {
                        if (q->cast())
                            return;
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        if (!myhero->has_buff(buff_hash("GarenW")) && myhero->count_enemies_in_range(400) >= combo::w_enemy_range->get_int()
            && myhero->get_health_percent() < combo::w_myhero_hp_under->get_int())
        {
            if (w->cast())
            {
                return;
            }
        }

        // Use w when incomming damage in 1 second is higher than 25% of myhero's health
        // get_incoming_damage(unit, time (in second), include_skillshot)
        //
        if (!myhero->has_buff(buff_hash("GarenW")) && health_prediction->get_incoming_damage(myhero, 1.f, true) >= myhero->get_health() * 0.25)
        {
            if (w->cast())
            {
                return;
            }
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(e->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (q->level() == 0 && !myhero->has_buff(buff_hash("GarenE")))
            {
                e->cast();
            }

            // Get cooldown time and check buffs
            if (q->cooldown_time() > 0 && !target->is_under_ally_turret() && !myhero->has_buff(buff_hash("GarenE")) && !myhero->has_buff(buff_hash("GarenQ")))
            {
                e->cast();
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(r->range(), damage_type::true_dmg);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            // Checking if the target will die from r damage
            if (r->get_damage(target) > target->get_health())
            {
                // Deactivation e when myhero has buff
                //
                if (myhero->has_buff(buff_hash("GarenE")))
                {
                    if (e->cast())
                    {
                        // Cast r on an object
                        if (r->cast(target))
                            return;
                    }
                }
                else
                {
                    // Cast r on an object
                    if (r->cast(target))
                        return;
                }
            }
        }
    }
#pragma endregion

    void on_before_attack(game_object_script target, bool* process)
    {
        if (q->is_ready())
        {
            // Using q before autoattack on enemies
            if (target->is_ai_hero() && ((orbwalker->combo_mode() && combo::use_q->get_bool()) || (orbwalker->harass() && harass::use_q->get_bool())))
            {
                if (q->cast())
                {
                    return;
                }
            }

            // Using q before autoattack on turrets
            if (orbwalker->lane_clear_mode() && myhero->is_under_enemy_turret() && laneclear::use_q_on_turret->get_bool() && target->is_ai_turret())
            {
                if (q->cast())
                {
                    return;
                }
            }
        }
    }

    void on_draw()
    {

        if (myhero->is_dead())
        {
            return;
        }

        // Draw Q range
        if (q->is_ready() && draw_settings::draw_range_q->get_bool())
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
};