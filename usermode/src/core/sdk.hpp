#pragma once

namespace sdk
{
	void update();
	float get_tick_interval();
	float get_game_time();

	inline c_cs_player_controller* m_local_controller = nullptr;
}
