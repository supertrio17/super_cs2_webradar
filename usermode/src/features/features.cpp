#include "pch.hpp"

bool f::run()
{
    if (!sdk::m_local_controller)
        return false;

    const auto local_team = sdk::m_local_controller->m_iTeamNum();
    if (local_team == e_team::none || local_team == e_team::spec)
        return false;

    m_data.clear();
    m_player_data.clear();

    m_data["m_local_team"] = local_team;

    get_map();
    get_player_info();
    return true;
}

void f::get_map()
{
    if (!i::m_global_vars)
    {
        m_data["m_map"] = "invalid";
        return;
    }

    auto map_name = i::m_global_vars->m_map_name();
    if (map_name.empty() || map_name.find("<empty>") != std::string::npos)
    {
        const auto [client_base, client_size] = m_memory->get_module_info(CLIENT_DLL);
        if (client_base.has_value() && client_size.has_value())
            i::m_global_vars = m_memory->read_t<c_global_vars*>(client_base.value() + dump_a2x::offsets::client_dll::dw_global_vars);

        if (!i::m_global_vars)
        {
            const auto global_vars_pattern = m_memory->find_pattern(CLIENT_DLL, GET_GLOBAL_VARS);
            if (global_vars_pattern.has_value())
                i::m_global_vars = m_memory->read_t<c_global_vars*>(global_vars_pattern.value().rip().as<c_global_vars*>());
        }

        map_name = i::m_global_vars ? i::m_global_vars->m_map_name() : "";
    }

    if (map_name.empty() || map_name.find("<empty>") != std::string::npos)
    {
        m_data["m_map"] = "invalid";
        return;
    }

    if (f::features_vars::map_name != map_name)
    {
        f::bomb::update_bomb_dmg_info(map_name);
        f::features_vars::map_name = map_name;
    }

    m_data["m_map"] = map_name;
}

void f::get_player_info()
{
    m_data["m_players"].clear();
    m_data["m_grenades"]["landed"].clear();
    m_data["m_grenades"]["thrown"].clear();
    m_data["m_dropped_weapons"].clear();
    m_bomb_idx = ENT_ENTRY_MASK;
    f::features_vars::bomb_blow_time = 0.f;

    auto* entity_system = i::m_game_entity_system;
    if (!entity_system)
        return;

    struct player_entity_t
    {
        int32_t idx;
        c_cs_player_controller* player;
        c_cs_player_pawn* player_pawn;
    };

    std::vector<player_entity_t> players_to_process{};
    players_to_process.reserve(16);

    bool planted_bomb_detected = false;

    const auto highest_entity_index = std::clamp(entity_system->get_highest_entity_index(), 0, ENT_MAX_NETWORKED_ENTRY);
    for (int32_t idx = 0; idx <= highest_entity_index; idx++)
    {
        const auto entity = entity_system->get(idx);
        if (!entity)
            continue;

        const auto entity_identity = entity->m_pEntity();
        if (!entity_identity)
            continue;

        const auto designer_name = entity_identity->m_designerName();
        const auto hashed_designer_name = designer_name.empty() ? 0 : fnv1a::hash(designer_name);

        const auto class_name = entity->get_schema_class_name();
        const auto hashed_class_name = class_name.empty() ? 0 : fnv1a::hash(class_name);

        const bool is_player_controller =
            hashed_class_name == hashes::PLAYER_CONTROLLER ||
            hashed_designer_name == hashes::PLAYER_CONTROLLER_DN;

        if (is_player_controller)
        {
            const auto player = reinterpret_cast<c_cs_player_controller*>(entity);
            if (!player)
                continue;

            const auto player_pawn = player->get_player_pawn();
            if (!player_pawn)
                continue;

            players_to_process.push_back({ idx, player, player_pawn });
            continue;
        }

        const bool is_carried_c4 =
            hashed_class_name == hashes::C4 ||
            hashed_designer_name == hashes::WEAPON_C4_DN;

        if (is_carried_c4)
        {
            if (!planted_bomb_detected)
                f::bomb::get_carried_bomb(entity);

            continue;
        }

        const bool is_planted_c4 =
            hashed_class_name == hashes::PLANTED_C4 ||
            hashed_designer_name == hashes::PLANTED_C4_DN;

        if (is_planted_c4)
        {
            if (f::bomb::get_planted_bomb(reinterpret_cast<c_planted_c4*>(entity)))
            {
                planted_bomb_detected = true;
                m_bomb_idx = ENT_ENTRY_MASK;
            }

            continue;
        }

        const bool is_smoke_projectile =
            hashed_class_name == hashes::SMOKE ||
            hashed_designer_name == hashes::SMOKE_DN;

        if (is_smoke_projectile)
        {
            m_grenade_data.clear();
            m_grenade_thrown_data.clear();

            if (!f::grenades::get_smoke(reinterpret_cast<c_smoke_grenade*>(entity)))
            {
                if (f::grenades::get_thrown(reinterpret_cast<c_base_grenade*>(entity)))
                {
                    m_grenade_thrown_data["m_idx"] = idx;
                    m_data["m_grenades"]["thrown"].push_back(m_grenade_thrown_data);
                }

                continue;
            }

            m_grenade_data["m_idx"] = idx;
            m_data["m_grenades"]["landed"].push_back(m_grenade_data);
            continue;
        }

        const bool is_inferno =
            hashed_class_name == hashes::INFERNO ||
            hashed_designer_name == hashes::INFERNO_DN;

        if (is_inferno)
        {
            m_grenade_data.clear();

            if (!f::grenades::get_molo(reinterpret_cast<c_molo_grenade*>(entity)))
                continue;

            m_grenade_data["m_idx"] = idx;
            m_data["m_grenades"]["landed"].push_back(m_grenade_data);
            continue;
        }

        const bool is_thrown_grenade =
            hashed_class_name == hashes::HE ||
            hashed_class_name == hashes::FLASH ||
            hashed_class_name == hashes::DECOY ||
            hashed_class_name == hashes::MOLOTOV ||
            hashed_designer_name == hashes::HE_DN ||
            hashed_designer_name == hashes::FLASH_DN ||
            hashed_designer_name == hashes::DECOY_DN ||
            hashed_designer_name == hashes::MOLOTOV_DN ||
            hashed_designer_name == hashes::INCGRENADE_DN;

        if (is_thrown_grenade)
        {
            m_grenade_thrown_data.clear();

            if (!f::grenades::get_thrown(reinterpret_cast<c_base_grenade*>(entity)))
                continue;

            m_grenade_thrown_data["m_idx"] = idx;
            m_data["m_grenades"]["thrown"].push_back(m_grenade_thrown_data);
            continue;
        }

        if (f::dropped_weapons::is_weapon(designer_name))
        {
            m_dropped_weapon_data.clear();

            if (!f::dropped_weapons::get_weapon(reinterpret_cast<c_base_entity*>(entity)))
                continue;

            m_dropped_weapon_data["m_idx"] = idx;
            m_data["m_dropped_weapons"].push_back(m_dropped_weapon_data);
        }
    }

    for (const auto& player_entry : players_to_process)
    {
        if (!f::players::get_data(player_entry.idx, player_entry.player, player_entry.player_pawn))
            continue;

        f::players::get_weapons(player_entry.player_pawn);
        f::players::get_active_weapon(player_entry.player_pawn);

        m_data["m_players"].push_back(m_player_data);
    }
}
