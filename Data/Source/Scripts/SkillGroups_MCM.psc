Scriptname SkillGroups_MCM extends MCM_ConfigBase

string Property MOD_SETTING_PREFIX = "" AutoReadOnly

string[] SkillLabels
string[] GroupLabels
string[] LogLevelOptions
string[] SkillXpProfileOptions
float[] CachedCharacterXpValues
float[] CachedCharacterGroupScaleValues
float[] CachedCharacterSkillScaleValues
float[] CachedSkillXpValues
bool CachedCharacterProfileEditable
bool CachedSkillProfileEditable
bool CachedUseFlatCharacterXp
float CachedFlatCharacterXp
float CachedLevelUpBase
float CachedLevelUpMult
int CachedCharacterProfileIndex
int CachedSkillProfileIndex
bool CachedSkillDivisor
bool SkillXpProfileOptionsLoaded

int OID_Enabled
int OID_LogLevel
int OID_MasterProfile
int OID_HookFailure
int OID_OpenLog
bool HookAvailable

int OID_CharacterXpProfile
int OID_CacheCharacterXp
int OID_ApplyCharacterXp
int OID_ResyncLevelThreshold
int OID_ImportCharacterXpPage
int OID_UseFlatCharacterXp
int OID_FlatCharacterXp
int OID_LevelUpBase
int OID_LevelUpMult
int OID_CharacterXpHeader
int[] OIDs_CustomCharacterXp
int[] OIDs_CharacterGroupScales
int[] OIDs_CharacterSkillScales

int OID_CacheSkillXp
int OID_SkillXpProfile
int OID_ImportSkillXpPage
int OID_DivideSkillXpByGroupSize
int OID_AutoApplySkillXp
int OID_ApplySkillXp
int OID_SkillXpHeader
int[] OIDs_CustomSkillXp

int Function GetVersion()
	return 9
EndFunction

Event OnConfigInit()
EndEvent

Event OnConfigRegister()
	InitializeMenu()
EndEvent

Event OnConfigOpen()
EndEvent

Event OnVersionUpdate(int a_version)
	InitializeMenu()
EndEvent

Event OnPageReset(string a_page)
	SetCursorFillMode(TOP_TO_BOTTOM)

	if a_page == "" || a_page == "$SkillGroups_Page_General" || a_page == "General"
		RenderGeneralPage()
	elseif a_page == "$SkillGroups_Page_CharacterXp" || a_page == "Level XP Configuration"
		RenderLevelXpPage()
	elseif a_page == "$SkillGroups_Page_SkillXp" || a_page == "Skill XP Configuration"
		RenderSkillXpPage()
	endif
EndEvent

Event OnOptionHighlight(int a_option)
	if a_option == OID_Enabled
		SetInfoText("Controls whether Skill Groups gates character XP by skill group.")
	elseif a_option == OID_LogLevel
		SetInfoText("Controls SkillGroups.log verbosity.")
	elseif a_option == OID_MasterProfile
		SetInfoText("Sets a master profile for all settings. Other profiles can still be set if the master profile is marked as editable")
	elseif a_option == OID_HookFailure
		SetInfoText("The character XP hook could not be applied. Check SkillGroups.log for details.")
	elseif a_option == OID_OpenLog
		SetInfoText("Opens SkillGroups.log.")
	elseif a_option == OID_CacheCharacterXp
		SetInfoText("Gets the current character level base and threshold increase values currently used by the game. Does not automatically apply the values to the selected profile.")
	elseif a_option == OID_ApplyCharacterXp
		SetInfoText("Applies values and saves them to the selected profile.")
	elseif a_option == OID_ResyncLevelThreshold
		SetInfoText("Recalculates the required player XP for your current level and gives you any levels you should have if your xp is higher than the new threshold.")
	elseif a_option == OID_ImportCharacterXpPage
		SetInfoText("Copies character XP settings from another profile. Does not automatically apply the values to the selected profile.")
	elseif a_option == OID_UseFlatCharacterXp
		SetInfoText("When enabled, skill rank-ups use a flat character XP amount. Skill Groups multipliers still apply.")
	elseif a_option == OID_FlatCharacterXp
		SetInfoText("Flat character XP awarded by a contributing skill rank-up. Skill Groups multipliers still apply.")
	elseif a_option == OID_LevelUpBase
		SetInfoText("Base character XP required to level up. Total XP = LeveLUpBase + (LevelUpMult * Level)")
	elseif a_option == OID_LevelUpMult
		SetInfoText("Additional character XP required to level up. Added to LeveLUpBase every level. Total XP = LeveLUpBase + (LevelUpMult * Level)")
	elseif a_option == OID_CharacterXpHeader
		SetInfoText("Controls how much each skill contributes to character level.")
	elseif a_option == OID_CacheSkillXp
		SetInfoText("Gets each skill XP multiplier currently used by the game. Does not automatically apply the values to the selected profile.")
	elseif a_option == OID_ImportSkillXpPage
		SetInfoText("Copies skill XP multipliers from another profile. Does not automatically apply the values to the selected profile.")
	elseif a_option == OID_DivideSkillXpByGroupSize
		SetInfoText("Divides skill XP by the number of skills in its group.")
	elseif a_option == OID_AutoApplySkillXp
		SetInfoText("Applies cached skill XP multipliers during each skill rank-up. This can help if another mod rewrites skill XP multipliers at runtime; this may have a performance impact.")
	elseif a_option == OID_ApplySkillXp
		SetInfoText("Applies the configured skill XP multipliers to the game.")
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
	elseif a_option == OID_CacheSkillXp
		if IsSelectedSkillXpProfileEditable()
			CacheSkillXpMultipliers()
		endif
	elseif a_option == OID_CacheCharacterXp
		if IsSelectedCharacterXpProfileEditable()
			CacheCharacterXpSettings()
		endif
	elseif a_option == OID_DivideSkillXpByGroupSize
		if IsSelectedSkillXpProfileEditable()
			ToggleBool("bDivideSkillXpByGroupSize:General", OID_DivideSkillXpByGroupSize)
			InvalidateSkillXpPageState()
			UpdateCustomSkillXpFlags()
		endif
	elseif a_option == OID_AutoApplySkillXp
		ToggleBool("bAutoApplySkillXpOnLevelXp:General", OID_AutoApplySkillXp)
	elseif a_option == OID_ApplyCharacterXp
		ApplyCharacterXpProfiles()
	elseif a_option == OID_ResyncLevelThreshold
		ResyncCurrentLevelThreshold()
	elseif a_option == OID_UseFlatCharacterXp
		ToggleProfileUseFlatCharacterXp()
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
	elseif a_option == OID_MasterProfile
		SetMasterProfile(GetDefaultMasterProfile())
	elseif a_option == OID_CharacterXpProfile
		SetCharacterXpProfile(0)
		UpdateCustomCharacterXpFlags()
	elseif a_option == OID_UseFlatCharacterXp
		SetProfileCharacterXpSettings(false, GetProfileFlatCharacterXp(), GetProfileLevelUpBase(), GetProfileLevelUpMult())
	elseif a_option == OID_FlatCharacterXp
		SetProfileCharacterXpSettings(GetProfileUseFlatCharacterXp(), 10.0, GetProfileLevelUpBase(), GetProfileLevelUpMult())
	elseif a_option == OID_LevelUpBase
		SetProfileCharacterXpSettings(GetProfileUseFlatCharacterXp(), GetProfileFlatCharacterXp(), 75.0, GetProfileLevelUpMult())
	elseif a_option == OID_LevelUpMult
		SetProfileCharacterXpSettings(GetProfileUseFlatCharacterXp(), GetProfileFlatCharacterXp(), GetProfileLevelUpBase(), 25.0)
	elseif a_option == OID_SkillXpProfile
		SetSkillXpProfile(0)
		UpdateCustomSkillXpFlags()
	elseif a_option == OID_DivideSkillXpByGroupSize
		SetBool("bDivideSkillXpByGroupSize:General", true, OID_DivideSkillXpByGroupSize)
		InvalidateSkillXpPageState()
		UpdateCustomSkillXpFlags()
	elseif a_option == OID_AutoApplySkillXp
		SetBool("bAutoApplySkillXpOnLevelXp:General", false, OID_AutoApplySkillXp)
	else
		ResetSliderOption(a_option)
	endif
