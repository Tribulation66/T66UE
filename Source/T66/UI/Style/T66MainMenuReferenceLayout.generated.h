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
		inline constexpr FT66ReferenceRect TopbarStripFull{0.0f, 0.0f, 1920.0f, 127.0f};
		inline constexpr FT66ReferenceRect ButtonSettings{35.0f, 28.0f, 100.0f, 76.0f};
		inline constexpr FT66ReferenceRect ButtonChat{152.0f, 28.0f, 100.0f, 76.0f};
		inline constexpr FT66ReferenceRect TabAccount{273.0f, 28.0f, 298.0f, 76.0f};
		inline constexpr FT66ReferenceRect BadgeProfile{601.0f, 28.0f, 328.0f, 76.0f};
		inline constexpr FT66ReferenceRect TabPowerUp{954.0f, 28.0f, 301.0f, 76.0f};
		inline constexpr FT66ReferenceRect TabAchievements{1281.0f, 28.0f, 317.0f, 76.0f};
		inline constexpr FT66ReferenceRect CurrencySlot{1594.0f, 28.0f, 182.0f, 76.0f};
		inline constexpr FT66ReferenceRect ButtonPower{1793.0f, 28.0f, 100.0f, 76.0f};
	}

	namespace Center
	{
		inline constexpr FT66ReferenceRect CenterBackdropFull{460.0f, 138.0f, 1003.0f, 942.0f};
		inline constexpr FT66ReferenceRect TitleLockup{558.0f, 350.0f, 760.0f, 264.0f};
		inline constexpr FT66ReferenceRect SubtitleLockup{725.0f, 484.0f, 524.0f, 71.0f};
		inline constexpr FT66ReferenceRect HeroStage{475.0f, 298.0f, 975.0f, 416.0f};
		inline constexpr FT66ReferenceRect CtaStackFull{646.0f, 650.0f, 585.0f, 312.0f};
		inline constexpr FT66ReferenceRect CtaButtonNewGame{646.0f, 650.0f, 585.0f, 164.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGame{646.0f, 814.0f, 585.0f, 148.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallenge{718.0f, 960.0f, 486.0f, 92.0f};
	}

	namespace CenterRuntime
	{
		inline constexpr FT66ReferenceRect CtaButtonNewGamePlate{646.0f, 650.0f, 585.0f, 164.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGamePlate{646.0f, 814.0f, 585.0f, 148.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallengePlate{718.0f, 960.0f, 486.0f, 92.0f};
	}

	namespace Left
	{
		inline constexpr FT66ReferenceRect ShellFullReference{16.0f, 148.0f, 464.0f, 884.0f};
		inline constexpr FT66ReferenceRect ProfileCardReference{36.0f, 162.0f, 420.0f, 150.0f};
		inline constexpr FT66ReferenceRect SearchFieldReference{36.0f, 390.0f, 420.0f, 54.0f};
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
		inline constexpr FT66ReferenceRect ShellFullReference{1424.0f, 232.0f, 476.0f, 800.0f};
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
		inline constexpr FT66ReferenceRect LeftPanelAssembly{0.0f, 145.0f, 640.0f, 935.0f};
		inline constexpr FT66ReferenceRect RightPanelAssembly{1340.0f, 130.0f, 580.0f, 950.0f};
	}

}
