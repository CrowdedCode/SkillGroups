#include "SkillGroups/Papyrus.h"

#include "SkillGroups/Hook.h"
#include "SkillGroups/Profiles.h"
#include "SkillGroups/Settings.h"

#include <Windows.h>
#include <Shellapi.h>
#include <array>
#include <filesystem>

namespace SkillGroups::Papyrus
{
	namespace
	{
		std::filesystem::path GetPluginDirectory()
		{
			std::array<wchar_t, MAX_PATH> path{};
			HMODULE module = nullptr;
			constexpr auto flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
			if (!GetModuleHandleExW(flags, reinterpret_cast<LPCWSTR>(&GetPluginDirectory), std::addressof(module))) {
				return {};
			}

			const auto length = GetModuleFileNameW(module, path.data(), static_cast<DWORD>(path.size()));
			if (length == 0 || length >= path.size()) {
				return {};
			}

			return std::filesystem::path{ std::wstring_view{ path.data(), length } }.parent_path();
		}

		bool RefreshSettings(RE::StaticFunctionTag*)
		{
			Settings::Load();
			if (!Hook::Install()) {
				return false;
			}

			(void)Hook::RefreshCharacterXpMultipliers();
			SKSE::log::info("SkillGroups refreshed runtime settings from MCM");
			return true;
		}

		bool OpenLogFile(RE::StaticFunctionTag*)
		{
			auto path = GetPluginDirectory();
			if (path.empty()) {
				path = SKSE::log::log_directory().value_or(std::filesystem::path{});
			}

			if (path.empty()) {
				SKSE::log::error("SkillGroups could not resolve a log directory for MCM open-log request");
				return false;
			}

			path /= "SkillGroups.log";
			const auto result = ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			if (reinterpret_cast<std::intptr_t>(result) <= 32) {
				SKSE::log::error("SkillGroups failed to open log file from MCM: {}", path.string());
				return false;
			}

			return true;
		}

		float GetCachedSkillXpMultiplier(RE::StaticFunctionTag*, std::uint32_t a_index)
		{
			return Hook::GetCachedSkillXpMultiplier(a_index);
		}

		std::uint32_t GetSkillXpProfileCount(RE::StaticFunctionTag*)
		{
			return static_cast<std::uint32_t>(Profiles::ProfileCount());
		}

		RE::BSFixedString GetSkillXpProfileName(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return RE::BSFixedString{ Profiles::ProfileName(a_profile) };
		}

		bool IsSkillXpProfileEditable(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return Profiles::IsProfileEditable(a_profile);
		}

		float GetSkillXpProfileMultiplier(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_skillIndex)
		{
			return Profiles::GetSkillXpMultiplier(a_profile, a_skillIndex);
		}

		float GetCharacterXpProfileMultiplier(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_skillIndex)
		{
			return Profiles::GetCharacterXpMultiplier(a_profile, a_skillIndex);
		}

		float GetGroupXpProfileMultiplierScale(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_groupIndex)
		{
			return Profiles::GetGroupXpMultiplierScale(a_profile, a_groupIndex);
		}

		float GetPlayerXpProfileMultiplierScale(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_skillIndex)
		{
			return Profiles::GetPlayerXpMultiplierScale(a_profile, a_skillIndex);
		}

		bool GetProfileUseFlatCharacterXp(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return Profiles::GetCharacterXpSettings(a_profile).useFlatCharacterXp;
		}

		float GetProfileFlatCharacterXp(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return Profiles::GetCharacterXpSettings(a_profile).flatCharacterXp;
		}

		float GetProfileLevelUpBase(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return Profiles::GetCharacterXpSettings(a_profile).levelUpBase;
		}

		float GetProfileLevelUpMult(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return Profiles::GetCharacterXpSettings(a_profile).levelUpMult;
		}

		bool SetCharacterXpProfileMultiplier(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_skillIndex, float a_value)
		{
			return Profiles::SetCharacterXpMultiplier(a_profile, a_skillIndex, a_value);
		}

