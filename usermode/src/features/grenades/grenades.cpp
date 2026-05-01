#include "pch.hpp"

namespace
{
	constexpr float SMOKE_DURATION = 21.5f;
	constexpr float MOLO_DURATION = 7.0f;
}

bool f::grenades::get_smoke(c_smoke_grenade* smoke)
{
	if (!smoke)
		return false;

	const auto begin_tick = smoke->m_nSmokeEffectTickBegin();
	if (begin_tick <= 0)
		return false;

	const auto curtime = sdk::get_game_time();
	if (curtime <= 0.f)
		return false;

	const auto begin_time = static_cast<float>(begin_tick) * sdk::get_tick_interval();
	const auto dis_time = SMOKE_DURATION - (curtime - begin_time);
	if (dis_time <= 0.f)
		return false;

	auto det_pos = smoke->m_vSmokeDetonationPos();
	if (det_pos.is_zero())
		det_pos = smoke->get_scene_origin();

	if (det_pos.is_zero())
		return false;

	m_grenade_data = {
		{"m_type", "smoke"},
		{"m_duration", SMOKE_DURATION},
		{"m_timeleft", std::max(0.f, dis_time)},
		{"m_x", det_pos.m_x},
		{"m_y", det_pos.m_y},
		{"m_z", det_pos.m_z}
	};

	return true;
}

bool f::grenades::get_molo(c_molo_grenade* molo)
{
	if (!molo)
		return false;

	const auto begin_tick = molo->m_nFireEffectTickBegin();
	if (begin_tick <= 0)
		return false;

	const auto curtime = sdk::get_game_time();
	if (curtime <= 0.f)
		return false;

	const auto begin_time = static_cast<float>(begin_tick) * sdk::get_tick_interval();
	const auto dis_time = MOLO_DURATION - (curtime - begin_time);
	if (dis_time <= 0.f)
		return false;

	const auto scene_node = molo->m_pGameSceneNode();
	if (!scene_node)
		return false;

	const auto vec_origin = scene_node->m_vecOrigin();

	auto fire_pos_local = nlohmann::json{};

	const auto fire_burning = molo->m_bFireIsBurning();
	const auto fire_positions = molo->m_firePositions();
	const auto fire_count = std::clamp(molo->m_fireCount(), 0, 64);

	for (int idx = 0; idx < fire_count; idx++)
	{
		if (!fire_burning[idx])
			continue;

		fire_pos_local.push_back({ fire_positions[idx].m_x, fire_positions[idx].m_y, fire_positions[idx].m_z });
	}

	if (fire_pos_local.empty())
		return false;

	m_grenade_data = {
		{"m_type", "molo"},
		{"m_duration", MOLO_DURATION},
		{"m_timeleft", std::max(0.f, dis_time)},
		{"m_x", vec_origin.m_x},
		{"m_y", vec_origin.m_y},
		{"m_z", vec_origin.m_z},
		{"m_firePositions", std::move(fire_pos_local)}
	};

	return true;
}

bool f::grenades::get_thrown(c_base_grenade* nade)
{
	if (!nade)
		return false;

	const auto scene_node = nade->m_pGameSceneNode();
	if (!scene_node)
		return false;

	const auto nade_pos = scene_node->m_vecOrigin();
	if (nade_pos.is_zero())
		return false;

	const auto entity_identity = nade->m_pEntity();
	if (!entity_identity)
		return false;

	std::string grenade_type = entity_identity->m_designerName();
	if (grenade_type.empty())
		return false;

	constexpr std::string_view projectile_suffix = "_projectile";
	if (grenade_type.size() > projectile_suffix.size() && grenade_type.ends_with(projectile_suffix))
		grenade_type.erase(grenade_type.size() - projectile_suffix.size());

	m_grenade_thrown_data = {
		{"m_type", grenade_type},
		{"m_x", nade_pos.m_x},
		{"m_y", nade_pos.m_y},
		{"m_z", nade_pos.m_z}
	};

	return true;
}
