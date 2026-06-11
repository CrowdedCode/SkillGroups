#include "SkillGroups/Hook.h"
#include "SkillGroups/Papyrus.h"
#include "SkillGroups/Settings.h"

#include <Windows.h>
#include <array>
#include <filesystem>
#include <spdlog/sinks/basic_file_sink.h>

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

	void SetupLog()
	{
		auto path = GetPluginDirectory();
		if (path.empty()) {
			path = SKSE::log::log_directory().value_or(std::filesystem::path{});
		}

		if (path.empty()) {
			return;
		}

		path /= "SkillGroups.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto log = std::make_shared<spdlog::logger>("SkillGroups", std::move(sink));
		log->set_level(spdlog::level::debug);
		log->flush_on(spdlog::level::debug);
		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
	}

	void OnDataLoaded()
	{
		SkillGroups::Settings::Load();
		if (!SkillGroups::Hook::CacheMultipliers()) {
			SKSE::log::warn("SkillGroups could not cache any skill multipliers");
		}
		if (!SkillGroups::Hook::Install()) {
			SKSE::log::error("SkillGroups hook could not be installed; grouped character XP will remain disabled");
		}
		SKSE::log::info("SkillGroups initialized");
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("SkillGroups loaded");

	SKSE::GetPapyrusInterface()->Register(SkillGroups::Papyrus::Register);
	SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* a_message) {
		if (a_message && a_message->type == SKSE::MessagingInterface::kDataLoaded) {
			OnDataLoaded();
		}
	});

	return true;
}