		bool SetGroupXpProfileMultiplierScale(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_groupIndex, float a_value)
		{
			return Profiles::SetGroupXpMultiplierScale(a_profile, a_groupIndex, a_value);
		}

		bool SetPlayerXpProfileMultiplierScale(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_skillIndex, float a_value)
		{
			return Profiles::SetPlayerXpMultiplierScale(a_profile, a_skillIndex, a_value);
		}

		bool SetSkillXpProfileMultiplier(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_skillIndex, float a_value)
		{
			return Profiles::SetSkillXpMultiplier(a_profile, a_skillIndex, a_value);
		}

		bool SetProfileCharacterXpSettings(RE::StaticFunctionTag*,
			std::uint32_t a_profile,
			bool a_useFlatCharacterXp,
			float a_flatCharacterXp,
			float a_levelUpBase,
			float a_levelUpMult)
		{
			return Profiles::SetCharacterXpSettings(
				a_profile,
				Profiles::CharacterXpSettings{
					a_useFlatCharacterXp,
					a_flatCharacterXp,
					a_levelUpBase,
					a_levelUpMult });
		}

		bool ResyncCurrentLevelThreshold(RE::StaticFunctionTag*, std::uint32_t a_characterProfile)
		{
			Settings::Load();
			const auto profileSettings = Profiles::GetCharacterXpSettings(a_characterProfile);
			Settings::SetCharacterXpRuntimeSettings(
				profileSettings.useFlatCharacterXp,
				profileSettings.flatCharacterXp,
				profileSettings.levelUpBase,
				profileSettings.levelUpMult);
			const auto appliedSettings = Hook::ApplyCharacterXpGameSettings(profileSettings.levelUpBase, profileSettings.levelUpMult);
			const auto resyncedThreshold = Hook::ResyncCurrentLevelThreshold(profileSettings.levelUpBase, profileSettings.levelUpMult);
			return appliedSettings && resyncedThreshold;
		}

		bool ApplySkillXpMultipliers(RE::StaticFunctionTag*, bool a_useDivisor, std::uint32_t a_profile)
		{
			if (Profiles::IsProfileEditable(a_profile) && !Profiles::SaveProfile(a_profile)) {
				SKSE::log::warn("SkillGroups could not save editable skill XP profile before applying");
				return false;
			}

			Settings::Load();
			const auto result = Hook::ApplySkillXpMultipliersToGame(a_useDivisor, a_profile);
			if (result) {
				SKSE::log::info("SkillGroups applied skill XP multipliers from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to apply skill XP multipliers from MCM");
			}

			return result;
		}

		bool ApplyCharacterXpProfiles(RE::StaticFunctionTag*, std::uint32_t a_characterProfile, std::uint32_t a_scalingProfile)
		{
			if (Profiles::IsProfileEditable(a_characterProfile) && !Profiles::SaveProfile(a_characterProfile)) {
				SKSE::log::warn("SkillGroups could not save editable character XP profile before applying");
				return false;
			}

			if (a_scalingProfile != a_characterProfile &&
				Profiles::IsProfileEditable(a_scalingProfile) &&
				!Profiles::SaveProfile(a_scalingProfile)) {
				SKSE::log::warn("SkillGroups could not save editable character XP scaling profile before applying");
				return false;
			}

			Settings::Load();
			const auto profileSettings = Profiles::GetCharacterXpSettings(a_characterProfile);
			Settings::SetCharacterXpRuntimeSettings(
				profileSettings.useFlatCharacterXp,
				profileSettings.flatCharacterXp,
				profileSettings.levelUpBase,
				profileSettings.levelUpMult);
			const auto appliedThresholds = Hook::ApplyCharacterXpGameSettings(profileSettings.levelUpBase, profileSettings.levelUpMult);
			const auto refreshedMultipliers = Hook::RefreshCharacterXpMultipliers();
			if (appliedThresholds && refreshedMultipliers) {
				SKSE::log::info("SkillGroups applied character XP profiles from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to apply character XP profiles from MCM");
			}

			return appliedThresholds && refreshedMultipliers;
		}

