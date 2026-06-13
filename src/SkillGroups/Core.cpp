#include "SkillGroups/Core.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace SkillGroups
{
	namespace
	{
		constexpr std::array<std::string_view, SkillCount> kSkillNames{
			"OneHanded",
			"TwoHanded",
			"Archery",
			"Block",
			"Smithing",
			"HeavyArmor",
			"LightArmor",
			"Pickpocket",
			"Lockpicking",
			"Sneak",
			"Alchemy",
			"Speech",
			"Alteration",
			"Conjuration",
			"Destruction",
			"Illusion",
			"Restoration",
			"Enchanting"
		};

		constexpr auto kFirstSkillActorValue = 6U;
		constexpr auto kInvalidGroupIndex = SkillCount;

		[[nodiscard]] std::vector<SkillGroup> MakeDefaultSkillGroups()
		{
			return {
				SkillGroup{ "Crafting", { Skill::Smithing, Skill::Enchanting, Skill::Alchemy } },
				SkillGroup{ "Control", { Skill::Illusion, Skill::Conjuration } },
				SkillGroup{ "Support", { Skill::Restoration, Skill::Alteration, Skill::Sneak } },
				SkillGroup{ "Defence", { Skill::LightArmor, Skill::HeavyArmor } },
				SkillGroup{ "Ranged", { Skill::Archery, Skill::Destruction } },
				SkillGroup{ "Utility", { Skill::Speech, Skill::Pickpocket, Skill::Lockpicking } },
				SkillGroup{ "Melee", { Skill::OneHanded, Skill::TwoHanded, Skill::Block } }
			};
		}

		const std::vector<SkillGroup> kDefaultSkillGroups = MakeDefaultSkillGroups();
		std::vector<SkillGroup> g_activeSkillGroups = MakeDefaultSkillGroups();
		std::array<std::size_t, SkillCount> g_activeSkillGroupIndexes{};
		bool g_activeSkillGroupIndexesBuilt{ false };

		void RebuildActiveSkillGroupIndexes()
		{
			g_activeSkillGroupIndexes.fill(kInvalidGroupIndex);
			for (std::size_t groupIndex = 0; groupIndex < g_activeSkillGroups.size(); ++groupIndex) {
				for (const auto skill : g_activeSkillGroups[groupIndex].skills) {
					g_activeSkillGroupIndexes[static_cast<std::size_t>(skill)] = groupIndex;
				}
			}
			g_activeSkillGroupIndexesBuilt = true;
		}
	}

	std::optional<Skill> ToSkill(std::uint32_t a_actorValue)
	{
		if (a_actorValue < kFirstSkillActorValue) {
			return std::nullopt;
		}

		const auto index = a_actorValue - kFirstSkillActorValue;
		if (index >= SkillCount) {
			return std::nullopt;
		}

		return static_cast<Skill>(index);
	}

	std::uint32_t ToActorValue(Skill a_skill)
	{
		return kFirstSkillActorValue + static_cast<std::uint32_t>(a_skill);
	}

	std::string_view SkillName(Skill a_skill)
	{
		const auto index = static_cast<std::size_t>(a_skill);
		return index < kSkillNames.size() ? kSkillNames[index] : "Unknown";
	}

	std::span<const SkillGroup> SkillGroups()
	{
		return g_activeSkillGroups;
	}

	std::span<const SkillGroup> DefaultSkillGroups()
	{
		return kDefaultSkillGroups;
	}

	std::vector<SkillGroup> BuildSkillGroupsFromAssignments(const std::array<std::string, SkillCount>& a_assignments)
	{
		std::vector<SkillGroup> groups;
		for (std::size_t index = 0; index < a_assignments.size(); ++index) {
			const auto& groupName = a_assignments[index];
			auto group = std::ranges::find_if(groups, [&](const SkillGroup& a_group) {
				return a_group.name == groupName;
			});
			if (group == groups.end()) {
				group = groups.emplace(groups.end(), SkillGroup{ groupName, {} });
			}

			group->skills.push_back(static_cast<Skill>(index));
		}

		return groups;
	}

	void SetActiveSkillGroups(std::vector<SkillGroup> a_groups)
	{
		g_activeSkillGroups = a_groups.empty() ? MakeDefaultSkillGroups() : std::move(a_groups);
		RebuildActiveSkillGroupIndexes();
	}

	const SkillGroup* FindSkillGroup(Skill a_skill)
	{
		const auto groupIndex = FindSkillGroupIndex(a_skill);
		if (!groupIndex) {
			return nullptr;
		}

		return std::addressof(g_activeSkillGroups[*groupIndex]);
	}

	std::optional<std::size_t> FindSkillGroupIndex(Skill a_skill)
	{
		const auto skillIndex = static_cast<std::size_t>(a_skill);
		if (skillIndex >= SkillCount) {
			return std::nullopt;
		}

		if (!g_activeSkillGroupIndexesBuilt) {
			RebuildActiveSkillGroupIndexes();
		}

		auto groupIndex = g_activeSkillGroupIndexes[skillIndex];
		if (groupIndex == kInvalidGroupIndex && !g_activeSkillGroups.empty()) {
			RebuildActiveSkillGroupIndexes();
			groupIndex = g_activeSkillGroupIndexes[skillIndex];
		}

		return groupIndex < g_activeSkillGroups.size() ? std::optional<std::size_t>{ groupIndex } : std::nullopt;
	}

	std::span<const Skill> ActiveGroupSkills(std::size_t a_groupIndex)
	{
		if (a_groupIndex >= g_activeSkillGroups.size()) {
			return {};
		}

		return g_activeSkillGroups[a_groupIndex].skills;
	}

	bool ShouldLevelIncreaseContributeAtLevel(
		const std::array<SkillState, SkillCount>& a_states,
		Skill a_skill,
		float a_activeLevelAfterIncrease)
	{
		const auto groupIndex = FindSkillGroupIndex(a_skill);
		if (!groupIndex) {
			return true;
		}

		const auto activeLevelAfterIncrease = std::floor(a_activeLevelAfterIncrease);
		float otherMax = 0.0F;
		for (const auto skill : g_activeSkillGroups[*groupIndex].skills) {
			if (skill == a_skill) {
				continue;
			}

			otherMax = std::max(otherMax, std::floor(a_states[static_cast<std::size_t>(skill)].level));
		}

		return activeLevelAfterIncrease > otherMax;
	}
}
