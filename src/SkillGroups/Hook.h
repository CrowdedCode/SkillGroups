#pragma once

#include <cstddef>

namespace SkillGroups::Hook
{
	[[nodiscard]] bool CacheMultipliers();
	[[nodiscard]] bool CacheSkillXpMultipliers();
	[[nodiscard]] float GetCachedPlayerXpMultiplier(std::size_t a_index);
	[[nodiscard]] float GetCachedSkillXpMultiplier(std::size_t a_index);
	[[nodiscard]] bool ApplyCustomMultipliersToGame();
	[[nodiscard]] bool ApplySkillXpMultipliersToGame();
	[[nodiscard]] bool Install();
}
