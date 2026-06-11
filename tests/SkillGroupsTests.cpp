#include "SkillGroups/Core.h"

#include <array>
#include <cstdlib>
#include <iostream>

namespace
{
	using namespace SkillGroups;

	void Require(bool a_condition, const char* a_message)
	{
		if (!a_condition) {
			std::cerr << a_message << '\n';
			std::exit(EXIT_FAILURE);
		}
	}

	std::array<SkillState, SkillCount> DefaultStates()
	{
		std::array<SkillState, SkillCount> states{};
		for (auto& state : states) {
			state.level = 15.0F;
		}

		return states;
	}
}

int main()
{
	auto states = DefaultStates();

	states[static_cast<std::size_t>(Skill::OneHanded)].level = 50.0F;
	states[static_cast<std::size_t>(Skill::TwoHanded)].level = 49.0F;
	Require(!ShouldLevelIncreaseContributeAtLevel(states, Skill::TwoHanded, 50.0F), "catch-up rank to a tied group maximum must not contribute");
	Require(ShouldLevelIncreaseContributeAtLevel(states, Skill::TwoHanded, 51.0F), "surpassing the old group maximum must contribute");

	states[static_cast<std::size_t>(Skill::TwoHanded)].level = 49.0F;
	Require(!ShouldLevelIncreaseContributeAtLevel(states, Skill::TwoHanded, 49.0F), "lower rank-up below the group maximum must not contribute");

	states[static_cast<std::size_t>(Skill::TwoHanded)].level = 50.0F;
	Require(ShouldLevelIncreaseContributeAtLevel(states, Skill::TwoHanded, 51.0F), "group leader rank-up should contribute");

	states[static_cast<std::size_t>(Skill::Block)].level = 60.0F;
	Require(!ShouldLevelIncreaseContributeAtLevel(states, Skill::TwoHanded, 51.0F), "leader in the same group should suppress lower rank-up contribution");

	Require(!ToSkill(0U).has_value(), "non-skill actor values must be ignored");
	Require(ToSkill(ToActorValue(Skill::Enchanting)) == Skill::Enchanting, "actor value mapping must round-trip");

	const auto* melee = FindSkillGroup(Skill::Block);
	Require(melee && melee->name == "Melee", "block should belong to the melee group");
	const auto* defense = FindSkillGroup(Skill::HeavyArmor);
	Require(defense && defense->name == "Defence", "heavy armor should belong to the defence group");

	std::cout << "SkillGroups tests passed\n";
	return EXIT_SUCCESS;
}
