#include "pch.hpp"

void sdk::update()
{
	sdk::m_local_controller = c_cs_player_controller::get_local_player_controller();
}

float sdk::get_tick_interval()
{
	return 1.f / 64.f;
}

float sdk::get_game_time()
{
	if (i::m_global_vars)
	{
		const auto curtime = i::m_global_vars->m_curtime();
		if (std::isfinite(curtime) && curtime > 0.f && curtime < 100000.f)
			return curtime;
	}

	if (i::m_network_game_client)
	{
		const auto server_tick_count = i::m_network_game_client->m_server_tick_count();
		if (server_tick_count > 0)
			return server_tick_count * sdk::get_tick_interval();
	}

	return 0.f;
}
