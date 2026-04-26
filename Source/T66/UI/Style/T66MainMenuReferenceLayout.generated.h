// Copyright Tribulation 66. All Rights Reserved.

#pragma once

namespace T66MainMenuReferenceLayout
{
	inline constexpr float CanvasWidth = 1920.0f;
	inline constexpr float CanvasHeight = 1080.0f;
	inline constexpr float TopBarReservedHeight = 132.0f;
	inline constexpr float TopBarSurfaceHeight = 132.0f;
	inline constexpr float TopBarSurfaceOffsetY = 0.0f;

	namespace TopBar
	{
		inline constexpr FT66ReferenceRect TopbarStripFull{17.0f, 11.0f, 1885.0f, 120.0f};
		inline constexpr FT66ReferenceRect ButtonSettings{33.0f, 26.0f, 117.0f, 91.0f};
		inline constexpr FT66ReferenceRect ButtonChat{161.0f, 28.0f, 134.0f, 89.0f};
		inline constexpr FT66ReferenceRect TabAccount{306.0f, 28.0f, 268.0f, 88.0f};
		inline constexpr FT66ReferenceRect BadgeProfile{585.0f, 18.0f, 123.0f, 104.0f};
		inline constexpr FT66ReferenceRect TabPowerUp{719.0f, 28.0f, 249.0f, 88.0f};
		inline constexpr FT66ReferenceRect TabAchievements{979.0f, 28.0f, 305.0f, 88.0f};
		inline constexpr FT66ReferenceRect TabMinigames{1296.0f, 28.0f, 270.0f, 88.0f};
		inline constexpr FT66ReferenceRect CurrencySlot{1577.0f, 28.0f, 180.0f, 88.0f};
		inline constexpr FT66ReferenceRect ButtonPower{1768.0f, 28.0f, 119.0f, 88.0f};
	}

	namespace Center
	{
		inline constexpr FT66ReferenceRect CenterBackdropFull{460.0f, 138.0f, 1003.0f, 942.0f};
		inline constexpr FT66ReferenceRect TitleLockup{558.0f, 118.0f, 804.0f, 150.0f};
		inline constexpr FT66ReferenceRect SubtitleLockup{725.0f, 252.0f, 524.0f, 71.0f};
		inline constexpr FT66ReferenceRect HeroStage{475.0f, 298.0f, 975.0f, 416.0f};
		inline constexpr FT66ReferenceRect CtaStackFull{718.0f, 718.0f, 486.0f, 226.0f};
		inline constexpr FT66ReferenceRect CtaButtonNewGame{718.0f, 718.0f, 486.0f, 104.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGame{718.0f, 840.0f, 486.0f, 104.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallenge{718.0f, 960.0f, 486.0f, 92.0f};
	}

	namespace CenterRuntime
	{
		inline constexpr FT66ReferenceRect CtaButtonNewGamePlate{718.0f, 718.0f, 486.0f, 104.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGamePlate{718.0f, 840.0f, 486.0f, 104.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallengePlate{718.0f, 960.0f, 486.0f, 92.0f};
	}

	namespace Left
	{
		inline constexpr FT66ReferenceRect ShellFullReference{17.0f, 147.0f, 466.0f, 886.0f};
		inline constexpr FT66ReferenceRect ProfileCardReference{36.0f, 162.0f, 424.0f, 108.0f};
		inline constexpr FT66ReferenceRect SearchFieldReference{36.0f, 291.0f, 424.0f, 54.0f};
		inline constexpr FT66ReferenceRect SearchIcon{48.0f, 302.0f, 34.0f, 32.0f};
		inline constexpr FT66ReferenceRect FriendStarButton{291.0f, 598.0f, 62.0f, 57.0f};
		inline constexpr FT66ReferenceRect FriendInviteButton{355.0f, 598.0f, 96.0f, 57.0f};
		inline constexpr FT66ReferenceRect FriendOfflineButton{355.0f, 771.0f, 97.0f, 58.0f};
		inline constexpr FT66ReferenceRect FriendAvatarFrameSource{45.0f, 596.0f, 55.0f, 56.0f};
		inline constexpr FT66ReferenceRect PartySlotSource{44.0f, 916.0f, 101.0f, 103.0f};
		inline constexpr FT66ReferenceRect CloseButton{415.0f, 862.0f, 51.0f, 48.0f};
	}

	namespace Right
	{
		inline constexpr FT66ReferenceRect ShellFullReference{1425.0f, 232.0f, 477.0f, 801.0f};
		inline constexpr FT66ReferenceRect FilterWorldButton{1435.0f, 149.0f, 136.0f, 72.0f};
		inline constexpr FT66ReferenceRect FilterFriendsButton{1585.0f, 149.0f, 136.0f, 72.0f};
		inline constexpr FT66ReferenceRect FilterCrownButton{1735.0f, 149.0f, 136.0f, 72.0f};
		inline constexpr FT66ReferenceRect TabWeeklyActive{1436.0f, 232.0f, 219.0f, 65.0f};
		inline constexpr FT66ReferenceRect TabAllTimeInactive{1664.0f, 232.0f, 219.0f, 65.0f};
		inline constexpr FT66ReferenceRect DropdownShellLeft{1436.0f, 294.0f, 219.0f, 57.0f};
		inline constexpr FT66ReferenceRect DropdownShellRight{1664.0f, 294.0f, 219.0f, 57.0f};
		inline constexpr FT66ReferenceRect ToggleScoreSelected{1436.0f, 352.0f, 219.0f, 61.0f};
		inline constexpr FT66ReferenceRect ToggleSpeedrunUnselected{1664.0f, 352.0f, 219.0f, 61.0f};
		inline constexpr FT66ReferenceRect LeaderboardAvatarFrameSource{1533.0f, 631.0f, 56.0f, 56.0f};
		inline constexpr FT66ReferenceRect LeaderboardAvatarLiveRect{1541.0f, 639.0f, 40.0f, 40.0f};
	}

	namespace MainMenu
	{
		inline constexpr FT66ReferenceRect FullCanvas{0.0f, 0.0f, 1920.0f, 1080.0f};
		inline constexpr FT66ReferenceRect LeftPanelAssembly{17.0f, 147.0f, 466.0f, 886.0f};
		inline constexpr FT66ReferenceRect RightPanelAssembly{1425.0f, 147.0f, 477.0f, 886.0f};
	}

}