EndEvent

Event OnOptionSliderOpen(int a_option)
	if a_option == OID_LevelUpBase
		SetSliderDialogRange(0.0, 10000.0)
		SetSliderDialogInterval(1.0)
	elseif a_option == OID_FlatCharacterXp || a_option == OID_LevelUpMult
		SetSliderDialogRange(0.0, 1000.0)
		SetSliderDialogInterval(1.0)
	elseif FindOption(OIDs_CustomSkillXp, a_option) >= 0
		SetSliderDialogRange(0.0, 1000.0)
		SetSliderDialogInterval(0.05)
	else
		SetSliderDialogRange(0.0, 5.0)
		SetSliderDialogInterval(0.05)
	endif
	SetSliderDialogStartValue(GetSliderValueForOption(a_option))
	SetSliderDialogDefaultValue(GetDefaultSliderValueForOption(a_option))
EndEvent

Event OnOptionSliderAccept(int a_option, float a_value)
	if a_option == OID_FlatCharacterXp
		SetProfileCharacterXpSettings(GetProfileUseFlatCharacterXp(), a_value, GetProfileLevelUpBase(), GetProfileLevelUpMult())
		return
	elseif a_option == OID_LevelUpBase
		SetProfileCharacterXpSettings(GetProfileUseFlatCharacterXp(), GetProfileFlatCharacterXp(), a_value, GetProfileLevelUpMult())
		return
	elseif a_option == OID_LevelUpMult
		SetProfileCharacterXpSettings(GetProfileUseFlatCharacterXp(), GetProfileFlatCharacterXp(), GetProfileLevelUpBase(), a_value)
		return
	endif

	int characterXpIndex = FindOption(OIDs_CustomCharacterXp, a_option)
	if characterXpIndex >= 0
		if IsSelectedCharacterXpProfileEditable() && SkillGroups_Native.SetCharacterXpProfileMultiplier(GetEffectiveCharacterXpProfile(), characterXpIndex, a_value)
			CachedCharacterXpValues[characterXpIndex] = a_value
			SetSliderOptionValue(a_option, a_value, "{2}")
		endif
		return
	endif

	int groupScaleIndex = FindOption(OIDs_CharacterGroupScales, a_option)
	if groupScaleIndex >= 0
		if IsSelectedCharacterXpProfileEditable() && SkillGroups_Native.SetGroupXpProfileMultiplierScale(GetEffectiveCharacterXpProfile(), groupScaleIndex, a_value)
			CachedCharacterGroupScaleValues[groupScaleIndex] = a_value
			SetSliderOptionValue(a_option, a_value, "{2}")
		endif
		return
	endif

	int skillScaleIndex = FindOption(OIDs_CharacterSkillScales, a_option)
	if skillScaleIndex >= 0
		if IsSelectedCharacterXpProfileEditable() && SkillGroups_Native.SetPlayerXpProfileMultiplierScale(GetEffectiveCharacterXpProfile(), skillScaleIndex, a_value)
			CachedCharacterSkillScaleValues[skillScaleIndex] = a_value
			SetSliderOptionValue(a_option, a_value, "{2}")
		endif
		return
	endif

	int skillXpIndex = FindOption(OIDs_CustomSkillXp, a_option)
	if skillXpIndex >= 0
		float rawValue = a_value * GetSkillXpDisplayDivisor(skillXpIndex)
		if IsSelectedSkillXpProfileEditable() && SkillGroups_Native.SetSkillXpProfileMultiplier(GetEffectiveSkillXpProfile(), skillXpIndex, rawValue)
			CachedSkillXpValues[skillXpIndex] = a_value
			SetSliderOptionValue(a_option, a_value, "{2}")
		endif
		return
	endif

EndEvent

Event OnOptionMenuOpen(int a_option)
	if a_option == OID_LogLevel
		SetMenuDialogOptions(LogLevelOptions)
		SetMenuDialogStartIndex(GetInt("iLogLevel:General"))
		SetMenuDialogDefaultIndex(1)
	elseif a_option == OID_MasterProfile
		SetMenuDialogOptions(SkillXpProfileOptions)
		SetMenuDialogStartIndex(GetMasterProfile())
		SetMenuDialogDefaultIndex(GetDefaultMasterProfile())
	elseif a_option == OID_CharacterXpProfile
		SetMenuDialogOptions(SkillXpProfileOptions)
		SetMenuDialogStartIndex(GetCharacterXpProfile())
		SetMenuDialogDefaultIndex(GetDefaultMasterProfile())
	elseif a_option == OID_ImportCharacterXpPage
		SetMenuDialogOptions(SkillXpProfileOptions)
		SetMenuDialogStartIndex(GetEffectiveCharacterXpProfile())
		SetMenuDialogDefaultIndex(0)
	elseif a_option == OID_SkillXpProfile
		SetMenuDialogOptions(SkillXpProfileOptions)
		SetMenuDialogStartIndex(GetSkillXpProfile())
		SetMenuDialogDefaultIndex(0)
	elseif a_option == OID_ImportSkillXpPage
		SetMenuDialogOptions(SkillXpProfileOptions)
		SetMenuDialogStartIndex(GetEffectiveSkillXpProfile())
		SetMenuDialogDefaultIndex(0)
	endif
EndEvent

Event OnOptionMenuAccept(int a_option, int a_index)
	if a_option == OID_LogLevel
		MCM.SetModSettingInt("SkillGroups", "iLogLevel:General", a_index)
		SetMenuOptionValue(OID_LogLevel, LogLevelOptions[a_index])
		RefreshNativeSettings()
	elseif a_option == OID_MasterProfile
		SetMasterProfile(a_index)
		ShowFinishedMessage("Skill Groups finished loading profile.")
	elseif a_option == OID_CharacterXpProfile
		SetCharacterXpProfile(a_index)
		UpdateCustomCharacterXpFlags()
		ShowFinishedMessage("Skill Groups finished loading profile.")
	elseif a_option == OID_ImportCharacterXpPage
		ImportCharacterXpPageFromProfile(a_index)
	elseif a_option == OID_SkillXpProfile
		SetSkillXpProfile(a_index)
		UpdateCustomSkillXpFlags()
		ShowFinishedMessage("Skill Groups finished loading profile.")
	elseif a_option == OID_ImportSkillXpPage
		ImportSkillXpPageFromProfile(a_index)
	endif
EndEvent

Function InitializeMenu()
	ModName = "SkillGroups"

	Pages = new string[3]
	Pages[0] = "$SkillGroups_Page_General"
	Pages[1] = "$SkillGroups_Page_CharacterXp"
	Pages[2] = "$SkillGroups_Page_SkillXp"

	InitializeData()
EndFunction

