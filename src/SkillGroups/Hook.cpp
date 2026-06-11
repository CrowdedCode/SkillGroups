#include "SkillGroups/Hook.h"

#include "SkillGroups/Core.h"
#include "SkillGroups/Settings.h"

#include <array>
#include <cmath>
#include <cstring>
#include <sstream>

namespace SkillGroups::Hook
{
	namespace
	{
		constexpr REL::RelocationID kImproveLevelExpBySkillLevel{ 40576, 41561 };
		constexpr REL::RelocationID kXpPerSkillRank{ 505484, 374914 };
		constexpr auto kImproveLevelExpBySkillLevelAeOffset = static_cast<std::uintptr_t>(0x2CB);
		constexpr auto kPatchSize = 12U;
		constexpr std::array<std::uint8_t, kPatchSize> kExpectedAeBytes{
			0xF3, 0x0F, 0x5C, 0xCA,
			0xF3, 0x0F, 0x59, 0x0D,
			0x00, 0x00, 0x00, 0x00
		};
		constexpr std::array<bool, kPatchSize> kExpectedAeMask{
			true, true, true, true,
			true, true, true, true,
			false, false, false, false
		};

		std::array<float, SkillCount> g_vanillaMultipliers{};
		std::array<float, SkillCount> g_effectiveMultipliers{};
		std::array<bool, SkillCount> g_hasMultiplier{};
		std::array<float, SkillCount> g_cachedSkillXpBaseMultipliers{};
		std::array<bool, SkillCount> g_hasSkillXpBaseMultiplier{};
		bool g_installed{ false };
		bool g_installAttempted{ false };
		bool g_cachedMultipliers{ false };
		bool g_cachedSkillXpMultipliers{ false };

		[[nodiscard]] RE::ActorValue ToReActorValue(Skill a_skill)
		{
			return static_cast<RE::ActorValue>(SkillGroups::ToActorValue(a_skill));
		}

		[[nodiscard]] RE::ActorValueInfo::Skill* GetActorValueSkill(Skill a_skill)
		{
			const auto* list = RE::ActorValueList::GetSingleton();
			if (!list) {
				return nullptr;
			}

			const auto* info = list->GetActorValue(ToReActorValue(a_skill));
			return info ? info->skill : nullptr;
		}

		[[nodiscard]] RE::PlayerCharacter* GetPlayer()
		{
			return RE::PlayerCharacter::GetSingleton();
		}

		[[nodiscard]] std::array<SkillState, SkillCount> ReadLiveSkillLevels(RE::PlayerCharacter& a_player)
		{
			std::array<SkillState, SkillCount> states{};
			auto* actorValueOwner = a_player.AsActorValueOwner();
			if (!actorValueOwner) {
				return states;
			}

			for (std::size_t index = 0; index < SkillCount; ++index) {
				const auto skill = static_cast<Skill>(index);
				states[index].level = actorValueOwner->GetBaseActorValue(ToReActorValue(skill));
			}

			return states;
		}

		[[nodiscard]] float GetXpPerSkillRank()
		{
			const REL::Relocation<float*> setting{ kXpPerSkillRank };
			return *setting;
		}

		[[nodiscard]] float MultiplierScale(Skill a_skill, float a_effectiveMultiplier)
		{
			const auto index = static_cast<std::size_t>(a_skill);
			if (!g_hasMultiplier[index]) {
				return 1.0F;
			}

			const auto vanilla = g_vanillaMultipliers[index];
			if (std::fabs(vanilla) <= 0.0001F) {
				return a_effectiveMultiplier == 0.0F ? 0.0F : 1.0F;
			}

			return a_effectiveMultiplier / vanilla;
		}

		[[nodiscard]] float GroupMultiplierSum(Skill a_skill)
		{
			const auto* group = FindSkillGroup(a_skill);
			if (!group) {
				const auto index = static_cast<std::size_t>(a_skill);
				return g_hasMultiplier[index] ? g_effectiveMultipliers[index] : 1.0F;
			}

			float result = 0.0F;
			for (const auto skill : group->skills) {
				const auto index = static_cast<std::size_t>(skill);
				if (!g_hasMultiplier[index]) {
					continue;
				}

				result += g_effectiveMultipliers[index];
			}

			return result;
		}

