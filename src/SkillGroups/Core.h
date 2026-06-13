#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace SkillGroups
{
	enum class Skill : std::uint32_t
	{
		OneHanded = 0,
		TwoHanded,
		Archery,
		Block,
		Smithing,
		HeavyArmor,
		LightArmor,
		Pickpocket,
		Lockpicking,
		Sneak,
		Alchemy,
		Speech,
		Alteration,
		Conjuration,
		Destruction,
		Illusion,
		Restoration,
		Enchanting,
		Total
	};

	struct SkillState
	{
		float level{ 0.0F };
	};

	struct SkillGroup
	{
		std::string name;
		std::vector<Skill> skills;
	};

	constexpr auto SkillCount = static_cast<std::size_t>(Skill::Total);
	constexpr auto MaxSkillGroupCount = SkillCount;

	[[nodiscard]] std::optional<Skill> ToSkill(std::uint32_t a_actorValue);
	[[nodiscard]] std::uint32_t ToActorValue(Skill a_skill);
	[[nodiscard]] std::string_view SkillName(Skill a_skill);
	[[nodiscard]] std::span<const SkillGroup> SkillGroups();
	[[nodiscard]] std::span<const SkillGroup> DefaultSkillGroups();
	[[nodiscard]] std::vector<SkillGroup> BuildSkillGroupsFromAssignments(const std::array<std::string, SkillCount>& a_assignments);
	void SetActiveSkillGroups(std::vector<SkillGroup> a_groups);
	[[nodiscard]] const SkillGroup* FindSkillGroup(Skill a_skill);
	[[nodiscard]] std::optional<std::size_t> FindSkillGroupIndex(Skill a_skill);
	[[nodiscard]] std::span<const Skill> ActiveGroupSkills(std::size_t a_groupIndex);
	[[nodiscard]] bool ShouldLevelIncreaseContributeAtLevel(
		const std::array<SkillState, SkillCount>& a_states,
		Skill a_skill,
		float a_activeLevelAfterIncrease);
}
