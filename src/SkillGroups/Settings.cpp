#include "SkillGroups/Settings.h"

#include "SkillGroups/Profiles.h"

#include <array>
#include <charconv>
#include <fstream>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>

namespace SkillGroups::Settings
{
	namespace
	{
		constexpr std::array<std::string_view, 3> kSettingsPaths{
			"Data\\SKSE\\Plugins\\SkillGroups.ini",
			"Data\\MCM\\Config\\SkillGroups\\settings.ini",
			"Data\\MCM\\Settings\\SkillGroups.ini"
		};

		Config g_config;

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

		[[nodiscard]] std::optional<int> ParseInt(std::string_view a_value)
		{
			const auto value = Trim(a_value);
			int result = 0;
			const auto* first = value.data();
			const auto* last = value.data() + value.size();
			const auto [ptr, ec] = std::from_chars(first, last, result);
			if (ec != std::errc{} || ptr != last) {
				return std::nullopt;
			}

			return result;
		}

		[[nodiscard]] LogLevel ParseLogLevel(std::string_view a_value)
		{
			const auto value = Trim(a_value);
			if (value == "Debug" || value == "debug") {
				return LogLevel::Debug;
			}

			if (value == "Error" || value == "error") {
				return LogLevel::Error;
			}

			return LogLevel::Info;
		}

		[[nodiscard]] LogLevel ParseLogLevelIndex(std::string_view a_value, LogLevel a_default)
		{
			const auto value = ParseInt(a_value);
			if (!value) {
				return a_default;
			}

			switch (*value) {
			case 0:
				return LogLevel::Error;
			case 2:
				return LogLevel::Debug;
			default:
				return LogLevel::Info;
			}
		}

		[[nodiscard]] int ParseProfileIndex(std::string_view a_value, int a_default)
		{
			const auto value = ParseInt(a_value);
			if (!value || *value < 0) {
				return a_default;
			}

			return *value;
		}

		void ApplySetting(std::string_view a_section, std::string_view a_key, std::string_view a_value)
		{
			if (a_section == "General") {
				if (a_key == "Enabled" || a_key == "bEnabled") {
					g_config.enabled = ParseBool(a_value, g_config.enabled);
				} else if (a_key == "CharacterXpProfile" || a_key == "iCharacterXpProfile") {
					g_config.characterXpProfileIndex = ParseProfileIndex(a_value, g_config.characterXpProfileIndex);
				} else if (a_key == "SkillXpProfile" || a_key == "iSkillXpProfile") {
					g_config.skillXpProfileIndex = ParseProfileIndex(a_value, g_config.skillXpProfileIndex);
				} else if (a_key == "AutoApplySkillXpOnLevelXp" || a_key == "bAutoApplySkillXpOnLevelXp") {
					g_config.autoApplySkillXpOnLevelXp = ParseBool(a_value, g_config.autoApplySkillXpOnLevelXp);
				} else if (a_key == "DivideSkillXpByGroupSize" || a_key == "bDivideSkillXpByGroupSize") {
					g_config.divideSkillXpByGroupSize = ParseBool(a_value, g_config.divideSkillXpByGroupSize);
				} else if (a_key == "LogLevel") {
					g_config.logLevel = ParseLogLevel(a_value);
				} else if (a_key == "iLogLevel") {
					g_config.logLevel = ParseLogLevelIndex(a_value, g_config.logLevel);
				}
				return;
			}
		}

		void LoadFile(std::string_view a_path)
		{
			std::ifstream file{ std::string{ a_path } };
			if (!file) {
				return;
			}

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
					continue;
				}

				const auto equals = line.find('=');
				if (equals == std::string::npos) {
					continue;
				}

				const auto key = Trim(std::string_view{ line }.substr(0, equals));
				const auto value = Trim(std::string_view{ line }.substr(equals + 1));
				ApplySetting(section, key, value);
			}
		}

		[[nodiscard]] spdlog::level::level_enum ToSpdlogLevel(LogLevel a_level)
		{
			switch (a_level) {
			case LogLevel::Error:
				return spdlog::level::err;
			case LogLevel::Debug:
				return spdlog::level::debug;
			case LogLevel::Info:
			default:
				return spdlog::level::info;
			}
		}
	}

	const Config& Get()
	{
		return g_config;
	}

	void Load()
	{
		g_config = {};
		for (const auto path : kSettingsPaths) {
			LoadFile(path);
		}
		ApplyLogLevel();
	}

	void LoadProfiles()
	{
		Profiles::Load();
		const auto profileSettings = Profiles::GetCharacterXpSettings(static_cast<std::size_t>(g_config.characterXpProfileIndex));
		SetCharacterXpRuntimeSettings(
			profileSettings.useFlatCharacterXp,
			profileSettings.flatCharacterXp,
			profileSettings.levelUpBase,
			profileSettings.levelUpMult);
	}

	void SetCharacterXpRuntimeSettings(bool a_useFlatCharacterXp, float a_flatCharacterXp, float a_levelUpBase, float a_levelUpMult)
	{
		g_config.useFlatCharacterXp = a_useFlatCharacterXp;
		g_config.flatCharacterXp = a_flatCharacterXp;
		g_config.levelUpBase = a_levelUpBase;
		g_config.levelUpMult = a_levelUpMult;
	}

	void ApplyLogLevel()
	{
		const auto level = ToSpdlogLevel(g_config.logLevel);
		spdlog::set_level(level);

		if (const auto logger = spdlog::default_logger()) {
			logger->set_level(level);
			logger->flush_on(level);
		}
	}

	bool IsDebugLoggingEnabled()
	{
		return g_config.logLevel == LogLevel::Debug;
	}
}
