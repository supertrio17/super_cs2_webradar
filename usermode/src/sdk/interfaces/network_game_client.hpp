#pragma once

class c_network_game_client
{
public:
	SCHEMA_ADD_OFFSET(int32_t, m_sign_on_state, dump_a2x::offsets::engine2_dll::dw_network_game_client_sign_on_state);
	SCHEMA_ADD_OFFSET(int32_t, m_max_clients, dump_a2x::offsets::engine2_dll::dw_network_game_client_max_clients);
	SCHEMA_ADD_OFFSET(int32_t, m_server_tick_count, dump_a2x::offsets::engine2_dll::dw_network_game_client_server_tick_count);
};
