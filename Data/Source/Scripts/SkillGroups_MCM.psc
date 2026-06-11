Scriptname SkillGroups_MCM extends MCM_ConfigBase

string Property MOD_SETTING_PREFIX = "" AutoReadOnly

string[] SkillKeys
string[] SkillLabels
string[] GroupKeys
string[] GroupLabels
string[] LogLevelOptions

int OID_Enabled
int OID_LogLevel
int OID_HookFailure
int OID_OpenLog
bool HookAvailable

int OID_CacheCharacterXp
int OID_AutoCacheCharacterXp
int OID_UseCustomCharacterXp
int OID_ApplyCustomCharacterXp
int OID_CharacterXpHeader
int[] OIDs_CustomCharacterXp
int[] OIDs_CharacterGroupScales
int[] OIDs_CharacterSkillScales

int OID_CacheSkillXp
int OID_UseCustomSkillXp
int OID_DivideSkillXpByGroupSize
int OID_AutoApplySkillXp
int OID_ApplySkillXp
int OID_SkillXpHeader
int[] OIDs_CustomSkillXp

int Function GetVersion()
	return 2
EndFunction

Event OnConfigInit()
	InitializeMenu()
EndEvent

Event OnConfigRegister()
	InitializeMenu()
EndEvent

Event OnConfigOpen()
	InitializeMenu()
EndEvent

Event OnVersionUpdate(int a_version)
	InitializeMenu()
EndEvent

Event OnPageReset(string a_page)
	InitializeMenu()
	SetCursorFillMode(TOP_TO_BOTTOM)

	if a_page == "" || a_page == "General"
		RenderGeneralPage()
	elseif a_page == "Level XP Configuration"
		RenderLevelXpPage()
	elseif a_page == "Skill XP Configuration"
		RenderSkillXpPage()
	endif
EndEvent

Event OnOptionHighlight(int a_option)
	if a_option == OID_Enabled
		SetInfoText("Controls whether Skill Groups gates character XP by skill group.")
	elseif a_option == OID_LogLevel
		SetInfoText("Controls SkillGroups.log verbosity.")
	elseif a_option == OID_HookFailure
		SetInfoText("The character XP hook could not be applied. Check SkillGroups.log for details.")
	elseif a_option == OID_OpenLog
		SetInfoText("Opens SkillGroups.log.")
	elseif a_option == OID_CacheCharacterXp
		SetInfoText("Reads each skill's contribution multiplier to character level from the game.")
	elseif a_option == OID_AutoCacheCharacterXp
		SetInfoText("Reads skill contribution multipliers during each skill rank-up. Use only if another mod changes those multipliers at runtime; this may have a performance impact.")
	elseif a_option == OID_UseCustomCharacterXp
		SetInfoText("Set other mods' multipliers to 1.0 for predictable results.")
	elseif a_option == OID_ApplyCustomCharacterXp
		SetInfoText("Applies the custom values below to the game.")
	elseif a_option == OID_CharacterXpHeader
		SetInfoText("Controls how much each skill contributes to character level.")
	elseif a_option == OID_CacheSkillXp
		SetInfoText("Reads each skill XP multiplier from the game.")
	elseif a_option == OID_UseCustomSkillXp
		SetInfoText("Set other mods' skill XP multipliers to 1.0 for predictable results.")
	elseif a_option == OID_DivideSkillXpByGroupSize
		SetInfoText("When applying skill XP, divides it by the number of skills in it's group.")
	elseif a_option == OID_AutoApplySkillXp
		SetInfoText("Applies cached skill XP multipliers during each skill rank-up. This can help if another mod rewrites skill XP multipliers at runtime; this may have a performance impact.")
	elseif a_option == OID_ApplySkillXp
		SetInfoText("Applies the configured skill XP multipliers into the game.")
	elseif FindOption(OIDs_CustomCharacterXp, a_option) >= 0
		SetInfoText("Custom character level contribution multiplier for this skill.")
	elseif FindOption(OIDs_CustomSkillXp, a_option) >= 0
		SetInfoText("Custom skill XP multiplier for this skill.")
	elseif FindOption(OIDs_CharacterGroupScales, a_option) >= 0
		SetInfoText("A multiplier applied to the character XP for XP from this group. (Final xp = this * (Sum of final Skill Mults)), future version groupXP will change depending on group XP behaviour")
	elseif FindOption(OIDs_CharacterSkillScales, a_option) >= 0
		SetInfoText("An additional multiplier to character XP for this skill. (Final Skill Mult = contributionMult * this)")
	else
		SetInfoText("")
	endif
