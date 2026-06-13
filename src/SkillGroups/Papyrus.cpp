#include "SkillGroups/Papyrus.h"

#include "SkillGroups/Hook.h"
#include "SkillGroups/Profiles.h"
#include "SkillGroups/Settings.h"

#include <Windows.h>
#include <Shellapi.h>
#include <array>
#include <filesystem>
#include <string>
#include <vector>

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

			SKSE::log::info("SkillGroups refreshed config settings from MCM");
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

		bool CacheCharacterXpSettings(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			if (!Profiles::IsProfileEditable(a_profile)) {
				return false;
			}

			const auto settings = Profiles::GetCharacterXpSettings(a_profile);
			return Profiles::SetCharacterXpSettings(
				a_profile,
				Profiles::CharacterXpSettings{
					settings.useFlatCharacterXp,
					settings.flatCharacterXp,
					Hook::GetCharacterXpLevelUpBase(),
					Hook::GetCharacterXpLevelUpMult() });
		}

		std::uint32_t GetSkillXpProfileCount(RE::StaticFunctionTag*)
		{
			return static_cast<std::uint32_t>(Profiles::ProfileCount());
		}

		RE::BSFixedString GetSkillXpProfileName(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return RE::BSFixedString{ Profiles::ProfileDisplayName(a_profile) };
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

		std::uint32_t GetProfileGroupCount(RE::StaticFunctionTag*, std::uint32_t a_profile)
		{
			return static_cast<std::uint32_t>(Profiles::GetGroupCount(a_profile));
		}

		RE::BSFixedString GetProfileGroupName(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_groupIndex)
		{
			return RE::BSFixedString{ Profiles::GetGroupName(a_profile, a_groupIndex) };
		}

		std::uint32_t GetProfileSkillGroupIndex(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_skillIndex)
		{
			return static_cast<std::uint32_t>(Profiles::GetSkillGroupIndex(a_profile, a_skillIndex));
		}

		RE::BSFixedString GetProfileGroupSlotName(RE::StaticFunctionTag*, std::uint32_t a_profile, std::uint32_t a_slotIndex)
		{
			const auto name = Profiles::GetGroupSlotName(a_profile, a_slotIndex);
			return RE::BSFixedString{ name.c_str() };
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

		bool SetGroupingEnabled(RE::StaticFunctionTag*, bool a_enabled)
		{
			Settings::SetRuntimeEnabled(a_enabled);
			return true;
		}

		bool IsHookAvailable(RE::StaticFunctionTag*)
		{
			return Hook::IsInstalled();
		}

		bool CommitProfileGroups(
			RE::StaticFunctionTag*,
			std::uint32_t a_profile,
			std::vector<RE::BSFixedString> a_groupSlotNames,
			std::vector<std::uint32_t> a_skillGroupSlots)
		{
			std::vector<std::string> groupSlotNames;
			groupSlotNames.reserve(a_groupSlotNames.size());
			for (const auto& name : a_groupSlotNames) {
				groupSlotNames.emplace_back(name.c_str());
			}

			return Profiles::SetSkillGroupAssignments(a_profile, groupSlotNames, a_skillGroupSlots);
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
			const auto multipliersEnabled = Settings::Get().multipliersEnabled;
			const auto profileSettings = Profiles::GetCharacterXpSettings(a_characterProfile);
			const auto levelUpBase = multipliersEnabled ? profileSettings.levelUpBase : 75.0F;
			const auto levelUpMult = multipliersEnabled ? profileSettings.levelUpMult : 25.0F;
			Settings::SetCharacterXpRuntimeSettings(
				profileSettings.useFlatCharacterXp,
				profileSettings.flatCharacterXp,
				profileSettings.levelUpBase,
				profileSettings.levelUpMult);
			const auto appliedSettings = Hook::ApplyCharacterXpGameSettings(levelUpBase, levelUpMult);
			const auto resyncedThreshold = Hook::ResyncCurrentLevelThreshold(levelUpBase, levelUpMult);
			return appliedSettings && resyncedThreshold;
		}

		bool ApplySkillXpMultipliers(RE::StaticFunctionTag*, bool a_useDivisor, std::uint32_t a_profile, std::uint32_t a_groupProfile)
		{
			if (Profiles::IsProfileEditable(a_profile) && !Profiles::SaveProfile(a_profile)) {
				SKSE::log::warn("SkillGroups could not save editable skill XP profile before applying");
				return false;
			}

			Settings::Load();
			const auto result = Settings::Get().multipliersEnabled ?
				Hook::ApplySkillXpMultipliersToGame(a_useDivisor, a_profile, a_groupProfile) :
				Hook::RestoreDefaultSkillXpMultipliersToGame();
			if (result) {
				SKSE::log::info("SkillGroups applied skill XP multipliers from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to apply skill XP multipliers from MCM");
			}

			return result;
		}

		bool ApplyCharacterXpProfiles(RE::StaticFunctionTag*, std::uint32_t a_characterProfile)
		{
			if (Profiles::IsProfileEditable(a_characterProfile) && !Profiles::SaveProfile(a_characterProfile)) {
				SKSE::log::warn("SkillGroups could not save editable character XP profile before applying");
				return false;
			}

			Settings::Load();
			const auto profileSettings = Profiles::GetCharacterXpSettings(a_characterProfile);
			Settings::SetCharacterXpRuntimeSettings(
				profileSettings.useFlatCharacterXp,
				profileSettings.flatCharacterXp,
				profileSettings.levelUpBase,
				profileSettings.levelUpMult);
			const auto appliedThresholds = Settings::Get().multipliersEnabled ?
				Hook::ApplyCharacterXpGameSettings(profileSettings.levelUpBase, profileSettings.levelUpMult) :
				Hook::RestoreDefaultCharacterXpGameSettings();
			const auto refreshedMultipliers = Hook::RefreshCharacterXpMultipliers(a_characterProfile);
			if (appliedThresholds && refreshedMultipliers) {
				SKSE::log::info("SkillGroups applied character XP profiles from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to apply character XP profiles from MCM");
			}

			return appliedThresholds && refreshedMultipliers;
		}

		bool ApplyProfileGroups(RE::StaticFunctionTag*, std::uint32_t a_profile, bool a_refreshSkillXp, std::uint32_t a_skillXpProfile)
		{
			if (Profiles::IsProfileEditable(a_profile) && !Profiles::SaveProfile(a_profile)) {
				SKSE::log::warn("SkillGroups could not save editable group profile before applying");
				return false;
			}

			auto result = Profiles::ApplyGroups(a_profile) && Hook::RefreshCharacterXpMultipliers(a_profile);
			if (result && a_refreshSkillXp && Settings::Get().multipliersEnabled) {
				result = Hook::ApplySkillXpMultipliersToGame(true, a_skillXpProfile, a_profile);
			}
			if (result) {
				SKSE::log::info("SkillGroups applied profile groups from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to apply profile groups from MCM");
			}

			return result;
		}

		bool ApplyMultiplierSettings(RE::StaticFunctionTag*, bool a_enabled, bool a_useDivisor, std::uint32_t a_skillXpProfile, std::uint32_t a_characterProfile)
		{
			Settings::Load();
			Settings::SetRuntimeMultipliersEnabled(a_enabled);
			Settings::SetRuntimeProfiles(static_cast<int>(a_characterProfile), static_cast<int>(a_skillXpProfile));
			const auto profileSettings = Profiles::GetCharacterXpSettings(a_characterProfile);
			Settings::SetCharacterXpRuntimeSettings(
				profileSettings.useFlatCharacterXp,
				profileSettings.flatCharacterXp,
				profileSettings.levelUpBase,
				profileSettings.levelUpMult);

			const auto appliedThresholds = a_enabled ?
				Hook::ApplyCharacterXpGameSettings(profileSettings.levelUpBase, profileSettings.levelUpMult) :
				Hook::RestoreDefaultCharacterXpGameSettings();
			const auto resyncedThreshold = a_enabled ?
				Hook::ResyncCurrentLevelThreshold(profileSettings.levelUpBase, profileSettings.levelUpMult) :
				Hook::ResyncCurrentLevelThreshold(75.0F, 25.0F);
			const auto refreshedMultipliers = Hook::RefreshCharacterXpMultipliers(a_characterProfile);
			const auto appliedSkillXp = a_enabled ?
				Hook::ApplySkillXpMultipliersToGame(a_useDivisor, a_skillXpProfile, a_characterProfile) :
				Hook::RestoreDefaultSkillXpMultipliersToGame();

			if (appliedThresholds && resyncedThreshold && refreshedMultipliers && appliedSkillXp) {
				SKSE::log::info("SkillGroups applied multiplier-enabled state from MCM: {}", a_enabled);
			} else {
				SKSE::log::warn("SkillGroups failed to apply multiplier-enabled state from MCM: {}", a_enabled);
			}

			return appliedThresholds && resyncedThreshold && refreshedMultipliers && appliedSkillXp;
		}

		bool ApplyRuntimeSettings(
			RE::StaticFunctionTag*,
			bool a_enabled,
			bool a_multipliersEnabled,
			bool a_useDivisor,
			std::uint32_t a_skillXpProfile,
			std::uint32_t a_characterProfile)
		{
			Settings::Load();
			Settings::SetRuntimeEnabled(a_enabled);
			Settings::SetRuntimeMultipliersEnabled(a_multipliersEnabled);
			Settings::SetRuntimeProfiles(static_cast<int>(a_characterProfile), static_cast<int>(a_skillXpProfile));

			const auto profileSettings = Profiles::GetCharacterXpSettings(a_characterProfile);
			Settings::SetCharacterXpRuntimeSettings(
				profileSettings.useFlatCharacterXp,
				profileSettings.flatCharacterXp,
				profileSettings.levelUpBase,
				profileSettings.levelUpMult);

			const auto appliedThresholds = a_multipliersEnabled ?
				Hook::ApplyCharacterXpGameSettings(profileSettings.levelUpBase, profileSettings.levelUpMult) :
				Hook::RestoreDefaultCharacterXpGameSettings();
			const auto resyncedThreshold = a_multipliersEnabled ?
				Hook::ResyncCurrentLevelThreshold(profileSettings.levelUpBase, profileSettings.levelUpMult) :
				Hook::ResyncCurrentLevelThreshold(75.0F, 25.0F);
			const auto refreshedMultipliers = Hook::RefreshCharacterXpMultipliers(a_characterProfile);
			const auto appliedSkillXp = a_multipliersEnabled ?
				Hook::ApplySkillXpMultipliersToGame(a_useDivisor, a_skillXpProfile, a_characterProfile) :
				Hook::RestoreDefaultSkillXpMultipliersToGame();

			if (appliedThresholds && resyncedThreshold && refreshedMultipliers && appliedSkillXp) {
				SKSE::log::info(
					"SkillGroups applied runtime settings from MCM: enabled={}, multipliersEnabled={}, characterProfile={}, skillProfile={}",
					a_enabled,
					a_multipliersEnabled,
					a_characterProfile,
					a_skillXpProfile);
			} else {
				SKSE::log::warn("SkillGroups failed to apply runtime settings from MCM");
			}

			return appliedThresholds && resyncedThreshold && refreshedMultipliers && appliedSkillXp;
		}

		bool CacheSkillXpMultipliers(RE::StaticFunctionTag*)
		{
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
		a_vm->RegisterFunction("ApplyMultiplierSettings", "SkillGroups_Native", ApplyMultiplierSettings);
		a_vm->RegisterFunction("ApplyRuntimeSettings", "SkillGroups_Native", ApplyRuntimeSettings);
		a_vm->RegisterFunction("ApplySkillXpMultipliers", "SkillGroups_Native", ApplySkillXpMultipliers);
		a_vm->RegisterFunction("CacheCharacterXpSettings", "SkillGroups_Native", CacheCharacterXpSettings);
		a_vm->RegisterFunction("CacheSkillXpMultipliers", "SkillGroups_Native", CacheSkillXpMultipliers);
		a_vm->RegisterFunction("GetCachedSkillXpMultiplier", "SkillGroups_Native", GetCachedSkillXpMultiplier);
		a_vm->RegisterFunction("GetCharacterXpProfileMultiplier", "SkillGroups_Native", GetCharacterXpProfileMultiplier);
		a_vm->RegisterFunction("GetGroupXpProfileMultiplierScale", "SkillGroups_Native", GetGroupXpProfileMultiplierScale);
		a_vm->RegisterFunction("GetPlayerXpProfileMultiplierScale", "SkillGroups_Native", GetPlayerXpProfileMultiplierScale);
		a_vm->RegisterFunction("GetProfileGroupCount", "SkillGroups_Native", GetProfileGroupCount);
		a_vm->RegisterFunction("GetProfileGroupName", "SkillGroups_Native", GetProfileGroupName);
		a_vm->RegisterFunction("GetProfileGroupSlotName", "SkillGroups_Native", GetProfileGroupSlotName);
		a_vm->RegisterFunction("GetProfileSkillGroupIndex", "SkillGroups_Native", GetProfileSkillGroupIndex);
		a_vm->RegisterFunction("GetProfileFlatCharacterXp", "SkillGroups_Native", GetProfileFlatCharacterXp);
		a_vm->RegisterFunction("GetProfileLevelUpBase", "SkillGroups_Native", GetProfileLevelUpBase);
		a_vm->RegisterFunction("GetProfileLevelUpMult", "SkillGroups_Native", GetProfileLevelUpMult);
		a_vm->RegisterFunction("GetProfileUseFlatCharacterXp", "SkillGroups_Native", GetProfileUseFlatCharacterXp);
		a_vm->RegisterFunction("GetSkillXpProfileCount", "SkillGroups_Native", GetSkillXpProfileCount);
		a_vm->RegisterFunction("GetSkillXpProfileName", "SkillGroups_Native", GetSkillXpProfileName);
		a_vm->RegisterFunction("GetSkillXpProfileMultiplier", "SkillGroups_Native", GetSkillXpProfileMultiplier);
		a_vm->RegisterFunction("IsHookAvailable", "SkillGroups_Native", IsHookAvailable);
		a_vm->RegisterFunction("IsSkillXpProfileEditable", "SkillGroups_Native", IsSkillXpProfileEditable);
		a_vm->RegisterFunction("OpenLogFile", "SkillGroups_Native", OpenLogFile);
		a_vm->RegisterFunction("RefreshSettings", "SkillGroups_Native", RefreshSettings);
		a_vm->RegisterFunction("ApplyProfileGroups", "SkillGroups_Native", ApplyProfileGroups);
		a_vm->RegisterFunction("CommitProfileGroups", "SkillGroups_Native", CommitProfileGroups);
		a_vm->RegisterFunction("ResyncCurrentLevelThreshold", "SkillGroups_Native", ResyncCurrentLevelThreshold);
		a_vm->RegisterFunction("SetCharacterXpProfileMultiplier", "SkillGroups_Native", SetCharacterXpProfileMultiplier);
		a_vm->RegisterFunction("SetGroupXpProfileMultiplierScale", "SkillGroups_Native", SetGroupXpProfileMultiplierScale);
		a_vm->RegisterFunction("SetPlayerXpProfileMultiplierScale", "SkillGroups_Native", SetPlayerXpProfileMultiplierScale);
		a_vm->RegisterFunction("SetProfileCharacterXpSettings", "SkillGroups_Native", SetProfileCharacterXpSettings);
		a_vm->RegisterFunction("SetGroupingEnabled", "SkillGroups_Native", SetGroupingEnabled);
		a_vm->RegisterFunction("SetSkillXpProfileMultiplier", "SkillGroups_Native", SetSkillXpProfileMultiplier);
		return true;
	}
}
