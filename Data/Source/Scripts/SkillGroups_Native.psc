Scriptname SkillGroups_Native Hidden

bool Function ApplyCharacterXpProfiles(int aiCharacterProfile) global native
bool Function ApplyMultiplierSettings(bool abEnabled, bool abUseDivisor, int aiSkillXpProfile, int aiCharacterProfile) global native
bool Function ApplyProfileGroups(int aiProfile, bool abRefreshSkillXp, int aiSkillXpProfile) global native
bool Function ApplyRuntimeSettings(bool abEnabled, bool abMultipliersEnabled, bool abUseDivisor, int aiSkillXpProfile, int aiCharacterProfile) global native
bool Function ApplySkillXpMultipliers(bool abUseDivisor, int aiSkillXpProfile, int aiGroupProfile) global native
bool Function CacheCharacterXpSettings(int aiCharacterProfile) global native
bool Function CacheSkillXpMultipliers() global native
bool Function CommitProfileGroups(int aiProfile, string[] asGroupSlotNames, int[] aiSkillGroupSlots) global native
int Function CreateProfileFrom(int aiSourceProfile, string asName) global native
float Function GetCachedSkillXpMultiplier(int aiIndex) global native
float Function GetCharacterXpProfileMultiplier(int aiProfile, int aiIndex) global native
float Function GetGroupXpProfileMultiplierScale(int aiProfile, int aiGroupIndex) global native
float Function GetPlayerXpProfileMultiplierScale(int aiProfile, int aiIndex) global native
int Function GetProfileGroupCount(int aiProfile) global native
string Function GetProfileGroupName(int aiProfile, int aiGroupIndex) global native
string Function GetProfileGroupSlotName(int aiProfile, int aiSlotIndex) global native
int Function GetProfileSkillGroupIndex(int aiProfile, int aiSkillIndex) global native
float Function GetProfileFlatCharacterXp(int aiProfile) global native
float Function GetProfileLevelUpBase(int aiProfile) global native
float Function GetProfileLevelUpMult(int aiProfile) global native
bool Function GetProfileUseFlatCharacterXp(int aiProfile) global native
int Function GetSkillXpProfileCount() global native
string Function GetSkillXpProfileName(int aiProfile) global native
float Function GetSkillXpProfileMultiplier(int aiProfile, int aiIndex) global native
bool Function IsHookAvailable() global native
bool Function IsSkillXpProfileEditable(int aiProfile) global native
bool Function OpenLogFile() global native
bool Function RefreshSettings() global native
bool Function RenameProfile(int aiProfile, string asName) global native
bool Function ResyncCurrentLevelThreshold(int aiCharacterProfile) global native
bool Function SetCharacterXpProfileMultiplier(int aiProfile, int aiIndex, float afValue) global native
bool Function SetGroupXpProfileMultiplierScale(int aiProfile, int aiGroupIndex, float afValue) global native
bool Function SetGroupingEnabled(bool abEnabled) global native
bool Function SetPlayerXpProfileMultiplierScale(int aiProfile, int aiIndex, float afValue) global native
bool Function SetProfileCharacterXpSettings(int aiProfile, bool abUseFlatCharacterXp, float afFlatCharacterXp, float afLevelUpBase, float afLevelUpMult) global native
bool Function SetSkillXpProfileMultiplier(int aiProfile, int aiIndex, float afValue) global native
