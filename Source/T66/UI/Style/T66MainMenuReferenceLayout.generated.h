// Copyright Tribulation 66. All Rights Reserved.

#pragma once

namespace T66MainMenuReferenceLayout
{
	inline constexpr float CanvasWidth = 1672.0f;
	inline constexpr float CanvasHeight = 941.0f;
	inline constexpr float TopBarReservedHeight = 142.0f;
	inline constexpr float TopBarSurfaceHeight = 126.0f;
	inline constexpr float TopBarSurfaceOffsetY = 14.0f;

	namespace TopBar
	{
		inline constexpr FT66ReferenceRect TopbarStripFull{0.0f, 0.0f, 1672.0f, 126.0f};
		inline constexpr FT66ReferenceRect ButtonSettings{12.0f, 16.0f, 96.0f, 92.0f};
		inline constexpr FT66ReferenceRect ButtonChat{126.0f, 16.0f, 96.0f, 92.0f};
		inline constexpr FT66ReferenceRect TabAccount{278.0f, 16.0f, 222.0f, 89.0f};
		inline constexpr FT66ReferenceRect BadgeProfile{521.0f, 12.0f, 79.0f, 107.0f};
		inline constexpr FT66ReferenceRect TabPowerUp{616.0f, 16.0f, 220.0f, 89.0f};
		inline constexpr FT66ReferenceRect TabAchievements{848.0f, 16.0f, 243.0f, 89.0f};
		inline constexpr FT66ReferenceRect TabMinigames{1104.0f, 16.0f, 224.0f, 89.0f};
		inline constexpr FT66ReferenceRect CurrencySlot{1369.0f, 23.0f, 165.0f, 75.0f};
		inline constexpr FT66ReferenceRect ButtonPower{1551.0f, 16.0f, 105.0f, 92.0f};
	}

	namespace Center
	{
		inline constexpr FT66ReferenceRect CenterBackdropFull{401.0f, 120.0f, 873.0f, 821.0f};
		inline constexpr FT66ReferenceRect TitleLockup{400.0f, 120.0f, 875.0f, 164.0f};
		inline constexpr FT66ReferenceRect SubtitleLockup{631.0f, 220.0f, 457.0f, 61.0f};
		inline constexpr FT66ReferenceRect HeroStage{414.0f, 260.0f, 849.0f, 362.0f};
		inline constexpr FT66ReferenceRect CtaStackFull{646.0f, 608.0f, 486.0f, 271.0f};
		inline constexpr FT66ReferenceRect CtaButtonNewGame{662.0f, 627.0f, 454.0f, 86.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGame{662.0f, 715.0f, 450.0f, 82.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallenge{664.0f, 799.0f, 449.0f, 79.0f};
	}

	namespace CenterRuntime
	{
		inline constexpr FT66ReferenceRect CtaButtonNewGamePlate{662.0f, 627.0f, 423.0f, 86.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGamePlate{662.0f, 715.0f, 420.0f, 82.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallengePlate{664.0f, 799.0f, 419.0f, 79.0f};
	}

	namespace Left
	{
		inline constexpr FT66ReferenceRect ShellFullReference{18.0f, 304.0f, 424.0f, 633.0f};
		inline constexpr FT66ReferenceRect ProfileCardReference{35.0f, 314.0f, 369.0f, 94.0f};
		inline constexpr FT66ReferenceRect SearchFieldReference{40.0f, 427.0f, 352.0f, 47.0f};
		inline constexpr FT66ReferenceRect SearchIcon{353.0f, 435.0f, 29.0f, 28.0f};
		inline constexpr FT66ReferenceRect FriendStarButton{253.0f, 521.0f, 54.0f, 50.0f};
		inline constexpr FT66ReferenceRect FriendInviteButton{309.0f, 521.0f, 84.0f, 50.0f};
		inline constexpr FT66ReferenceRect FriendOfflineButton{309.0f, 672.0f, 85.0f, 50.0f};
		inline constexpr FT66ReferenceRect FriendAvatarFrameSource{39.0f, 519.0f, 48.0f, 49.0f};
		inline constexpr FT66ReferenceRect PartySlotSource{38.0f, 798.0f, 81.0f, 90.0f};
		inline constexpr FT66ReferenceRect CloseButton{361.0f, 751.0f, 45.0f, 42.0f};
	}

	namespace Right
	{
		inline constexpr FT66ReferenceRect ShellFullReference{1245.0f, 321.0f, 399.0f, 617.0f};
		inline constexpr FT66ReferenceRect FilterWorldButton{1304.0f, 253.0f, 64.0f, 62.0f};
		inline constexpr FT66ReferenceRect FilterFriendsButton{1374.0f, 253.0f, 65.0f, 62.0f};
		inline constexpr FT66ReferenceRect FilterCrownButton{1445.0f, 253.0f, 66.0f, 62.0f};
		inline constexpr FT66ReferenceRect TabWeeklyActive{1258.0f, 334.0f, 182.0f, 56.0f};
		inline constexpr FT66ReferenceRect TabAllTimeInactive{1439.0f, 334.0f, 190.0f, 56.0f};
		inline constexpr FT66ReferenceRect DropdownShellLeft{1260.0f, 397.0f, 181.0f, 50.0f};
		inline constexpr FT66ReferenceRect DropdownShellRight{1444.0f, 397.0f, 184.0f, 50.0f};
		inline constexpr FT66ReferenceRect ToggleScoreSelected{1258.0f, 454.0f, 183.0f, 53.0f};
		inline constexpr FT66ReferenceRect ToggleSpeedrunUnselected{1444.0f, 454.0f, 188.0f, 53.0f};
		inline constexpr FT66ReferenceRect LeaderboardAvatarFrameSource{1335.0f, 550.0f, 49.0f, 49.0f};
	}

	namespace MainMenu
	{
		inline constexpr FT66ReferenceRect LeftPanelAssembly{18.0f, 304.0f, 424.0f, 633.0f};
		inline constexpr FT66ReferenceRect RightPanelAssembly{1245.0f, 253.0f, 399.0f, 685.0f};
	}

}