		[[nodiscard]] std::string DescribeGroupLevels(const std::array<SkillState, SkillCount>& a_states, Skill a_skill)
		{
			const auto* group = FindSkillGroup(a_skill);
			if (!group) {
				return "no group";
			}

			std::ostringstream stream;
			stream << group->name << " [";
			for (std::size_t index = 0; index < group->skills.size(); ++index) {
				const auto skill = group->skills[index];
				if (index > 0) {
					stream << ", ";
				}

				stream << SkillName(skill) << '=' << a_states[static_cast<std::size_t>(skill)].level;
			}
			stream << ']';
			return stream.str();
		}

		[[nodiscard]] float OtherGroupMaxLevel(const std::array<SkillState, SkillCount>& a_states, Skill a_skill)
		{
			const auto* group = FindSkillGroup(a_skill);
			if (!group) {
				return 0.0F;
			}

			float result = 0.0F;
			for (const auto skill : group->skills) {
				if (skill == a_skill) {
					continue;
				}

				result = std::max(result, std::floor(a_states[static_cast<std::size_t>(skill)].level));
			}

			return result;
		}

		extern "C" float ImproveLevelExpBySkillLevelHook(float a_experience, std::uint32_t a_rawSkill)
		{
			const auto baseResult = a_experience * GetXpPerSkillRank();
			if (!Settings::Get().enabled || !g_cachedMultipliers) {
				return baseResult;
			}

			const auto skill = ToSkill(a_rawSkill);
			if (!skill) {
				return baseResult;
			}

			if (Settings::Get().autoCacheOnLevelXp) {
				(void)CacheMultipliers();
			}

			if (Settings::Get().autoApplySkillXpOnLevelXp) {
				(void)ApplySkillXpMultipliersToGame();
			}

			auto* player = GetPlayer();
			if (!player) {
				SKSE::log::warn("SkillGroups could not read player while gating level XP");
				return baseResult;
			}

			auto states = ReadLiveSkillLevels(*player);
			const auto awardedLevel = std::floor(a_experience);
			states[static_cast<std::size_t>(*skill)].level = awardedLevel;
			const auto contributes = ShouldLevelIncreaseContributeAtLevel(states, *skill, awardedLevel);
			const auto groupMultiplierSum = GroupMultiplierSum(*skill);
			const auto groupMultiplierScale = MultiplierScale(*skill, groupMultiplierSum);
			const auto awardedPlayerXp = contributes ? baseResult * groupMultiplierScale : 0.0F;
			if (Settings::IsDebugLoggingEnabled()) {
				const auto activeLevel = states[static_cast<std::size_t>(*skill)].level;
				const auto otherMax = OtherGroupMaxLevel(states, *skill);
				const auto skillIndex = static_cast<std::size_t>(*skill);
				const auto skillMultiplierScale = MultiplierScale(*skill, g_effectiveMultipliers[skillIndex]);
				SKSE::log::debug(
					"{} rank-up level XP: actorValue={}, skillRank={}, vanillaPlayerXp={}, skillMultiplierScale={}, groupMultiplierSum={}, groupMultiplierScale={}, awardedPlayerXp={}, activeLevel={}, otherGroupMax={}, contributes={}, {}",
					SkillName(*skill),
					a_rawSkill,
					awardedLevel,
					baseResult,
					skillMultiplierScale,
					groupMultiplierSum,
					groupMultiplierScale,
					awardedPlayerXp,
					activeLevel,
					otherMax,
					contributes,
					DescribeGroupLevels(states, *skill));
			}

			return awardedPlayerXp;
		}

		[[nodiscard]] bool MatchesExpectedBytes(std::uintptr_t a_address)
		{
			for (std::size_t index = 0; index < kPatchSize; ++index) {
				if (kExpectedAeMask[index] &&
					*reinterpret_cast<const std::uint8_t*>(a_address + index) != kExpectedAeBytes[index]) {
					return false;
				}
			}

			return true;
		}

		void WriteAbsoluteCall(std::uintptr_t a_address, const void* a_target)
		{
			std::array<std::uint8_t, kPatchSize> bytes{
				0x48, 0xBA,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0xFF, 0xD2
			};
			const auto target = reinterpret_cast<std::uintptr_t>(a_target);
			std::memcpy(bytes.data() + 2, std::addressof(target), sizeof(target));
			REL::safe_write(a_address, bytes.data(), bytes.size());
		}