Function InitializeData()
	SkillLabels = new string[18]

	SkillLabels[0] = "$SkillGroups_Skill_OneHanded"
	SkillLabels[1] = "$SkillGroups_Skill_TwoHanded"
	SkillLabels[2] = "$SkillGroups_Skill_Archery"
	SkillLabels[3] = "$SkillGroups_Skill_Block"
	SkillLabels[4] = "$SkillGroups_Skill_Smithing"
	SkillLabels[5] = "$SkillGroups_Skill_HeavyArmor"
	SkillLabels[6] = "$SkillGroups_Skill_LightArmor"
	SkillLabels[7] = "$SkillGroups_Skill_Pickpocket"
	SkillLabels[8] = "$SkillGroups_Skill_Lockpicking"
	SkillLabels[9] = "$SkillGroups_Skill_Sneak"
	SkillLabels[10] = "$SkillGroups_Skill_Alchemy"
	SkillLabels[11] = "$SkillGroups_Skill_Speech"
	SkillLabels[12] = "$SkillGroups_Skill_Alteration"
	SkillLabels[13] = "$SkillGroups_Skill_Conjuration"
	SkillLabels[14] = "$SkillGroups_Skill_Destruction"
	SkillLabels[15] = "$SkillGroups_Skill_Illusion"
	SkillLabels[16] = "$SkillGroups_Skill_Restoration"
	SkillLabels[17] = "$SkillGroups_Skill_Enchanting"

	GroupLabels = new string[7]
	GroupLabels[0] = "$SkillGroups_Group_Crafting"
	GroupLabels[1] = "$SkillGroups_Group_Control"
	GroupLabels[2] = "$SkillGroups_Group_Support"
	GroupLabels[3] = "$SkillGroups_Group_Defence"
	GroupLabels[4] = "$SkillGroups_Group_Ranged"
	GroupLabels[5] = "$SkillGroups_Group_Utility"
	GroupLabels[6] = "$SkillGroups_Group_Melee"

	LogLevelOptions = new string[3]
	LogLevelOptions[0] = "$SkillGroups_Log_Error"
	LogLevelOptions[1] = "$SkillGroups_Log_Info"
	LogLevelOptions[2] = "$SkillGroups_Log_Debug"

	OIDs_CustomCharacterXp = new int[18]
	OIDs_CharacterGroupScales = new int[7]
	OIDs_CharacterSkillScales = new int[18]
	OIDs_CustomSkillXp = new int[18]
	CachedCharacterXpValues = new float[18]
	CachedCharacterGroupScaleValues = new float[7]
	CachedCharacterSkillScaleValues = new float[18]
	CachedSkillXpValues = new float[18]
	CachedCharacterProfileIndex = -1
	CachedSkillProfileIndex = -1
	SkillXpProfileOptionsLoaded = false
EndFunction

Function RefreshSkillXpProfileOptions()
	if SkillXpProfileOptionsLoaded
		return
	endif

	int count = SkillGroups_Native.GetSkillXpProfileCount()
	if count < 1
		count = 1
	elseif count > 128
		count = 128
	endif

	CreateSkillXpProfileOptions(count)
	int i = 0
	while i < count
		SkillXpProfileOptions[i] = SkillGroups_Native.GetSkillXpProfileName(i)
		if SkillXpProfileOptions[i] == ""
			SkillXpProfileOptions[i] = "$SkillGroups_Profile_Default"
		endif
		i += 1
	endwhile
	SkillXpProfileOptionsLoaded = true
EndFunction

