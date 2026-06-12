# SkillGroups

SkillGroups is an SKSE/CommonLibSSE NG plugin for Skyrim Special Edition that changes how skill rank-ups contribute to character level. Skills still gain XP and level normally, but character XP is gated by skill groups so that redundant skills do not affect player level.

The goal is to make character level track peak capability instead of total accumulated skills. Learning a second combat style can add versatility without making the world scale as if the character's max power increases.

## Current Behavior

SkillGroups hooks Skyrim's character-XP calculation for skill rank-ups. When a skill ranks up, the plugin checks that skill's group and only awards character XP if the new rank exceeds the other skills in that group.

For example, if One-Handed is 50 and Two-Handed ranks from 49 to 50, Two-Handed does not contribute character XP. If Two-Handed later ranks from 50 to 51, it contributes character XP because it has exceeded the previous group maximum.

The plugin does not replace Skyrim's skill progression. Skill XP, skill rank-ups, level-up UI, overflow behavior, and perk progression continue to run through the game.

## Up next

	- make divideByGroup apply to values immediately for transparency and do not affect cached live values for consistency. 
		- E.g. toggling it will turn all 1.0 values to 1.0/groupSize, and caching will not apply any groupSize operation since if it is ticked, it should be getting the scaled values.
		- If another mod changes it, it should be unticked before caching to prevent inconsistent scaling. 
			- E.g. a mod changes a 0.33 value back to 1. reading that should return 1, but if it is ticked, unticking it would result in 3, not 0.99 (0.99 is close enough, or could be rounded to the nearest 0.05)
 
## V1.0 Groups

The V1.0 groups are fixed:

```text
Melee: One-Handed, Two-Handed, Block
Ranged: Archery, Destruction
Defence: Light Armor, Heavy Armor
Support: Restoration, Alteration, Sneak
Control: Illusion, Conjuration
Utility: Speech, Pickpocket, Lockpicking
Crafting: Smithing, Enchanting, Alchemy
```

## Configuration

Settings are available through the SkyUI MCM. The plugin also ships default settings in:

```text
Data/SKSE/Plugins/SkillGroups.ini
Data/MCM/Config/SkillGroups/settings.ini
```

Character XP configuration supports per-skill contribution multipliers, per-skill character XP scaling, and per-group multipliers. The group contribution is based on the sum of the configured effective multipliers for skills in that group.

Skill XP configuration is separate. It can cache and apply Skyrim's `useMult` values, optionally dividing the applied value by fixed group size.

Profiles are loaded from:

```text
Data/SKSE/Plugins/SkillGroups/Profiles/*.ini
```

The shipped profiles are `Default.ini`, `Custom.ini`, `Warrior.ini`, `Hunter.ini`, `Thief.ini`, and `Mage.ini`. Profile files are complete profiles: each valid file must include the character XP settings section, character XP contribution section, group XP scaling section, character skill scaling section, and skill XP multiplier section. `Custom.ini` is editable from the MCM and can be shared as a standalone profile file.

The General page profile acts as a master profile. Locked presets force the configuration pages to the same profile. Custom leaves page-level profile controls available. Character XP changes are committed with the Character XP page Apply button. Skill XP Cache reads live game values into the selected editable profile in memory; Skill XP Apply saves the editable profile INI and writes the selected profile values to the game.

Character XP profiles can also opt into a flat skill-rank XP amount and configure Skyrim's base character level threshold and threshold increase per level. These profile-backed settings are applied from the Character XP page Apply button.

## Compatibility

SkillGroups is intended to be compatible with mods that change skill XP rates or edit normal skill-use progression. Character XP contribution scaling is internal to SkillGroups; skill XP configuration only changes `useMult` when the user explicitly applies it from the MCM.

The plugin is not expected to be compatible with mods that replace skill-based leveling entirely, independently award player XP through a separate system, or patch the same character-XP calculation site. If the hook cannot be installed, SkillGroups disables its runtime behavior and reports the failure in its MCM and log instead of crashing.

## Building

Requirements:

- Visual Studio 2022
- CMake with Ninja
- vcpkg
- CommonLibSSE NG through the included vcpkg manifest

Configure and build with the included CMake presets:

```powershell
cmake --preset debug
cmake --build build/debug --target SkillGroups SkillGroupsTests
build/debug/SkillGroupsTests.exe

cmake --preset release
cmake --build build/release --target SkillGroups
```

## License

SkillGroups is released under the MIT License.