		[[nodiscard]] void* WriteAeWrapper(SKSE::Trampoline& a_trampoline)
		{
			std::array<std::uint8_t, 160> code{
				0x50,
				0x51,
				0x52,
				0x41, 0x50,
				0x41, 0x51,
				0x41, 0x52,
				0x41, 0x53,
				0x48, 0x81, 0xEC, 0x90, 0x00, 0x00, 0x00,
				0xF3, 0x0F, 0x7F, 0x44, 0x24, 0x20,
				0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x30,
				0xF3, 0x0F, 0x7F, 0x54, 0x24, 0x40,
				0xF3, 0x0F, 0x7F, 0x5C, 0x24, 0x50,
				0xF3, 0x0F, 0x7F, 0x64, 0x24, 0x60,
				0xF3, 0x0F, 0x7F, 0x6C, 0x24, 0x70,
				0xF3, 0x0F, 0x5C, 0xCA,
				0xF3, 0x0F, 0x10, 0xC1,
				0x8B, 0xD6,
				0x48, 0xB8,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0xFF, 0xD0,
				0xF3, 0x0F, 0x11, 0x44, 0x24, 0x30,
				0xF3, 0x0F, 0x6F, 0x6C, 0x24, 0x70,
				0xF3, 0x0F, 0x6F, 0x64, 0x24, 0x60,
				0xF3, 0x0F, 0x6F, 0x5C, 0x24, 0x50,
				0xF3, 0x0F, 0x6F, 0x54, 0x24, 0x40,
				0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x30,
				0xF3, 0x0F, 0x6F, 0x44, 0x24, 0x20,
				0x48, 0x81, 0xC4, 0x90, 0x00, 0x00, 0x00,
				0x41, 0x5B,
				0x41, 0x5A,
				0x41, 0x59,
				0x41, 0x58,
				0x5A,
				0x59,
				0x58,
				0xC3
			};

			const auto target = reinterpret_cast<std::uintptr_t>(ImproveLevelExpBySkillLevelHook);
			std::memcpy(code.data() + 66, std::addressof(target), sizeof(target));

			auto* wrapper = a_trampoline.allocate(code.size());
			std::memcpy(wrapper, code.data(), code.size());
			return wrapper;
		}
	}

	bool CacheMultipliers()
	{
		const auto& settings = Settings::Get();
		auto cachedAnyMultiplier = false;

		for (std::size_t index = 0; index < SkillCount; ++index) {
			const auto skill = static_cast<Skill>(index);
			auto* data = GetActorValueSkill(skill);
			if (!data) {
				g_hasMultiplier[index] = false;
				SKSE::log::warn("SkillGroups could not cache multiplier for {}", SkillName(skill));
				continue;
			}

			const auto groupIndex = FindSkillGroupIndex(skill);
			const auto groupScale = groupIndex ? settings.groupXpMultiplierScales[*groupIndex] : 1.0F;
			const auto liveMultiplier = data->improveMult;
			const auto configuredMultiplier = settings.useCustomCachedPlayerXpMultipliers ?
				settings.customCachedPlayerXpMultipliers[index] :
				settings.playerXpMultipliers[index].value_or(liveMultiplier);
			const auto skillScale = settings.playerXpMultiplierScales[index];

			g_vanillaMultipliers[index] = liveMultiplier;
			g_effectiveMultipliers[index] =
				configuredMultiplier *
				skillScale *
				groupScale;
			g_hasMultiplier[index] = true;
			cachedAnyMultiplier = true;
		}

		g_cachedMultipliers = cachedAnyMultiplier;
		return cachedAnyMultiplier;
	}

	bool ApplyCustomMultipliersToGame()
	{
		const auto& settings = Settings::Get();
		auto appliedAnyMultiplier = false;

		for (std::size_t index = 0; index < SkillCount; ++index) {
			const auto skill = static_cast<Skill>(index);
			auto* data = GetActorValueSkill(skill);
			if (!data) {
				SKSE::log::warn("SkillGroups could not apply custom multiplier for {}", SkillName(skill));
				continue;
			}

			const auto customMultiplier = settings.customCachedPlayerXpMultipliers[index];
			data->improveMult = customMultiplier;
			appliedAnyMultiplier = true;
		}

		if (appliedAnyMultiplier) {
			(void)CacheMultipliers();
		}

		return appliedAnyMultiplier;
	}