Function CreateSkillXpProfileOptions(int a_count)
	if a_count <= 1
		SkillXpProfileOptions = new string[1]
	elseif a_count <= 2
		SkillXpProfileOptions = new string[2]
	elseif a_count <= 3
		SkillXpProfileOptions = new string[3]
	elseif a_count <= 4
		SkillXpProfileOptions = new string[4]
	elseif a_count <= 5
		SkillXpProfileOptions = new string[5]
	elseif a_count <= 6
		SkillXpProfileOptions = new string[6]
	elseif a_count <= 7
		SkillXpProfileOptions = new string[7]
	elseif a_count <= 8
		SkillXpProfileOptions = new string[8]
	elseif a_count <= 9
		SkillXpProfileOptions = new string[9]
	elseif a_count <= 10
		SkillXpProfileOptions = new string[10]
	elseif a_count <= 11
		SkillXpProfileOptions = new string[11]
	elseif a_count <= 12
		SkillXpProfileOptions = new string[12]
	elseif a_count <= 13
		SkillXpProfileOptions = new string[13]
	elseif a_count <= 14
		SkillXpProfileOptions = new string[14]
	elseif a_count <= 15
		SkillXpProfileOptions = new string[15]
	elseif a_count <= 16
		SkillXpProfileOptions = new string[16]
	elseif a_count <= 17
		SkillXpProfileOptions = new string[17]
	elseif a_count <= 18
		SkillXpProfileOptions = new string[18]
	elseif a_count <= 19
		SkillXpProfileOptions = new string[19]
	elseif a_count <= 20
		SkillXpProfileOptions = new string[20]
	elseif a_count <= 21
		SkillXpProfileOptions = new string[21]
	elseif a_count <= 22
		SkillXpProfileOptions = new string[22]
	elseif a_count <= 23
		SkillXpProfileOptions = new string[23]
	elseif a_count <= 24
		SkillXpProfileOptions = new string[24]
	elseif a_count <= 25
		SkillXpProfileOptions = new string[25]
	elseif a_count <= 26
		SkillXpProfileOptions = new string[26]
	elseif a_count <= 27
		SkillXpProfileOptions = new string[27]
	elseif a_count <= 28
		SkillXpProfileOptions = new string[28]
	elseif a_count <= 29
		SkillXpProfileOptions = new string[29]
	elseif a_count <= 30
		SkillXpProfileOptions = new string[30]
	elseif a_count <= 31
		SkillXpProfileOptions = new string[31]
	elseif a_count <= 32
		SkillXpProfileOptions = new string[32]
	elseif a_count <= 33
		SkillXpProfileOptions = new string[33]
	elseif a_count <= 34
		SkillXpProfileOptions = new string[34]
	elseif a_count <= 35
		SkillXpProfileOptions = new string[35]
	elseif a_count <= 36
		SkillXpProfileOptions = new string[36]
	elseif a_count <= 37
		SkillXpProfileOptions = new string[37]
	elseif a_count <= 38
		SkillXpProfileOptions = new string[38]
	elseif a_count <= 39
		SkillXpProfileOptions = new string[39]
	elseif a_count <= 40
		SkillXpProfileOptions = new string[40]
	elseif a_count <= 41
		SkillXpProfileOptions = new string[41]
	elseif a_count <= 42
		SkillXpProfileOptions = new string[42]
	elseif a_count <= 43
		SkillXpProfileOptions = new string[43]
	elseif a_count <= 44
		SkillXpProfileOptions = new string[44]
	elseif a_count <= 45
		SkillXpProfileOptions = new string[45]
	elseif a_count <= 46
		SkillXpProfileOptions = new string[46]
	elseif a_count <= 47
		SkillXpProfileOptions = new string[47]
	elseif a_count <= 48
		SkillXpProfileOptions = new string[48]
	elseif a_count <= 49
		SkillXpProfileOptions = new string[49]
	elseif a_count <= 50
		SkillXpProfileOptions = new string[50]
	elseif a_count <= 51
		SkillXpProfileOptions = new string[51]
	elseif a_count <= 52
		SkillXpProfileOptions = new string[52]
	elseif a_count <= 53
		SkillXpProfileOptions = new string[53]
	elseif a_count <= 54
		SkillXpProfileOptions = new string[54]
	elseif a_count <= 55
		SkillXpProfileOptions = new string[55]
	elseif a_count <= 56
		SkillXpProfileOptions = new string[56]
	elseif a_count <= 57
		SkillXpProfileOptions = new string[57]
	elseif a_count <= 58
		SkillXpProfileOptions = new string[58]
	elseif a_count <= 59
		SkillXpProfileOptions = new string[59]
	elseif a_count <= 60
		SkillXpProfileOptions = new string[60]
	elseif a_count <= 61
		SkillXpProfileOptions = new string[61]
	elseif a_count <= 62
		SkillXpProfileOptions = new string[62]
	elseif a_count <= 63
		SkillXpProfileOptions = new string[63]
	elseif a_count <= 64
		SkillXpProfileOptions = new string[64]
	elseif a_count <= 65
		SkillXpProfileOptions = new string[65]
	elseif a_count <= 66
		SkillXpProfileOptions = new string[66]
	elseif a_count <= 67
		SkillXpProfileOptions = new string[67]
	elseif a_count <= 68
		SkillXpProfileOptions = new string[68]
	elseif a_count <= 69
		SkillXpProfileOptions = new string[69]
	elseif a_count <= 70
		SkillXpProfileOptions = new string[70]
	elseif a_count <= 71
		SkillXpProfileOptions = new string[71]
	elseif a_count <= 72
		SkillXpProfileOptions = new string[72]
	elseif a_count <= 73
		SkillXpProfileOptions = new string[73]
	elseif a_count <= 74
		SkillXpProfileOptions = new string[74]
	elseif a_count <= 75
		SkillXpProfileOptions = new string[75]
	elseif a_count <= 76
		SkillXpProfileOptions = new string[76]
	elseif a_count <= 77
		SkillXpProfileOptions = new string[77]
	elseif a_count <= 78
		SkillXpProfileOptions = new string[78]
	elseif a_count <= 79
		SkillXpProfileOptions = new string[79]
	elseif a_count <= 80
		SkillXpProfileOptions = new string[80]
	elseif a_count <= 81
		SkillXpProfileOptions = new string[81]
	elseif a_count <= 82
		SkillXpProfileOptions = new string[82]
	elseif a_count <= 83
		SkillXpProfileOptions = new string[83]
	elseif a_count <= 84
		SkillXpProfileOptions = new string[84]
	elseif a_count <= 85
		SkillXpProfileOptions = new string[85]
	elseif a_count <= 86
		SkillXpProfileOptions = new string[86]
	elseif a_count <= 87
		SkillXpProfileOptions = new string[87]
	elseif a_count <= 88
		SkillXpProfileOptions = new string[88]
	elseif a_count <= 89
		SkillXpProfileOptions = new string[89]
	elseif a_count <= 90
		SkillXpProfileOptions = new string[90]
	elseif a_count <= 91
		SkillXpProfileOptions = new string[91]
	elseif a_count <= 92
		SkillXpProfileOptions = new string[92]
	elseif a_count <= 93
		SkillXpProfileOptions = new string[93]
	elseif a_count <= 94
		SkillXpProfileOptions = new string[94]
	elseif a_count <= 95
		SkillXpProfileOptions = new string[95]
	elseif a_count <= 96
		SkillXpProfileOptions = new string[96]
	elseif a_count <= 97
		SkillXpProfileOptions = new string[97]
	elseif a_count <= 98
		SkillXpProfileOptions = new string[98]
	elseif a_count <= 99
		SkillXpProfileOptions = new string[99]
	elseif a_count <= 100
		SkillXpProfileOptions = new string[100]
	elseif a_count <= 101
		SkillXpProfileOptions = new string[101]
	elseif a_count <= 102
		SkillXpProfileOptions = new string[102]
	elseif a_count <= 103
		SkillXpProfileOptions = new string[103]
	elseif a_count <= 104
		SkillXpProfileOptions = new string[104]
	elseif a_count <= 105
		SkillXpProfileOptions = new string[105]
	elseif a_count <= 106
		SkillXpProfileOptions = new string[106]
	elseif a_count <= 107
		SkillXpProfileOptions = new string[107]
	elseif a_count <= 108
		SkillXpProfileOptions = new string[108]
	elseif a_count <= 109
		SkillXpProfileOptions = new string[109]
	elseif a_count <= 110
		SkillXpProfileOptions = new string[110]
	elseif a_count <= 111
		SkillXpProfileOptions = new string[111]
	elseif a_count <= 112
		SkillXpProfileOptions = new string[112]
	elseif a_count <= 113
		SkillXpProfileOptions = new string[113]
	elseif a_count <= 114
		SkillXpProfileOptions = new string[114]
	elseif a_count <= 115
		SkillXpProfileOptions = new string[115]
	elseif a_count <= 116
		SkillXpProfileOptions = new string[116]
	elseif a_count <= 117
		SkillXpProfileOptions = new string[117]
	elseif a_count <= 118
		SkillXpProfileOptions = new string[118]
	elseif a_count <= 119
		SkillXpProfileOptions = new string[119]
	elseif a_count <= 120
		SkillXpProfileOptions = new string[120]
	elseif a_count <= 121
		SkillXpProfileOptions = new string[121]
	elseif a_count <= 122
		SkillXpProfileOptions = new string[122]
	elseif a_count <= 123
		SkillXpProfileOptions = new string[123]
	elseif a_count <= 124
		SkillXpProfileOptions = new string[124]
	elseif a_count <= 125
		SkillXpProfileOptions = new string[125]
	elseif a_count <= 126
		SkillXpProfileOptions = new string[126]
	elseif a_count <= 127
		SkillXpProfileOptions = new string[127]
	else
		SkillXpProfileOptions = new string[128]
	endif
EndFunction

Function RenderGeneralPage()
	RefreshRuntimeState()
	RefreshSkillXpProfileOptions()
	SetTitleText("$SkillGroups_Title_Main")
	AddHeaderOption("$SkillGroups_Header_Runtime")
	int enabledFlags = OPTION_FLAG_NONE
	if !HookAvailable
		enabledFlags = OPTION_FLAG_DISABLED
	endif

	OID_Enabled = AddToggleOption("$SkillGroups_Option_Enabled", GetBool("bEnabled:General"), enabledFlags)
	OID_LogLevel = AddMenuOption("$SkillGroups_Option_LogLevel", LogLevelOptions[GetInt("iLogLevel:General")])
	OID_MasterProfile = AddMenuOption("$SkillGroups_Option_Profile", SkillXpProfileOptions[GetMasterProfile()])

	if !HookAvailable
		AddHeaderOption("$SkillGroups_Header_Status")
		OID_HookFailure = AddTextOption("$SkillGroups_Option_HookUnavailable", "", OPTION_FLAG_DISABLED)
		OID_OpenLog = AddTextOption("$SkillGroups_Option_OpenLog", "")
	endif
EndFunction

