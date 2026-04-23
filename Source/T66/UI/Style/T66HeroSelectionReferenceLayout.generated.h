// Copyright Tribulation 66. All Rights Reserved.

#pragma once

namespace T66HeroSelectionReferenceLayout
{
	inline constexpr float CanvasWidth = 1920.0f;
	inline constexpr float CanvasHeight = 1080.0f;
	inline constexpr float SidePanelWidth = 404.0f;
	inline constexpr float CenterColumnWidth = 1112.0f;
	inline constexpr float TopStripMinHeight = 60.0f;
	inline constexpr float CompanionStripMinHeight = 50.0f;
	inline constexpr float BodyToggleBlockMinHeight = 35.0f;
	inline constexpr float BottomPartyShellMinHeight = 162.0f;
	inline constexpr float BottomRunShellMinHeight = 84.0f;
	inline constexpr float RunPrimaryButtonHeight = 46.0f;

	namespace TopBar
	{
		inline constexpr FT66ReferenceRect HeroStripCenterBar{404.0f, 0.0f, 1112.0f, 60.0f};
	}

	namespace Center
	{
		inline constexpr FT66ReferenceRect ColumnFull{404.0f, 0.0f, 1112.0f, 1080.0f};
		inline constexpr FT66ReferenceRect PreviewViewportBand{404.0f, 60.0f, 1112.0f, 927.0f};
		inline constexpr FT66ReferenceRect CompanionStripBand{404.0f, 987.0f, 1112.0f, 50.0f};
		inline constexpr FT66ReferenceRect BodyToggleBand{404.0f, 1045.0f, 1112.0f, 35.0f};
	}

	namespace Left
	{
		inline constexpr FT66ReferenceRect ColumnFull{0.0f, 0.0f, 404.0f, 1080.0f};
		inline constexpr FT66ReferenceRect TopRowContract{0.0f, 0.0f, 404.0f, 64.0f};
		inline constexpr FT66ReferenceRect BottomPartyShellContract{0.0f, 918.0f, 404.0f, 162.0f};
	}

	namespace Right
	{
		inline constexpr FT66ReferenceRect ColumnFull{1516.0f, 0.0f, 404.0f, 1080.0f};
		inline constexpr FT66ReferenceRect TitleBand{1516.0f, 0.0f, 404.0f, 44.0f};
		inline constexpr FT66ReferenceRect InfoTargetBand{1516.0f, 54.0f, 404.0f, 36.0f};
		inline constexpr FT66ReferenceRect BottomRunShellContract{1516.0f, 996.0f, 404.0f, 84.0f};
	}

	namespace Decor
	{
		inline constexpr FT66ReferenceRect RightPreviewInfoCardContentHeight{0.0f, 0.0f, 0.0f, 204.0f};
		inline constexpr FT66ReferenceRect RightAbilityButtonSize{0.0f, 0.0f, 64.0f, 64.0f};
		inline constexpr FT66ReferenceRect RightAbilityIconBoxSize{0.0f, 0.0f, 42.0f, 42.0f};
	}
}
