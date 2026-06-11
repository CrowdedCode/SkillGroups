# SkillGroups

SkillGroups is an SKSE/CommonLibSSE NG plugin for Skyrim Special Edition that changes how skill rank-ups contribute to character level. Skills still gain XP and level normally, but character XP is gated by skill groups so that redundant skills do not affect player level.

The goal is to make character level track peak capability instead of total accumulated skills. Learning a second combat style can add versatility without making the world scale as if the character's max power increases.

## Current Behavior

SkillGroups hooks Skyrim's character-XP calculation for skill rank-ups. When a skill ranks up, the plugin checks that skill's group and only awards character XP if the new rank exceeds the other skills in that group.

For example, if One-Handed is 50 and Two-Handed ranks from 49 to 50, Two-Handed does not contribute character XP. If Two-Handed later ranks from 50 to 51, it contributes character XP because it has exceeded the previous group maximum.

The plugin does not replace Skyrim's skill progression. Skill XP, skill rank-ups, level-up UI, overflow behavior, and perk progression continue to run through the game.

## V1.0 Groups

The V1.0 groups are fixed:

```text
Melee: One-Handed, Two-Handed, Block
Ranged: Archery, Destruction
Defence: Light Armor, Heavy Armor
Support: Restoration, Alteration, Sneak
Command: Illusion, Conjuration
Wealth: Speech, Pickpocket, Lockpicking
Crafting: Smithing, Enchanting, Alchemy
```

## Configuration

Settings are available through the SkyUI MCM. The plugin also ships default settings in:

```text
Data/SKSE/Plugins/SkillGroups.ini
Data/MCM/Config/SkillGroups/settings.ini
```

Character XP configuration supports per-skill contribution multipliers, per-skill XP multiplier, and per-group multpliers. The group contribution is based on the sum of the configured effective multipliers for skills in that group.

## Compatibility

SkillGroups is intended to be compatible with mods that change skill XP rates or edit normal skill-use progression. It also has soft compatibility with mods that change `improveMult`, because character XP scaling is calculated relative to the live or cached multiplier value.

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