Function RenderLevelXpPage()
	RefreshSkillXpProfileOptions()
	CacheCharacterXpPageState()
	SetTitleText("$SkillGroups_Title_CharacterXp")
	AddHeaderOption("$SkillGroups_Header_CharacterGroupSkill")
	RenderLevelXpGroup(6, 0, 1, 3)
	RenderLevelXpGroup(4, 2, 14, -1)
	RenderLevelXpGroup(3, 6, 5, -1)
	RenderLevelXpGroup(2, 16, 12, 9)
	RenderLevelXpGroup(1, 15, 13, -1)
	RenderLevelXpGroup(5, 11, 7, 8)
	RenderLevelXpGroup(0, 4, 17, 10)

	SetCursorPosition(1)

	AddHeaderOption("$SkillGroups_Header_CharacterProfile")
	int profileFlags = OPTION_FLAG_NONE
	if IsMasterProfileLocked()
		profileFlags = OPTION_FLAG_DISABLED
	endif
	OID_CharacterXpProfile = AddMenuOption("$SkillGroups_Option_Profile", SkillXpProfileOptions[GetEffectiveCharacterXpProfile()], profileFlags)
	AddImportCharacterXpPageOption()
	int cacheFlags = OPTION_FLAG_NONE
	if !CachedCharacterProfileEditable
		cacheFlags = OPTION_FLAG_DISABLED
	endif
	OID_ApplyCharacterXp = AddTextOption("$SkillGroups_Option_ApplyCharacterProfile", "")
	OID_CacheCharacterXp = AddTextOption("$SkillGroups_Option_CacheThresholds", "", cacheFlags)
	OID_ResyncLevelThreshold = AddTextOption("$SkillGroups_Option_RecalculateThreshold", "")
	AddHeaderOption("$SkillGroups_Header_CharacterSettings")
	AddCharacterXpSettingsOptions()
	OID_CharacterXpHeader = AddHeaderOption("$SkillGroups_Header_CharacterContribution")
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

Function AddCharacterXpSettingsOptions()
	int flags = OPTION_FLAG_NONE
	if !CachedCharacterProfileEditable
		flags = OPTION_FLAG_DISABLED
	endif
	OID_UseFlatCharacterXp = AddToggleOption("$SkillGroups_Option_FlatSkillRankXp", CachedUseFlatCharacterXp, flags)
	int flatFlags = flags
	if !CachedUseFlatCharacterXp
		flatFlags = OPTION_FLAG_DISABLED
	endif
	OID_FlatCharacterXp = AddSliderOption("$SkillGroups_Option_FlatXpAmount", CachedFlatCharacterXp, "{0}", flatFlags)
	OID_LevelUpBase = AddSliderOption("$SkillGroups_Option_BaseLevelXp", CachedLevelUpBase, "{0}", flags)
	OID_LevelUpMult = AddSliderOption("$SkillGroups_Option_ThresholdIncrease", CachedLevelUpMult, "{0}", flags)
EndFunction

Function AddImportCharacterXpPageOption()
	if CachedCharacterProfileEditable
		OID_ImportCharacterXpPage = AddMenuOption("$SkillGroups_Option_ImportCharacterPage", "$SkillGroups_Value_Select")
	endif
EndFunction

Function AddImportSkillXpPageOption()
	if CachedSkillProfileEditable
		OID_ImportSkillXpPage = AddMenuOption("$SkillGroups_Option_ImportSkillPage", "$SkillGroups_Value_Select")
	endif
EndFunction

Function RenderSkillXpPage()
	RefreshSkillXpProfileOptions()
	CacheSkillXpPageState()
	SetTitleText("$SkillGroups_Title_SkillXp")
	int profileFlags = OPTION_FLAG_NONE
	if IsMasterProfileLocked()
		profileFlags = OPTION_FLAG_DISABLED
	endif
	OID_SkillXpHeader = AddHeaderOption("$SkillGroups_Header_SkillMultiplier")
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

	SetCursorPosition(1)

	AddHeaderOption("$SkillGroups_Header_SkillProfile")
	OID_SkillXpProfile = AddMenuOption("$SkillGroups_Option_Profile", SkillXpProfileOptions[GetEffectiveSkillXpProfile()], profileFlags)
	AddImportSkillXpPageOption()
	int cacheFlags = OPTION_FLAG_NONE
	if !CachedSkillProfileEditable
		cacheFlags = OPTION_FLAG_DISABLED
	endif
	int divisorFlags = OPTION_FLAG_NONE
	if !CachedSkillProfileEditable
		divisorFlags = OPTION_FLAG_DISABLED
	endif
	OID_CacheSkillXp = AddTextOption("$SkillGroups_Option_CacheSkillXp", "", cacheFlags)
	OID_ApplySkillXp = AddTextOption("$SkillGroups_Option_ApplySkillXp", "")
	OID_DivideSkillXpByGroupSize = AddToggleOption("$SkillGroups_Option_DivideByGroupSize", GetBool("bDivideSkillXpByGroupSize:General"), divisorFlags)
	OID_AutoApplySkillXp = AddToggleOption("$SkillGroups_Option_AutoApplyRankUp", GetBool("bAutoApplySkillXpOnLevelXp:General"))

EndFunction

Function AddCustomCharacterXpSlider(int a_skillIndex)
	int flags = OPTION_FLAG_NONE
	if !CachedCharacterProfileEditable
		flags = OPTION_FLAG_DISABLED
	endif
	OIDs_CustomCharacterXp[a_skillIndex] = AddSliderOption(SkillLabels[a_skillIndex], CachedCharacterXpValues[a_skillIndex], "{2}", flags)
EndFunction

Function AddCustomSkillXpSlider(int a_skillIndex)
	int flags = OPTION_FLAG_NONE
	if !CachedSkillProfileEditable
		flags = OPTION_FLAG_DISABLED
	endif
	OIDs_CustomSkillXp[a_skillIndex] = AddSliderOption(SkillLabels[a_skillIndex], CachedSkillXpValues[a_skillIndex], "{2}", flags)
EndFunction

Function RenderLevelXpGroup(int a_groupIndex, int a_skillA, int a_skillB, int a_skillC)
	AddHeaderOption(GroupLabels[a_groupIndex])
	int flags = OPTION_FLAG_NONE
	if !CachedCharacterProfileEditable
		flags = OPTION_FLAG_DISABLED
	endif
	OIDs_CharacterGroupScales[a_groupIndex] = AddSliderOption("$SkillGroups_Option_Group", CachedCharacterGroupScaleValues[a_groupIndex], "{2}", flags)
	AddCharacterSkillScale(a_skillA)
	AddCharacterSkillScale(a_skillB)
	if a_skillC >= 0
		AddCharacterSkillScale(a_skillC)
	endif
EndFunction

Function AddCharacterSkillScale(int a_skillIndex)
	int flags = OPTION_FLAG_NONE
	if !CachedCharacterProfileEditable
		flags = OPTION_FLAG_DISABLED
	endif
	OIDs_CharacterSkillScales[a_skillIndex] = AddSliderOption(SkillLabels[a_skillIndex], CachedCharacterSkillScaleValues[a_skillIndex], "{2}", flags)
EndFunction

Function CacheCharacterXpPageState()
	int profile = GetEffectiveCharacterXpProfile()
	if CachedCharacterProfileIndex == profile
		return
	endif

	CachedCharacterProfileIndex = profile
	CachedCharacterProfileEditable = IsSelectedCharacterXpProfileEditable()
	CachedUseFlatCharacterXp = GetProfileUseFlatCharacterXp()
	CachedFlatCharacterXp = GetProfileFlatCharacterXp()
	CachedLevelUpBase = GetProfileLevelUpBase()
	CachedLevelUpMult = GetProfileLevelUpMult()

	int i = 0
	while i < SkillLabels.Length
		CachedCharacterXpValues[i] = GetCharacterXpDisplayValue(i)
		CachedCharacterSkillScaleValues[i] = GetCharacterSkillScaleDisplayValue(i)
		i += 1
	endwhile

	i = 0
	while i < GroupLabels.Length
		CachedCharacterGroupScaleValues[i] = GetCharacterGroupScaleDisplayValue(i)
		i += 1
	endwhile
EndFunction

Function CacheSkillXpPageState()
	int profile = GetEffectiveSkillXpProfile()
	bool divisor = GetBool("bDivideSkillXpByGroupSize:General")
	if CachedSkillProfileIndex == profile && CachedSkillDivisor == divisor
		return
	endif

	CachedSkillProfileIndex = profile
	CachedSkillDivisor = divisor
	CachedSkillProfileEditable = IsSelectedSkillXpProfileEditable()
	int i = 0
	while i < SkillLabels.Length
		CachedSkillXpValues[i] = GetSkillXpDisplayValue(i)
		i += 1
	endwhile
