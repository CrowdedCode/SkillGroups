#include "SkillGroups/Profiles.h"

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SkillGroups::Profiles
{
	namespace
	{
		constexpr std::string_view kProfileFolder{ "Data\\SKSE\\Plugins\\SkillGroups\\Profiles" };
		constexpr std::array<std::string_view, 2> kTranslationPaths{
			"Data\\Interface\\Translations\\SkillGroups_english.txt",
			"Data\\Interface\\Translations\\SkillGroups_ENGLISH.txt"
		};
		constexpr std::array<float, SkillCount> kDefaultSkillXpMultipliers{
			6.3F,
			5.95F,
			9.3F,
			8.1F,
			1.0F,
			3.8F,
			4.0F,
			8.1F,
			45.0F,
			11.25F,
			0.75F,
			0.36F,
			3.0F,
			2.1F,
			1.35F,
			4.6F,
			2.0F,
			900.0F
		};
		constexpr std::array<float, 7> kDefaultGroupXpMultiplierScales{
			0.75F,
			1.15F,
			0.75F,
			1.4F,
			1.4F,
			0.15F,
			1.4F
		};

		struct Profile
		{
			std::filesystem::path path;
			std::string name;
			std::string description;
			bool editable{ false };
			CharacterXpSettings characterXpSettings{};
			std::array<float, SkillCount> characterXpMultipliers{};
			std::vector<SkillGroup> groups;
			std::vector<float> groupXpMultiplierScales;
			std::array<float, SkillCount> playerXpMultiplierScales{};
			std::array<float, SkillCount> multipliers{ kDefaultSkillXpMultipliers };
		};

		std::vector<Profile> g_profiles;
		std::unordered_map<std::string, std::string> g_translations;

		[[nodiscard]] std::string Trim(std::string_view a_value)
		{
			const auto begin = a_value.find_first_not_of(" \t\r\n");
			if (begin == std::string_view::npos) {
				return {};
			}

			const auto end = a_value.find_last_not_of(" \t\r\n");
			return std::string{ a_value.substr(begin, end - begin + 1) };
		}

		[[nodiscard]] std::string Utf8FromWide(std::wstring_view a_value)
		{
			std::string result;
			result.reserve(a_value.size() * 3);
			for (const auto ch : a_value) {
				const auto value = static_cast<std::uint32_t>(ch);
				if (value <= 0x7F) {
					result.push_back(static_cast<char>(value));
				} else if (value <= 0x7FF) {
					result.push_back(static_cast<char>(0xC0 | (value >> 6)));
					result.push_back(static_cast<char>(0x80 | (value & 0x3F)));
				} else {
					result.push_back(static_cast<char>(0xE0 | (value >> 12)));
					result.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3F)));
					result.push_back(static_cast<char>(0x80 | (value & 0x3F)));
				}
			}

			return result;
		}

		void LoadTranslations()
		{
			g_translations.clear();

			for (const auto path : kTranslationPaths) {
				std::ifstream file{ std::string{ path }, std::ios::binary };
				if (!file) {
					continue;
				}

				std::vector<char> bytes{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
				if (bytes.size() < 2 ||
					static_cast<unsigned char>(bytes[0]) != 0xFF ||
					static_cast<unsigned char>(bytes[1]) != 0xFE) {
					continue;
				}

				std::wstring text;
				text.reserve((bytes.size() - 2) / 2);
				for (std::size_t index = 2; index + 1 < bytes.size(); index += 2) {
					const auto lo = static_cast<unsigned char>(bytes[index]);
					const auto hi = static_cast<unsigned char>(bytes[index + 1]);
					text.push_back(static_cast<wchar_t>(lo | (hi << 8)));
				}

				std::size_t lineStart = 0;
				while (lineStart < text.size()) {
					auto lineEnd = text.find_first_of(L"\r\n", lineStart);
					if (lineEnd == std::wstring::npos) {
						lineEnd = text.size();
					}

					const auto line = std::wstring_view{ text }.substr(lineStart, lineEnd - lineStart);
					const auto tab = line.find(L'\t');
					if (tab != std::wstring_view::npos && tab > 0 && tab + 1 < line.size()) {
						g_translations[Utf8FromWide(line.substr(0, tab))] = Utf8FromWide(line.substr(tab + 1));
					}

					lineStart = lineEnd + 1;
					while (lineStart < text.size() && (text[lineStart] == L'\r' || text[lineStart] == L'\n')) {
						++lineStart;
					}
				}

				SKSE::log::info("SkillGroups loaded {} translation(s) from {}", g_translations.size(), path);
				return;
			}
		}

		[[nodiscard]] bool ParseBool(std::string_view a_value, bool a_default)
		{
			const auto value = Trim(a_value);
			if (value == "1" || value == "true" || value == "True" || value == "TRUE") {
				return true;
			}

			if (value == "0" || value == "false" || value == "False" || value == "FALSE") {
				return false;
			}

			return a_default;
		}

		[[nodiscard]] std::optional<bool> ParseBool(std::string_view a_value)
		{
			const auto value = Trim(a_value);
			if (value == "1" || value == "true" || value == "True" || value == "TRUE") {
				return true;
			}

			if (value == "0" || value == "false" || value == "False" || value == "FALSE") {
				return false;
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional<float> ParseFloat(std::string_view a_value)
		{
			const auto value = Trim(a_value);
			float result = 0.0F;
			const auto* first = value.data();
			const auto* last = value.data() + value.size();
			const auto [ptr, ec] = std::from_chars(first, last, result);
			if (ec != std::errc{} || ptr != last) {
				return std::nullopt;
			}

			return result;
		}

		[[nodiscard]] std::optional<std::size_t> SkillIndexForKey(std::string_view a_key)
		{
			for (std::size_t index = 0; index < SkillCount; ++index) {
				if (a_key == SkillName(static_cast<Skill>(index))) {
					return index;
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::size_t> GroupIndexForKey(std::string_view a_key)
		{
			const auto groups = DefaultSkillGroups();
			for (std::size_t index = 0; index < groups.size(); ++index) {
				if (a_key == groups[index].name) {
					return index;
				}
			}

			return std::nullopt;
		}

		[[nodiscard]] std::vector<float> DefaultGroupScalesFor(std::span<const SkillGroup> a_groups)
		{
			std::vector<float> scales;
			scales.reserve(a_groups.size());
			for (const auto& group : a_groups) {
				const auto index = GroupIndexForKey(group.name);
				scales.push_back(index ? kDefaultGroupXpMultiplierScales[*index] : 1.0F);
			}

			return scales;
		}

		[[nodiscard]] std::array<std::string, SkillCount> AssignmentsFor(const Profile& a_profile)
		{
			std::array<std::string, SkillCount> assignments{};
			for (const auto& group : a_profile.groups) {
				for (const auto skill : group.skills) {
					assignments[static_cast<std::size_t>(skill)] = group.name;
				}
			}

			return assignments;
		}

		void RebuildGroups(Profile& a_profile, const std::array<std::string, SkillCount>& a_assignments)
		{
			std::unordered_map<std::string, float> oldScales;
			for (std::size_t index = 0; index < a_profile.groups.size() && index < a_profile.groupXpMultiplierScales.size(); ++index) {
				oldScales[a_profile.groups[index].name] = a_profile.groupXpMultiplierScales[index];
			}

			std::vector<SkillGroup> rebuilt;
			rebuilt.reserve(MaxSkillGroupCount);
			for (const auto& group : a_profile.groups) {
				SkillGroup next{ group.name, {} };
				for (std::size_t skillIndex = 0; skillIndex < a_assignments.size(); ++skillIndex) {
					if (a_assignments[skillIndex] == group.name) {
						next.skills.push_back(static_cast<Skill>(skillIndex));
					}
				}
				if (!next.skills.empty()) {
					rebuilt.push_back(std::move(next));
				}
			}

			for (std::size_t skillIndex = 0; skillIndex < a_assignments.size(); ++skillIndex) {
				const auto& groupName = a_assignments[skillIndex];
				const auto exists = std::ranges::find_if(rebuilt, [&](const SkillGroup& a_group) {
					return a_group.name == groupName;
				}) != rebuilt.end();
				if (!exists) {
					SkillGroup next{ groupName, {} };
					for (std::size_t memberIndex = skillIndex; memberIndex < a_assignments.size(); ++memberIndex) {
						if (a_assignments[memberIndex] == groupName) {
							next.skills.push_back(static_cast<Skill>(memberIndex));
						}
					}
					rebuilt.push_back(std::move(next));
				}
			}

			std::vector<float> scales;
			scales.reserve(rebuilt.size());
			for (const auto& group : rebuilt) {
				if (const auto scale = oldScales.find(group.name); scale != oldScales.end()) {
					scales.push_back(scale->second);
				} else {
					scales.push_back(1.0F);
				}
			}

			a_profile.groups = std::move(rebuilt);
			a_profile.groupXpMultiplierScales = std::move(scales);
		}

		[[nodiscard]] Profile BuiltInProfile(std::string a_name, bool a_editable)
		{
			Profile profile;
			profile.path = std::filesystem::path{ kProfileFolder } / (a_name + ".ini");
			profile.name = std::move(a_name);
			profile.editable = a_editable;
			profile.description = profile.editable ?
				"Editable SkillGroups profile." :
				"SkillGroups default profile.";
			profile.characterXpSettings = {};
			profile.characterXpMultipliers.fill(1.0F);
			profile.groups = std::vector<SkillGroup>{ DefaultSkillGroups().begin(), DefaultSkillGroups().end() };
			profile.groupXpMultiplierScales = DefaultGroupScalesFor(profile.groups);
			profile.playerXpMultiplierScales.fill(1.0F);
			profile.multipliers = kDefaultSkillXpMultipliers;
			return profile;
		}

		[[nodiscard]] std::optional<Profile> ParseProfileFile(const std::filesystem::path& a_path)
		{
			std::ifstream file{ a_path };
			if (!file) {
				return std::nullopt;
			}

			auto profile = BuiltInProfile(a_path.stem().string(), false);
			profile.path = a_path;
			auto sawProfileSection = false;
			std::array<bool, SkillCount> sawCharacterXpMultiplier{};
			std::array<bool, SkillCount> sawPlayerXpMultiplierScale{};
			std::array<bool, SkillCount> sawSkillXpMultiplier{};
			std::array<bool, SkillCount> sawSkillGroup{};
			std::array<std::string, SkillCount> skillGroupAssignments{};
			std::vector<std::pair<std::string, float>> rawGroupXpMultiplierScales;
			auto sawCharacterXpSettings = false;
			auto sawUseFlatCharacterXp = false;
			auto sawFlatCharacterXp = false;
			auto sawLevelUpBase = false;
			auto sawLevelUpMult = false;
			auto sawSkillGroups = false;
			auto invalidCharacterXpSettings = false;
			auto invalidSkillGroups = false;
			auto invalidGroupScales = false;

			std::string section;
			std::string line;
			while (std::getline(file, line)) {
				const auto comment = line.find_first_of(";#");
				if (comment != std::string::npos) {
					line.erase(comment);
				}

				line = Trim(line);
				if (line.empty()) {
					continue;
				}

				if (line.front() == '[' && line.back() == ']') {
					section = Trim(std::string_view{ line }.substr(1, line.size() - 2));
					if (section == "Profile") {
						sawProfileSection = true;
					}
					continue;
				}

				const auto equals = line.find('=');
				if (equals == std::string::npos) {
					continue;
				}

				const auto key = Trim(std::string_view{ line }.substr(0, equals));
				const auto value = Trim(std::string_view{ line }.substr(equals + 1));
				if (section == "Profile") {
					if (key == "Name") {
						profile.name = value;
					} else if (key == "Description") {
						profile.description = value;
					} else if (key == "Editable") {
						profile.editable = ParseBool(value, profile.editable);
					}
				} else if (section == "CharacterXpSettings") {
					sawCharacterXpSettings = true;
					if (key == "UseFlatCharacterXp") {
						const auto parsed = ParseBool(value);
						if (!parsed || sawUseFlatCharacterXp) {
							invalidCharacterXpSettings = true;
						} else {
							profile.characterXpSettings.useFlatCharacterXp = *parsed;
							sawUseFlatCharacterXp = true;
						}
					} else if (key == "FlatCharacterXp") {
						const auto parsed = ParseFloat(value);
						if (!parsed || sawFlatCharacterXp) {
							invalidCharacterXpSettings = true;
						} else {
							profile.characterXpSettings.flatCharacterXp = *parsed;
							sawFlatCharacterXp = true;
						}
					} else if (key == "LevelUpBase") {
						const auto parsed = ParseFloat(value);
						if (!parsed || sawLevelUpBase) {
							invalidCharacterXpSettings = true;
						} else {
							profile.characterXpSettings.levelUpBase = *parsed;
							sawLevelUpBase = true;
						}
					} else if (key == "LevelUpMult") {
						const auto parsed = ParseFloat(value);
						if (!parsed || sawLevelUpMult) {
							invalidCharacterXpSettings = true;
						} else {
							profile.characterXpSettings.levelUpMult = *parsed;
							sawLevelUpMult = true;
						}
					}
				} else if (section == "CharacterXpMultipliers") {
					const auto skillIndex = SkillIndexForKey(key);
					if (!skillIndex) {
						continue;
					}

					const auto multiplier = ParseFloat(value);
					if (!multiplier) {
						continue;
					}

					profile.characterXpMultipliers[*skillIndex] = *multiplier;
					sawCharacterXpMultiplier[*skillIndex] = true;
				} else if (section == "SkillGroups") {
					sawSkillGroups = true;
					const auto skillIndex = SkillIndexForKey(key);
					if (!skillIndex || sawSkillGroup[*skillIndex] || value.empty() || value.find(',') != std::string::npos) {
						invalidSkillGroups = true;
						continue;
					}

					skillGroupAssignments[*skillIndex] = value;
					sawSkillGroup[*skillIndex] = true;
				} else if (section == "GroupXpMultiplierScales") {
					const auto multiplier = ParseFloat(value);
					if (!multiplier) {
						invalidGroupScales = true;
						continue;
					}

					const auto duplicate = std::ranges::find_if(rawGroupXpMultiplierScales, [&](const auto& a_scale) {
						return a_scale.first == key;
					}) != rawGroupXpMultiplierScales.end();
					if (duplicate) {
						invalidGroupScales = true;
					} else {
						rawGroupXpMultiplierScales.emplace_back(key, *multiplier);
					}
				} else if (section == "PlayerXpMultiplierScales") {
					const auto skillIndex = SkillIndexForKey(key);
					if (!skillIndex) {
						continue;
					}

					const auto multiplier = ParseFloat(value);
					if (!multiplier) {
						continue;
					}

					profile.playerXpMultiplierScales[*skillIndex] = *multiplier;
					sawPlayerXpMultiplierScale[*skillIndex] = true;
				} else if (section == "SkillXpMultipliers") {
					const auto skillIndex = SkillIndexForKey(key);
					if (!skillIndex) {
						continue;
					}

					const auto multiplier = ParseFloat(value);
					if (!multiplier) {
						continue;
					}

					profile.multipliers[*skillIndex] = *multiplier;
					sawSkillXpMultiplier[*skillIndex] = true;
				}
			}

			const auto hasAllCharacterXpMultipliers = std::all_of(sawCharacterXpMultiplier.begin(), sawCharacterXpMultiplier.end(), [](bool a_value) { return a_value; });
			const auto hasAllPlayerXpMultiplierScales = std::all_of(sawPlayerXpMultiplierScale.begin(), sawPlayerXpMultiplierScale.end(), [](bool a_value) { return a_value; });
			const auto hasAllSkillXpMultipliers = std::all_of(sawSkillXpMultiplier.begin(), sawSkillXpMultiplier.end(), [](bool a_value) { return a_value; });
			const auto hasAllSkillGroups = std::all_of(sawSkillGroup.begin(), sawSkillGroup.end(), [](bool a_value) { return a_value; });
			if (sawSkillGroups && hasAllSkillGroups && !invalidSkillGroups) {
				profile.groups.clear();
				profile.groupXpMultiplierScales.clear();
				profile.groups.reserve(rawGroupXpMultiplierScales.size());
				profile.groupXpMultiplierScales.reserve(rawGroupXpMultiplierScales.size());

				for (const auto& [groupName, scale] : rawGroupXpMultiplierScales) {
					SkillGroup group{ groupName, {} };
					for (std::size_t skillIndex = 0; skillIndex < skillGroupAssignments.size(); ++skillIndex) {
						if (skillGroupAssignments[skillIndex] == groupName) {
							group.skills.push_back(static_cast<Skill>(skillIndex));
						}
					}
					if (group.skills.empty()) {
						invalidGroupScales = true;
					} else {
						profile.groups.push_back(std::move(group));
						profile.groupXpMultiplierScales.push_back(scale);
					}
				}

				const auto assignedGroups = BuildSkillGroupsFromAssignments(skillGroupAssignments);
				if (assignedGroups.size() != profile.groups.size()) {
					invalidGroupScales = true;
				}
			}

			if (!sawProfileSection ||
				!sawCharacterXpSettings ||
				!sawUseFlatCharacterXp ||
				!sawFlatCharacterXp ||
				!sawLevelUpBase ||
				!sawLevelUpMult ||
				invalidCharacterXpSettings ||
				!hasAllCharacterXpMultipliers ||
				!sawSkillGroups ||
				!hasAllSkillGroups ||
				invalidSkillGroups ||
				invalidGroupScales ||
				!hasAllPlayerXpMultiplierScales ||
				!hasAllSkillXpMultipliers ||
				profile.name.empty()) {
				SKSE::log::warn("SkillGroups ignored incomplete profile: {}", a_path.string());
				return std::nullopt;
			}

			return profile;
		}

		void AddFallbackProfiles()
		{
			g_profiles.push_back(BuiltInProfile("Default", false));
			g_profiles.push_back(BuiltInProfile("Custom", true));
		}

		[[nodiscard]] const Profile& DefaultProfile()
		{
			if (g_profiles.empty()) {
				AddFallbackProfiles();
			}

			return g_profiles.front();
		}

		[[nodiscard]] std::string_view ProfileName(std::size_t a_profileIndex)
		{
			if (a_profileIndex >= g_profiles.size()) {
				return DefaultProfile().name;
			}

			return g_profiles[a_profileIndex].name;
		}
	}

	void Load()
	{
		g_profiles.clear();
		LoadTranslations();

		const std::filesystem::path folder{ kProfileFolder };
		std::error_code ec;
		if (std::filesystem::exists(folder, ec)) {
			for (const auto& entry : std::filesystem::directory_iterator{ folder, ec }) {
				if (ec) {
					break;
				}

				if (!entry.is_regular_file() || entry.path().extension() != ".ini") {
					continue;
				}

				if (auto profile = ParseProfileFile(entry.path())) {
					g_profiles.push_back(std::move(*profile));
				}
			}
		}

		std::sort(g_profiles.begin(), g_profiles.end(), [](const auto& a_lhs, const auto& a_rhs) {
			if (a_lhs.name == "Default") {
				return true;
			}

			if (a_rhs.name == "Default") {
				return false;
			}

			if (a_lhs.name == "Custom") {
				return a_rhs.name != "Default";
			}

			if (a_rhs.name == "Custom") {
				return false;
			}

			return a_lhs.name < a_rhs.name;
		});

		if (g_profiles.empty()) {
			AddFallbackProfiles();
		}

		SKSE::log::info("SkillGroups loaded {} profile(s)", g_profiles.size());
	}

	std::size_t ProfileCount()
	{
		(void)DefaultProfile();
		return g_profiles.size();
	}

	std::string_view ProfileDisplayName(std::size_t a_profileIndex)
	{
		const auto name = ProfileName(a_profileIndex);
		const auto key = std::string{ "$SkillGroups_Profile_" }.append(name);
		if (const auto translation = g_translations.find(key); translation != g_translations.end()) {
			return translation->second;
		}

		return name;
	}

	bool IsProfileEditable(std::size_t a_profileIndex)
	{
		return a_profileIndex < ProfileCount() && g_profiles[a_profileIndex].editable;
	}

	float GetCharacterXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex)
	{
		if (a_skillIndex >= SkillCount || a_profileIndex >= ProfileCount()) {
			return DefaultProfile().characterXpMultipliers[std::min(a_skillIndex, SkillCount - 1)];
		}

		return g_profiles[a_profileIndex].characterXpMultipliers[a_skillIndex];
	}

	float GetGroupXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_groupIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			const auto& defaults = DefaultProfile().groupXpMultiplierScales;
			return defaults[std::min(a_groupIndex, defaults.size() - 1)];
		}

		const auto& scales = g_profiles[a_profileIndex].groupXpMultiplierScales;
		if (a_groupIndex >= scales.size()) {
			return 1.0F;
		}

		return scales[a_groupIndex];
	}

	float GetPlayerXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_skillIndex)
	{
		if (a_skillIndex >= SkillCount || a_profileIndex >= ProfileCount()) {
			return DefaultProfile().playerXpMultiplierScales[std::min(a_skillIndex, SkillCount - 1)];
		}

		return g_profiles[a_profileIndex].playerXpMultiplierScales[a_skillIndex];
	}

	float GetSkillXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex)
	{
		if (a_skillIndex >= SkillCount || a_profileIndex >= ProfileCount()) {
			return DefaultProfile().multipliers[std::min(a_skillIndex, SkillCount - 1)];
		}

		return g_profiles[a_profileIndex].multipliers[a_skillIndex];
	}

	CharacterXpSettings GetCharacterXpSettings(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			return DefaultProfile().characterXpSettings;
		}

		return g_profiles[a_profileIndex].characterXpSettings;
	}

	const std::array<float, SkillCount>& GetCharacterXpMultipliers(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			return DefaultProfile().characterXpMultipliers;
		}

		return g_profiles[a_profileIndex].characterXpMultipliers;
	}

	const std::array<float, SkillCount>& GetPlayerXpMultiplierScales(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			return DefaultProfile().playerXpMultiplierScales;
		}

		return g_profiles[a_profileIndex].playerXpMultiplierScales;
	}

	const std::array<float, SkillCount>& GetSkillXpMultipliers(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			return DefaultProfile().multipliers;
		}

		return g_profiles[a_profileIndex].multipliers;
	}

	std::span<const SkillGroup> GetSkillGroups(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			return DefaultProfile().groups;
		}

		return g_profiles[a_profileIndex].groups;
	}

	std::size_t GetGroupCount(std::size_t a_profileIndex)
	{
		return GetSkillGroups(a_profileIndex).size();
	}

	std::string_view GetGroupName(std::size_t a_profileIndex, std::size_t a_groupIndex)
	{
		const auto groups = GetSkillGroups(a_profileIndex);
		if (a_groupIndex >= groups.size()) {
			return {};
		}

		return groups[a_groupIndex].name;
	}

	std::size_t GetSkillGroupIndex(std::size_t a_profileIndex, std::size_t a_skillIndex)
	{
		if (a_skillIndex >= SkillCount) {
			return 0;
		}

		const auto groups = GetSkillGroups(a_profileIndex);
		const auto skill = static_cast<Skill>(a_skillIndex);
		for (std::size_t groupIndex = 0; groupIndex < groups.size(); ++groupIndex) {
			if (std::ranges::find(groups[groupIndex].skills, skill) != groups[groupIndex].skills.end()) {
				return groupIndex;
			}
		}

		return 0;
	}

	std::size_t GetSkillGroupSize(std::size_t a_profileIndex, std::size_t a_skillIndex)
	{
		if (a_skillIndex >= SkillCount) {
			return 1;
		}

		const auto groups = GetSkillGroups(a_profileIndex);
		const auto skill = static_cast<Skill>(a_skillIndex);
		for (const auto& group : groups) {
			if (std::ranges::find(group.skills, skill) != group.skills.end()) {
				return std::max<std::size_t>(1, group.skills.size());
			}
		}

		return 1;
	}

	std::string GetGroupSlotName(std::size_t a_profileIndex, std::size_t a_slotIndex)
	{
		if (a_slotIndex >= MaxSkillGroupCount) {
			return {};
		}

		const auto groups = GetSkillGroups(a_profileIndex);
		if (a_slotIndex < groups.size()) {
			return groups[a_slotIndex].name;
		}

		return std::string{ "Group " }.append(std::to_string(a_slotIndex + 1));
	}

	bool SetCharacterXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex, float a_value)
	{
		if (!IsProfileEditable(a_profileIndex) || a_skillIndex >= SkillCount) {
			return false;
		}

		g_profiles[a_profileIndex].characterXpMultipliers[a_skillIndex] = a_value;
		return true;
	}

	bool SetGroupXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_groupIndex, float a_value)
	{
		if (!IsProfileEditable(a_profileIndex) || a_groupIndex >= g_profiles[a_profileIndex].groupXpMultiplierScales.size()) {
			return false;
		}

		g_profiles[a_profileIndex].groupXpMultiplierScales[a_groupIndex] = a_value;
		return true;
	}

	bool SetPlayerXpMultiplierScale(std::size_t a_profileIndex, std::size_t a_skillIndex, float a_value)
	{
		if (!IsProfileEditable(a_profileIndex) || a_skillIndex >= SkillCount) {
			return false;
		}

		g_profiles[a_profileIndex].playerXpMultiplierScales[a_skillIndex] = a_value;
		return true;
	}

	bool SetSkillXpMultiplier(std::size_t a_profileIndex, std::size_t a_skillIndex, float a_value)
	{
		if (!IsProfileEditable(a_profileIndex) || a_skillIndex >= SkillCount) {
			return false;
		}

		g_profiles[a_profileIndex].multipliers[a_skillIndex] = a_value;
		return true;
	}

	bool SetCharacterXpSettings(std::size_t a_profileIndex, CharacterXpSettings a_settings)
	{
		if (!IsProfileEditable(a_profileIndex)) {
			return false;
		}

		g_profiles[a_profileIndex].characterXpSettings = a_settings;
		return true;
	}

	bool SetSkillGroupAssignments(
		std::size_t a_profileIndex,
		std::span<const std::string> a_groupSlotNames,
		std::span<const std::uint32_t> a_skillGroupSlots)
	{
		if (!IsProfileEditable(a_profileIndex) ||
			a_groupSlotNames.size() < MaxSkillGroupCount ||
			a_skillGroupSlots.size() < SkillCount) {
			return false;
		}

		auto& profile = g_profiles[a_profileIndex];
		std::array<bool, MaxSkillGroupCount> usedSlots{};
		for (std::size_t skillIndex = 0; skillIndex < SkillCount; ++skillIndex) {
			if (a_skillGroupSlots[skillIndex] >= MaxSkillGroupCount) {
				return false;
			}
			usedSlots[a_skillGroupSlots[skillIndex]] = true;
		}

		std::vector<SkillGroup> rebuilt;
		std::vector<float> scales;
		rebuilt.reserve(MaxSkillGroupCount);
		scales.reserve(MaxSkillGroupCount);

		for (std::size_t slotIndex = 0; slotIndex < MaxSkillGroupCount; ++slotIndex) {
			if (!usedSlots[slotIndex]) {
				continue;
			}

			const auto name = Trim(a_groupSlotNames[slotIndex]);
			if (name.empty() || name.find(',') != std::string::npos) {
				return false;
			}

			const auto duplicate = std::ranges::find_if(rebuilt, [&](const SkillGroup& a_group) {
				return a_group.name == name;
			}) != rebuilt.end();
			if (duplicate) {
				return false;
			}

			SkillGroup group{ name, {} };
			for (std::size_t skillIndex = 0; skillIndex < SkillCount; ++skillIndex) {
				if (a_skillGroupSlots[skillIndex] == slotIndex) {
					group.skills.push_back(static_cast<Skill>(skillIndex));
				}
			}

			rebuilt.push_back(std::move(group));
			scales.push_back(slotIndex < profile.groupXpMultiplierScales.size() ? profile.groupXpMultiplierScales[slotIndex] : 1.0F);
		}

		if (rebuilt.empty()) {
			return false;
		}

		profile.groups = std::move(rebuilt);
		profile.groupXpMultiplierScales = std::move(scales);
		return true;
	}

	bool ApplyGroups(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			SetActiveSkillGroups(std::vector<SkillGroup>{ DefaultProfile().groups.begin(), DefaultProfile().groups.end() });
			return false;
		}

		SetActiveSkillGroups(std::vector<SkillGroup>{ g_profiles[a_profileIndex].groups.begin(), g_profiles[a_profileIndex].groups.end() });
		return true;
	}

	bool SaveProfile(std::size_t a_profileIndex)
	{
		if (!IsProfileEditable(a_profileIndex)) {
			return false;
		}

		const auto& profile = g_profiles[a_profileIndex];
		std::error_code ec;
		std::filesystem::create_directories(profile.path.parent_path(), ec);
		if (ec) {
			SKSE::log::warn("SkillGroups could not create profile folder {}: {}", profile.path.parent_path().string(), ec.message());
			return false;
		}

		std::ofstream file{ profile.path };
		if (!file) {
			SKSE::log::warn("SkillGroups could not save profile: {}", profile.path.string());
			return false;
		}

		file << "[Profile]\n";
		file << "Name=" << profile.name << '\n';
		file << "Description=" << profile.description << '\n';
		file << "Editable=" << (profile.editable ? "true" : "false") << "\n\n";
		file << std::fixed << std::setprecision(6);

		file << "[CharacterXpSettings]\n";
		file << "UseFlatCharacterXp=" << (profile.characterXpSettings.useFlatCharacterXp ? "true" : "false") << '\n';
		file << "FlatCharacterXp=" << profile.characterXpSettings.flatCharacterXp << '\n';
		file << "LevelUpBase=" << profile.characterXpSettings.levelUpBase << '\n';
		file << "LevelUpMult=" << profile.characterXpSettings.levelUpMult << "\n\n";

		file << "[CharacterXpMultipliers]\n";
		for (std::size_t index = 0; index < SkillCount; ++index) {
			file << SkillName(static_cast<Skill>(index)) << '=' << profile.characterXpMultipliers[index] << '\n';
		}

		file << "\n[SkillGroups]\n";
		const auto assignments = AssignmentsFor(profile);
		for (std::size_t index = 0; index < SkillCount; ++index) {
			file << SkillName(static_cast<Skill>(index)) << '=' << assignments[index] << '\n';
		}

		file << "\n[GroupXpMultiplierScales]\n";
		for (std::size_t index = 0; index < profile.groups.size(); ++index) {
			file << profile.groups[index].name << '=' << profile.groupXpMultiplierScales[index] << '\n';
		}

		file << "\n[PlayerXpMultiplierScales]\n";
		for (std::size_t index = 0; index < SkillCount; ++index) {
			file << SkillName(static_cast<Skill>(index)) << '=' << profile.playerXpMultiplierScales[index] << '\n';
		}

		file << "\n[SkillXpMultipliers]\n";
		for (std::size_t index = 0; index < SkillCount; ++index) {
			file << SkillName(static_cast<Skill>(index)) << '=' << profile.multipliers[index] << '\n';
		}

		return true;
	}
}

