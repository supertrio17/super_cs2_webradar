#pragma once

namespace dump_a2x
{
	namespace offsets
	{
		namespace client_dll
		{
			/* generated using https://github.com/a2x/cs2-dumper (2026-04-30) */
			constexpr uintptr_t dw_entity_list = 0x24D1DF0;
			constexpr uintptr_t dw_game_entity_system = 0x24D1DF0;
			constexpr uintptr_t dw_game_entity_system_highest_entity_index = 0x2090;
			constexpr uintptr_t dw_global_vars = 0x204C5D8;
			constexpr uintptr_t dw_local_player_controller = 0x230B5D0;
		}

		namespace engine2_dll
		{
			constexpr uintptr_t dw_network_game_client = 0x90A0C0;
			constexpr uintptr_t dw_network_game_client_sign_on_state = 0x230;
			constexpr uintptr_t dw_network_game_client_max_clients = 0x240;
			constexpr uintptr_t dw_network_game_client_server_tick_count = 0x24C;
		}
	}
}