EndFunction

Function InvalidateCharacterXpPageState()
	CachedCharacterProfileIndex = -1
EndFunction

Function InvalidateSkillXpPageState()
	CachedSkillProfileIndex = -1
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

Function SetCharacterXpProfile(int a_value)
	MCM.SetModSettingInt("SkillGroups", "iCharacterXpProfile:General", a_value)
	InvalidateCharacterXpPageState()
	SetMenuOptionValue(OID_CharacterXpProfile, SkillXpProfileOptions[a_value])
EndFunction

Function SetSkillXpProfile(int a_value)
	MCM.SetModSettingInt("SkillGroups", "iSkillXpProfile:General", a_value)
	InvalidateSkillXpPageState()
	SetMenuOptionValue(OID_SkillXpProfile, SkillXpProfileOptions[a_value])
	RefreshNativeSettings()
EndFunction

Function SetMasterProfile(int a_value)
	MCM.SetModSettingInt("SkillGroups", "iMasterProfile:General", a_value)
	SetMenuOptionValue(OID_MasterProfile, SkillXpProfileOptions[a_value])
	if !SkillGroups_Native.IsSkillXpProfileEditable(a_value)
		MCM.SetModSettingInt("SkillGroups", "iSkillXpProfile:General", a_value)
		MCM.SetModSettingInt("SkillGroups", "iCharacterXpProfile:General", a_value)
	endif
	InvalidateCharacterXpPageState()
	InvalidateSkillXpPageState()
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
	int characterXpIndex = FindOption(OIDs_CustomCharacterXp, a_option)
	if characterXpIndex >= 0
		if IsSelectedCharacterXpProfileEditable() && SkillGroups_Native.SetCharacterXpProfileMultiplier(GetEffectiveCharacterXpProfile(), characterXpIndex, 1.0)
			SetSliderOptionValue(a_option, 1.0, "{2}")
		endif
		return
	endif

	int groupScaleIndex = FindOption(OIDs_CharacterGroupScales, a_option)
	if groupScaleIndex >= 0
		float defaultGroupValue = GetDefaultGroupScale(groupScaleIndex)
		if IsSelectedCharacterXpProfileEditable() && SkillGroups_Native.SetGroupXpProfileMultiplierScale(GetEffectiveCharacterXpProfile(), groupScaleIndex, defaultGroupValue)
			SetSliderOptionValue(a_option, defaultGroupValue, "{2}")
		endif
		return
	endif

	int skillScaleIndex = FindOption(OIDs_CharacterSkillScales, a_option)
	if skillScaleIndex >= 0
		if IsSelectedCharacterXpProfileEditable() && SkillGroups_Native.SetPlayerXpProfileMultiplierScale(GetEffectiveCharacterXpProfile(), skillScaleIndex, 1.0)
			SetSliderOptionValue(a_option, 1.0, "{2}")
		endif
		return
	endif

	int skillXpIndex = FindOption(OIDs_CustomSkillXp, a_option)
	if skillXpIndex >= 0
		float defaultSkillXpValue = GetDefaultSkillXpMultiplier(skillXpIndex)
		if IsSelectedSkillXpProfileEditable() && SkillGroups_Native.SetSkillXpProfileMultiplier(GetEffectiveSkillXpProfile(), skillXpIndex, defaultSkillXpValue)
			SetSliderOptionValue(a_option, GetSkillXpDisplayValue(skillXpIndex), "{2}")
		endif
		return
	endif

EndFunction

Function UpdateCustomCharacterXpFlags()
	int flags = OPTION_FLAG_NONE
	if !IsSelectedCharacterXpProfileEditable()
		flags = OPTION_FLAG_DISABLED
	endif
	UpdateCharacterXpSettingsFlags()
	if OID_CharacterXpProfile > 0
		int profileFlags = OPTION_FLAG_NONE
		if IsMasterProfileLocked()
			profileFlags = OPTION_FLAG_DISABLED
		endif
		SetOptionFlags(OID_CharacterXpProfile, profileFlags)
		SetMenuOptionValue(OID_CharacterXpProfile, SkillXpProfileOptions[GetEffectiveCharacterXpProfile()])
	endif
	if OID_CacheCharacterXp > 0
		SetOptionFlags(OID_CacheCharacterXp, flags)
	endif
	if OID_ImportCharacterXpPage > 0
		SetOptionFlags(OID_ImportCharacterXpPage, flags)
	endif

	int i = 0
	while i < OIDs_CustomCharacterXp.Length
		if OIDs_CustomCharacterXp[i] > 0
			SetOptionFlags(OIDs_CustomCharacterXp[i], flags)
			SetSliderOptionValue(OIDs_CustomCharacterXp[i], CachedCharacterXpValues[i], "{2}")
		endif
		i += 1
	endwhile

	i = 0
	while i < OIDs_CharacterGroupScales.Length
		if OIDs_CharacterGroupScales[i] > 0
			SetOptionFlags(OIDs_CharacterGroupScales[i], flags)
			SetSliderOptionValue(OIDs_CharacterGroupScales[i], CachedCharacterGroupScaleValues[i], "{2}")
		endif
		i += 1
	endwhile

	i = 0
	while i < OIDs_CharacterSkillScales.Length
		if OIDs_CharacterSkillScales[i] > 0
			SetOptionFlags(OIDs_CharacterSkillScales[i], flags)
			SetSliderOptionValue(OIDs_CharacterSkillScales[i], CachedCharacterSkillScaleValues[i], "{2}")
		endif
		i += 1
	endwhile
EndFunction

Function UpdateCharacterXpSettingsFlags()
	CacheCharacterXpPageState()
	int flags = OPTION_FLAG_NONE
	if !IsSelectedCharacterXpProfileEditable()
		flags = OPTION_FLAG_DISABLED
	endif
	if OID_UseFlatCharacterXp > 0
		SetOptionFlags(OID_UseFlatCharacterXp, flags)
		SetToggleOptionValue(OID_UseFlatCharacterXp, CachedUseFlatCharacterXp)
	endif
	if OID_FlatCharacterXp > 0
		int flatFlags = flags
		if !CachedUseFlatCharacterXp
			flatFlags = OPTION_FLAG_DISABLED
		endif
		SetOptionFlags(OID_FlatCharacterXp, flatFlags)
		SetSliderOptionValue(OID_FlatCharacterXp, CachedFlatCharacterXp, "{0}")
	endif
	if OID_LevelUpBase > 0
		SetOptionFlags(OID_LevelUpBase, flags)
		SetSliderOptionValue(OID_LevelUpBase, CachedLevelUpBase, "{0}")
	endif
	if OID_LevelUpMult > 0
		SetOptionFlags(OID_LevelUpMult, flags)
		SetSliderOptionValue(OID_LevelUpMult, CachedLevelUpMult, "{0}")
	endif
EndFunction

