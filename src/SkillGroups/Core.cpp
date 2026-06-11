#include "SkillGroups/Core.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace SkillGroups
{
	namespace
	{
		constexpr std::array<Skill, 3> kCrafting{ Skill::Smithing, Skill::Enchanting, Skill::Alchemy };
		constexpr std::array<Skill, 2> kCommand{ Skill::Illusion, Skill::Conjuration };
		constexpr std::array<Skill, 3> kSupport{ Skill::Restoration, Skill::Alteration, Skill::Sneak };
		constexpr std::array<Skill, 2> kDefence{ Skill::LightArmor, Skill::HeavyArmor };
		constexpr std::array<Skill, 2> kRanged{ Skill::Archery, Skill::Destruction };
		constexpr std::array<Skill, 3> kWealth{ Skill::Speech, Skill::Pickpocket, Skill::Lockpicking };
		constexpr std::array<Skill, 3> kMelee{ Skill::OneHanded, Skill::TwoHanded, Skill::Block };

		constexpr std::array<SkillGroup, SkillGroupCount> kSkillGroups{
			SkillGroup{ "Crafting", kCrafting },
			SkillGroup{ "Command", kCommand },
			SkillGroup{ "Support", kSupport },
			SkillGroup{ "Defence", kDefence },
			SkillGroup{ "Ranged", kRanged },
			SkillGroup{ "Wealth", kWealth },
			SkillGroup{ "Melee", kMelee }
		};

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
		return kSkillGroups;
	}

	const SkillGroup* FindSkillGroup(Skill a_skill)
	{
		for (const auto& group : kSkillGroups) {
			if (std::ranges::find(group.skills, a_skill) != group.skills.end()) {
				return std::addressof(group);
			}
		}

		return nullptr;
	}

	std::optional<std::size_t> FindSkillGroupIndex(Skill a_skill)
	{
		for (std::size_t index = 0; index < kSkillGroups.size(); ++index) {
			if (std::ranges::find(kSkillGroups[index].skills, a_skill) != kSkillGroups[index].skills.end()) {
				return index;
			}
		}

		return std::nullopt;
	}

	bool ShouldLevelIncreaseContributeAtLevel(
		const std::array<SkillState, SkillCount>& a_states,
		Skill a_skill,
		float a_activeLevelAfterIncrease)
	{
		const auto* group = FindSkillGroup(a_skill);
		if (!group) {
			return true;
		}

		const auto activeLevelAfterIncrease = std::floor(a_activeLevelAfterIncrease);
		float otherMax = 0.0F;
		for (const auto skill : group->skills) {
			if (skill == a_skill) {
				continue;
			}

			otherMax = std::max(otherMax, std::floor(a_states[static_cast<std::size_t>(skill)].level));
		}

		return activeLevelAfterIncrease > otherMax;
	}
}
