// Copyright Tribulation 66. All Rights Reserved.

#pragma once

namespace T66SettingsReferenceLayout
{
	inline constexpr float CanvasWidth = 1920.0f;
	inline constexpr float CanvasHeight = 1080.0f;
	inline constexpr float TopBarOverlapPx = 22.0f;
	inline constexpr float TabButtonHeight = 48.0f;
	inline constexpr float TabStripMinHeight = 72.0f;
	inline constexpr float CompactButtonHeight = 44.0f;
	inline constexpr float GenericRowMinHeight = 64.0f;
	inline constexpr float ToggleButtonMinWidth = 88.0f;
	inline constexpr float ConfirmButtonMinWidth = 100.0f;

	namespace TopBar
	{
		inline constexpr FT66ReferenceRect SettingsSurfaceRelativeTopBand{0.0f, 0.0f, 1920.0f, 72.0f};
		inline constexpr FT66ReferenceRect CloseButtonContract{0.0f, 0.0f, 36.0f, 36.0f};
	}

	namespace Center
	{
		inline constexpr FT66ReferenceRect SettingsSurfaceFull{0.0f, 0.0f, 1920.0f, 1080.0f};
		inline constexpr FT66ReferenceRect TabStripRelative{0.0f, 0.0f, 1920.0f, 72.0f};
		inline constexpr FT66ReferenceRect ContentShellRelative{8.0f, 72.0f, 1904.0f, 1000.0f};
	}

	namespace Left
	{
		// Settings currently uses a single wide body shell rather than distinct left-side panels.
	}

	namespace Right
	{
		// Settings currently uses a single wide body shell rather than distinct right-side panels.
	}

	namespace Decor
	{
		inline constexpr FT66ReferenceRect TogglePairMinCluster{0.0f, 0.0f, 180.0f, 44.0f};
		inline constexpr FT66ReferenceRect DropdownTriggerContract{0.0f, 0.0f, 0.0f, 44.0f};
		inline constexpr FT66ReferenceRect ConfirmDialogMinimumActionRow{0.0f, 0.0f, 224.0f, 56.0f};
	}
}