EndEvent

Event OnOptionSelect(int a_option)
	if a_option == OID_Enabled
		if !HookAvailable
			MCM.SetModSettingBool("SkillGroups", "bEnabled:General", false)
			SetToggleOptionValue(OID_Enabled, false)
			Debug.Notification("Skill Groups hook unavailable. Check SkillGroups.log")
		else
			ToggleBool("bEnabled:General", OID_Enabled)
		endif
	elseif a_option == OID_OpenLog
		OpenLogFile()
	elseif a_option == OID_AutoCacheCharacterXp
		ToggleBool("bAutoCacheOnLevelXp:General", OID_AutoCacheCharacterXp)
	elseif a_option == OID_UseCustomCharacterXp
		ToggleBool("bUseCustomCachedPlayerXpMultipliers:General", OID_UseCustomCharacterXp)
		UpdateCustomCharacterXpFlags()
	elseif a_option == OID_CacheCharacterXp
		CacheCharacterXpMultipliers()
	elseif a_option == OID_ApplyCustomCharacterXp
		ApplyCustomCharacterXpMultipliers()
	elseif a_option == OID_CacheSkillXp
		CacheSkillXpMultipliers()
	elseif a_option == OID_UseCustomSkillXp
		ToggleBool("bUseCustomCachedSkillXpMultipliers:General", OID_UseCustomSkillXp)
		UpdateCustomSkillXpFlags()
	elseif a_option == OID_DivideSkillXpByGroupSize
		ToggleBool("bDivideSkillXpByGroupSize:General", OID_DivideSkillXpByGroupSize)
	elseif a_option == OID_AutoApplySkillXp
		ToggleBool("bAutoApplySkillXpOnLevelXp:General", OID_AutoApplySkillXp)
	elseif a_option == OID_ApplySkillXp
		ApplySkillXpMultipliers()
	endif
EndEvent

Event OnOptionDefault(int a_option)
	if a_option == OID_Enabled
		if HookAvailable
			SetBool("bEnabled:General", true, OID_Enabled)
		else
			SetBool("bEnabled:General", false, OID_Enabled)
		endif
	elseif a_option == OID_LogLevel
		SetInt("iLogLevel:General", 1, OID_LogLevel)
	elseif a_option == OID_AutoCacheCharacterXp
		SetBool("bAutoCacheOnLevelXp:General", false, OID_AutoCacheCharacterXp)
	elseif a_option == OID_UseCustomCharacterXp
		SetBool("bUseCustomCachedPlayerXpMultipliers:General", false, OID_UseCustomCharacterXp)
		UpdateCustomCharacterXpFlags()
	elseif a_option == OID_UseCustomSkillXp
		SetBool("bUseCustomCachedSkillXpMultipliers:General", false, OID_UseCustomSkillXp)
		UpdateCustomSkillXpFlags()
	elseif a_option == OID_DivideSkillXpByGroupSize
		SetBool("bDivideSkillXpByGroupSize:General", true, OID_DivideSkillXpByGroupSize)
	elseif a_option == OID_AutoApplySkillXp
		SetBool("bAutoApplySkillXpOnLevelXp:General", false, OID_AutoApplySkillXp)
	else
		ResetSliderOption(a_option)
	endif
EndEvent

Event OnOptionSliderOpen(int a_option)
	SetSliderDialogRange(0.0, 5.0)
	SetSliderDialogInterval(0.05)
	SetSliderDialogStartValue(GetSliderValueForOption(a_option))
	SetSliderDialogDefaultValue(GetDefaultSliderValueForOption(a_option))
EndEvent

Event OnOptionSliderAccept(int a_option, float a_value)
	string settingKey = GetSliderSettingForOption(a_option)
	if settingKey != ""
		MCM.SetModSettingFloat("SkillGroups", settingKey, a_value)
		SetSliderOptionValue(a_option, a_value, "{2}")
		RefreshNativeSettings()
	endif
EndEvent

Event OnOptionMenuOpen(int a_option)
	if a_option == OID_LogLevel
		SetMenuDialogOptions(LogLevelOptions)
		SetMenuDialogStartIndex(GetInt("iLogLevel:General"))
		SetMenuDialogDefaultIndex(1)
	endif
EndEvent

