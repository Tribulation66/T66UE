// Copyright Tribulation 66. All Rights Reserved.

#pragma once

namespace T66MainMenuReferenceLayout
{
	inline constexpr float CanvasWidth = 1920.0f;
	inline constexpr float CanvasHeight = 1080.0f;
	inline constexpr float TopBarReservedHeight = 163.0f;
	inline constexpr float TopBarSurfaceHeight = 145.0f;
	inline constexpr float TopBarSurfaceOffsetY = 0.0f;

	namespace TopBar
	{
		inline constexpr FT66ReferenceRect TopbarStripFull{0.0f, 0.0f, 1920.0f, 145.0f};
		inline constexpr FT66ReferenceRect ButtonSettings{14.0f, 18.0f, 110.0f, 106.0f};
		inline constexpr FT66ReferenceRect ButtonChat{145.0f, 18.0f, 110.0f, 106.0f};
		inline constexpr FT66ReferenceRect TabAccount{319.0f, 18.0f, 255.0f, 103.0f};
		inline constexpr FT66ReferenceRect BadgeProfile{598.0f, 14.0f, 91.0f, 123.0f};
		inline constexpr FT66ReferenceRect TabPowerUp{707.0f, 18.0f, 253.0f, 103.0f};
		inline constexpr FT66ReferenceRect TabAchievements{974.0f, 18.0f, 279.0f, 103.0f};
		inline constexpr FT66ReferenceRect TabMinigames{1268.0f, 18.0f, 257.0f, 103.0f};
		inline constexpr FT66ReferenceRect CurrencySlot{1572.0f, 26.0f, 190.0f, 86.0f};
		inline constexpr FT66ReferenceRect ButtonPower{1781.0f, 18.0f, 121.0f, 106.0f};
	}

	namespace Center
	{
		inline constexpr FT66ReferenceRect CenterBackdropFull{460.0f, 138.0f, 1003.0f, 942.0f};
		inline constexpr FT66ReferenceRect TitleLockup{459.0f, 138.0f, 1005.0f, 188.0f};
		inline constexpr FT66ReferenceRect SubtitleLockup{725.0f, 252.0f, 524.0f, 71.0f};
		inline constexpr FT66ReferenceRect HeroStage{475.0f, 298.0f, 975.0f, 416.0f};
		inline constexpr FT66ReferenceRect CtaStackFull{742.0f, 698.0f, 445.0f, 311.0f};
		inline constexpr FT66ReferenceRect CtaButtonNewGame{770.0f, 719.0f, 388.0f, 100.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGame{770.0f, 819.0f, 388.0f, 97.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallenge{770.0f, 916.0f, 388.0f, 92.0f};
	}

	namespace CenterRuntime
	{
		inline constexpr FT66ReferenceRect CtaButtonNewGamePlate{770.0f, 719.0f, 388.0f, 100.0f};
		inline constexpr FT66ReferenceRect CtaButtonLoadGamePlate{770.0f, 819.0f, 388.0f, 97.0f};
		inline constexpr FT66ReferenceRect CtaButtonDailyChallengePlate{770.0f, 916.0f, 388.0f, 92.0f};
	}

	namespace Left
	{
		inline constexpr FT66ReferenceRect ShellFullReference{21.0f, 349.0f, 487.0f, 726.0f};
		inline constexpr FT66ReferenceRect ProfileCardReference{40.0f, 360.0f, 424.0f, 108.0f};
		inline constexpr FT66ReferenceRect SearchFieldReference{46.0f, 490.0f, 404.0f, 54.0f};
		inline constexpr FT66ReferenceRect SearchIcon{405.0f, 499.0f, 34.0f, 32.0f};
		inline constexpr FT66ReferenceRect FriendStarButton{291.0f, 598.0f, 62.0f, 57.0f};
		inline constexpr FT66ReferenceRect FriendInviteButton{355.0f, 598.0f, 96.0f, 57.0f};
		inline constexpr FT66ReferenceRect FriendOfflineButton{355.0f, 771.0f, 97.0f, 58.0f};
		inline constexpr FT66ReferenceRect FriendAvatarFrameSource{45.0f, 596.0f, 55.0f, 56.0f};
		inline constexpr FT66ReferenceRect PartySlotSource{44.0f, 916.0f, 93.0f, 103.0f};
		inline constexpr FT66ReferenceRect CloseButton{415.0f, 862.0f, 51.0f, 48.0f};
	}

	namespace Right
	{
		inline constexpr FT66ReferenceRect ShellFullReference{1430.0f, 368.0f, 458.0f, 709.0f};
		inline constexpr FT66ReferenceRect FilterWorldButton{1497.0f, 290.0f, 74.0f, 72.0f};
		inline constexpr FT66ReferenceRect FilterFriendsButton{1578.0f, 290.0f, 74.0f, 72.0f};
		inline constexpr FT66ReferenceRect FilterCrownButton{1659.0f, 290.0f, 76.0f, 72.0f};
		inline constexpr FT66ReferenceRect TabWeeklyActive{1445.0f, 383.0f, 209.0f, 65.0f};
		inline constexpr FT66ReferenceRect TabAllTimeInactive{1652.0f, 383.0f, 219.0f, 65.0f};
		inline constexpr FT66ReferenceRect DropdownShellLeft{1447.0f, 456.0f, 208.0f, 57.0f};
		inline constexpr FT66ReferenceRect DropdownShellRight{1658.0f, 456.0f, 211.0f, 57.0f};
		inline constexpr FT66ReferenceRect ToggleScoreSelected{1445.0f, 521.0f, 210.0f, 61.0f};
		inline constexpr FT66ReferenceRect ToggleSpeedrunUnselected{1658.0f, 521.0f, 216.0f, 61.0f};
		inline constexpr FT66ReferenceRect LeaderboardAvatarFrameSource{1533.0f, 631.0f, 56.0f, 56.0f};
		inline constexpr FT66ReferenceRect LeaderboardAvatarLiveRect{1541.0f, 639.0f, 40.0f, 40.0f};
	}

	namespace MainMenu
	{
		inline constexpr FT66ReferenceRect FullCanvas{0.0f, 0.0f, 1920.0f, 1080.0f};
		inline constexpr FT66ReferenceRect LeftPanelAssembly{21.0f, 349.0f, 487.0f, 726.0f};
		inline constexpr FT66ReferenceRect RightPanelAssembly{1430.0f, 290.0f, 458.0f, 787.0f};
	}

}