	bool CacheSkillXpMultipliers()
	{
		auto cachedAnyMultiplier = false;

		for (std::size_t index = 0; index < SkillCount; ++index) {
			const auto skill = static_cast<Skill>(index);
			auto* data = GetActorValueSkill(skill);
			if (!data) {
				g_hasSkillXpBaseMultiplier[index] = false;
				SKSE::log::warn("SkillGroups could not cache skill XP multiplier for {}", SkillName(skill));
				continue;
			}

			g_cachedSkillXpBaseMultipliers[index] = data->useMult;
			g_hasSkillXpBaseMultiplier[index] = true;
			cachedAnyMultiplier = true;
		}

		g_cachedSkillXpMultipliers = cachedAnyMultiplier;
		return cachedAnyMultiplier;
	}

	float GetCachedPlayerXpMultiplier(std::size_t a_index)
	{
		if (a_index >= SkillCount || !g_hasMultiplier[a_index]) {
			return 1.0F;
		}

		return g_vanillaMultipliers[a_index];
	}

	float GetCachedSkillXpMultiplier(std::size_t a_index)
	{
		if (a_index >= SkillCount || !g_hasSkillXpBaseMultiplier[a_index]) {
			return 1.0F;
		}

		return g_cachedSkillXpBaseMultipliers[a_index];
	}

	bool ApplySkillXpMultipliersToGame()
	{
		const auto& settings = Settings::Get();
		if (!g_cachedSkillXpMultipliers) {
			(void)CacheSkillXpMultipliers();
		}

		auto appliedAnyMultiplier = false;

		for (std::size_t index = 0; index < SkillCount; ++index) {
			const auto skill = static_cast<Skill>(index);
			auto* data = GetActorValueSkill(skill);
			if (!data) {
				SKSE::log::warn("SkillGroups could not apply skill XP multiplier for {}", SkillName(skill));
				continue;
			}

			const auto* group = FindSkillGroup(skill);
			const auto groupSize = group ? static_cast<float>(group->skills.size()) : 1.0F;
			const auto groupSizeDivisor = settings.divideSkillXpByGroupSize ? groupSize : 1.0F;
			const auto cachedBaseMultiplier = g_hasSkillXpBaseMultiplier[index] ? g_cachedSkillXpBaseMultipliers[index] : data->useMult;
			const auto configuredMultiplier = settings.useCustomCachedSkillXpMultipliers ?
				settings.customCachedSkillXpMultipliers[index] :
				cachedBaseMultiplier;

			data->useMult = configuredMultiplier / groupSizeDivisor;
			appliedAnyMultiplier = true;
		}

		return appliedAnyMultiplier;
	}

	bool Install()
	{
		if (g_installed) {
			return true;
		}

		if (g_installAttempted) {
			return false;
		}

		g_installAttempted = true;

		if (!REL::Module::IsAE()) {
			SKSE::log::error("SkillGroups level XP hook currently supports only AE runtime");
			return false;
		}

		const REL::Relocation<std::uintptr_t> base{ kImproveLevelExpBySkillLevel };
		const auto patchAddress = base.address() + kImproveLevelExpBySkillLevelAeOffset;
		if (!MatchesExpectedBytes(patchAddress)) {
			SKSE::log::error("SkillGroups ImproveLevelExpBySkillLevel signature check failed at relocation {} + {:#x}", kImproveLevelExpBySkillLevel.id(), kImproveLevelExpBySkillLevelAeOffset);
			return false;
		}

		SKSE::AllocTrampoline(256);
		auto& trampoline = SKSE::GetTrampoline();
		const auto* wrapper = WriteAeWrapper(trampoline);
		WriteAbsoluteCall(patchAddress, wrapper);

		g_installed = true;
		SKSE::log::info("SkillGroups installed ImproveLevelExpBySkillLevel hook at relocation {} + {:#x}", kImproveLevelExpBySkillLevel.id(), kImproveLevelExpBySkillLevelAeOffset);
		return true;
	}
}
