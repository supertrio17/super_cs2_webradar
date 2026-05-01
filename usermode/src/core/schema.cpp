#include "pch.hpp"

struct schema_data_t
{
	fnv1a_t m_hashed_field_name = 0;
	uint32_t m_offset = 0;
};

static std::vector<schema_data_t> m_schema_data = {};
static std::set<fnv1a_t> m_missing_schema_data = {};

static const std::vector<schema_data_t> m_schema_fallback_data =
{
	{ fnv1a::hash_const("CBasePlayerController->m_hPawn"), 0x6BC },
	{ fnv1a::hash_const("CBasePlayerController->m_steamID"), 0x778 },
	{ fnv1a::hash_const("CCSPlayerController->m_hPlayerPawn"), 0x904 },
	{ fnv1a::hash_const("CCSPlayerController->m_iCompTeammateColor"), 0x840 },
	{ fnv1a::hash_const("CCSPlayerController->m_pInGameMoneyServices"), 0x800 },
	{ fnv1a::hash_const("CCSPlayerController->m_sSanitizedPlayerName"), 0x858 },
	{ fnv1a::hash_const("CCSPlayerController_InGameMoneyServices->m_iAccount"), 0x40 },
	{ fnv1a::hash_const("CCSPlayer_ItemServices->m_bHasDefuser"), 0x48 },
	{ fnv1a::hash_const("CCSPlayer_ItemServices->m_bHasHelmet"), 0x49 },
	{ fnv1a::hash_const("CCSWeaponBaseVData->m_WeaponType"), 0x520 },
	{ fnv1a::hash_const("CCSWeaponBaseVData->m_szName"), 0x720 },
	{ fnv1a::hash_const("CEntityIdentity->m_designerName"), 0x20 },
	{ fnv1a::hash_const("CEntityIdentity->m_flags"), 0x30 },
	{ fnv1a::hash_const("CEntityInstance->m_pEntity"), 0x10 },
	{ fnv1a::hash_const("CGameSceneNode->m_vecAbsOrigin"), 0xC8 },
	{ fnv1a::hash_const("CGameSceneNode->m_vecOrigin"), 0x80 },
	{ fnv1a::hash_const("CModelState->m_ModelName"), 0xA8 },
	{ fnv1a::hash_const("CPlayer_WeaponServices->m_hActiveWeapon"), 0x60 },
	{ fnv1a::hash_const("CPlayer_WeaponServices->m_hMyWeapons"), 0x48 },
	{ fnv1a::hash_const("CSkeletonInstance->m_modelState"), 0x150 },
	{ fnv1a::hash_const("C_BaseEntity->m_hOwnerEntity"), 0x520 },
	{ fnv1a::hash_const("C_BaseEntity->m_iHealth"), 0x34C },
	{ fnv1a::hash_const("C_BaseEntity->m_iTeamNum"), 0x3EB },
	{ fnv1a::hash_const("C_BaseEntity->m_nSubclassID"), 0x380 },
	{ fnv1a::hash_const("C_BaseEntity->m_pGameSceneNode"), 0x330 },
	{ fnv1a::hash_const("C_BasePlayerPawn->m_pItemServices"), 0x11E8 },
	{ fnv1a::hash_const("C_BasePlayerPawn->m_pWeaponServices"), 0x11E0 },
	{ fnv1a::hash_const("C_CSPlayerPawn->m_ArmorValue"), 0x1C7C },
	{ fnv1a::hash_const("C_CSPlayerPawn->m_angEyeAngles"), 0x3360 },
	{ fnv1a::hash_const("C_CSPlayerPawn->m_bIsScoped"), 0x1C50 },
	{ fnv1a::hash_const("C_CSPlayerPawnBase->m_flFlashOverlayAlpha"), 0x13F4 },
	{ fnv1a::hash_const("C_Inferno->m_bFireIsBurning"), 0x1618 },
	{ fnv1a::hash_const("C_Inferno->m_fireCount"), 0x1958 },
	{ fnv1a::hash_const("C_Inferno->m_firePositions"), 0x1018 },
	{ fnv1a::hash_const("C_Inferno->m_nFireEffectTickBegin"), 0x196C },
	{ fnv1a::hash_const("C_PlantedC4->m_bBeingDefused"), 0x119C },
	{ fnv1a::hash_const("C_PlantedC4->m_bBombDefused"), 0x11B4 },
	{ fnv1a::hash_const("C_PlantedC4->m_bBombTicking"), 0x1160 },
	{ fnv1a::hash_const("C_PlantedC4->m_flC4Blow"), 0x1190 },
	{ fnv1a::hash_const("C_PlantedC4->m_flDefuseCountDown"), 0x11B0 },
	{ fnv1a::hash_const("C_SmokeGrenadeProjectile->m_nSmokeEffectTickBegin"), 0x1250 },
	{ fnv1a::hash_const("C_SmokeGrenadeProjectile->m_vSmokeDetonationPos"), 0x1268 },
};

bool schema::setup()
{
	m_schema_data.clear();
	m_missing_schema_data.clear();

	const auto type_scope = i::m_schema_system->find_type_scope_for_module(CLIENT_DLL);
	if (!type_scope)
		return false;

	const auto table_size = type_scope->m_hash_classes().size();
	LOG_INFO("found '%d' schema classes in module '%s'", table_size, CLIENT_DLL);

	std::unique_ptr<uintptr_t[]> elements = std::make_unique_for_overwrite<uintptr_t[]>(table_size);

	const auto elements_size = type_scope->m_hash_classes().get_elements(0, table_size, elements.get());
	for (uint32_t idx = 0; idx < elements_size; idx++)
	{
		const auto element = elements[idx];
		if (!element)
			continue;

		const auto class_binding = type_scope->m_hash_classes()[element];
		if (!class_binding)
			continue;

		auto [schema_field_size, schema_field] = class_binding->get_fields();
		for (uint32_t f_idx = 0; f_idx < schema_field_size; f_idx++)
		{
			if (!schema_field)
				break;

			auto buff = format("{}->{}", class_binding->m_binary_name(), schema_field->m_name());
			m_schema_data.emplace_back(fnv1a::hash(buff), schema_field->m_single_inheritance_offset());

			schema_field = reinterpret_cast<c_schema_class_field_data*>(reinterpret_cast<uintptr_t>(schema_field) + sizeof(c_schema_class_field_data));
		}
	}

	if (!m_schema_data.size())
		return false;

	return true;
}

uint32_t schema::get_offset(const fnv1a_t hashed_field_name)
{
	if (const auto it = std::ranges::find_if(m_schema_data, [hashed_field_name](const schema_data_t& data)
	{
		return data.m_hashed_field_name == hashed_field_name;
	}); it != m_schema_data.end())
		return it->m_offset;

	if (const auto it = std::ranges::find_if(m_schema_fallback_data, [hashed_field_name](const schema_data_t& data)
	{
		return data.m_hashed_field_name == hashed_field_name;
	}); it != m_schema_fallback_data.end())
		return it->m_offset;

	if (m_missing_schema_data.insert(hashed_field_name).second)
		LOG_ERROR("failed to find an offset for the field with the hash value '%llu'", static_cast<unsigned long long>(hashed_field_name));

	return {};
}