Function UpdateCustomSkillXpFlags()
	CacheSkillXpPageState()
	int flags = OPTION_FLAG_NONE
	if !IsSelectedSkillXpProfileEditable()
		flags = OPTION_FLAG_DISABLED
	endif

	if OID_CacheSkillXp > 0
		SetOptionFlags(OID_CacheSkillXp, flags)
	endif
	if OID_DivideSkillXpByGroupSize > 0
		SetOptionFlags(OID_DivideSkillXpByGroupSize, flags)
	endif
	if OID_ImportSkillXpPage > 0
		SetOptionFlags(OID_ImportSkillXpPage, flags)
	endif
	if OID_SkillXpProfile > 0
		int profileFlags = OPTION_FLAG_NONE
		if IsMasterProfileLocked()
			profileFlags = OPTION_FLAG_DISABLED
		endif
		SetOptionFlags(OID_SkillXpProfile, profileFlags)
		SetMenuOptionValue(OID_SkillXpProfile, SkillXpProfileOptions[GetEffectiveSkillXpProfile()])
	endif

	int i = 0
	while i < OIDs_CustomSkillXp.Length
		if OIDs_CustomSkillXp[i] > 0
			SetOptionFlags(OIDs_CustomSkillXp[i], flags)
			SetSliderOptionValue(OIDs_CustomSkillXp[i], CachedSkillXpValues[i], "{2}")
		endif
		i += 1
	endwhile
EndFunction

Function ApplyCharacterXpProfiles()
	if SkillGroups_Native.ApplyCharacterXpProfiles(GetEffectiveCharacterXpProfile())
		ShowFinishedMessage("Skill Groups finished saving and applying the character XP profile.")
	else
		ShowFinishedMessage("Skill Groups failed to save or apply the character XP profile.")
	endif
EndFunction

Function CacheCharacterXpSettings()
	if SkillGroups_Native.CacheCharacterXpSettings(GetEffectiveCharacterXpProfile())
		InvalidateCharacterXpPageState()
		UpdateCharacterXpSettingsFlags()
		ShowFinishedMessage("Skill Groups finished getting character XP settings.")
	else
		ShowFinishedMessage("Skill Groups failed to get character XP settings.")
	endif
EndFunction

Function ResyncCurrentLevelThreshold()
	bool accepted = ShowMessage("This can trigger level-up behavior if current character XP already meets the recalculated threshold. Continue?", true, "Recalculate", "Cancel")
	if !accepted
		return
	endif

	if SkillGroups_Native.ResyncCurrentLevelThreshold(GetEffectiveCharacterXpProfile())
		ShowFinishedMessage("Skill Groups finished recalculating the current level threshold.")
	else
		ShowFinishedMessage("Skill Groups failed to recalculate the current level threshold.")
	endif
EndFunction

Function ImportCharacterXpPageFromProfile(int a_sourceProfile)
	int characterTarget = GetEffectiveCharacterXpProfile()
	if IsSelectedCharacterXpProfileEditable()
		SkillGroups_Native.SetProfileCharacterXpSettings(characterTarget, SkillGroups_Native.GetProfileUseFlatCharacterXp(a_sourceProfile), SkillGroups_Native.GetProfileFlatCharacterXp(a_sourceProfile), SkillGroups_Native.GetProfileLevelUpBase(a_sourceProfile), SkillGroups_Native.GetProfileLevelUpMult(a_sourceProfile))
		int i = 0
		while i < SkillLabels.Length
			SkillGroups_Native.SetCharacterXpProfileMultiplier(characterTarget, i, SkillGroups_Native.GetCharacterXpProfileMultiplier(a_sourceProfile, i))
			i += 1
		endwhile
		int groupIndex = 0
		while groupIndex < GroupLabels.Length
			SkillGroups_Native.SetGroupXpProfileMultiplierScale(characterTarget, groupIndex, SkillGroups_Native.GetGroupXpProfileMultiplierScale(a_sourceProfile, groupIndex))
			groupIndex += 1
		endwhile
		int skillIndex = 0
		while skillIndex < SkillLabels.Length
			SkillGroups_Native.SetPlayerXpProfileMultiplierScale(characterTarget, skillIndex, SkillGroups_Native.GetPlayerXpProfileMultiplierScale(a_sourceProfile, skillIndex))
			skillIndex += 1
		endwhile
		InvalidateCharacterXpPageState()
		SetMenuOptionValue(OID_ImportCharacterXpPage, SkillXpProfileOptions[a_sourceProfile])
		UpdateCustomCharacterXpFlags()
		ShowFinishedMessage("Skill Groups finished loading profile.")
	endif
EndFunction

Function ImportSkillXpPageFromProfile(int a_sourceProfile)
	if !IsSelectedSkillXpProfileEditable()
		return
	endif

	int target = GetEffectiveSkillXpProfile()
	int i = 0
	while i < SkillLabels.Length
		SkillGroups_Native.SetSkillXpProfileMultiplier(target, i, SkillGroups_Native.GetSkillXpProfileMultiplier(a_sourceProfile, i))
		i += 1
	endwhile
	InvalidateSkillXpPageState()
	SetMenuOptionValue(OID_ImportSkillXpPage, SkillXpProfileOptions[a_sourceProfile])
	UpdateCustomSkillXpFlags()
	ShowFinishedMessage("Skill Groups finished loading profile.")
EndFunction

Function ToggleProfileUseFlatCharacterXp()
	if !IsSelectedCharacterXpProfileEditable()
		return
	endif
	SetProfileCharacterXpSettings(!GetProfileUseFlatCharacterXp(), GetProfileFlatCharacterXp(), GetProfileLevelUpBase(), GetProfileLevelUpMult())
EndFunction

Function SetProfileCharacterXpSettings(bool a_useFlatCharacterXp, float a_flatCharacterXp, float a_levelUpBase, float a_levelUpMult)
	if !IsSelectedCharacterXpProfileEditable()
		return
	endif
	if SkillGroups_Native.SetProfileCharacterXpSettings(GetEffectiveCharacterXpProfile(), a_useFlatCharacterXp, a_flatCharacterXp, a_levelUpBase, a_levelUpMult)
		CachedUseFlatCharacterXp = a_useFlatCharacterXp
		CachedFlatCharacterXp = a_flatCharacterXp
		CachedLevelUpBase = a_levelUpBase
		CachedLevelUpMult = a_levelUpMult
		UpdateCharacterXpSettingsFlags()
	endif
EndFunction

Function ApplySkillXpMultipliers()
	if SkillGroups_Native.ApplySkillXpMultipliers(GetBool("bDivideSkillXpByGroupSize:General"), GetEffectiveSkillXpProfile())
		UpdateCustomSkillXpFlags()
		ShowFinishedMessage("Skill Groups finished saving and applying the skill XP profile.")
	else
		ShowFinishedMessage("Skill Groups failed to save or apply the skill XP profile.")
	endif
EndFunction

Function CacheSkillXpMultipliers()
	if SkillGroups_Native.CacheSkillXpMultipliers()
		InvalidateSkillXpPageState()
		UpdateCachedSkillXpSettings(true)
		ShowFinishedMessage("Skill Groups finished caching skill XP multipliers.")
	else
		ShowFinishedMessage("Skill Groups failed to cache skill XP multipliers.")
	endif
EndFunction

Function ShowFinishedMessage(string a_message)
	bool ignored = ShowMessage(a_message, false, "OK")
EndFunction

Function UpdateCachedSkillXpSettings(bool a_updateProfile)
	bool updateProfile = a_updateProfile && IsSelectedSkillXpProfileEditable()
	int profile = GetEffectiveSkillXpProfile()
	int i = 0
	while i < SkillLabels.Length
		float value = SkillGroups_Native.GetCachedSkillXpMultiplier(i)
		if updateProfile
			SkillGroups_Native.SetSkillXpProfileMultiplier(profile, i, value * GetSkillXpDisplayDivisor(i))
		endif
		CachedSkillXpValues[i] = value
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

int Function GetCharacterXpProfile()
	int value = MCM.GetModSettingInt("SkillGroups", "iCharacterXpProfile:General")
	if value < 0 || value >= SkillXpProfileOptions.Length
		return 0
	endif
	return value
EndFunction