Event OnOptionMenuAccept(int a_option, int a_index)
	if a_option == OID_LogLevel
		MCM.SetModSettingInt("SkillGroups", "iLogLevel:General", a_index)
		SetMenuOptionValue(OID_LogLevel, LogLevelOptions[a_index])
		RefreshNativeSettings()
	endif
EndEvent

Function InitializeMenu()
	ModName = "SkillGroups"

	Pages = new string[3]
	Pages[0] = "General"
	Pages[1] = "Level XP Configuration"
	Pages[2] = "Skill XP Configuration"

	if SkillKeys == None || SkillKeys.Length != 18
		InitializeData()
	endif
EndFunction

Function InitializeData()
	SkillKeys = new string[18]
	SkillLabels = new string[18]

	SkillKeys[0] = "OneHanded"
	SkillLabels[0] = "One-Handed"
	SkillKeys[1] = "TwoHanded"
	SkillLabels[1] = "Two-Handed"
	SkillKeys[2] = "Archery"
	SkillLabels[2] = "Archery"
	SkillKeys[3] = "Block"
	SkillLabels[3] = "Block"
	SkillKeys[4] = "Smithing"
	SkillLabels[4] = "Smithing"
	SkillKeys[5] = "HeavyArmor"
	SkillLabels[5] = "Heavy Armor"
	SkillKeys[6] = "LightArmor"
	SkillLabels[6] = "Light Armor"
	SkillKeys[7] = "Pickpocket"
	SkillLabels[7] = "Pickpocket"
	SkillKeys[8] = "Lockpicking"
	SkillLabels[8] = "Lockpicking"
	SkillKeys[9] = "Sneak"
	SkillLabels[9] = "Sneak"
	SkillKeys[10] = "Alchemy"
	SkillLabels[10] = "Alchemy"
	SkillKeys[11] = "Speech"
	SkillLabels[11] = "Speech"
	SkillKeys[12] = "Alteration"
	SkillLabels[12] = "Alteration"
	SkillKeys[13] = "Conjuration"
	SkillLabels[13] = "Conjuration"
	SkillKeys[14] = "Destruction"
	SkillLabels[14] = "Destruction"
	SkillKeys[15] = "Illusion"
	SkillLabels[15] = "Illusion"
	SkillKeys[16] = "Restoration"
	SkillLabels[16] = "Restoration"
	SkillKeys[17] = "Enchanting"
	SkillLabels[17] = "Enchanting"

	GroupKeys = new string[7]
	GroupLabels = new string[7]
	GroupKeys[0] = "Crafting"
	GroupLabels[0] = "Crafting"
	GroupKeys[1] = "Command"
	GroupLabels[1] = "Command"
	GroupKeys[2] = "Support"
	GroupLabels[2] = "Support"
	GroupKeys[3] = "Defence"
	GroupLabels[3] = "Defence"
	GroupKeys[4] = "Ranged"
	GroupLabels[4] = "Ranged"
	GroupKeys[5] = "Wealth"
	GroupLabels[5] = "Wealth"
	GroupKeys[6] = "Melee"
	GroupLabels[6] = "Melee"

	LogLevelOptions = new string[3]
	LogLevelOptions[0] = "Error"
	LogLevelOptions[1] = "Info"
	LogLevelOptions[2] = "Debug"

	OIDs_CustomCharacterXp = new int[18]
	OIDs_CharacterGroupScales = new int[7]
	OIDs_CharacterSkillScales = new int[18]
	OIDs_CustomSkillXp = new int[18]
EndFunction

Function RenderGeneralPage()
	RefreshRuntimeState()
	SetTitleText("Skill Groups")
	AddHeaderOption("Runtime")
	int enabledFlags = OPTION_FLAG_NONE
	if !HookAvailable
		enabledFlags = OPTION_FLAG_DISABLED
	endif

	OID_Enabled = AddToggleOption("Enabled", GetBool("bEnabled:General"), enabledFlags)
	OID_LogLevel = AddMenuOption("Log level", LogLevelOptions[GetInt("iLogLevel:General")])

	if !HookAvailable
		AddHeaderOption("Status")
		OID_HookFailure = AddTextOption("Hook could not be applied", "", OPTION_FLAG_DISABLED)
		OID_OpenLog = AddTextOption("CHECK SKILLGROUPS.LOG", "")
	endif
EndFunction

