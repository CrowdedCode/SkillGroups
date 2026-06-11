#include "SkillGroups/Profiles.h"

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace SkillGroups::Profiles
{
	namespace
	{
		constexpr std::string_view kProfileFolder{ "Data\\SKSE\\Plugins\\SkillGroups\\Profiles" };
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
		constexpr std::array<float, SkillGroupCount> kDefaultGroupXpMultiplierScales{
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
			std::array<float, SkillGroupCount> groupXpMultiplierScales{ kDefaultGroupXpMultiplierScales };
			std::array<float, SkillCount> playerXpMultiplierScales{};
			std::array<float, SkillCount> multipliers{ kDefaultSkillXpMultipliers };
		};

		std::vector<Profile> g_profiles;

		[[nodiscard]] std::string Trim(std::string_view a_value)
		{
			const auto begin = a_value.find_first_not_of(" \t\r\n");
			if (begin == std::string_view::npos) {
				return {};
			}

			const auto end = a_value.find_last_not_of(" \t\r\n");
			return std::string{ a_value.substr(begin, end - begin + 1) };
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
			const auto groups = SkillGroups();
			for (std::size_t index = 0; index < groups.size(); ++index) {
				if (a_key == groups[index].name) {
					return index;
				}
			}

			return std::nullopt;
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
			profile.groupXpMultiplierScales = kDefaultGroupXpMultiplierScales;
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
			std::array<bool, SkillGroupCount> sawGroupXpMultiplierScale{};
			std::array<bool, SkillCount> sawPlayerXpMultiplierScale{};
			std::array<bool, SkillCount> sawSkillXpMultiplier{};
			auto sawCharacterXpSettings = false;

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
						profile.characterXpSettings.useFlatCharacterXp = ParseBool(value, profile.characterXpSettings.useFlatCharacterXp);
					} else if (key == "FlatCharacterXp") {
						profile.characterXpSettings.flatCharacterXp = ParseFloat(value).value_or(profile.characterXpSettings.flatCharacterXp);
					} else if (key == "LevelUpBase") {
						profile.characterXpSettings.levelUpBase = ParseFloat(value).value_or(profile.characterXpSettings.levelUpBase);
					} else if (key == "LevelUpMult") {
						profile.characterXpSettings.levelUpMult = ParseFloat(value).value_or(profile.characterXpSettings.levelUpMult);
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
				} else if (section == "GroupXpMultiplierScales") {
					const auto groupIndex = GroupIndexForKey(key);
					if (!groupIndex) {
						continue;
					}

					const auto multiplier = ParseFloat(value);
					if (!multiplier) {
						continue;
					}

					profile.groupXpMultiplierScales[*groupIndex] = *multiplier;
					sawGroupXpMultiplierScale[*groupIndex] = true;
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
			const auto hasAllGroupXpMultiplierScales = std::all_of(sawGroupXpMultiplierScale.begin(), sawGroupXpMultiplierScale.end(), [](bool a_value) { return a_value; });
			const auto hasAllPlayerXpMultiplierScales = std::all_of(sawPlayerXpMultiplierScale.begin(), sawPlayerXpMultiplierScale.end(), [](bool a_value) { return a_value; });
			const auto hasAllSkillXpMultipliers = std::all_of(sawSkillXpMultiplier.begin(), sawSkillXpMultiplier.end(), [](bool a_value) { return a_value; });
			if (!sawProfileSection ||
				!sawCharacterXpSettings ||
				!hasAllCharacterXpMultipliers ||
				!hasAllGroupXpMultiplierScales ||
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
	}

	void Load()
	{
		g_profiles.clear();

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

	std::string_view ProfileName(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			return DefaultProfile().name;
		}

		return g_profiles[a_profileIndex].name;
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
		if (a_groupIndex >= SkillGroupCount || a_profileIndex >= ProfileCount()) {
			return DefaultProfile().groupXpMultiplierScales[std::min(a_groupIndex, SkillGroupCount - 1)];
		}

		return g_profiles[a_profileIndex].groupXpMultiplierScales[a_groupIndex];
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

	const std::array<float, SkillGroupCount>& GetGroupXpMultiplierScales(std::size_t a_profileIndex)
	{
		if (a_profileIndex >= ProfileCount()) {
			return DefaultProfile().groupXpMultiplierScales;
		}

		return g_profiles[a_profileIndex].groupXpMultiplierScales;
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
		if (!IsProfileEditable(a_profileIndex) || a_groupIndex >= SkillGroupCount) {
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

		file << "\n[GroupXpMultiplierScales]\n";
		const auto groups = SkillGroups();
		for (std::size_t index = 0; index < SkillGroupCount; ++index) {
			file << groups[index].name << '=' << profile.groupXpMultiplierScales[index] << '\n';
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

