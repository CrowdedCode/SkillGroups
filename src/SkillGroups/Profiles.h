#pragma once

#include "SkillGroups/Core.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace SkillGroups::Profiles
{
	struct CharacterXpSettings
	{
		bool useFlatCharacterXp{ false };
		float flatCharacterXp{ 10.0F };
		float levelUpBase{ 75.0F };
		float levelUpMult{ 25.0F };
	};

	void Load();
	[[nodiscard]] std::size_t ProfileCount();
	[[nodiscard]] std::string_view ProfileDisplayName(std::size_t a_profileIndex);
	[[nodiscard]] bool IsProfileEditable(std::size_t a_profileIndex);
	[[nodiscard]] int CreateProfileFrom(std::size_t a_sourceProfileIndex, std::string_view a_name);
	[[nodiscard]] int RenameProfile(std::size_t a_profileIndex, std::string_view a_name);
	[[nodiscard]] float GetCharacterXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex);
	[[nodiscard]] float GetGroupXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_groupIndex);
	[[nodiscard]] float GetPlayerXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_skillIndex);
	[[nodiscard]] float GetSkillXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex);
	[[nodiscard]] CharacterXpSettings GetCharacterXpSettings(std::size_t a_profileIndex);
	[[nodiscard]] const std::array<float, SkillCount>& GetCharacterXpMultipliers(std::size_t a_profileIndex);
	[[nodiscard]] const std::array<float, SkillCount>& GetPlayerXpMultiplierScales(std::size_t a_profileIndex);
	[[nodiscard]] const std::array<float, SkillCount>& GetSkillXpMultipliers(std::size_t a_profileIndex);
	[[nodiscard]] std::span<const SkillGroup> GetSkillGroups(std::size_t a_profileIndex);
	[[nodiscard]] std::size_t GetGroupCount(std::size_t a_profileIndex);
	[[nodiscard]] std::string_view GetGroupName(std::size_t a_profileIndex, std::size_t a_groupIndex);
	[[nodiscard]] std::size_t GetSkillGroupIndex(std::size_t a_profileIndex, std::size_t a_skillIndex);
	[[nodiscard]] std::size_t GetSkillGroupSize(std::size_t a_profileIndex, std::size_t a_skillIndex);
	[[nodiscard]] std::string GetGroupSlotName(std::size_t a_profileIndex, std::size_t a_slotIndex);
	[[nodiscard]] bool SetCharacterXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex, float a_value);
	[[nodiscard]] bool SetGroupXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_groupIndex, float a_value);
	[[nodiscard]] bool SetPlayerXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_skillIndex, float a_value);
	[[nodiscard]] bool SetSkillXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex, float a_value);
	[[nodiscard]] bool SetCharacterXpSettings(std::size_t a_profileIndex, CharacterXpSettings a_settings);
	[[nodiscard]] bool SetSkillGroupAssignments(
		std::size_t a_profileIndex,
		std::span<const std::string> a_groupSlotNames,
		std::span<const std::uint32_t> a_skillGroupSlots);
	[[nodiscard]] bool ApplyGroups(std::size_t a_profileIndex);
	[[nodiscard]] bool SaveProfile(std::size_t a_profileIndex);
}
