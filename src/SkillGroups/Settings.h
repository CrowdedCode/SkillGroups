#pragma once

#include "SkillGroups/Core.h"

#include <array>
#include <optional>

namespace SkillGroups::Settings
{
	enum class LogLevel
	{
		Error,
		Info,
		Debug
	};

	struct Config
	{
		bool enabled{ true };
		LogLevel logLevel{ LogLevel::Info };
		bool autoCacheOnLevelXp{ false };
		bool autoApplySkillXpOnLevelXp{ false };
		bool divideSkillXpByGroupSize{ true };
		bool useCustomCachedPlayerXpMultipliers{ false };
		bool useCustomCachedSkillXpMultipliers{ false };
		std::array<std::optional<float>, SkillCount> playerXpMultipliers{};
		std::array<float, SkillCount> customCachedPlayerXpMultipliers{};
		std::array<float, SkillGroupCount> groupXpMultiplierScales{};
		std::array<float, SkillCount> playerXpMultiplierScales{};
		std::array<float, SkillCount> customCachedSkillXpMultipliers{};
	};

	[[nodiscard]] const Config& Get();
	void Load();
	void ApplyLogLevel();
	[[nodiscard]] bool IsDebugLoggingEnabled();
}
