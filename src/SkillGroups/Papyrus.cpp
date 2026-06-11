#include "SkillGroups/Papyrus.h"

#include "SkillGroups/Hook.h"
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

			(void)Hook::CacheMultipliers();
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

		float GetCachedPlayerXpMultiplier(RE::StaticFunctionTag*, std::uint32_t a_index)
		{
			return Hook::GetCachedPlayerXpMultiplier(a_index);
		}

		float GetCachedSkillXpMultiplier(RE::StaticFunctionTag*, std::uint32_t a_index)
		{
			return Hook::GetCachedSkillXpMultiplier(a_index);
		}

		bool RecacheMultipliers(RE::StaticFunctionTag*)
		{
			Settings::Load();
			const auto result = Hook::CacheMultipliers();
			if (result) {
				SKSE::log::info("SkillGroups recached multiplier settings from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to recache multiplier settings from MCM");
			}

			return result;
		}

		bool ApplyCustomMultipliers(RE::StaticFunctionTag*)
		{
			Settings::Load();
			const auto result = Hook::ApplyCustomMultipliersToGame();
			if (result) {
				SKSE::log::info("SkillGroups applied custom player XP multipliers from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to apply custom player XP multipliers from MCM");
			}

			return result;
		}

		bool ApplySkillXpMultipliers(RE::StaticFunctionTag*)
		{
			Settings::Load();
			const auto result = Hook::ApplySkillXpMultipliersToGame();
			if (result) {
				SKSE::log::info("SkillGroups applied skill XP multipliers from MCM");
			} else {
				SKSE::log::warn("SkillGroups failed to apply skill XP multipliers from MCM");
			}

			return result;
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

		a_vm->RegisterFunction("ApplyCustomMultipliers", "SkillGroups_Native", ApplyCustomMultipliers);
		a_vm->RegisterFunction("ApplySkillXpMultipliers", "SkillGroups_Native", ApplySkillXpMultipliers);
		a_vm->RegisterFunction("CacheSkillXpMultipliers", "SkillGroups_Native", CacheSkillXpMultipliers);
		a_vm->RegisterFunction("GetCachedPlayerXpMultiplier", "SkillGroups_Native", GetCachedPlayerXpMultiplier);
		a_vm->RegisterFunction("GetCachedSkillXpMultiplier", "SkillGroups_Native", GetCachedSkillXpMultiplier);
		a_vm->RegisterFunction("OpenLogFile", "SkillGroups_Native", OpenLogFile);
		a_vm->RegisterFunction("RecacheMultipliers", "SkillGroups_Native", RecacheMultipliers);
		a_vm->RegisterFunction("RefreshSettings", "SkillGroups_Native", RefreshSettings);
		return true;
	}
}