int Function GetSkillXpProfile()
	int value = MCM.GetModSettingInt("SkillGroups", "iSkillXpProfile:General")
	if value < 0 || value >= SkillXpProfileOptions.Length
		return 0
	endif
	return value
EndFunction

int Function GetMasterProfile()
	int value = MCM.GetModSettingInt("SkillGroups", "iMasterProfile:General")
	if value < 0 || value >= SkillXpProfileOptions.Length
		return GetDefaultMasterProfile()
	endif
	return value
EndFunction

int Function GetDefaultMasterProfile()
	int i = 0
	while i < SkillXpProfileOptions.Length
		if SkillGroups_Native.IsSkillXpProfileEditable(i)
			return i
		endif
		i += 1
	endwhile

	return 0
EndFunction

bool Function IsMasterProfileLocked()
	return !SkillGroups_Native.IsSkillXpProfileEditable(GetMasterProfile())
EndFunction

int Function GetEffectiveSkillXpProfile()
	if IsMasterProfileLocked()
		return GetMasterProfile()
	endif
	return GetSkillXpProfile()
EndFunction

int Function GetEffectiveCharacterXpProfile()
	if IsMasterProfileLocked()
		return GetMasterProfile()
	endif
	return GetCharacterXpProfile()
EndFunction

bool Function IsSelectedCharacterXpProfileEditable()
	return SkillGroups_Native.IsSkillXpProfileEditable(GetEffectiveCharacterXpProfile())
EndFunction

bool Function IsSelectedSkillXpProfileEditable()
	return SkillGroups_Native.IsSkillXpProfileEditable(GetEffectiveSkillXpProfile())
EndFunction

float Function GetCharacterXpDisplayValue(int a_skillIndex)
	return SkillGroups_Native.GetCharacterXpProfileMultiplier(GetEffectiveCharacterXpProfile(), a_skillIndex)
EndFunction

float Function GetCharacterGroupScaleDisplayValue(int a_groupIndex)
	return SkillGroups_Native.GetGroupXpProfileMultiplierScale(GetEffectiveCharacterXpProfile(), a_groupIndex)
EndFunction

float Function GetCharacterSkillScaleDisplayValue(int a_skillIndex)
	return SkillGroups_Native.GetPlayerXpProfileMultiplierScale(GetEffectiveCharacterXpProfile(), a_skillIndex)
EndFunction

float Function GetSkillXpDisplayValue(int a_skillIndex)
	return SkillGroups_Native.GetSkillXpProfileMultiplier(GetEffectiveSkillXpProfile(), a_skillIndex) / GetSkillXpDisplayDivisor(a_skillIndex)
EndFunction

float Function GetSkillXpDisplayDivisor(int a_skillIndex)
	if !GetBool("bDivideSkillXpByGroupSize:General")
		return 1.0
	endif

	if a_skillIndex == 0 || a_skillIndex == 1 || a_skillIndex == 3
		return 3.0
	elseif a_skillIndex == 2 || a_skillIndex == 14
		return 2.0
	elseif a_skillIndex == 5 || a_skillIndex == 6
		return 2.0
	elseif a_skillIndex == 9 || a_skillIndex == 12 || a_skillIndex == 16
		return 3.0
	elseif a_skillIndex == 13 || a_skillIndex == 15
		return 2.0
	elseif a_skillIndex == 7 || a_skillIndex == 8 || a_skillIndex == 11
		return 3.0
	elseif a_skillIndex == 4 || a_skillIndex == 10 || a_skillIndex == 17
		return 3.0
	endif

	return 1.0
EndFunction

bool Function GetProfileUseFlatCharacterXp()
	return SkillGroups_Native.GetProfileUseFlatCharacterXp(GetEffectiveCharacterXpProfile())
EndFunction

float Function GetProfileFlatCharacterXp()
	return SkillGroups_Native.GetProfileFlatCharacterXp(GetEffectiveCharacterXpProfile())
EndFunction

float Function GetProfileLevelUpBase()
	return SkillGroups_Native.GetProfileLevelUpBase(GetEffectiveCharacterXpProfile())
EndFunction

float Function GetProfileLevelUpMult()
	return SkillGroups_Native.GetProfileLevelUpMult(GetEffectiveCharacterXpProfile())
EndFunction

int Function FindOption(int[] a_options, int a_option)
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
	if a_option == OID_FlatCharacterXp
		return GetProfileFlatCharacterXp()
	elseif a_option == OID_LevelUpBase
		return GetProfileLevelUpBase()
	elseif a_option == OID_LevelUpMult
		return GetProfileLevelUpMult()
	endif

	int characterXpIndex = FindOption(OIDs_CustomCharacterXp, a_option)
	if characterXpIndex >= 0
		return GetCharacterXpDisplayValue(characterXpIndex)
	endif

	int groupScaleIndex = FindOption(OIDs_CharacterGroupScales, a_option)
	if groupScaleIndex >= 0
		return GetCharacterGroupScaleDisplayValue(groupScaleIndex)
	endif

	int skillScaleIndex = FindOption(OIDs_CharacterSkillScales, a_option)
	if skillScaleIndex >= 0
		return GetCharacterSkillScaleDisplayValue(skillScaleIndex)
	endif

	int skillXpIndex = FindOption(OIDs_CustomSkillXp, a_option)
	if skillXpIndex >= 0
		return GetSkillXpDisplayValue(skillXpIndex)
	endif

	return 1.0
EndFunction

float Function GetDefaultSliderValueForOption(int a_option)
	if a_option == OID_FlatCharacterXp
		return 10.0
	elseif a_option == OID_LevelUpBase
		return 75.0
	elseif a_option == OID_LevelUpMult
		return 25.0
	endif

	int index = FindOption(OIDs_CharacterGroupScales, a_option)
	if index >= 0
		return GetDefaultGroupScale(index)
	endif

	index = FindOption(OIDs_CustomSkillXp, a_option)
	if index >= 0
		return GetDefaultSkillXpMultiplier(index) / GetSkillXpDisplayDivisor(index)
	endif

	return 1.0
EndFunction

float Function GetDefaultGroupScale(int a_groupIndex)
	if a_groupIndex == 0
		return 0.75
	elseif a_groupIndex == 1
		return 1.15
	elseif a_groupIndex == 2
		return 0.75
	elseif a_groupIndex == 3
		return 1.4
	elseif a_groupIndex == 4
		return 1.4
	elseif a_groupIndex == 5
		return 0.15
	elseif a_groupIndex == 6
		return 1.4
	endif

	return 1.0
EndFunction

float Function GetDefaultSkillXpMultiplier(int a_skillIndex)
	if a_skillIndex == 0
		return 6.3
	elseif a_skillIndex == 1
		return 5.95
	elseif a_skillIndex == 2
		return 9.3
	elseif a_skillIndex == 3
		return 8.1
	elseif a_skillIndex == 4
		return 1.0
	elseif a_skillIndex == 5
		return 3.8
	elseif a_skillIndex == 6
		return 4.0
	elseif a_skillIndex == 7
		return 8.1
	elseif a_skillIndex == 8
		return 45.0
	elseif a_skillIndex == 9
		return 11.25
	elseif a_skillIndex == 10
		return 0.75
	elseif a_skillIndex == 11
		return 0.36
	elseif a_skillIndex == 12
		return 3.0
	elseif a_skillIndex == 13
		return 2.1
	elseif a_skillIndex == 14
		return 1.35
	elseif a_skillIndex == 15
		return 4.6
	elseif a_skillIndex == 16
		return 2.0
	elseif a_skillIndex == 17
		return 900.0
	endif

	return 1.0
EndFunction
