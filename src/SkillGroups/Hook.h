#pragma once

#include <cstddef>

namespace SkillGroups::Hook
{
	[[nodiscard]] bool RefreshCharacterXpMultipliers();
	[[nodiscard]] bool CacheSkillXpMultipliers();
	[[nodiscard]] float GetCachedSkillXpMultiplier(std::size_t a_index);
	[[nodiscard]] bool ApplyCharacterXpGameSettings(float a_levelUpBase, float a_levelUpMult);
	[[nodiscard]] float GetCharacterXpLevelUpBase();
	[[nodiscard]] float GetCharacterXpLevelUpMult();
	[[nodiscard]] bool ResyncCurrentLevelThreshold(float a_levelUpBase, float a_levelUpMult);
	[[nodiscard]] bool ApplySkillXpMultipliersToGame(bool a_useDivisor, std::size_t a_profileIndex);
	[[nodiscard]] bool Install();
}