		bool CacheSkillXpMultipliers(RE::StaticFunctionTag*)
		{
			Settings::Load();
			const auto result = Hook::CacheSkillXpMultipliers();
			if (result) {
				SKSE::log::info("SkillGroups cached skill XP multipliers from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to cache skill XP multipliers from MCM");
			}

			return result;
		}
	}

	bool Register(RE::BSScript::IVirtualMachine* a_vm)
	{
		if (!a_vm) {
			return false;
		}

		a_vm->RegisterFunction("ApplyCharacterXpProfiles", "SkillGroups_Native", ApplyCharacterXpProfiles);
		a_vm->RegisterFunction("ApplySkillXpMultipliers", "SkillGroups_Native", ApplySkillXpMultipliers);
		a_vm->RegisterFunction("CacheSkillXpMultipliers", "SkillGroups_Native", CacheSkillXpMultipliers);
		a_vm->RegisterFunction("GetCachedSkillXpMultiplier", "SkillGroups_Native", GetCachedSkillXpMultiplier);
		a_vm->RegisterFunction("GetCharacterXpProfileMultiplier", "SkillGroups_Native", GetCharacterXpProfileMultiplier);
		a_vm->RegisterFunction("GetGroupXpProfileMultiplierScale", "SkillGroups_Native", GetGroupXpProfileMultiplierScale);
		a_vm->RegisterFunction("GetPlayerXpProfileMultiplierScale", "SkillGroups_Native", GetPlayerXpProfileMultiplierScale);
		a_vm->RegisterFunction("GetProfileFlatCharacterXp", "SkillGroups_Native", GetProfileFlatCharacterXp);
		a_vm->RegisterFunction("GetProfileLevelUpBase", "SkillGroups_Native", GetProfileLevelUpBase);
		a_vm->RegisterFunction("GetProfileLevelUpMult", "SkillGroups_Native", GetProfileLevelUpMult);
		a_vm->RegisterFunction("GetProfileUseFlatCharacterXp", "SkillGroups_Native", GetProfileUseFlatCharacterXp);
		a_vm->RegisterFunction("GetSkillXpProfileCount", "SkillGroups_Native", GetSkillXpProfileCount);
		a_vm->RegisterFunction("GetSkillXpProfileName", "SkillGroups_Native", GetSkillXpProfileName);
		a_vm->RegisterFunction("GetSkillXpProfileMultiplier", "SkillGroups_Native", GetSkillXpProfileMultiplier);
		a_vm->RegisterFunction("IsSkillXpProfileEditable", "SkillGroups_Native", IsSkillXpProfileEditable);
		a_vm->RegisterFunction("OpenLogFile", "SkillGroups_Native", OpenLogFile);
		a_vm->RegisterFunction("RefreshSettings", "SkillGroups_Native", RefreshSettings);
		a_vm->RegisterFunction("ResyncCurrentLevelThreshold", "SkillGroups_Native", ResyncCurrentLevelThreshold);
		a_vm->RegisterFunction("SetCharacterXpProfileMultiplier", "SkillGroups_Native", SetCharacterXpProfileMultiplier);
		a_vm->RegisterFunction("SetGroupXpProfileMultiplierScale", "SkillGroups_Native", SetGroupXpProfileMultiplierScale);
		a_vm->RegisterFunction("SetPlayerXpProfileMultiplierScale", "SkillGroups_Native", SetPlayerXpProfileMultiplierScale);
		a_vm->RegisterFunction("SetProfileCharacterXpSettings", "SkillGroups_Native", SetProfileCharacterXpSettings);
		a_vm->RegisterFunction("SetSkillXpProfileMultiplier", "SkillGroups_Native", SetSkillXpProfileMultiplier);
		return true;
	}
}