Function RenderLevelXpPage()
	SetTitleText("Character XP Configuration")
	AddHeaderOption("Character XP Group and Skill Multipliers")
	RenderLevelXpGroup(6, 0, 1, 3)
	RenderLevelXpGroup(4, 2, 14, -1)
	RenderLevelXpGroup(3, 6, 5, -1)
	RenderLevelXpGroup(2, 16, 12, 9)
	RenderLevelXpGroup(1, 15, 13, -1)
	RenderLevelXpGroup(5, 11, 7, 8)
	RenderLevelXpGroup(0, 4, 17, 10)

	SetCursorPosition(1)

	AddHeaderOption("Cache")
	OID_CacheCharacterXp = AddTextOption("Cache character XP multipliers", "")
	OID_ApplyCustomCharacterXp = AddTextOption("Apply custom character XP multipliers", "")
	OID_AutoCacheCharacterXp = AddToggleOption("Auto-cache on rank-up", GetBool("bAutoCacheOnLevelXp:General"))
	OID_UseCustomCharacterXp = AddToggleOption("Use custom character XP multipliers", GetBool("bUseCustomCachedPlayerXpMultipliers:General"))

	OID_CharacterXpHeader = AddHeaderOption("Character Level Contribution Multiplier")
	AddCustomCharacterXpSlider(0)
	AddCustomCharacterXpSlider(1)
	AddCustomCharacterXpSlider(2)
	AddCustomCharacterXpSlider(3)
	AddCustomCharacterXpSlider(4)
	AddCustomCharacterXpSlider(5)
	AddCustomCharacterXpSlider(6)
	AddCustomCharacterXpSlider(7)
	AddCustomCharacterXpSlider(8)
	AddCustomCharacterXpSlider(9)
	AddCustomCharacterXpSlider(10)
	AddCustomCharacterXpSlider(11)
	AddCustomCharacterXpSlider(12)
	AddCustomCharacterXpSlider(13)
	AddCustomCharacterXpSlider(14)
	AddCustomCharacterXpSlider(15)
	AddCustomCharacterXpSlider(16)
	AddCustomCharacterXpSlider(17)
EndFunction

Function RenderSkillXpPage()
	SetTitleText("Skill XP Configuration")
	AddHeaderOption("Cache")
	OID_CacheSkillXp = AddTextOption("Cache skill XP multipliers", "")
	OID_ApplySkillXp = AddTextOption("Apply skill XP multipliers", "")
	OID_UseCustomSkillXp = AddToggleOption("Use custom skill XP multipliers", GetBool("bUseCustomCachedSkillXpMultipliers:General"))
	OID_DivideSkillXpByGroupSize = AddToggleOption("Divide by group size", GetBool("bDivideSkillXpByGroupSize:General"))
	OID_AutoApplySkillXp = AddToggleOption("Auto-apply on rank-up", GetBool("bAutoApplySkillXpOnLevelXp:General"))

	SetCursorPosition(1)

	OID_SkillXpHeader = AddHeaderOption("Skill XP Multiplier")
	AddCustomSkillXpSlider(0)
	AddCustomSkillXpSlider(1)
	AddCustomSkillXpSlider(2)
	AddCustomSkillXpSlider(3)
	AddCustomSkillXpSlider(4)
	AddCustomSkillXpSlider(5)
	AddCustomSkillXpSlider(6)
	AddCustomSkillXpSlider(7)
	AddCustomSkillXpSlider(8)
	AddCustomSkillXpSlider(9)
	AddCustomSkillXpSlider(10)
	AddCustomSkillXpSlider(11)
	AddCustomSkillXpSlider(12)
	AddCustomSkillXpSlider(13)
	AddCustomSkillXpSlider(14)
	AddCustomSkillXpSlider(15)
	AddCustomSkillXpSlider(16)
	AddCustomSkillXpSlider(17)

EndFunction

Function AddCustomCharacterXpSlider(int a_skillIndex)
	int flags = OPTION_FLAG_NONE
	if !GetBool("bUseCustomCachedPlayerXpMultipliers:General")
		flags = OPTION_FLAG_DISABLED
	endif
	OIDs_CustomCharacterXp[a_skillIndex] = AddSliderOption(SkillLabels[a_skillIndex], GetFloat("f" + SkillKeys[a_skillIndex] + ":CustomCachedPlayerXpMultipliers"), "{2}", flags)
EndFunction

