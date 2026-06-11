#include "SkillGroups/Settings.h"

#include <charconv>
#include <fstream>
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

		void ApplySetting(std::string_view a_section, std::string_view a_key, std::string_view a_value)
		{
			if (a_section == "General") {
				if (a_key == "Enabled" || a_key == "bEnabled") {
					g_config.enabled = ParseBool(a_value, g_config.enabled);
				} else if (a_key == "AutoCacheOnLevelXp" || a_key == "bAutoCacheOnLevelXp") {
					g_config.autoCacheOnLevelXp = ParseBool(a_value, g_config.autoCacheOnLevelXp);
				} else if (a_key == "AutoApplySkillXpOnLevelXp" || a_key == "bAutoApplySkillXpOnLevelXp") {
					g_config.autoApplySkillXpOnLevelXp = ParseBool(a_value, g_config.autoApplySkillXpOnLevelXp);
				} else if (a_key == "DivideSkillXpByGroupSize" || a_key == "bDivideSkillXpByGroupSize") {
					g_config.divideSkillXpByGroupSize = ParseBool(a_value, g_config.divideSkillXpByGroupSize);
				} else if (a_key == "UseCustomCachedPlayerXpMultipliers" || a_key == "bUseCustomCachedPlayerXpMultipliers") {
					g_config.useCustomCachedPlayerXpMultipliers = ParseBool(a_value, g_config.useCustomCachedPlayerXpMultipliers);
				} else if (a_key == "UseCustomCachedSkillXpMultipliers" || a_key == "bUseCustomCachedSkillXpMultipliers") {
					g_config.useCustomCachedSkillXpMultipliers = ParseBool(a_value, g_config.useCustomCachedSkillXpMultipliers);
				} else if (a_key == "LogLevel") {
					g_config.logLevel = ParseLogLevel(a_value);
				} else if (a_key == "iLogLevel") {
					g_config.logLevel = ParseLogLevelIndex(a_value, g_config.logLevel);
				}
				return;
			}

			if (a_section == "GroupXpMultiplierScales") {
				const auto groups = SkillGroups();
				for (std::size_t index = 0; index < groups.size(); ++index) {
					const auto groupName = groups[index].name;
					const auto mcmKey = "f" + std::string{ groupName };
					if (a_key == groupName || a_key == mcmKey) {
						g_config.groupXpMultiplierScales[index] =
							ParseFloat(a_value).value_or(g_config.groupXpMultiplierScales[index]);
						return;
					}
				}

				return;
			}

			if (a_section != "PlayerXpMultipliers" &&
				a_section != "CustomCachedPlayerXpMultipliers" &&
				a_section != "PlayerXpMultiplierScales" &&
				a_section != "CustomCachedSkillXpMultipliers") {
				return;
			}

			for (std::size_t index = 0; index < SkillCount; ++index) {
				const auto skillName = SkillName(static_cast<Skill>(index));
				if (a_section == "PlayerXpMultipliers" && a_key == skillName) {
					g_config.playerXpMultipliers[index] = ParseFloat(a_value);
					return;
				}

				const auto mcmKey = "f" + std::string{ skillName };
				if (a_section == "CustomCachedPlayerXpMultipliers" && (a_key == skillName || a_key == mcmKey)) {
					g_config.customCachedPlayerXpMultipliers[index] =
						ParseFloat(a_value).value_or(g_config.customCachedPlayerXpMultipliers[index]);
					return;
				}

				if (a_section == "PlayerXpMultiplierScales" && a_key == mcmKey) {
					g_config.playerXpMultiplierScales[index] = ParseFloat(a_value).value_or(g_config.playerXpMultiplierScales[index]);
					return;
				}

				if (a_section == "CustomCachedSkillXpMultipliers" && (a_key == skillName || a_key == mcmKey)) {
					g_config.customCachedSkillXpMultipliers[index] =
						ParseFloat(a_value).value_or(g_config.customCachedSkillXpMultipliers[index]);
					return;
				}

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
		g_config.customCachedPlayerXpMultipliers.fill(1.0F);
		g_config.customCachedSkillXpMultipliers.fill(1.0F);
		g_config.groupXpMultiplierScales = {
			0.75F,
			1.15F,
			0.75F,
			1.4F,
			1.4F,
			0.15F,
			1.4F
		};
		g_config.playerXpMultiplierScales.fill(1.0F);
		for (const auto path : kSettingsPaths) {
			LoadFile(path);
		}
		ApplyLogLevel();
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
