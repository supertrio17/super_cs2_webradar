#include "pch.hpp"

bool i::setup()
{
    bool success = true;

    const auto [client_base, client_size] = m_memory->get_module_info(CLIENT_DLL);
    if (!client_base.has_value() || !client_size.has_value())
        return {};

    const auto [engine2_base, engine2_size] = m_memory->get_module_info(ENGINE2_DLL);
    if (!engine2_base.has_value() || !engine2_size.has_value())
        return {};

    const auto schema_pattern = m_memory->find_pattern(SCHEMASYSTEM_DLL, GET_SCHEMA_SYSTEM);
    if (!schema_pattern.has_value())
        return {};

    m_schema_system = schema_pattern.value().rip().as<c_schema_system*>();
    success &= (m_schema_system != nullptr);

    m_global_vars = m_memory->read_t<c_global_vars*>(client_base.value() + dump_a2x::offsets::client_dll::dw_global_vars);
    if (!m_global_vars)
    {
        const auto global_vars_pattern = m_memory->find_pattern(CLIENT_DLL, GET_GLOBAL_VARS);
        if (global_vars_pattern.has_value())
            m_global_vars = m_memory->read_t<c_global_vars*>(global_vars_pattern.value().rip().as<c_global_vars*>());
    }
    success &= (m_global_vars != nullptr);

    m_game_entity_system = m_memory->read_t<c_game_entity_system*>(client_base.value() + dump_a2x::offsets::client_dll::dw_game_entity_system);
    if (!m_game_entity_system)
    {
        const auto entity_list_pattern = m_memory->find_pattern(CLIENT_DLL, GET_ENTITY_LIST);
        if (entity_list_pattern.has_value())
            m_game_entity_system = m_memory->read_t<c_game_entity_system*>(entity_list_pattern.value().rip().as<c_game_entity_system*>());
    }
    success &= (m_game_entity_system != nullptr);

    m_network_game_client = m_memory->read_t<c_network_game_client*>(engine2_base.value() + dump_a2x::offsets::engine2_dll::dw_network_game_client);
    if (!m_network_game_client)
        LOG_WARNING("failed to initialize network game client");

    return success;
}
