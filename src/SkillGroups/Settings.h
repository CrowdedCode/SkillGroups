#pragma once

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
		int characterXpProfileIndex{ 0 };
		int skillXpProfileIndex{ 0 };
		bool autoApplySkillXpOnLevelXp{ false };
		bool divideSkillXpByGroupSize{ true };
		bool useFlatCharacterXp{ false };
		float flatCharacterXp{ 10.0F };
		float levelUpBase{ 75.0F };
		float levelUpMult{ 25.0F };
	};

	[[nodiscard]] const Config& Get();
	void Load();
	void LoadProfiles();
	void SetCharacterXpRuntimeSettings(bool a_useFlatCharacterXp, float a_flatCharacterXp, float a_levelUpBase, float a_levelUpMult);
	void ApplyLogLevel();
	[[nodiscard]] bool IsDebugLoggingEnabled();
}