Function AddCustomSkillXpSlider(int a_skillIndex)
	int flags = OPTION_FLAG_NONE
	if !GetBool("bUseCustomCachedSkillXpMultipliers:General")
		flags = OPTION_FLAG_DISABLED
	endif
	OIDs_CustomSkillXp[a_skillIndex] = AddSliderOption(SkillLabels[a_skillIndex], GetFloat("f" + SkillKeys[a_skillIndex] + ":CustomCachedSkillXpMultipliers"), "{2}", flags)
EndFunction

Function RenderLevelXpGroup(int a_groupIndex, int a_skillA, int a_skillB, int a_skillC)
	AddHeaderOption(" - " + GroupLabels[a_groupIndex] + " - ")
	OIDs_CharacterGroupScales[a_groupIndex] = AddSliderOption("Group", GetFloat("f" + GroupKeys[a_groupIndex] + ":GroupXpMultiplierScales"), "{2}")
	AddCharacterSkillScale(a_skillA)
	AddCharacterSkillScale(a_skillB)
	if a_skillC >= 0
		AddCharacterSkillScale(a_skillC)
	endif
EndFunction

Function AddCharacterSkillScale(int a_skillIndex)
	OIDs_CharacterSkillScales[a_skillIndex] = AddSliderOption(SkillLabels[a_skillIndex], GetFloat("f" + SkillKeys[a_skillIndex] + ":PlayerXpMultiplierScales"), "{2}")
EndFunction

Function ToggleBool(string a_settingKey, int a_option)
	bool value = !GetBool(a_settingKey)
	SetBool(a_settingKey, value, a_option)
EndFunction

Function SetBool(string a_settingKey, bool a_value, int a_option)
	MCM.SetModSettingBool("SkillGroups", a_settingKey, a_value)
	SetToggleOptionValue(a_option, a_value)
	RefreshNativeSettings()
EndFunction

Function SetInt(string a_settingKey, int a_value, int a_option)
	MCM.SetModSettingInt("SkillGroups", a_settingKey, a_value)
	SetMenuOptionValue(a_option, LogLevelOptions[a_value])
	RefreshNativeSettings()
EndFunction

Function RefreshRuntimeState()
	HookAvailable = SkillGroups_Native.RefreshSettings()
	if !HookAvailable
		MCM.SetModSettingBool("SkillGroups", "bEnabled:General", false)
	endif
EndFunction

Function RefreshNativeSettings()
	if !SkillGroups_Native.RefreshSettings()
		HookAvailable = false
		MCM.SetModSettingBool("SkillGroups", "bEnabled:General", false)
		if OID_Enabled > 0
			SetToggleOptionValue(OID_Enabled, false)
			SetOptionFlags(OID_Enabled, OPTION_FLAG_DISABLED)
		endif
	else
		HookAvailable = true
	endif
EndFunction

Function ResetSliderOption(int a_option)
	string settingKey = GetSliderSettingForOption(a_option)
	if settingKey == ""
		return
	endif

	float defaultValue = GetDefaultSliderValueForOption(a_option)
	MCM.SetModSettingFloat("SkillGroups", settingKey, defaultValue)
	SetSliderOptionValue(a_option, defaultValue, "{2}")
	RefreshNativeSettings()
EndFunction

Function UpdateCustomCharacterXpFlags()
	int flags = OPTION_FLAG_NONE
	if !GetBool("bUseCustomCachedPlayerXpMultipliers:General")
		flags = OPTION_FLAG_DISABLED
	endif

	int i = 0
	while i < OIDs_CustomCharacterXp.Length
		if OIDs_CustomCharacterXp[i] > 0
			SetOptionFlags(OIDs_CustomCharacterXp[i], flags)
		endif
		i += 1
	endwhile
EndFunction

Function UpdateCustomSkillXpFlags()
	int flags = OPTION_FLAG_NONE
	if !GetBool("bUseCustomCachedSkillXpMultipliers:General")
		flags = OPTION_FLAG_DISABLED
	endif

	int i = 0
	while i < OIDs_CustomSkillXp.Length
		if OIDs_CustomSkillXp[i] > 0
			SetOptionFlags(OIDs_CustomSkillXp[i], flags)
		endif
		i += 1
	endwhile
EndFunction

Function CacheCharacterXpMultipliers()
	if SkillGroups_Native.RecacheMultipliers()
		UpdateCachedCharacterXpSettings()
		Debug.Notification("Skill Groups character XP multipliers cached")
	else
		Debug.Notification("Skill Groups character XP multiplier cache failed")
	endif
EndFunction

Function ApplyCustomCharacterXpMultipliers()
	if SkillGroups_Native.ApplyCustomMultipliers()
		Debug.Notification("Skill Groups custom character XP multipliers applied")
	else
		Debug.Notification("Skill Groups custom character XP multiplier apply failed")
	endif
EndFunction

Function ApplySkillXpMultipliers()
	if SkillGroups_Native.ApplySkillXpMultipliers()
		Debug.Notification("Skill Groups skill XP multipliers applied")
	else
		Debug.Notification("Skill Groups skill XP multiplier apply failed")
	endif
EndFunction

Function CacheSkillXpMultipliers()
	if SkillGroups_Native.CacheSkillXpMultipliers()
		UpdateCachedSkillXpSettings()
		Debug.Notification("Skill Groups skill XP multipliers cached")
	else
		Debug.Notification("Skill Groups skill XP multiplier cache failed")
	endif
EndFunction

Function UpdateCachedCharacterXpSettings()
	int i = 0
	while i < SkillKeys.Length
		float value = SkillGroups_Native.GetCachedPlayerXpMultiplier(i)
		MCM.SetModSettingFloat("SkillGroups", "f" + SkillKeys[i] + ":CustomCachedPlayerXpMultipliers", value)
		if OIDs_CustomCharacterXp[i] > 0
			SetSliderOptionValue(OIDs_CustomCharacterXp[i], value, "{2}")
		endif
		i += 1
	endwhile
EndFunction

Function UpdateCachedSkillXpSettings()
	int i = 0
	while i < SkillKeys.Length
		float value = SkillGroups_Native.GetCachedSkillXpMultiplier(i)
		MCM.SetModSettingFloat("SkillGroups", "f" + SkillKeys[i] + ":CustomCachedSkillXpMultipliers", value)
		if OIDs_CustomSkillXp[i] > 0
			SetSliderOptionValue(OIDs_CustomSkillXp[i], value, "{2}")
		endif
		i += 1
	endwhile
EndFunction

Function OpenLogFile()
	if !SkillGroups_Native.OpenLogFile()
		Debug.Notification("Skill Groups could not open SkillGroups.log")
	endif
EndFunction

bool Function GetBool(string a_settingKey)
	return MCM.GetModSettingBool("SkillGroups", a_settingKey)
EndFunction

int Function GetInt(string a_settingKey)
	int value = MCM.GetModSettingInt("SkillGroups", a_settingKey)
	if value < 0
		return 0
	elseif value > 2
		return 2
	endif
	return value
EndFunction

float Function GetFloat(string a_settingKey)
	return MCM.GetModSettingFloat("SkillGroups", a_settingKey)
EndFunction

int Function FindOption(int[] a_options, int a_option)
	if a_options == None
		return -1
	endif

	int i = 0
	while i < a_options.Length
		if a_options[i] == a_option
			return i
		endif
		i += 1
	endwhile

	return -1
EndFunction

float Function GetSliderValueForOption(int a_option)
	string settingKey = GetSliderSettingForOption(a_option)
	if settingKey != ""
		return GetFloat(settingKey)
	endif
	return 1.0
EndFunction

float Function GetDefaultSliderValueForOption(int a_option)
	int index = FindOption(OIDs_CharacterGroupScales, a_option)
	if index == 0
		return 0.75
	elseif index == 1
		return 1.15
	elseif index == 2
		return 0.75
	elseif index == 3
		return 1.4
	elseif index == 4
		return 1.4
	elseif index == 5
		return 0.15
	elseif index == 6
		return 1.4
	endif

	return 1.0
EndFunction

string Function GetSliderSettingForOption(int a_option)
	int index = FindOption(OIDs_CustomCharacterXp, a_option)
	if index >= 0
		return "f" + SkillKeys[index] + ":CustomCachedPlayerXpMultipliers"
	endif

	index = FindOption(OIDs_CustomSkillXp, a_option)
	if index >= 0
		return "f" + SkillKeys[index] + ":CustomCachedSkillXpMultipliers"
	endif

	index = FindOption(OIDs_CharacterGroupScales, a_option)
	if index >= 0
		return "f" + GroupKeys[index] + ":GroupXpMultiplierScales"
	endif

	index = FindOption(OIDs_CharacterSkillScales, a_option)
	if index >= 0
		return "f" + SkillKeys[index] + ":PlayerXpMultiplierScales"
	endif

	return ""
EndFunction
