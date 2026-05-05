// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66TDBattleScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66TDDataSubsystem.h"
#include "Core/T66TDFrontendStateSubsystem.h"
#include "Core/T66TDVisualSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Engine/GameInstance.h"
#include "Data/T66TDDataTypes.h"
#include "Engine/Texture2D.h"
#include "Input/DragAndDrop.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"
#include "Save/T66TDRunSaveGame.h"
#include "Save/T66TDSaveSubsystem.h"
#include "Styling/CoreStyle.h"
#include "UI/T66TDUIStyle.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UITypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FVector2D TDBattleBoardSize(960.f, 540.f);

	void EnsureBrush(const TSharedPtr<FSlateBrush>& Brush, const FVector2D& ImageSize)
	{
		if (!Brush.IsValid())
		{
			return;
		}

		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = ImageSize;
	}

	TSharedPtr<FSlateBrush> MakeTextureBrush(UTexture2D* Texture, const FVector2D& ImageSize)
	{
		if (!Texture)
		{
			return nullptr;
		}

		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->SetResourceObject(Texture);
		EnsureBrush(Brush, ImageSize);
		return Brush;
	}

	TSharedRef<SWidget> MakeOptionalImage(const TSharedPtr<FSlateBrush>& Brush)
	{
		if (!Brush.IsValid() || !Brush->GetResourceObject())
		{
			return SNew(SBox);
		}

		return SNew(SScaleBox)
			.Stretch(EStretch::Fill)
			[
				SNew(SImage)
				.Image(Brush.Get())
				.ColorAndOpacity(FLinearColor::White)
			];
	}

	const FSlateBrush* ResolveBattleBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const FSlateBrush* TDBattleRosterPanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveBattleBrush(
			Entry,
			T66TDUI::MasterBasicPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDBattleMasterRosterPanel"));
	}

	const FSlateBrush* TDBattleRosterRowBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveBattleBrush(
			Entry,
			T66TDUI::MasterInnerPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDBattleMasterRosterRow"));
	}

	const FSlateBrush* TDBattleStatsBarBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveBattleBrush(
			Entry,
			T66TDUI::MasterInnerPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDBattleMasterStatsBar"));
	}

	const FSlateBrush* TDBattleBoardFrameBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveBattleBrush(
			Entry,
			T66TDUI::MasterBasicPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDBattleMasterBoardFrame"));
	}

	const FSlateBrush* TDBattleStatusBarBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveBattleBrush(
			Entry,
			T66TDUI::MasterInnerPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDBattleMasterStatusBar"));
	}

	const FSlateBrush* TDBattleMatchPanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveBattleBrush(
			Entry,
			T66TDUI::MasterBasicPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDBattleMasterMatchPanel"));
	}

	const FSlateBrush* TDBattleUpgradePanelBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Entry;
		return ResolveBattleBrush(
			Entry,
			T66TDUI::MasterBasicPanelPath(),
			T66TDUI::MasterPanelMargin(),
			TEXT("TDBattleMasterUpgradePanel"));
	}

	const FSlateBrush* TDBattleButtonBrush(const ET66ButtonType Type)
	{
		return T66TDUI::ButtonPlateBrush(Type);
	}

	TSharedRef<SWidget> MakeSpriteWidget(const TSharedPtr<FSlateBrush>& Brush, const FString& FallbackText, const FLinearColor& PlaceholderColor, const FVector2D& Size, const int32 FontSize)
	{
		TSharedRef<SWidget> SpriteContent = SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(PlaceholderColor)
			.Padding(FMargin(10.f, 8.f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(FallbackText))
				.Font(FT66Style::MakeFont(TEXT("Black"), FontSize))
				.ColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.06f, 1.0f))
				.Justification(ETextJustify::Center)
			];

		if (Brush.IsValid() && Brush->GetResourceObject())
		{
			SpriteContent =
				SNew(SImage)
				.Image(Brush.Get())
				.ColorAndOpacity(FLinearColor::White);
		}

		return SNew(SBox)
			.WidthOverride(Size.X)
			.HeightOverride(Size.Y)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.07f, 0.08f, 0.92f))
				.Padding(FMargin(2.f))
				[
					SpriteContent
				]
			];
	}

	FVector2D SafeNormalizePoint(const FVector2D& LocalPoint, const FVector2D& LocalSize)
	{
		return FVector2D(
			FMath::Clamp(LocalSize.X > KINDA_SMALL_NUMBER ? LocalPoint.X / LocalSize.X : 0.f, 0.f, 1.f),
			FMath::Clamp(LocalSize.Y > KINDA_SMALL_NUMBER ? LocalPoint.Y / LocalSize.Y : 0.f, 0.f, 1.f));
	}

	FVector2D ToLocalPoint(const FVector2D& NormalizedPoint, const FVector2D& LocalSize)
	{
		return FVector2D(NormalizedPoint.X * LocalSize.X, NormalizedPoint.Y * LocalSize.Y);
	}

	float DistanceSquared(const FVector2D& A, const FVector2D& B)
	{
		return FVector2D::DistSquared(A, B);
	}

	struct FT66TDHeroCombatProfile
	{
		int32 Cost = 0;
		float Damage = 0.f;
		float Range = 0.f;
		float FireInterval = 0.f;
		int32 ChainBounces = 0;
		float ChainRadius = 0.f;
		float SplashRadius = 0.0f;
		float DotDamagePerSecond = 0.0f;
		float DotDuration = 0.0f;
		float SlowMultiplier = 1.0f;
		float SlowDuration = 0.0f;
		float BossDamageMultiplier = 1.0f;
		float FlatArmorPierce = 0.0f;
		float ShieldDamageMultiplier = 1.0f;
		bool bCanTargetHidden = false;
		bool bPrioritizeBoss = false;
		int32 VolleyShots = 1;
		int32 BonusMaterialsOnKill = 0;
		FString CombatLabel;
	};

	enum class ET66TDEnemyFamily : uint8
	{
		Roost,
		Goat,
		Cow,
		Pig,
		Boss
	};

	enum class ET66TDEnemyModifier : uint8
	{
		None = 0,
		Hidden = 1 << 0,
		Armored = 1 << 1,
		Regenerating = 1 << 2,
		Shielded = 1 << 3
	};
	ENUM_CLASS_FLAGS(ET66TDEnemyModifier);

	ET66TDEnemyModifier BuildEnemyModifierMask(const int32 ModifierMask)
	{
		ET66TDEnemyModifier Modifiers = ET66TDEnemyModifier::None;
		if ((ModifierMask & static_cast<int32>(ET66TDEnemyModifier::Hidden)) != 0)
		{
			Modifiers |= ET66TDEnemyModifier::Hidden;
		}
		if ((ModifierMask & static_cast<int32>(ET66TDEnemyModifier::Armored)) != 0)
		{
			Modifiers |= ET66TDEnemyModifier::Armored;
		}
		if ((ModifierMask & static_cast<int32>(ET66TDEnemyModifier::Regenerating)) != 0)
		{
			Modifiers |= ET66TDEnemyModifier::Regenerating;
		}
		if ((ModifierMask & static_cast<int32>(ET66TDEnemyModifier::Shielded)) != 0)
		{
			Modifiers |= ET66TDEnemyModifier::Shielded;
		}
		return Modifiers;
	}

	struct FT66TDEnemyArchetype
	{
		FString DisplayName;
		FString VisualID;
		float BaseHealth = 0.f;
		float BaseSpeed = 0.f;
		int32 LeakDamage = 0;
		int32 Bounty = 0;
		float Radius = 0.f;
		FLinearColor Tint = FLinearColor::White;
	};

	struct FT66TDPathRuntime
	{
		TArray<FVector2D> Points;
		TArray<float> SegmentLengths;
		float TotalLength = 0.f;
	};

	struct FT66TDQueuedSpawn
	{
		ET66TDEnemyFamily Family = ET66TDEnemyFamily::Roost;
		int32 LaneIndex = 0;
		float SpawnDelay = 0.50f;
		float HealthScalar = 1.0f;
		float SpeedScalar = 1.0f;
		ET66TDEnemyModifier Modifiers = ET66TDEnemyModifier::None;
		bool bBoss = false;
	};

	struct FT66TDBeamEffect
	{
		FVector2D StartNormalized = FVector2D::ZeroVector;
		FVector2D EndNormalized = FVector2D::ZeroVector;
		FLinearColor Tint = FLinearColor::White;
		float RemainingTime = 0.08f;
		float TotalLifetime = 0.08f;
		float Thickness = 2.0f;
	};

	struct FT66TDPlacedTower
	{
		FName HeroID = NAME_None;
		FString DisplayName;
		FLinearColor Tint = FLinearColor::White;
		FT66TDHeroCombatProfile BaseProfile;
		FT66TDHeroCombatProfile Profile;
		int32 PadIndex = INDEX_NONE;
		FVector2D PositionNormalized = FVector2D::ZeroVector;
		float Cooldown = 0.f;
		int32 Kills = 0;
		int32 MaterialsInvested = 0;
		int32 DamageUpgradeLevel = 0;
		int32 RangeUpgradeLevel = 0;
		int32 TempoUpgradeLevel = 0;
	};

	struct FT66TDActiveEnemy
	{
		ET66TDEnemyFamily Family = ET66TDEnemyFamily::Roost;
		FString DisplayName;
		FString VisualID;
		int32 LaneIndex = 0;
		float ProgressRatio = 0.f;
		float Speed = 0.16f;
		float Health = 40.f;
		float MaxHealth = 40.f;
		float Radius = 0.012f;
		float SlowMultiplier = 1.f;
		float SlowRemaining = 0.f;
		float DotDamagePerSecond = 0.f;
		float DotRemaining = 0.f;
		float Armor = 0.f;
		float Shield = 0.f;
		float MaxShield = 0.f;
		float RegenPerSecond = 0.f;
		int32 LeakDamage = 1;
		int32 Bounty = 6;
		FLinearColor Tint = FLinearColor::White;
		ET66TDEnemyModifier Modifiers = ET66TDEnemyModifier::None;
		bool bBoss = false;
		bool bPendingRemoval = false;
	};

	FT66TDHeroCombatProfile BuildHeroProfile(const FT66TDHeroDefinition& HeroDefinition, const FT66TDHeroCombatDefinition* CombatDefinition)
	{
		FT66TDHeroCombatProfile Profile;
		Profile.CombatLabel = HeroDefinition.RoleLabel;
		if (CombatDefinition)
		{
			Profile.Cost = CombatDefinition->Cost;
			Profile.Damage = CombatDefinition->Damage;
			Profile.Range = CombatDefinition->Range;
			Profile.FireInterval = CombatDefinition->FireInterval;
			Profile.ChainBounces = CombatDefinition->ChainBounces;
			Profile.ChainRadius = CombatDefinition->ChainRadius;
			Profile.SplashRadius = CombatDefinition->SplashRadius;
			Profile.DotDamagePerSecond = CombatDefinition->DotDamagePerSecond;
			Profile.DotDuration = CombatDefinition->DotDuration;
			Profile.SlowMultiplier = CombatDefinition->SlowMultiplier;
			Profile.SlowDuration = CombatDefinition->SlowDuration;
			Profile.BossDamageMultiplier = CombatDefinition->BossDamageMultiplier;
			Profile.FlatArmorPierce = CombatDefinition->FlatArmorPierce;
			Profile.ShieldDamageMultiplier = CombatDefinition->ShieldDamageMultiplier;
			Profile.bCanTargetHidden = CombatDefinition->bCanTargetHidden;
			Profile.bPrioritizeBoss = CombatDefinition->bPrioritizeBoss;
			Profile.VolleyShots = FMath::Max(1, CombatDefinition->VolleyShots);
			Profile.BonusMaterialsOnKill = CombatDefinition->BonusMaterialsOnKill;
			if (!CombatDefinition->CombatLabel.IsEmpty())
			{
				Profile.CombatLabel = CombatDefinition->CombatLabel;
			}
		}
		return Profile;
	}

	FName GetEnemyFamilyID(const ET66TDEnemyFamily Family)
	{
		switch (Family)
		{
		case ET66TDEnemyFamily::Goat:
			return FName(TEXT("Goat"));
		case ET66TDEnemyFamily::Cow:
			return FName(TEXT("Cow"));
		case ET66TDEnemyFamily::Pig:
			return FName(TEXT("Pig"));
		case ET66TDEnemyFamily::Boss:
			return FName(TEXT("Boss"));
		case ET66TDEnemyFamily::Roost:
		default:
			return FName(TEXT("Roost"));
		}
	}

	FT66TDEnemyArchetype BuildEnemyArchetype(const FT66TDEnemyArchetypeDefinition& Definition)
	{
		FT66TDEnemyArchetype Archetype;
		Archetype.DisplayName = Definition.DisplayName;
		Archetype.VisualID = Definition.VisualID;
		Archetype.BaseHealth = Definition.BaseHealth;
		Archetype.BaseSpeed = Definition.BaseSpeed;
		Archetype.LeakDamage = Definition.LeakDamage;
		Archetype.Bounty = Definition.Bounty;
		Archetype.Radius = Definition.Radius;
		Archetype.Tint = Definition.Tint;
		return Archetype;
	}

	bool HasEnemyModifier(const ET66TDEnemyModifier Source, const ET66TDEnemyModifier Modifier)
	{
		return EnumHasAllFlags(Source, Modifier);
	}

	FString GetEnemyModifierLabel(const ET66TDEnemyModifier Modifiers)
	{
		TArray<FString> Labels;
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Hidden))
		{
			Labels.Add(TEXT("Hidden"));
		}
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Armored))
		{
			Labels.Add(TEXT("Armored"));
		}
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Regenerating))
		{
			Labels.Add(TEXT("Regen"));
		}
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Shielded))
		{
			Labels.Add(TEXT("Shielded"));
		}

		return Labels.Num() > 0 ? FString::Join(Labels, TEXT(" | ")) : TEXT("Baseline");
	}

	FLinearColor GetEnemyTint(const FT66TDEnemyArchetype& Archetype, const ET66TDEnemyModifier Modifiers, const bool bBoss)
	{
		FLinearColor Tint = Archetype.Tint;
		if (bBoss)
		{
			Tint = FMath::Lerp(Tint, FLinearColor(0.99f, 0.84f, 0.34f, 1.0f), 0.28f);
		}
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Hidden))
		{
			Tint = FMath::Lerp(Tint, FLinearColor(0.24f, 0.18f, 0.34f, 1.0f), 0.45f);
		}
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Armored))
		{
			Tint = FMath::Lerp(Tint, FLinearColor(0.74f, 0.62f, 0.28f, 1.0f), 0.28f);
		}
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Regenerating))
		{
			Tint = FMath::Lerp(Tint, FLinearColor(0.30f, 0.76f, 0.44f, 1.0f), 0.24f);
		}
		if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Shielded))
		{
			Tint = FMath::Lerp(Tint, FLinearColor(0.32f, 0.70f, 0.94f, 1.0f), 0.34f);
		}
		return Tint;
	}

	FString GetTowerCounterSummary(const FT66TDHeroCombatProfile& Profile)
	{
		TArray<FString> Counters;
		if (Profile.bCanTargetHidden)
		{
			Counters.Add(TEXT("Hidden"));
		}
		if (Profile.FlatArmorPierce > 0.f)
		{
			Counters.Add(FString::Printf(TEXT("Armor +%d"), FMath::RoundToInt(Profile.FlatArmorPierce)));
		}
		if (Profile.ShieldDamageMultiplier > 1.01f)
		{
			Counters.Add(FString::Printf(TEXT("Shield x%.2f"), Profile.ShieldDamageMultiplier));
		}

		return Counters.Num() > 0 ? FString::Join(Counters, TEXT("  |  ")) : TEXT("Counters: baseline lane pressure");
	}

	FVector2D SamplePathPoint(const FT66TDPathRuntime& Path, const float Ratio)
	{
		if (Path.Points.Num() <= 0)
		{
			return FVector2D::ZeroVector;
		}
		if (Path.Points.Num() == 1 || Path.TotalLength <= KINDA_SMALL_NUMBER)
		{
			return Path.Points[0];
		}

		const float ClampedRatio = FMath::Clamp(Ratio, 0.f, 1.f);
		const float TargetDistance = Path.TotalLength * ClampedRatio;
		float WalkedDistance = 0.f;

		for (int32 SegmentIndex = 0; SegmentIndex < Path.SegmentLengths.Num(); ++SegmentIndex)
		{
			const float SegmentLength = Path.SegmentLengths[SegmentIndex];
			if (WalkedDistance + SegmentLength >= TargetDistance || SegmentIndex == Path.SegmentLengths.Num() - 1)
			{
				const FVector2D Start = Path.Points[SegmentIndex];
				const FVector2D End = Path.Points[SegmentIndex + 1];
				const float LocalAlpha = SegmentLength <= KINDA_SMALL_NUMBER ? 0.f : (TargetDistance - WalkedDistance) / SegmentLength;
				return FMath::Lerp(Start, End, LocalAlpha);
			}

			WalkedDistance += SegmentLength;
		}

		return Path.Points.Last();
	}

	void BuildCirclePoints(const FVector2D& Center, const float Radius, const int32 SegmentCount, TArray<FVector2f>& OutPoints)
	{
		OutPoints.Reset();
		OutPoints.Reserve(SegmentCount + 1);
		for (int32 Index = 0; Index <= SegmentCount; ++Index)
		{
			const float Angle = (static_cast<float>(Index) / static_cast<float>(SegmentCount)) * PI * 2.f;
			OutPoints.Add(FVector2f(
				static_cast<float>(Center.X + (FMath::Cos(Angle) * Radius)),
				static_cast<float>(Center.Y + (FMath::Sin(Angle) * Radius))));
		}
	}

	class FT66TDHeroDragDropOp : public FDragDropOperation
	{
	public:
		DRAG_DROP_OPERATOR_TYPE(FT66TDHeroDragDropOp, FDragDropOperation)

		FName HeroID = NAME_None;
		FString DisplayName;
		int32 Cost = 0;
		FLinearColor Tint = FLinearColor::White;

		static TSharedRef<FT66TDHeroDragDropOp> New(const FName HeroID, const FString& DisplayName, const int32 Cost, const FLinearColor& Tint)
		{
			TSharedRef<FT66TDHeroDragDropOp> Op = MakeShared<FT66TDHeroDragDropOp>();
			Op->HeroID = HeroID;
			Op->DisplayName = DisplayName;
			Op->Cost = Cost;
			Op->Tint = Tint;
			Op->Construct();
			return Op;
		}

		virtual void Construct() override
		{
			MouseCursor = EMouseCursor::GrabHandClosed;
			Decorator =
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.96f))
				.Padding(FMargin(12.f, 10.f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(Tint)
						.Padding(FMargin(10.f, 8.f))
						[
							SNew(STextBlock)
							.Text(FText::FromString(DisplayName.Left(1).ToUpper()))
							.Font(FT66Style::MakeFont(TEXT("Black"), 16))
							.ColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.06f, 1.0f))
						]
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(DisplayName))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 15))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::Format(NSLOCTEXT("T66TD.Battle", "DragCostFmt", "{0} materials"), FText::AsNumber(Cost)))
							.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
							.ColorAndOpacity(FLinearColor(0.88f, 0.90f, 0.92f, 1.0f))
						]
					]
				];

			FDragDropOperation::Construct();
		}

		virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
		{
			return Decorator;
		}

	private:
		TSharedPtr<SWidget> Decorator;
	};

	class ST66TDHeroRosterTile : public SBorder
	{
	public:
		SLATE_BEGIN_ARGS(ST66TDHeroRosterTile)
			: _HeroID(NAME_None)
			, _DisplayName()
			, _Cost(0)
			, _Tint(FLinearColor::White)
			, _BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			, _BorderBackgroundColor(FLinearColor::White)
			, _Padding(FMargin(0.f))
		{}
			SLATE_ARGUMENT(FName, HeroID)
			SLATE_ARGUMENT(FString, DisplayName)
			SLATE_ARGUMENT(int32, Cost)
			SLATE_ARGUMENT(FLinearColor, Tint)
			SLATE_ARGUMENT(const FSlateBrush*, BorderImage)
			SLATE_ARGUMENT(FSlateColor, BorderBackgroundColor)
			SLATE_ARGUMENT(FMargin, Padding)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			HeroID = InArgs._HeroID;
			DisplayName = InArgs._DisplayName;
			Cost = InArgs._Cost;
			Tint = InArgs._Tint;
			SBorder::Construct(
				SBorder::FArguments()
				.BorderImage(InArgs._BorderImage)
				.BorderBackgroundColor(InArgs._BorderBackgroundColor)
				.Padding(InArgs._Padding)
				[
					InArgs._Content.Widget
				]);
		}

		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
			}
			return SBorder::OnMouseButtonDown(MyGeometry, MouseEvent);
		}

		virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (HeroID == NAME_None)
			{
				return FReply::Unhandled();
			}
			return FReply::Handled().BeginDragDrop(FT66TDHeroDragDropOp::New(HeroID, DisplayName, Cost, Tint));
		}

	private:
		FName HeroID = NAME_None;
		FString DisplayName;
		int32 Cost = 0;
		FLinearColor Tint = FLinearColor::White;
	};

	enum class ET66TDMatchState : uint8
	{
		AwaitingWave,
		WaveActive,
		Victory,
		Defeat
	};

	enum class ET66TDTowerUpgradeType : uint8
	{
		Damage,
		Range,
		Tempo
	};

	using FT66TDHeroBrushMap = TMap<FName, TSharedPtr<FSlateBrush>>;
	using FT66TDEnemyBrushMap = TMap<FString, TSharedPtr<FSlateBrush>>;
	using FT66TDBattleTuningMap = TMap<FName, float>;

	class ST66TDBattleBoardWidget : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66TDBattleBoardWidget)
			: _MapDefinition()
			, _DifficultyDefinition()
			, _LayoutDefinition()
			, _StageDefinition()
			, _Heroes()
			, _HeroCombatDefinitions()
			, _EnemyArchetypes()
			, _BattleTuningValues()
			, _ThemeModifierRules()
			, _HeroBrushes()
			, _EnemyBrushes()
			, _BossBrushes()
			, _OwningGameInstance(nullptr)
		{}
			SLATE_ARGUMENT(FT66TDMapDefinition, MapDefinition)
			SLATE_ARGUMENT(FT66TDDifficultyDefinition, DifficultyDefinition)
			SLATE_ARGUMENT(FT66TDMapLayoutDefinition, LayoutDefinition)
			SLATE_ARGUMENT(FT66TDStageDefinition, StageDefinition)
			SLATE_ARGUMENT(TArray<FT66TDHeroDefinition>, Heroes)
			SLATE_ARGUMENT(TArray<FT66TDHeroCombatDefinition>, HeroCombatDefinitions)
			SLATE_ARGUMENT(TArray<FT66TDEnemyArchetypeDefinition>, EnemyArchetypes)
			SLATE_ARGUMENT(FT66TDBattleTuningMap, BattleTuningValues)
			SLATE_ARGUMENT(TArray<FT66TDThemeModifierRule>, ThemeModifierRules)
			SLATE_ARGUMENT(FT66TDHeroBrushMap, HeroBrushes)
			SLATE_ARGUMENT(FT66TDEnemyBrushMap, EnemyBrushes)
			SLATE_ARGUMENT(FT66TDEnemyBrushMap, BossBrushes)
			SLATE_ARGUMENT(TWeakObjectPtr<UGameInstance>, OwningGameInstance)
		SLATE_END_ARGS()

		~ST66TDBattleBoardWidget() override
		{
			if (ActiveTimerHandle.IsValid())
			{
				UnRegisterActiveTimer(ActiveTimerHandle.ToSharedRef());
				ActiveTimerHandle.Reset();
			}
		}

		void Construct(const FArguments& InArgs)
		{
			MapDefinition = InArgs._MapDefinition;
			DifficultyDefinition = InArgs._DifficultyDefinition;
			LayoutDefinition = InArgs._LayoutDefinition;
			StageDefinition = InArgs._StageDefinition;
			HeroDefinitions = InArgs._Heroes;
			HeroCombatDefinitions = InArgs._HeroCombatDefinitions;
			EnemyArchetypeDefinitions = InArgs._EnemyArchetypes;
			BattleTuningValues = InArgs._BattleTuningValues;
			ThemeModifierRules = InArgs._ThemeModifierRules;
			HeroBrushes = InArgs._HeroBrushes;
			EnemyBrushes = InArgs._EnemyBrushes;
			BossBrushes = InArgs._BossBrushes;
			OwningGameInstance = InArgs._OwningGameInstance;

			for (const FT66TDHeroCombatDefinition& CombatDefinition : HeroCombatDefinitions)
			{
				HeroCombatLookup.Add(CombatDefinition.HeroID, CombatDefinition);
			}
			for (const FT66TDEnemyArchetypeDefinition& ArchetypeDefinition : EnemyArchetypeDefinitions)
			{
				EnemyArchetypeLookup.Add(ArchetypeDefinition.EnemyID, BuildEnemyArchetype(ArchetypeDefinition));
			}
			for (const FT66TDThemeModifierRule& Rule : ThemeModifierRules)
			{
				ThemeModifierLookup.Add(FName(*Rule.ThemeLabel), Rule);
			}
			for (const FT66TDHeroDefinition& HeroDefinition : HeroDefinitions)
			{
				HeroLookup.Add(HeroDefinition.HeroID, HeroDefinition);
				HeroProfiles.Add(HeroDefinition.HeroID, BuildHeroProfile(HeroDefinition, HeroCombatLookup.Find(HeroDefinition.HeroID)));
			}

			for (const TArray<FVector2D>& PathPoints : LayoutDefinition.Paths)
			{
				FT66TDPathRuntime PathRuntime;
				PathRuntime.Points = PathPoints;
				PathRuntime.TotalLength = 0.f;

				for (int32 Index = 0; Index + 1 < PathRuntime.Points.Num(); ++Index)
				{
					const float SegmentLength = FVector2D::Distance(PathRuntime.Points[Index], PathRuntime.Points[Index + 1]);
					PathRuntime.SegmentLengths.Add(SegmentLength);
					PathRuntime.TotalLength += SegmentLength;
				}

				if (PathRuntime.Points.Num() >= 2 && PathRuntime.TotalLength > KINDA_SMALL_NUMBER)
				{
					PathRuntimes.Add(MoveTemp(PathRuntime));
				}
			}

			ResetMatch();
			if (FParse::Param(FCommandLine::Get(), TEXT("T66TDAutoStartWave")))
			{
				StartNextWave();
			}
			ActiveTimerHandle = RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &ST66TDBattleBoardWidget::HandleActiveTimer));
		}

		virtual FVector2D ComputeDesiredSize(const float) const override
		{
			return TDBattleBoardSize;
		}

		FText GetMaterialsText() const
		{
			return FText::Format(NSLOCTEXT("T66TD.Battle", "MaterialsFmt", "Materials {0}"), FText::AsNumber(Materials));
		}

		FText GetHeartsText() const
		{
			return FText::Format(NSLOCTEXT("T66TD.Battle", "HeartsFmt", "Hearts {0}"), FText::AsNumber(Hearts));
		}

		FText GetWaveText() const
		{
			return FText::Format(NSLOCTEXT("T66TD.Battle", "WaveFmt", "Wave {0}/{1}"), FText::AsNumber(CurrentWave), FText::AsNumber(FMath::Max(1, MapDefinition.BossWave)));
		}

		FText GetThreatText() const
		{
			const int32 PreviewWave = MatchState == ET66TDMatchState::WaveActive ? CurrentWave : (CurrentWave + 1);
			TArray<FString> Threats;
			auto AppendModifierThreats = [&Threats](const ET66TDEnemyModifier Modifiers)
			{
				if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Hidden))
				{
					Threats.Add(TEXT("Hidden"));
				}
				if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Armored))
				{
					Threats.Add(TEXT("Armored"));
				}
				if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Regenerating))
				{
					Threats.Add(TEXT("Regen"));
				}
				if (HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Shielded))
				{
					Threats.Add(TEXT("Shielded"));
				}
			};

			if (MatchState == ET66TDMatchState::WaveActive)
			{
				for (const FT66TDActiveEnemy& Enemy : Enemies)
				{
					if (Enemy.bPendingRemoval)
					{
						continue;
					}

					if (Enemy.bBoss)
					{
						Threats.Add(TEXT("Boss on lane"));
					}

					AppendModifierThreats(Enemy.Modifiers);
				}
			}
			else
			{
				FRandomStream ThreatStream(static_cast<int32>(GetTypeHash(MapDefinition.MapID) ^ (PreviewWave * 977)));
				const int32 ThreatSampleCount = FMath::Max(1, GetTuningInt(TEXT("ThreatPreviewSampleCount"), 16));
				const int32 ThreatBossSampleIndex = FMath::Clamp(GetTuningInt(TEXT("ThreatPreviewBossSampleIndex"), ThreatSampleCount - 1), 0, ThreatSampleCount - 1);
				for (int32 SampleIndex = 0; SampleIndex < ThreatSampleCount; ++SampleIndex)
				{
					const ET66TDEnemyFamily Family =
						(PreviewWave >= MapDefinition.BossWave && SampleIndex == ThreatBossSampleIndex)
						? ET66TDEnemyFamily::Boss
						: (SampleIndex % 4 == 0 ? ET66TDEnemyFamily::Roost : (SampleIndex % 4 == 1 ? ET66TDEnemyFamily::Goat : (SampleIndex % 4 == 2 ? ET66TDEnemyFamily::Cow : ET66TDEnemyFamily::Pig)));
					const ET66TDEnemyModifier Modifiers = BuildSpawnModifiers(Family, PreviewWave, SampleIndex, Family == ET66TDEnemyFamily::Boss, ThreatStream);
					if (Family == ET66TDEnemyFamily::Boss)
					{
						Threats.Add(TEXT("Boss check"));
					}

					AppendModifierThreats(Modifiers);
				}
			}

			Threats.Sort();
			for (int32 ThreatIndex = Threats.Num() - 1; ThreatIndex > 0; --ThreatIndex)
			{
				if (Threats[ThreatIndex].Equals(Threats[ThreatIndex - 1], ESearchCase::CaseSensitive))
				{
					Threats.RemoveAt(ThreatIndex);
				}
			}

			const FString ThreatBody = Threats.Num() > 0 ? FString::Join(Threats, TEXT("  |  ")) : TEXT("Baseline lane pressure");
			return FText::FromString(FString::Printf(TEXT("Wave Threats: %s"), *ThreatBody));
		}

		FText GetStatusText() const
		{
			switch (MatchState)
			{
			case ET66TDMatchState::Victory:
				return NSLOCTEXT("T66TD.Battle", "StatusVictory", "Victory: the core survived.");
			case ET66TDMatchState::Defeat:
				return NSLOCTEXT("T66TD.Battle", "StatusDefeat", "Defeat: leaks reached the core.");
			case ET66TDMatchState::WaveActive:
				return FText::Format(
					NSLOCTEXT("T66TD.Battle", "StatusWaveLive", "Wave {0} is live. Drag heroes onto open pads while the lane is hot."),
					FText::AsNumber(CurrentWave));
			case ET66TDMatchState::AwaitingWave:
			default:
				return FText::Format(
					NSLOCTEXT("T66TD.Battle", "StatusAwaiting", "Ready for wave {0}. Drag heroes from the roster, then start the wave."),
					FText::AsNumber(CurrentWave + 1));
			}
		}

		FText GetPrimaryActionText() const
		{
			if (MatchState == ET66TDMatchState::Victory || MatchState == ET66TDMatchState::Defeat)
			{
				return NSLOCTEXT("T66TD.Battle", "RestartMatch", "RESTART MATCH");
			}
			if (MatchState == ET66TDMatchState::WaveActive)
			{
				return NSLOCTEXT("T66TD.Battle", "WaveRunning", "WAVE RUNNING");
			}
			return FText::Format(
				NSLOCTEXT("T66TD.Battle", "StartWaveFmt", "START WAVE {0}"),
				FText::AsNumber(CurrentWave + 1));
		}

		FText GetSpeedText() const
		{
			return FText::Format(NSLOCTEXT("T66TD.Battle", "SpeedFmt", "SPEED x{0}"), FText::AsNumber(FMath::RoundToInt(SimulationSpeed)));
		}

		FText GetSelectedTowerTitle() const
		{
			const FT66TDPlacedTower* SelectedTower = FindTowerByPad(SelectedPadIndex);
			return SelectedTower
				? FText::FromString(SelectedTower->DisplayName.ToUpper())
				: NSLOCTEXT("T66TD.Battle", "NoTowerSelected", "NO HERO SELECTED");
		}

		FText GetSelectedTowerBody() const
		{
			const FT66TDPlacedTower* SelectedTower = FindTowerByPad(SelectedPadIndex);
			if (!SelectedTower)
			{
				return NSLOCTEXT("T66TD.Battle", "NoTowerBody", "Drag a hero from the roster onto an empty pad. Right-click any placed hero to sell it for 70% of its cost.");
			}

			const FString SpecialLine = FString::Printf(
				TEXT("Boss x%.2f  |  Volley %d"),
				SelectedTower->Profile.BossDamageMultiplier,
				SelectedTower->Profile.VolleyShots);
			const FString CounterLine = GetTowerCounterSummary(SelectedTower->Profile);

			const FString Body = FString::Printf(
				TEXT("%s\nDamage %d  |  Range %d\nAttack every %.2fs  |  Kills %d\nInvested %d  |  Sell %d\n%s\n%s\n%s\n%s\n%s"),
				*SelectedTower->Profile.CombatLabel,
				FMath::RoundToInt(SelectedTower->Profile.Damage),
				FMath::RoundToInt(SelectedTower->Profile.Range * 1000.f),
				SelectedTower->Profile.FireInterval,
				SelectedTower->Kills,
				SelectedTower->MaterialsInvested,
				GetTowerSellValue(*SelectedTower),
				*SpecialLine,
				*CounterLine,
				*GetUpgradeTrackSummaryString(*SelectedTower, ET66TDTowerUpgradeType::Damage),
				*GetUpgradeTrackSummaryString(*SelectedTower, ET66TDTowerUpgradeType::Range),
				*GetUpgradeTrackSummaryString(*SelectedTower, ET66TDTowerUpgradeType::Tempo));

			return FText::FromString(Body);
		}

		FText GetDamageUpgradeText() const
		{
			return GetUpgradeButtonText(ET66TDTowerUpgradeType::Damage);
		}

		FText GetRangeUpgradeText() const
		{
			return GetUpgradeButtonText(ET66TDTowerUpgradeType::Range);
		}

		FText GetTempoUpgradeText() const
		{
			return GetUpgradeButtonText(ET66TDTowerUpgradeType::Tempo);
		}

		FText GetSellSelectedTowerText() const
		{
			const FT66TDPlacedTower* SelectedTower = FindTowerByPad(SelectedPadIndex);
			if (!SelectedTower)
			{
				return NSLOCTEXT("T66TD.Battle", "SellHeroIdle", "SELL HERO");
			}

			return FText::Format(
				NSLOCTEXT("T66TD.Battle", "SellHeroFmt", "SELL HERO  |  +{0}"),
				FText::AsNumber(GetTowerSellValue(*SelectedTower)));
		}

		bool CanUpgradeDamage() const
		{
			return CanUpgradeSelectedTower(ET66TDTowerUpgradeType::Damage);
		}

		bool CanUpgradeRange() const
		{
			return CanUpgradeSelectedTower(ET66TDTowerUpgradeType::Range);
		}

		bool CanUpgradeTempo() const
		{
			return CanUpgradeSelectedTower(ET66TDTowerUpgradeType::Tempo);
		}

		bool CanSellSelectedTower() const
		{
			return FindTowerByPad(SelectedPadIndex) != nullptr;
		}

		FReply HandlePrimaryActionClicked()
		{
			if (MatchState == ET66TDMatchState::Victory || MatchState == ET66TDMatchState::Defeat)
			{
				ResetMatch();
				return FReply::Handled();
			}

			if (MatchState == ET66TDMatchState::AwaitingWave)
			{
				StartNextWave();
			}

			return FReply::Handled();
		}

		FReply HandleSpeedClicked()
		{
			const float NormalSpeed = GetTuningValue(TEXT("SimulationSpeedNormal"), 1.f);
			const float FastSpeed = GetTuningValue(TEXT("SimulationSpeedFast"), 2.f);
			SimulationSpeed = (SimulationSpeed >= (FastSpeed - 0.1f)) ? NormalSpeed : FastSpeed;
			return FReply::Handled();
		}

		FReply HandleDamageUpgradeClicked()
		{
			TryUpgradeSelectedTower(ET66TDTowerUpgradeType::Damage);
			return FReply::Handled();
		}

		FReply HandleRangeUpgradeClicked()
		{
			TryUpgradeSelectedTower(ET66TDTowerUpgradeType::Range);
			return FReply::Handled();
		}

		FReply HandleTempoUpgradeClicked()
		{
			TryUpgradeSelectedTower(ET66TDTowerUpgradeType::Tempo);
			return FReply::Handled();
		}

		FReply HandleSellSelectedTowerClicked()
		{
			const int32 TowerIndex = FindTowerIndexByPad(SelectedPadIndex);
			if (!Towers.IsValidIndex(TowerIndex))
			{
				return FReply::Handled();
			}

			Materials += GetTowerSellValue(Towers[TowerIndex]);
			Towers.RemoveAtSwap(TowerIndex);
			SelectedPadIndex = INDEX_NONE;
			return FReply::Handled();
		}

		virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
		{
			const TSharedPtr<FT66TDHeroDragDropOp> DragOperation = DragDropEvent.GetOperationAs<FT66TDHeroDragDropOp>();
			if (!DragOperation.IsValid())
			{
				return FReply::Unhandled();
			}

			const FVector2D NormalizedDropPoint = SafeNormalizePoint(MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition()), MyGeometry.GetLocalSize());
			PreviewPadIndex = FindNearestOpenPad(NormalizedDropPoint, 0.060f);
			bPreviewPlacementValid = PreviewPadIndex != INDEX_NONE && Materials >= DragOperation->Cost;
			return FReply::Handled();
		}

		virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override
		{
			SLeafWidget::OnDragLeave(DragDropEvent);
			PreviewPadIndex = INDEX_NONE;
			bPreviewPlacementValid = false;
		}

		virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
		{
			const TSharedPtr<FT66TDHeroDragDropOp> DragOperation = DragDropEvent.GetOperationAs<FT66TDHeroDragDropOp>();
			if (!DragOperation.IsValid())
			{
				return FReply::Unhandled();
			}

			const FVector2D NormalizedDropPoint = SafeNormalizePoint(MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition()), MyGeometry.GetLocalSize());
			const int32 TargetPadIndex = FindNearestOpenPad(NormalizedDropPoint, 0.060f);
			PreviewPadIndex = INDEX_NONE;
			bPreviewPlacementValid = false;

			if (TargetPadIndex == INDEX_NONE || Materials < DragOperation->Cost)
			{
				return FReply::Handled();
			}

			const FT66TDHeroDefinition* HeroDefinition = HeroLookup.Find(DragOperation->HeroID);
			const FT66TDHeroCombatProfile* HeroProfile = HeroProfiles.Find(DragOperation->HeroID);
			if (!HeroDefinition || !HeroProfile || !LayoutDefinition.Pads.IsValidIndex(TargetPadIndex))
			{
				return FReply::Handled();
			}

			FT66TDPlacedTower& NewTower = Towers.AddDefaulted_GetRef();
			NewTower.HeroID = HeroDefinition->HeroID;
			NewTower.DisplayName = HeroDefinition->DisplayName;
			NewTower.Tint = HeroDefinition->PlaceholderColor;
			NewTower.BaseProfile = *HeroProfile;
			NewTower.Profile = *HeroProfile;
			NewTower.PadIndex = TargetPadIndex;
			NewTower.PositionNormalized = LayoutDefinition.Pads[TargetPadIndex].PositionNormalized;
			NewTower.Cooldown = 0.f;
			NewTower.MaterialsInvested = HeroProfile->Cost;
			SelectedPadIndex = TargetPadIndex;
			Materials -= HeroProfile->Cost;
			return FReply::Handled();
		}

		virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			const FVector2D LocalPoint = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			const FVector2D NormalizedPoint = SafeNormalizePoint(LocalPoint, MyGeometry.GetLocalSize());
			const int32 PadIndex = FindTowerPadNearPoint(NormalizedPoint, 0.028f);

			if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				if (PadIndex != INDEX_NONE)
				{
					const int32 TowerIndex = FindTowerIndexByPad(PadIndex);
					if (Towers.IsValidIndex(TowerIndex))
					{
						Materials += GetTowerSellValue(Towers[TowerIndex]);
						Towers.RemoveAtSwap(TowerIndex);
						if (SelectedPadIndex == PadIndex)
						{
							SelectedPadIndex = INDEX_NONE;
						}
					}
					return FReply::Handled();
				}
			}

			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				SelectedPadIndex = PadIndex;
				return FReply::Handled();
			}

			return SLeafWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
		}

		virtual int32 OnPaint(
			const FPaintArgs& Args,
			const FGeometry& AllottedGeometry,
			const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements,
			int32 LayerId,
			const FWidgetStyle& InWidgetStyle,
			bool bParentEnabled) const override
		{
			const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
			const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
			const FLinearColor Tint = InWidgetStyle.GetColorAndOpacityTint();
			if (!WhiteBrush)
			{
				return LayerId;
			}

			auto DrawBoxAt = [&](const FVector2D& Center, const FVector2D& Size, const FLinearColor& Color, const int32 PaintLayer)
			{
				const FVector2D TopLeft = Center - (Size * 0.5f);
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					PaintLayer,
					AllottedGeometry.ToPaintGeometry(
						FVector2f(static_cast<float>(Size.X), static_cast<float>(Size.Y)),
						FSlateLayoutTransform(FVector2f(static_cast<float>(TopLeft.X), static_cast<float>(TopLeft.Y)))),
					WhiteBrush,
					ESlateDrawEffect::None,
					Color * Tint);
			};

			auto DrawBrushAt = [&](const TSharedPtr<FSlateBrush>& Brush, const FVector2D& Center, const FVector2D& Size, const FLinearColor& Color, const int32 PaintLayer)
			{
				if (!Brush.IsValid() || !Brush->GetResourceObject())
				{
					DrawBoxAt(Center, Size, Color, PaintLayer);
					return;
				}

				const FVector2D TopLeft = Center - (Size * 0.5f);
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					PaintLayer,
					AllottedGeometry.ToPaintGeometry(
						FVector2f(static_cast<float>(Size.X), static_cast<float>(Size.Y)),
						FSlateLayoutTransform(FVector2f(static_cast<float>(TopLeft.X), static_cast<float>(TopLeft.Y)))),
					Brush.Get(),
					ESlateDrawEffect::None,
					Color * Tint);
			};

			auto DrawLineStrip = [&](const TArray<FVector2f>& Points, const float Thickness, const FLinearColor& Color, const int32 PaintLayer)
			{
				if (Points.Num() >= 2)
				{
					FSlateDrawElement::MakeLines(
						OutDrawElements,
						PaintLayer,
						AllottedGeometry.ToPaintGeometry(),
						Points,
						ESlateDrawEffect::None,
						Color * Tint,
						true,
						Thickness);
				}
			};

			int32 PaintLayer = LayerId;

			for (int32 PathIndex = 0; PathIndex < PathRuntimes.Num(); ++PathIndex)
			{
				TArray<FVector2f> PathPoints;
				for (const FVector2D& Point : PathRuntimes[PathIndex].Points)
				{
					const FVector2D LocalPoint = ToLocalPoint(Point, LocalSize);
					PathPoints.Add(FVector2f(static_cast<float>(LocalPoint.X), static_cast<float>(LocalPoint.Y)));
				}

				DrawLineStrip(PathPoints, 3.f, FLinearColor(1.f, 0.92f, 0.56f, 0.05f), PaintLayer + 1);
			}

			for (int32 PadIndex = 0; PadIndex < LayoutDefinition.Pads.Num(); ++PadIndex)
			{
				const FT66TDMapPadDefinition& Pad = LayoutDefinition.Pads[PadIndex];
				const FVector2D PadCenter = ToLocalPoint(Pad.PositionNormalized, LocalSize);
				const float PadRadius = FMath::Max(8.f, Pad.RadiusNormalized * LocalSize.X * 1.35f);
				FLinearColor PadColor = FLinearColor(0.96f, 0.86f, 0.46f, 0.04f);
				if (FindTowerIndexByPad(PadIndex) != INDEX_NONE)
				{
					PadColor = FLinearColor(0.18f, 0.18f, 0.20f, 0.36f);
				}
				else if (PadIndex == PreviewPadIndex)
				{
					PadColor = bPreviewPlacementValid
						? FLinearColor(0.22f, 0.80f, 0.42f, 0.48f)
						: FLinearColor(0.92f, 0.22f, 0.18f, 0.46f);
				}
				else if (PadIndex == SelectedPadIndex)
				{
					PadColor = FLinearColor(0.98f, 0.96f, 0.70f, 0.34f);
				}

				DrawBoxAt(PadCenter, FVector2D(PadRadius * 2.0f, PadRadius * 2.0f), PadColor, PaintLayer + 2);
			}

			for (const FT66TDPlacedTower& Tower : Towers)
			{
				const FVector2D TowerCenter = ToLocalPoint(Tower.PositionNormalized, LocalSize);
				const FVector2D TowerSize(22.f, 22.f);
				const bool bSelected = Tower.PadIndex == SelectedPadIndex;
				if (bSelected)
				{
					TArray<FVector2f> RangePoints;
					BuildCirclePoints(TowerCenter, Tower.Profile.Range * LocalSize.X, 28, RangePoints);
					DrawLineStrip(RangePoints, 1.5f, FLinearColor(Tower.Tint.R, Tower.Tint.G, Tower.Tint.B, 0.42f), PaintLayer + 3);
				}

				DrawBoxAt(TowerCenter, TowerSize + FVector2D(10.f, 10.f), FLinearColor(0.02f, 0.02f, 0.03f, 0.88f), PaintLayer + 4);
				const TSharedPtr<FSlateBrush> TowerBrush = HeroBrushes.FindRef(Tower.HeroID);
				DrawBrushAt(TowerBrush, TowerCenter, TowerSize + FVector2D(14.f, 14.f), FLinearColor::White, PaintLayer + 5);
			}

			for (const FT66TDActiveEnemy& Enemy : Enemies)
			{
				const FVector2D EnemyCenter = ToLocalPoint(SampleEnemyPosition(Enemy), LocalSize);
				const float EnemyRadius = FMath::Max(8.f, Enemy.Radius * LocalSize.X);
				if (HasEnemyModifier(Enemy.Modifiers, ET66TDEnemyModifier::Shielded))
				{
					TArray<FVector2f> ShieldRing;
					BuildCirclePoints(EnemyCenter, EnemyRadius + 7.f, 24, ShieldRing);
					DrawLineStrip(ShieldRing, 2.2f, FLinearColor(0.36f, 0.72f, 0.98f, 0.65f), PaintLayer + 6);
				}
				if (HasEnemyModifier(Enemy.Modifiers, ET66TDEnemyModifier::Armored))
				{
					TArray<FVector2f> ArmorRing;
					BuildCirclePoints(EnemyCenter, EnemyRadius + 4.f, 20, ArmorRing);
					DrawLineStrip(ArmorRing, 1.8f, FLinearColor(0.82f, 0.68f, 0.26f, 0.62f), PaintLayer + 6);
				}
				if (HasEnemyModifier(Enemy.Modifiers, ET66TDEnemyModifier::Regenerating))
				{
					TArray<FVector2f> RegenRing;
					BuildCirclePoints(EnemyCenter, EnemyRadius + 10.f, 18, RegenRing);
					DrawLineStrip(RegenRing, 1.4f, FLinearColor(0.32f, 0.86f, 0.48f, 0.52f), PaintLayer + 6);
				}
				if (HasEnemyModifier(Enemy.Modifiers, ET66TDEnemyModifier::Hidden))
				{
					TArray<FVector2f> HiddenRing;
					BuildCirclePoints(EnemyCenter, EnemyRadius + 13.f, 16, HiddenRing);
					DrawLineStrip(HiddenRing, 1.2f, FLinearColor(0.42f, 0.30f, 0.62f, 0.48f), PaintLayer + 6);
				}

				DrawBoxAt(EnemyCenter, FVector2D((EnemyRadius * 2.f) + 6.f, (EnemyRadius * 2.f) + 6.f), FLinearColor(0.04f, 0.04f, 0.05f, 0.88f), PaintLayer + 7);
				const TSharedPtr<FSlateBrush> EnemyBrush = Enemy.bBoss ? BossBrushes.FindRef(Enemy.VisualID) : EnemyBrushes.FindRef(Enemy.VisualID);
				const float SpriteOpacity = HasEnemyModifier(Enemy.Modifiers, ET66TDEnemyModifier::Hidden) ? 0.78f : 0.98f;
				DrawBrushAt(EnemyBrush, EnemyCenter, FVector2D(EnemyRadius * 2.f, EnemyRadius * 2.f), Enemy.Tint.CopyWithNewOpacity(SpriteOpacity), PaintLayer + 8);

				const float HealthPct = Enemy.MaxHealth > KINDA_SMALL_NUMBER ? FMath::Clamp(Enemy.Health / Enemy.MaxHealth, 0.f, 1.f) : 0.f;
				const FVector2D BarCenter = EnemyCenter + FVector2D(0.f, -EnemyRadius - 8.f);
				DrawBoxAt(BarCenter, FVector2D(EnemyRadius * 2.f + 2.f, 4.f), FLinearColor(0.02f, 0.02f, 0.03f, 0.90f), PaintLayer + 9);
				DrawBoxAt(BarCenter - FVector2D((EnemyRadius * 2.f * (1.f - HealthPct)) * 0.5f, 0.f), FVector2D((EnemyRadius * 2.f) * HealthPct, 2.6f), FLinearColor(0.40f, 0.92f, 0.50f, 0.95f), PaintLayer + 10);
				if (Enemy.MaxShield > KINDA_SMALL_NUMBER)
				{
					const float ShieldPct = FMath::Clamp(Enemy.Shield / Enemy.MaxShield, 0.f, 1.f);
					const FVector2D ShieldBarCenter = EnemyCenter + FVector2D(0.f, -EnemyRadius - 14.f);
					DrawBoxAt(ShieldBarCenter, FVector2D(EnemyRadius * 2.f + 2.f, 3.f), FLinearColor(0.02f, 0.02f, 0.03f, 0.72f), PaintLayer + 9);
					DrawBoxAt(ShieldBarCenter - FVector2D((EnemyRadius * 2.f * (1.f - ShieldPct)) * 0.5f, 0.f), FVector2D((EnemyRadius * 2.f) * ShieldPct, 1.8f), FLinearColor(0.34f, 0.74f, 0.98f, 0.96f), PaintLayer + 10);
				}
			}

			for (const FT66TDBeamEffect& Beam : BeamEffects)
			{
				TArray<FVector2f> BeamPoints;
				const FVector2D Start = ToLocalPoint(Beam.StartNormalized, LocalSize);
				const FVector2D End = ToLocalPoint(Beam.EndNormalized, LocalSize);
				BeamPoints.Add(FVector2f(static_cast<float>(Start.X), static_cast<float>(Start.Y)));
				BeamPoints.Add(FVector2f(static_cast<float>(End.X), static_cast<float>(End.Y)));
				const float Alpha = Beam.TotalLifetime <= KINDA_SMALL_NUMBER ? 0.f : Beam.RemainingTime / Beam.TotalLifetime;
				DrawLineStrip(BeamPoints, Beam.Thickness, Beam.Tint.CopyWithNewOpacity(Alpha), PaintLayer + 10);
			}

			if (MatchState == ET66TDMatchState::Victory || MatchState == ET66TDMatchState::Defeat)
			{
				DrawBoxAt(LocalSize * 0.5f, FVector2D(420.f, 120.f), FLinearColor(0.01f, 0.01f, 0.02f, 0.82f), PaintLayer + 11);
				FSlateDrawElement::MakeText(
					OutDrawElements,
					PaintLayer + 12,
					AllottedGeometry.ToPaintGeometry(
						FVector2f(360.f, 40.f),
						FSlateLayoutTransform(FVector2f(static_cast<float>((LocalSize.X * 0.5f) - 140.f), static_cast<float>((LocalSize.Y * 0.5f) - 28.f)))),
					MatchState == ET66TDMatchState::Victory ? TEXT("VICTORY") : TEXT("DEFEAT"),
					FT66Style::MakeFont(TEXT("Black"), 28),
					ESlateDrawEffect::None,
					MatchState == ET66TDMatchState::Victory ? FLinearColor(0.98f, 0.90f, 0.46f, 1.0f) : FLinearColor(0.98f, 0.46f, 0.42f, 1.0f));
			}

			return PaintLayer + 12;
		}

	private:
		EActiveTimerReturnType HandleActiveTimer(double, float InDeltaTime)
		{
			AdvanceMatch(InDeltaTime * SimulationSpeed);
			ResolveVictoryRewardIfNeeded();
			Invalidate(EInvalidateWidgetReason::Paint);
			return EActiveTimerReturnType::Continue;
		}

		float GetTuningValue(const TCHAR* Key, const float DefaultValue = 0.f) const
		{
			if (const float* FoundValue = BattleTuningValues.Find(FName(Key)))
			{
				return *FoundValue;
			}
			return DefaultValue;
		}

		int32 GetTuningInt(const TCHAR* Key, const int32 DefaultValue = 0) const
		{
			return FMath::RoundToInt(GetTuningValue(Key, static_cast<float>(DefaultValue)));
		}

		FT66TDEnemyArchetype GetEnemyArchetype(const ET66TDEnemyFamily Family) const
		{
			if (const FT66TDEnemyArchetype* FoundArchetype = EnemyArchetypeLookup.Find(GetEnemyFamilyID(Family)))
			{
				return *FoundArchetype;
			}
			return FT66TDEnemyArchetype();
		}

		const FT66TDThemeModifierRule* FindThemeModifierRule() const
		{
			if (const FT66TDThemeModifierRule* Rule = ThemeModifierLookup.Find(FName(*MapDefinition.ThemeLabel)))
			{
				return Rule;
			}
			return ThemeModifierLookup.Find(FName(TEXT("Hell")));
		}

		FString ResolveBossVisualID() const
		{
			if (const FT66TDThemeModifierRule* Rule = FindThemeModifierRule())
			{
				if (!Rule->BossVisualID.IsEmpty())
				{
					return Rule->BossVisualID;
				}
			}

			return GetEnemyArchetype(ET66TDEnemyFamily::Boss).VisualID;
		}

		void ResetMatch()
		{
			Materials = StageDefinition.StartingMaterials > 0
				? StageDefinition.StartingMaterials
				: GetTuningInt(TEXT("StartingMaterials"), 340);
			Gold = FMath::Max(0, StageDefinition.StartingGold);
			Hearts = GetTuningInt(TEXT("StartingHearts"), 20);
			CurrentWave = 0;
			MatchState = ET66TDMatchState::AwaitingWave;
			SimulationSpeed = GetTuningValue(TEXT("SimulationSpeedNormal"), 1.f);
			SelectedPadIndex = INDEX_NONE;
			PreviewPadIndex = INDEX_NONE;
			bPreviewPlacementValid = false;
			Towers.Reset();
			Enemies.Reset();
			BeamEffects.Reset();
			PendingSpawns.Reset();
			NextSpawnIndex = 0;
			TimeUntilNextSpawn = 0.f;
			bLeaderboardResultSubmitted = false;
			bStageResultRecorded = false;
			PersistRunState(false);
			if (UGameInstance* GameInstance = OwningGameInstance.Get())
			{
				if (UT66TDFrontendStateSubsystem* FrontendState = GameInstance->GetSubsystem<UT66TDFrontendStateSubsystem>())
				{
					FrontendState->ResetBattleRewardGrant();
				}
			}
		}

		void ResolveVictoryRewardIfNeeded()
		{
			if (MatchState != ET66TDMatchState::Victory)
			{
				return;
			}

			UGameInstance* GameInstance = OwningGameInstance.Get();
			UT66TDFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
			if (!GameInstance || !FrontendState || !FrontendState->TryMarkBattleRewardGranted())
			{
				return;
			}

			if (UT66AchievementsSubsystem* Achievements = GameInstance->GetSubsystem<UT66AchievementsSubsystem>())
			{
				const int32 CouponsAwarded = StageDefinition.ClearChadCoupons > 0
					? StageDefinition.ClearChadCoupons
					: DifficultyDefinition.StageClearChadCoupons;
				Achievements->AddChadCoupons(FMath::Max(0, CouponsAwarded));
			}

			Gold += FMath::Max(0, StageDefinition.ClearGoldReward);
			Materials += FMath::Max(0, StageDefinition.ClearMaterialReward);
			RecordStageResultIfNeeded(true);
		}

		int32 BuildStageScore() const
		{
			const int32 StageScore = StageDefinition.StageIndex > 0 ? StageDefinition.StageIndex * 10000 : 0;
			return FMath::Max(0, StageScore + (CurrentWave * 1000) + (Hearts * 100) + Materials + Gold + (MatchState == ET66TDMatchState::Victory ? 5000 : 0));
		}

		void PersistRunState(const bool bStageCleared) const
		{
			UGameInstance* GameInstance = OwningGameInstance.Get();
			UT66TDSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDSaveSubsystem>() : nullptr;
			const UT66TDFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
			const UT66TDDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDDataSubsystem>() : nullptr;
			UT66TDRunSaveGame* RunSave = SaveSubsystem ? SaveSubsystem->CreateSeededRunSave(FrontendState, DataSubsystem) : nullptr;
			if (!SaveSubsystem || !RunSave)
			{
				return;
			}

			RunSave->StageID = StageDefinition.StageID;
			RunSave->StageIndex = StageDefinition.StageIndex > 0 ? StageDefinition.StageIndex : 1;
			RunSave->MapID = MapDefinition.MapID;
			RunSave->CurrentWave = CurrentWave;
			RunSave->Hearts = Hearts;
			RunSave->Gold = Gold;
			RunSave->Materials = Materials;
			RunSave->LastScore = BuildStageScore();
			RunSave->bStageCleared = bStageCleared;
			SaveSubsystem->SaveRun(RunSave);
		}

		void RecordStageResultIfNeeded(const bool bVictory)
		{
			if (bStageResultRecorded)
			{
				return;
			}

			bStageResultRecorded = true;
			const int32 Score = BuildStageScore();
			if (UGameInstance* GameInstance = OwningGameInstance.Get())
			{
				if (UT66TDSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UT66TDSaveSubsystem>())
				{
					const FT66TDStageDefinition* Stage = StageDefinition.StageID != NAME_None ? &StageDefinition : nullptr;
					SaveSubsystem->RecordStageResult(
						Stage,
						bVictory,
						Score,
						bVictory ? FMath::Max(0, StageDefinition.ClearGoldReward) : 0,
						bVictory ? FMath::Max(0, StageDefinition.ClearMaterialReward) : 0);
				}
			}
			PersistRunState(bVictory);
		}

		void SubmitLeaderboardResultIfNeeded()
		{
			if (bLeaderboardResultSubmitted)
			{
				return;
			}

			UGameInstance* GameInstance = OwningGameInstance.Get();
			UT66BackendSubsystem* Backend = GameInstance ? GameInstance->GetSubsystem<UT66BackendSubsystem>() : nullptr;
			if (!Backend)
			{
				return;
			}

			const UT66TDFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
			const UT66SteamHelper* SteamHelper = GameInstance ? GameInstance->GetSubsystem<UT66SteamHelper>() : nullptr;
			const bool bDailyRun = FrontendState && FrontendState->IsDailyRun();
			const FName DifficultyID = FrontendState && FrontendState->GetSelectedDifficultyID() != NAME_None
				? FrontendState->GetSelectedDifficultyID()
				: DifficultyDefinition.DifficultyID;
			const int32 Score = BuildStageScore();

			Backend->SubmitMinigameScore(
				SteamHelper ? SteamHelper->GetLocalDisplayName() : FString(TEXT("Player")),
				TEXT("td"),
				bDailyRun ? TEXT("daily") : TEXT("alltime"),
				DifficultyID.ToString().ToLower(),
				Score,
				bDailyRun ? FrontendState->GetDailyChallengeId() : FString(),
				bDailyRun ? FrontendState->GetDailySeed() : 0);
			bLeaderboardResultSubmitted = true;
		}

		void StartNextWave()
		{
			if (MatchState != ET66TDMatchState::AwaitingWave)
			{
				return;
			}

			++CurrentWave;
			BuildWaveQueue(CurrentWave);
			NextSpawnIndex = 0;
			TimeUntilNextSpawn = PendingSpawns.Num() > 0 ? PendingSpawns[0].SpawnDelay : 0.f;
			MatchState = ET66TDMatchState::WaveActive;
		}

		ET66TDEnemyModifier GetBossModifiers() const
		{
			if (const FT66TDThemeModifierRule* Rule = FindThemeModifierRule())
			{
				return BuildEnemyModifierMask(Rule->BossModifierMask);
			}
			return ET66TDEnemyModifier::None;
		}

		ET66TDEnemyModifier BuildSpawnModifiers(const ET66TDEnemyFamily Family, const int32 WaveNumber, const int32 SpawnIndex, const bool bBoss, FRandomStream& Stream) const
		{
			if (bBoss)
			{
				return GetBossModifiers();
			}

			const FT66TDThemeModifierRule* ThemeRule = FindThemeModifierRule();
			const bool bImpossible = DifficultyDefinition.DifficultyID == FName(TEXT("Difficulty_Impossible"));

			auto Roll = [&Stream](const float Chance)
			{
				return Stream.FRand() <= Chance;
			};

			ET66TDEnemyModifier Modifiers = ET66TDEnemyModifier::None;

			if (WaveNumber >= GetTuningInt(TEXT("ArmorStartWave"), 4)
				&& (Family == ET66TDEnemyFamily::Cow
					|| Family == ET66TDEnemyFamily::Pig
					|| (WaveNumber >= GetTuningInt(TEXT("ArmorGoatStartWave"), 10) && Family == ET66TDEnemyFamily::Goat)))
			{
				float ArmorChance = GetTuningValue(TEXT("ArmorChanceBase"), 0.18f)
					+ (WaveNumber * GetTuningValue(TEXT("ArmorChancePerWave"), 0.012f));
				if (ThemeRule)
				{
					ArmorChance += ThemeRule->ArmorChanceBonus;
				}
				if (bImpossible)
				{
					ArmorChance += GetTuningValue(TEXT("ImpossibleArmorChanceBonus"), 0.08f);
				}
				if (((SpawnIndex + WaveNumber) % FMath::Max(1, GetTuningInt(TEXT("ArmorGuaranteedModulo"), 5)) == 0) || Roll(ArmorChance))
				{
					Modifiers |= ET66TDEnemyModifier::Armored;
				}
			}

			if (ThemeRule
				&& ThemeRule->HiddenMinWave > 0
				&& (Family == ET66TDEnemyFamily::Roost || Family == ET66TDEnemyFamily::Goat)
				&& WaveNumber >= ThemeRule->HiddenMinWave)
			{
				const float HiddenChance = GetTuningValue(TEXT("HiddenChanceBase"), 0.20f)
					+ (WaveNumber * GetTuningValue(TEXT("HiddenChancePerWave"), 0.010f))
					+ ThemeRule->HiddenChanceBonus;
				if (((SpawnIndex + WaveNumber) % FMath::Max(1, GetTuningInt(TEXT("HiddenGuaranteedModulo"), 4)) == 0) || Roll(HiddenChance))
				{
					Modifiers |= ET66TDEnemyModifier::Hidden;
				}
			}

			if (ThemeRule && ThemeRule->bEnableRegen && WaveNumber >= GetTuningInt(TEXT("RegenStartWave"), 6))
			{
				float RegenChance = GetTuningValue(TEXT("RegenChanceBase"), 0.12f)
					+ (WaveNumber * GetTuningValue(TEXT("RegenChancePerWave"), 0.008f));
				if (Family == ET66TDEnemyFamily::Pig || Family == ET66TDEnemyFamily::Cow)
				{
					RegenChance += GetTuningValue(TEXT("RegenHeavyChanceBonus"), 0.06f);
				}
				RegenChance += ThemeRule->RegenChanceBonus;
				if (((SpawnIndex + (WaveNumber * GetTuningInt(TEXT("RegenWaveModuloMultiplier"), 2))) % FMath::Max(1, GetTuningInt(TEXT("RegenGuaranteedModulo"), 6)) == 0) || Roll(RegenChance))
				{
					Modifiers |= ET66TDEnemyModifier::Regenerating;
				}
			}

			if (ThemeRule
				&& ThemeRule->ShieldMinWave > 0
				&& WaveNumber >= ThemeRule->ShieldMinWave
				&& (Family == ET66TDEnemyFamily::Cow
					|| Family == ET66TDEnemyFamily::Pig
					|| (ThemeRule->bShieldGoat && Family == ET66TDEnemyFamily::Goat)))
			{
				const float ShieldChance = GetTuningValue(TEXT("ShieldChanceBase"), 0.18f)
					+ (WaveNumber * GetTuningValue(TEXT("ShieldChancePerWave"), 0.011f))
					+ ThemeRule->ShieldChanceBonus;
				if (((SpawnIndex + WaveNumber) % FMath::Max(1, GetTuningInt(TEXT("ShieldGuaranteedModulo"), 3)) == 0) || Roll(ShieldChance))
				{
					Modifiers |= ET66TDEnemyModifier::Shielded;
				}
			}

			if (bImpossible && WaveNumber >= GetTuningInt(TEXT("ImpossibleForcedModifierStartWave"), 10))
			{
				if ((Family == ET66TDEnemyFamily::Cow || Family == ET66TDEnemyFamily::Pig)
					&& !HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Shielded)
					&& (SpawnIndex % FMath::Max(1, GetTuningInt(TEXT("ImpossibleForcedShieldModulo"), 4)) == GetTuningInt(TEXT("ImpossibleForcedShieldRemainder"), 1)))
				{
					Modifiers |= ET66TDEnemyModifier::Shielded;
				}
				if ((Family == ET66TDEnemyFamily::Roost || Family == ET66TDEnemyFamily::Goat)
					&& !HasEnemyModifier(Modifiers, ET66TDEnemyModifier::Hidden)
					&& (SpawnIndex % FMath::Max(1, GetTuningInt(TEXT("ImpossibleForcedHiddenModulo"), 5)) == GetTuningInt(TEXT("ImpossibleForcedHiddenRemainder"), 2)))
				{
					Modifiers |= ET66TDEnemyModifier::Hidden;
				}
			}

			return Modifiers;
		}

		void BuildWaveQueue(const int32 WaveNumber)
		{
			PendingSpawns.Reset();
			const int32 LaneCount = FMath::Max(1, PathRuntimes.Num());
			const float SpawnDelayBase = GetTuningValue(TEXT("SpawnDelayBase"), 0.54f);
			const float SpawnDelay = FMath::Clamp(
				SpawnDelayBase - (WaveNumber * GetTuningValue(TEXT("SpawnDelayPerWave"), 0.02f)),
				GetTuningValue(TEXT("SpawnDelayMin"), 0.18f),
				SpawnDelayBase);
			FRandomStream Stream(static_cast<int32>(GetTypeHash(MapDefinition.MapID) ^ (WaveNumber * 977)));

			if (WaveNumber >= MapDefinition.BossWave)
			{
				float DelayCursor = GetTuningValue(TEXT("BossPreludeInitialDelay"), 0.35f);
				const int32 PreludeCount = FMath::Max(0, GetTuningInt(TEXT("BossPreludeCount"), 12));
				const int32 PreludeHeavyCount = FMath::Clamp(GetTuningInt(TEXT("BossPreludeHeavyCount"), 6), 0, PreludeCount);
				for (int32 Index = 0; Index < PreludeCount; ++Index)
				{
					const bool bHeavyPrelude = Index < PreludeHeavyCount;
					const ET66TDEnemyFamily Family = bHeavyPrelude
						? (Index % 2 == 0 ? ET66TDEnemyFamily::Pig : ET66TDEnemyFamily::Cow)
						: (Index % 2 == 0 ? ET66TDEnemyFamily::Goat : ET66TDEnemyFamily::Roost);
					const ET66TDEnemyModifier Modifiers = BuildSpawnModifiers(Family, WaveNumber, Index, false, Stream);
					PendingSpawns.Add({
						Family,
						Index % LaneCount,
						DelayCursor,
						bHeavyPrelude ? GetTuningValue(TEXT("BossPreludeHeavyHealthScalar"), 1.65f) : GetTuningValue(TEXT("BossPreludeLightHealthScalar"), 1.28f),
						bHeavyPrelude ? GetTuningValue(TEXT("BossPreludeHeavySpeedScalar"), 1.04f) : GetTuningValue(TEXT("BossPreludeLightSpeedScalar"), 1.14f),
						Modifiers,
						false });
					DelayCursor += SpawnDelay * GetTuningValue(TEXT("BossPreludeDelayScalar"), 0.70f);
				}
				PendingSpawns.Add({ ET66TDEnemyFamily::Boss, (LaneCount - 1) % LaneCount, DelayCursor + GetTuningValue(TEXT("BossSpawnDelayBonus"), 0.8f), 1.0f, 1.0f, BuildSpawnModifiers(ET66TDEnemyFamily::Boss, WaveNumber, 99, true, Stream), true });
				return;
			}

			int32 RoostCount = GetTuningInt(TEXT("RoostBaseCount"), 6) + (WaveNumber * GetTuningInt(TEXT("RoostPerWave"), 2));
			int32 GoatCount = GetTuningInt(TEXT("GoatBaseCount"), 2) + (WaveNumber * GetTuningInt(TEXT("GoatPerWave"), 1)) + (LaneCount - 1);
			int32 CowCount = WaveNumber >= GetTuningInt(TEXT("CowStartWave"), 4) ? (WaveNumber / FMath::Max(1, GetTuningInt(TEXT("CowWaveDivisor"), 2))) + LaneCount : 0;
			int32 PigCount = WaveNumber >= GetTuningInt(TEXT("PigStartWave"), 7) ? (WaveNumber / FMath::Max(1, GetTuningInt(TEXT("PigWaveDivisor"), 3))) + LaneCount - 1 : 0;

			if (WaveNumber == GetTuningInt(TEXT("MilestoneWaveA"), 5) || WaveNumber == GetTuningInt(TEXT("MilestoneWaveB"), 10))
			{
				CowCount += GetTuningInt(TEXT("MilestoneCowBonus"), 3);
				PigCount += GetTuningInt(TEXT("MilestonePigBonus"), 2);
			}

			TArray<ET66TDEnemyFamily> Families;
			Families.Reserve(RoostCount + GoatCount + CowCount + PigCount);
			for (int32 Index = 0; Index < RoostCount; ++Index) { Families.Add(ET66TDEnemyFamily::Roost); }
			for (int32 Index = 0; Index < GoatCount; ++Index) { Families.Add(ET66TDEnemyFamily::Goat); }
			for (int32 Index = 0; Index < CowCount; ++Index) { Families.Add(ET66TDEnemyFamily::Cow); }
			for (int32 Index = 0; Index < PigCount; ++Index) { Families.Add(ET66TDEnemyFamily::Pig); }

			for (int32 Index = Families.Num() - 1; Index > 0; --Index)
			{
				Families.Swap(Index, Stream.RandRange(0, Index));
			}

			float DelayCursor = GetTuningValue(TEXT("NormalInitialDelay"), 0.25f);
			for (int32 SpawnIndex = 0; SpawnIndex < Families.Num(); ++SpawnIndex)
			{
				const ET66TDEnemyFamily Family = Families[SpawnIndex];
				const float FamilyDelayMultiplier = (Family == ET66TDEnemyFamily::Roost)
					? GetTuningValue(TEXT("RoostDelayMultiplier"), 0.82f)
					: (Family == ET66TDEnemyFamily::Cow || Family == ET66TDEnemyFamily::Pig ? GetTuningValue(TEXT("HeavyDelayMultiplier"), 1.18f) : 1.0f);
				const float HealthScalar = 1.0f
					+ (WaveNumber * GetTuningValue(TEXT("WaveHealthPerWave"), 0.09f))
					+ ((Family == ET66TDEnemyFamily::Cow || Family == ET66TDEnemyFamily::Pig) ? GetTuningValue(TEXT("WaveHeavyHealthBonus"), 0.18f) : 0.f);
				const float SpeedScalar = 1.0f
					+ (WaveNumber * GetTuningValue(TEXT("WaveSpeedPerWave"), 0.02f))
					+ (Family == ET66TDEnemyFamily::Roost ? GetTuningValue(TEXT("RoostSpeedBonus"), 0.05f) : 0.f);
				const ET66TDEnemyModifier Modifiers = BuildSpawnModifiers(Family, WaveNumber, SpawnIndex, false, Stream);
				PendingSpawns.Add({ Family, SpawnIndex % LaneCount, DelayCursor, HealthScalar, SpeedScalar, Modifiers, false });
				DelayCursor += SpawnDelay * FamilyDelayMultiplier;
			}
		}

		void AdvanceMatch(const float DeltaTime)
		{
			for (int32 BeamIndex = BeamEffects.Num() - 1; BeamIndex >= 0; --BeamIndex)
			{
				BeamEffects[BeamIndex].RemainingTime -= DeltaTime;
				if (BeamEffects[BeamIndex].RemainingTime <= 0.f)
				{
					BeamEffects.RemoveAtSwap(BeamIndex);
				}
			}

			if (MatchState != ET66TDMatchState::WaveActive)
			{
				return;
			}

			TimeUntilNextSpawn -= DeltaTime;
			while (NextSpawnIndex < PendingSpawns.Num() && TimeUntilNextSpawn <= 0.f)
			{
				SpawnEnemy(PendingSpawns[NextSpawnIndex]);
				++NextSpawnIndex;
				if (NextSpawnIndex < PendingSpawns.Num())
				{
					TimeUntilNextSpawn += PendingSpawns[NextSpawnIndex].SpawnDelay;
				}
			}

			for (FT66TDActiveEnemy& Enemy : Enemies)
			{
				if (Enemy.bPendingRemoval)
				{
					continue;
				}

				if (Enemy.DotRemaining > 0.f && Enemy.DotDamagePerSecond > 0.f)
				{
					Enemy.Health -= Enemy.DotDamagePerSecond * DeltaTime;
					Enemy.DotRemaining = FMath::Max(0.f, Enemy.DotRemaining - DeltaTime);
					if (Enemy.Health <= 0.f)
					{
						Enemy.bPendingRemoval = true;
						Materials += Enemy.Bounty;
						continue;
					}
				}

				if (Enemy.RegenPerSecond > 0.f && Enemy.DotRemaining <= 0.f && Enemy.Health < Enemy.MaxHealth)
				{
					Enemy.Health = FMath::Min(Enemy.MaxHealth, Enemy.Health + (Enemy.RegenPerSecond * DeltaTime));
				}

				if (Enemy.SlowRemaining > 0.f)
				{
					Enemy.SlowRemaining = FMath::Max(0.f, Enemy.SlowRemaining - DeltaTime);
					if (Enemy.SlowRemaining <= 0.f)
					{
						Enemy.SlowMultiplier = 1.f;
					}
				}

				const FT66TDPathRuntime& Path = PathRuntimes[Enemy.LaneIndex];
				const float EffectiveSpeed = Enemy.Speed * Enemy.SlowMultiplier;
				Enemy.ProgressRatio += (Path.TotalLength > KINDA_SMALL_NUMBER ? (EffectiveSpeed / Path.TotalLength) : 0.f) * DeltaTime;
				if (Enemy.ProgressRatio >= 1.f)
				{
					Enemy.bPendingRemoval = true;
					Hearts -= Enemy.LeakDamage;
					if (Hearts <= 0)
					{
						Hearts = 0;
						MatchState = ET66TDMatchState::Defeat;
						RecordStageResultIfNeeded(false);
						SubmitLeaderboardResultIfNeeded();
					}
				}
			}

			for (FT66TDPlacedTower& Tower : Towers)
			{
				Tower.Cooldown -= DeltaTime;
				if (Tower.Cooldown > 0.f || MatchState != ET66TDMatchState::WaveActive)
				{
					continue;
				}

				const int32 TargetIndex = FindBestTargetIndex(Tower);
				if (TargetIndex == INDEX_NONE)
				{
					continue;
				}

				FireTowerAtTarget(Tower, TargetIndex);
				Tower.Cooldown = Tower.Profile.FireInterval;
			}

			for (int32 EnemyIndex = Enemies.Num() - 1; EnemyIndex >= 0; --EnemyIndex)
			{
				if (Enemies[EnemyIndex].bPendingRemoval)
				{
					Enemies.RemoveAtSwap(EnemyIndex);
				}
			}

			if (MatchState == ET66TDMatchState::WaveActive && NextSpawnIndex >= PendingSpawns.Num() && Enemies.Num() == 0)
			{
				if (CurrentWave >= MapDefinition.BossWave)
				{
					MatchState = ET66TDMatchState::Victory;
					ResolveVictoryRewardIfNeeded();
					SubmitLeaderboardResultIfNeeded();
				}
				else
				{
					MatchState = ET66TDMatchState::AwaitingWave;
					Materials += GetTuningInt(TEXT("WaveClearRewardBase"), 18) + (CurrentWave * GetTuningInt(TEXT("WaveClearRewardPerWave"), 3));
				}
			}
		}

		void SpawnEnemy(const FT66TDQueuedSpawn& QueuedSpawn)
		{
			if (!PathRuntimes.IsValidIndex(QueuedSpawn.LaneIndex))
			{
				return;
			}

			const FT66TDEnemyArchetype Archetype = GetEnemyArchetype(QueuedSpawn.Family);
			FT66TDActiveEnemy& Enemy = Enemies.AddDefaulted_GetRef();
			Enemy.Family = QueuedSpawn.Family;
			Enemy.DisplayName = Archetype.DisplayName;
			const FString ModifierLabel = GetEnemyModifierLabel(QueuedSpawn.Modifiers);
			if (!ModifierLabel.Equals(TEXT("Baseline")))
			{
				Enemy.DisplayName = FString::Printf(TEXT("%s [%s]"), *Archetype.DisplayName, *ModifierLabel);
			}
			Enemy.VisualID = QueuedSpawn.bBoss ? ResolveBossVisualID() : Archetype.VisualID;
			Enemy.LaneIndex = QueuedSpawn.LaneIndex;
			Enemy.ProgressRatio = 0.f;
			Enemy.MaxHealth =
				Archetype.BaseHealth *
				QueuedSpawn.HealthScalar *
				DifficultyDefinition.EnemyHealthScalar *
				(QueuedSpawn.bBoss ? DifficultyDefinition.BossScalar : 1.0f);
			Enemy.Health = Enemy.MaxHealth;
			Enemy.Speed = Archetype.BaseSpeed * QueuedSpawn.SpeedScalar * DifficultyDefinition.EnemySpeedScalar;
			Enemy.Radius = Archetype.Radius;
			Enemy.LeakDamage = QueuedSpawn.bBoss ? GetTuningInt(TEXT("BossLeakDamage"), Archetype.LeakDamage) : Archetype.LeakDamage;
			Enemy.Bounty = FMath::RoundToInt(Archetype.Bounty * DifficultyDefinition.RewardScalar * (QueuedSpawn.bBoss ? GetTuningValue(TEXT("BossRewardMultiplier"), 1.8f) : 1.0f));
			Enemy.Modifiers = QueuedSpawn.Modifiers;
			Enemy.Tint = GetEnemyTint(Archetype, QueuedSpawn.Modifiers, QueuedSpawn.bBoss);
			if (HasEnemyModifier(QueuedSpawn.Modifiers, ET66TDEnemyModifier::Armored))
			{
				Enemy.Armor = FMath::Max(GetTuningValue(TEXT("ArmorMin"), 4.f), Enemy.MaxHealth * GetTuningValue(TEXT("ArmorHealthScalar"), 0.050f));
			}
			if (HasEnemyModifier(QueuedSpawn.Modifiers, ET66TDEnemyModifier::Shielded))
			{
				Enemy.MaxShield = FMath::Max(
					GetTuningValue(TEXT("ShieldMin"), 18.f),
					Enemy.MaxHealth * (QueuedSpawn.bBoss ? GetTuningValue(TEXT("BossShieldScalar"), 0.30f) : GetTuningValue(TEXT("ShieldScalar"), 0.22f)));
				Enemy.Shield = Enemy.MaxShield;
			}
			if (HasEnemyModifier(QueuedSpawn.Modifiers, ET66TDEnemyModifier::Regenerating))
			{
				Enemy.RegenPerSecond = FMath::Max(GetTuningValue(TEXT("RegenMin"), 3.5f), Enemy.MaxHealth * GetTuningValue(TEXT("RegenScalar"), 0.028f));
			}
			Enemy.bBoss = QueuedSpawn.bBoss;
		}

		FVector2D SampleEnemyPosition(const FT66TDActiveEnemy& Enemy) const
		{
			if (!PathRuntimes.IsValidIndex(Enemy.LaneIndex))
			{
				return FVector2D(0.5f, 0.5f);
			}
			return SamplePathPoint(PathRuntimes[Enemy.LaneIndex], Enemy.ProgressRatio);
		}

		bool CanTowerHitEnemy(const FT66TDHeroCombatProfile& Profile, const FT66TDActiveEnemy& Enemy) const
		{
			return !HasEnemyModifier(Enemy.Modifiers, ET66TDEnemyModifier::Hidden) || Profile.bCanTargetHidden;
		}

		int32 FindBestTargetIndex(const FT66TDPlacedTower& Tower) const
		{
			int32 BestIndex = INDEX_NONE;
			float BestScore = -FLT_MAX;
			for (int32 EnemyIndex = 0; EnemyIndex < Enemies.Num(); ++EnemyIndex)
			{
				const FT66TDActiveEnemy& Enemy = Enemies[EnemyIndex];
				if (Enemy.bPendingRemoval)
				{
					continue;
				}
				if (!CanTowerHitEnemy(Tower.Profile, Enemy))
				{
					continue;
				}

				const FVector2D EnemyPosition = SampleEnemyPosition(Enemy);
				const float DistanceSq = DistanceSquared(Tower.PositionNormalized, EnemyPosition);
				if (DistanceSq > FMath::Square(Tower.Profile.Range))
				{
					continue;
				}

				float Score = (Enemy.ProgressRatio * 1000.f) - DistanceSq + (Enemy.bBoss ? 100.f : 0.f);
				if (Tower.Profile.bPrioritizeBoss && Enemy.bBoss)
				{
					Score += 320.f;
				}
				if (Score > BestScore)
				{
					BestScore = Score;
					BestIndex = EnemyIndex;
				}
			}

			return BestIndex;
		}

		void ApplyDamageToEnemy(FT66TDActiveEnemy& Enemy, const float DamageAmount, const FT66TDHeroCombatProfile& Profile, FT66TDPlacedTower& SourceTower)
		{
			if (Enemy.bPendingRemoval)
			{
				return;
			}

			float FinalDamage = DamageAmount;
			if (Enemy.bBoss)
			{
				FinalDamage *= Profile.BossDamageMultiplier;
			}

			if (Enemy.Shield > 0.f)
			{
				const float ShieldDamage = FinalDamage * FMath::Max(1.0f, Profile.ShieldDamageMultiplier);
				Enemy.Shield -= ShieldDamage;
				if (Enemy.Shield < 0.f)
				{
					FinalDamage = -Enemy.Shield / FMath::Max(1.0f, Profile.ShieldDamageMultiplier);
					Enemy.Shield = 0.f;
				}
				else
				{
					FinalDamage = 0.f;
				}
			}

			if (FinalDamage > 0.f && HasEnemyModifier(Enemy.Modifiers, ET66TDEnemyModifier::Armored))
			{
				FinalDamage = FMath::Max(1.0f, FinalDamage - FMath::Max(0.f, Enemy.Armor - Profile.FlatArmorPierce));
			}

			Enemy.Health -= FinalDamage;
			if (Profile.DotDuration > 0.f && Profile.DotDamagePerSecond > 0.f)
			{
				Enemy.DotRemaining = FMath::Max(Enemy.DotRemaining, Profile.DotDuration);
				Enemy.DotDamagePerSecond = FMath::Max(Enemy.DotDamagePerSecond, Profile.DotDamagePerSecond);
			}
			if (Profile.SlowDuration > 0.f && Profile.SlowMultiplier < 0.999f)
			{
				Enemy.SlowRemaining = FMath::Max(Enemy.SlowRemaining, Profile.SlowDuration);
				Enemy.SlowMultiplier = FMath::Min(Enemy.SlowMultiplier, Profile.SlowMultiplier);
			}

			if (Enemy.Health <= 0.f)
			{
				Enemy.bPendingRemoval = true;
				Materials += Enemy.Bounty + Profile.BonusMaterialsOnKill;
				++SourceTower.Kills;
			}
		}

		void FireTowerAtTarget(FT66TDPlacedTower& Tower, const int32 TargetIndex)
		{
			if (!Enemies.IsValidIndex(TargetIndex))
			{
				return;
			}

			for (int32 VolleyIndex = 0; VolleyIndex < FMath::Max(1, Tower.Profile.VolleyShots); ++VolleyIndex)
			{
				const int32 VolleyTargetIndex = (VolleyIndex == 0) ? TargetIndex : FindBestTargetIndex(Tower);
				if (!Enemies.IsValidIndex(VolleyTargetIndex))
				{
					break;
				}

				TSet<int32> HitEnemyIndices;
				TArray<int32> PendingChainTargets;
				PendingChainTargets.Add(VolleyTargetIndex);

				FVector2D PreviousBeamOrigin = Tower.PositionNormalized;
				float DamageScale = VolleyIndex == 0 ? 1.0f : 0.72f;

				for (int32 ChainIndex = 0; ChainIndex < PendingChainTargets.Num(); ++ChainIndex)
				{
					const int32 CurrentEnemyIndex = PendingChainTargets[ChainIndex];
					if (!Enemies.IsValidIndex(CurrentEnemyIndex))
					{
						continue;
					}

					FT66TDActiveEnemy& Enemy = Enemies[CurrentEnemyIndex];
					if (Enemy.bPendingRemoval)
					{
						continue;
					}

					const FVector2D EnemyPosition = SampleEnemyPosition(Enemy);
					BeamEffects.Add({ PreviousBeamOrigin, EnemyPosition, Tower.Tint, 0.10f, 0.10f, ChainIndex == 0 ? 3.2f : 2.0f });
					ApplyDamageToEnemy(Enemy, Tower.Profile.Damage * DamageScale, Tower.Profile, Tower);
					HitEnemyIndices.Add(CurrentEnemyIndex);

					if (Tower.Profile.SplashRadius > 0.f)
					{
						for (int32 SplashIndex = 0; SplashIndex < Enemies.Num(); ++SplashIndex)
						{
							if (SplashIndex == CurrentEnemyIndex || HitEnemyIndices.Contains(SplashIndex) || !Enemies.IsValidIndex(SplashIndex) || Enemies[SplashIndex].bPendingRemoval)
							{
								continue;
							}
							if (!CanTowerHitEnemy(Tower.Profile, Enemies[SplashIndex]))
							{
								continue;
							}

							const FVector2D SplashPosition = SampleEnemyPosition(Enemies[SplashIndex]);
							if (DistanceSquared(EnemyPosition, SplashPosition) <= FMath::Square(Tower.Profile.SplashRadius))
							{
								ApplyDamageToEnemy(Enemies[SplashIndex], Tower.Profile.Damage * 0.68f, Tower.Profile, Tower);
							}
						}
					}

					if (ChainIndex < Tower.Profile.ChainBounces)
					{
						int32 BestBounceTarget = INDEX_NONE;
						float BestBounceDistance = Tower.Profile.ChainRadius * Tower.Profile.ChainRadius;
						for (int32 CandidateIndex = 0; CandidateIndex < Enemies.Num(); ++CandidateIndex)
						{
							if (HitEnemyIndices.Contains(CandidateIndex) || !Enemies.IsValidIndex(CandidateIndex) || Enemies[CandidateIndex].bPendingRemoval)
							{
								continue;
							}
							if (!CanTowerHitEnemy(Tower.Profile, Enemies[CandidateIndex]))
							{
								continue;
							}

							const FVector2D CandidatePosition = SampleEnemyPosition(Enemies[CandidateIndex]);
							const float CandidateDistance = DistanceSquared(EnemyPosition, CandidatePosition);
							if (CandidateDistance <= BestBounceDistance)
							{
								BestBounceDistance = CandidateDistance;
								BestBounceTarget = CandidateIndex;
							}
						}

						if (BestBounceTarget != INDEX_NONE)
						{
							PendingChainTargets.Add(BestBounceTarget);
							PreviousBeamOrigin = EnemyPosition;
							DamageScale *= 0.78f;
						}
					}
				}
			}
		}

		int32 FindNearestOpenPad(const FVector2D& NormalizedPoint, const float Threshold) const
		{
			int32 BestPadIndex = INDEX_NONE;
			float BestDistanceSq = FMath::Square(Threshold);

			for (int32 PadIndex = 0; PadIndex < LayoutDefinition.Pads.Num(); ++PadIndex)
			{
				if (FindTowerIndexByPad(PadIndex) != INDEX_NONE)
				{
					continue;
				}

				const float CandidateDistanceSq = DistanceSquared(LayoutDefinition.Pads[PadIndex].PositionNormalized, NormalizedPoint);
				if (CandidateDistanceSq <= BestDistanceSq)
				{
					BestDistanceSq = CandidateDistanceSq;
					BestPadIndex = PadIndex;
				}
			}

			return BestPadIndex;
		}

		int32 FindTowerPadNearPoint(const FVector2D& NormalizedPoint, const float Threshold) const
		{
			int32 BestPadIndex = INDEX_NONE;
			float BestDistanceSq = FMath::Square(Threshold);

			for (const FT66TDPlacedTower& Tower : Towers)
			{
				const float CandidateDistanceSq = DistanceSquared(Tower.PositionNormalized, NormalizedPoint);
				if (CandidateDistanceSq <= BestDistanceSq)
				{
					BestDistanceSq = CandidateDistanceSq;
					BestPadIndex = Tower.PadIndex;
				}
			}

			return BestPadIndex;
		}

		int32 FindTowerIndexByPad(const int32 PadIndex) const
		{
			for (int32 TowerIndex = 0; TowerIndex < Towers.Num(); ++TowerIndex)
			{
				if (Towers[TowerIndex].PadIndex == PadIndex)
				{
					return TowerIndex;
				}
			}
			return INDEX_NONE;
		}

		const FT66TDPlacedTower* FindTowerByPad(const int32 PadIndex) const
		{
			const int32 TowerIndex = FindTowerIndexByPad(PadIndex);
			return Towers.IsValidIndex(TowerIndex) ? &Towers[TowerIndex] : nullptr;
		}

		const FT66TDHeroDefinition* FindHeroDefinitionByID(const FName HeroID) const
		{
			return HeroLookup.Find(HeroID);
		}

		FT66TDPlacedTower* FindMutableTowerByPad(const int32 PadIndex)
		{
			const int32 TowerIndex = FindTowerIndexByPad(PadIndex);
			return Towers.IsValidIndex(TowerIndex) ? &Towers[TowerIndex] : nullptr;
		}

		int32 GetUpgradeLevel(const FT66TDPlacedTower& Tower, const ET66TDTowerUpgradeType UpgradeType) const
		{
			switch (UpgradeType)
			{
			case ET66TDTowerUpgradeType::Damage:
				return Tower.DamageUpgradeLevel;
			case ET66TDTowerUpgradeType::Range:
				return Tower.RangeUpgradeLevel;
			case ET66TDTowerUpgradeType::Tempo:
			default:
				return Tower.TempoUpgradeLevel;
			}
		}

		int32 GetUpgradeCost(const FT66TDPlacedTower& Tower, const ET66TDTowerUpgradeType UpgradeType) const
		{
			const int32 CurrentLevel = GetUpgradeLevel(Tower, UpgradeType);
			switch (UpgradeType)
			{
			case ET66TDTowerUpgradeType::Damage:
				return GetTuningInt(TEXT("DamageUpgradeCostBase"), 75) + (CurrentLevel * GetTuningInt(TEXT("DamageUpgradeCostPerLevel"), 55));
			case ET66TDTowerUpgradeType::Range:
				return GetTuningInt(TEXT("RangeUpgradeCostBase"), 65) + (CurrentLevel * GetTuningInt(TEXT("RangeUpgradeCostPerLevel"), 45));
			case ET66TDTowerUpgradeType::Tempo:
			default:
				return GetTuningInt(TEXT("TempoUpgradeCostBase"), 80) + (CurrentLevel * GetTuningInt(TEXT("TempoUpgradeCostPerLevel"), 60));
			}
		}

		int32 GetTowerSellValue(const FT66TDPlacedTower& Tower) const
		{
			return FMath::RoundToInt(Tower.MaterialsInvested * GetTuningValue(TEXT("TowerSellScalar"), 0.70f));
		}

		FString GetUpgradeTrackLabelString(const FT66TDPlacedTower* Tower, const ET66TDTowerUpgradeType UpgradeType) const
		{
			if (!Tower)
			{
				switch (UpgradeType)
				{
				case ET66TDTowerUpgradeType::Damage:
					return TEXT("Power Upgrade");
				case ET66TDTowerUpgradeType::Range:
					return TEXT("Range Upgrade");
				case ET66TDTowerUpgradeType::Tempo:
				default:
					return TEXT("Tempo Upgrade");
				}
			}

			const FT66TDHeroDefinition* HeroDefinition = FindHeroDefinitionByID(Tower->HeroID);
			if (!HeroDefinition)
			{
				return GetUpgradeTrackLabelString(nullptr, UpgradeType);
			}

			switch (UpgradeType)
			{
			case ET66TDTowerUpgradeType::Damage:
				return HeroDefinition->PassiveLabel;
			case ET66TDTowerUpgradeType::Range:
				return HeroDefinition->Signature;
			case ET66TDTowerUpgradeType::Tempo:
			default:
				return HeroDefinition->UltimateLabel;
			}
		}

		FString GetUpgradeTrackEffectHint(const FT66TDPlacedTower& Tower, const ET66TDTowerUpgradeType UpgradeType) const
		{
			const FT66TDHeroCombatProfile& FlavorProfile = Tower.BaseProfile;
			switch (UpgradeType)
			{
			case ET66TDTowerUpgradeType::Damage:
				if (FlavorProfile.ChainBounces > 0)
				{
					return TEXT("heavier arc hits and nastier lane punishment");
				}
				if (FlavorProfile.SplashRadius > 0.f)
				{
					return TEXT("harder impact bursts and stronger crowd clears");
				}
				if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					return TEXT("sharper DOT pressure and stronger attrition");
				}
				if (FlavorProfile.SlowDuration > 0.f)
				{
					return TEXT("harder control hits and better stall pressure");
				}
				if (FlavorProfile.BonusMaterialsOnKill > 0)
				{
					return TEXT("heavier farm clears and better payout scaling");
				}
				if (FlavorProfile.Range >= 0.20f)
				{
					return TEXT("deadlier priority shots and stronger boss pressure");
				}
				return TEXT("harder lane hits and cleaner leak cleanup");

			case ET66TDTowerUpgradeType::Range:
				if (FlavorProfile.ChainBounces > 0)
				{
					return TEXT("more reach plus wider bounce routing");
				}
				if (FlavorProfile.SplashRadius > 0.f)
				{
					return TEXT("broader coverage and larger blast reach");
				}
				if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					return TEXT("longer infection windows and safer spread setups");
				}
				if (FlavorProfile.SlowDuration > 0.f)
				{
					return TEXT("wider stall coverage and longer freeze windows");
				}
				return TEXT("safer coverage and stronger cross-map pressure");

			case ET66TDTowerUpgradeType::Tempo:
			default:
				if (FlavorProfile.FireInterval <= 0.34f)
				{
					return TEXT("faster attack rhythm with multi-shot capstones");
				}
				if (FlavorProfile.ChainBounces > 0)
				{
					return TEXT("storm cadence spikes and chains hit more often");
				}
				if (FlavorProfile.SplashRadius > 0.f)
				{
					return TEXT("faster bursts and better panic clearing");
				}
				if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					return TEXT("faster applications and better stack uptime");
				}
				if (FlavorProfile.BonusMaterialsOnKill > 0)
				{
					return TEXT("faster farming cadence and greed scaling");
				}
				return TEXT("faster attacks and stronger late-wave tempo");
			}
		}

		FString GetUpgradeCapstoneHint(const FT66TDPlacedTower& Tower, const ET66TDTowerUpgradeType UpgradeType) const
		{
			const FT66TDHeroCombatProfile& FlavorProfile = Tower.BaseProfile;
			switch (UpgradeType)
			{
			case ET66TDTowerUpgradeType::Damage:
				if (FlavorProfile.ChainBounces > 0)
				{
					return TEXT("III adds another bounce");
				}
				if (FlavorProfile.SplashRadius > 0.f)
				{
					return TEXT("III expands blast radius");
				}
				if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					return TEXT("III sharpens toxin ticks");
				}
				if (FlavorProfile.SlowDuration > 0.f)
				{
					return TEXT("III makes control hits bite harder");
				}
				if (FlavorProfile.BonusMaterialsOnKill > 0)
				{
					return TEXT("III raises kill payouts");
				}
				if (FlavorProfile.Range >= 0.20f)
				{
					return TEXT("III locks onto bosses harder");
				}
				return TEXT("III spikes leak-clutch damage");

			case ET66TDTowerUpgradeType::Range:
				if (FlavorProfile.ChainBounces > 0)
				{
					return TEXT("III widens jump radius");
				}
				if (FlavorProfile.SplashRadius > 0.f)
				{
					return TEXT("III broadens splash coverage");
				}
				if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					return TEXT("III extends infection duration");
				}
				if (FlavorProfile.SlowDuration > 0.f)
				{
					return TEXT("III lengthens stall windows");
				}
				if (FlavorProfile.Range >= 0.18f)
				{
					return TEXT("III improves boss sightlines");
				}
				return TEXT("III opens wider crossfire windows");

			case ET66TDTowerUpgradeType::Tempo:
			default:
				if (FlavorProfile.FireInterval <= 0.34f)
				{
					return TEXT("III adds an extra volley shot");
				}
				if (FlavorProfile.ChainBounces > 0)
				{
					return TEXT("III adds another storm jump");
				}
				if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					return TEXT("III refreshes damage-over-time longer");
				}
				if (FlavorProfile.SplashRadius > 0.f)
				{
					return TEXT("III accelerates burst clears");
				}
				if (FlavorProfile.BonusMaterialsOnKill > 0)
				{
					return TEXT("III speeds up farm cycles");
				}
				return TEXT("III pushes attack cadence harder");
			}
		}

		FString GetUpgradeTrackSummaryString(const FT66TDPlacedTower& Tower, const ET66TDTowerUpgradeType UpgradeType) const
		{
			const int32 CurrentLevel = GetUpgradeLevel(Tower, UpgradeType);
			return FString::Printf(
				TEXT("%s %d/%d: %s. %s."),
				*GetUpgradeTrackLabelString(&Tower, UpgradeType),
				CurrentLevel,
				GetTuningInt(TEXT("UpgradeMaxLevel"), 3),
				*GetUpgradeTrackEffectHint(Tower, UpgradeType),
				*GetUpgradeCapstoneHint(Tower, UpgradeType));
		}

		bool CanUpgradeTower(const FT66TDPlacedTower& Tower, const ET66TDTowerUpgradeType UpgradeType) const
		{
			return GetUpgradeLevel(Tower, UpgradeType) < GetTuningInt(TEXT("UpgradeMaxLevel"), 3) && Materials >= GetUpgradeCost(Tower, UpgradeType);
		}

		bool CanUpgradeSelectedTower(const ET66TDTowerUpgradeType UpgradeType) const
		{
			const FT66TDPlacedTower* SelectedTower = FindTowerByPad(SelectedPadIndex);
			return SelectedTower != nullptr && CanUpgradeTower(*SelectedTower, UpgradeType);
		}

		FText GetUpgradeButtonText(const ET66TDTowerUpgradeType UpgradeType) const
		{
			const FT66TDPlacedTower* SelectedTower = FindTowerByPad(SelectedPadIndex);
			if (!SelectedTower)
			{
				return FText::FromString(GetUpgradeTrackLabelString(nullptr, UpgradeType).ToUpper());
			}

			const int32 CurrentLevel = GetUpgradeLevel(*SelectedTower, UpgradeType);
			const FString UpgradeLabel = GetUpgradeTrackLabelString(SelectedTower, UpgradeType).ToUpper();
			if (CurrentLevel >= GetTuningInt(TEXT("UpgradeMaxLevel"), 3))
			{
				return FText::FromString(FString::Printf(TEXT("%s  |  MAX"), *UpgradeLabel));
			}

			return FText::Format(
				NSLOCTEXT("T66TD.Battle", "UpgradeButtonFmt", "{0}  |  {1}"),
				FText::FromString(UpgradeLabel),
				FText::AsNumber(GetUpgradeCost(*SelectedTower, UpgradeType)));
		}

		void ApplyUpgradeCapstone(FT66TDPlacedTower& Tower, const ET66TDTowerUpgradeType UpgradeType)
		{
			const FT66TDHeroCombatProfile& FlavorProfile = Tower.BaseProfile;
			switch (UpgradeType)
			{
			case ET66TDTowerUpgradeType::Damage:
				if (FlavorProfile.ChainBounces > 0)
				{
					++Tower.Profile.ChainBounces;
				}
				else if (FlavorProfile.SplashRadius > 0.f)
				{
					Tower.Profile.SplashRadius += 0.018f;
				}
				else if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					Tower.Profile.DotDamagePerSecond += 3.5f;
					Tower.Profile.DotDuration += 0.35f;
				}
				else if (FlavorProfile.SlowDuration > 0.f)
				{
					Tower.Profile.Damage += 8.f;
					Tower.Profile.SlowDuration += 0.25f;
				}
				else if (FlavorProfile.BonusMaterialsOnKill > 0)
				{
					Tower.Profile.BonusMaterialsOnKill += 2;
				}
				else if (FlavorProfile.Range >= 0.20f)
				{
					Tower.Profile.BossDamageMultiplier += GetTuningValue(TEXT("CapstoneDamageBossMultiplierAdd"), 0.35f);
					Tower.Profile.bPrioritizeBoss = true;
				}
				else
				{
					Tower.Profile.Damage += 10.f;
				}
				break;

			case ET66TDTowerUpgradeType::Range:
				if (FlavorProfile.ChainBounces > 0)
				{
					Tower.Profile.ChainRadius += 0.05f;
				}
				else if (FlavorProfile.SplashRadius > 0.f)
				{
					Tower.Profile.SplashRadius += 0.016f;
				}
				else if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					Tower.Profile.DotDuration += 0.45f;
				}
				else if (FlavorProfile.SlowDuration > 0.f)
				{
					Tower.Profile.SlowDuration += 0.45f;
				}
				else if (FlavorProfile.Range >= 0.18f)
				{
					Tower.Profile.bPrioritizeBoss = true;
					Tower.Profile.BossDamageMultiplier += GetTuningValue(TEXT("CapstoneRangeBossMultiplierAdd"), 0.15f);
				}
				else
				{
					Tower.Profile.Range *= 1.04f;
				}
				break;

			case ET66TDTowerUpgradeType::Tempo:
			default:
				if (FlavorProfile.FireInterval <= 0.34f)
				{
					++Tower.Profile.VolleyShots;
				}
				else if (FlavorProfile.ChainBounces > 0)
				{
					++Tower.Profile.ChainBounces;
				}
				else if (FlavorProfile.DotDamagePerSecond > 0.f)
				{
					Tower.Profile.DotDuration += 0.60f;
					Tower.Profile.DotDamagePerSecond += 2.0f;
				}
				else if (FlavorProfile.SplashRadius > 0.f)
				{
					Tower.Profile.Damage *= 1.10f;
				}
				else if (FlavorProfile.BonusMaterialsOnKill > 0)
				{
					++Tower.Profile.BonusMaterialsOnKill;
				}
				else
				{
					Tower.Profile.FireInterval = FMath::Max(
						GetTuningValue(TEXT("CapstoneFireIntervalMin"), 0.10f),
						Tower.Profile.FireInterval * GetTuningValue(TEXT("CapstoneTempoFireIntervalMultiplier"), 0.88f));
				}
				break;
			}
		}

		bool TryUpgradeSelectedTower(const ET66TDTowerUpgradeType UpgradeType)
		{
			FT66TDPlacedTower* SelectedTower = FindMutableTowerByPad(SelectedPadIndex);
			if (!SelectedTower || !CanUpgradeTower(*SelectedTower, UpgradeType))
			{
				return false;
			}

			const int32 UpgradeCost = GetUpgradeCost(*SelectedTower, UpgradeType);
			Materials -= UpgradeCost;
			SelectedTower->MaterialsInvested += UpgradeCost;

			switch (UpgradeType)
			{
			case ET66TDTowerUpgradeType::Damage:
				++SelectedTower->DamageUpgradeLevel;
				SelectedTower->Profile.Damage *= GetTuningValue(TEXT("DamageUpgradeMultiplier"), 1.35f);
				SelectedTower->Profile.DotDamagePerSecond *= GetTuningValue(TEXT("DotDamageUpgradeMultiplier"), 1.25f);
				if (SelectedTower->Profile.SplashRadius > 0.f)
				{
					SelectedTower->Profile.SplashRadius *= GetTuningValue(TEXT("SplashDamageUpgradeMultiplier"), 1.05f);
				}
				break;

			case ET66TDTowerUpgradeType::Range:
				++SelectedTower->RangeUpgradeLevel;
				SelectedTower->Profile.Range *= GetTuningValue(TEXT("RangeUpgradeMultiplier"), 1.16f);
				SelectedTower->Profile.ChainRadius *= GetTuningValue(TEXT("ChainRadiusUpgradeMultiplier"), 1.18f);
				if (SelectedTower->Profile.SplashRadius > 0.f)
				{
					SelectedTower->Profile.SplashRadius *= GetTuningValue(TEXT("SplashRadiusUpgradeMultiplier"), 1.14f);
				}
				if (SelectedTower->Profile.SlowDuration > 0.f)
				{
					SelectedTower->Profile.SlowDuration += GetTuningValue(TEXT("SlowDurationUpgradeAdd"), 0.20f);
				}
				break;

			case ET66TDTowerUpgradeType::Tempo:
			default:
				++SelectedTower->TempoUpgradeLevel;
				SelectedTower->Profile.FireInterval = FMath::Max(
					GetTuningValue(TEXT("TempoFireIntervalMin"), 0.12f),
					SelectedTower->Profile.FireInterval * GetTuningValue(TEXT("TempoFireIntervalMultiplier"), 0.84f));
				if (SelectedTower->Profile.DotDuration > 0.f)
				{
					SelectedTower->Profile.DotDuration += GetTuningValue(TEXT("TempoDotDurationAdd"), 0.35f);
				}
				if (SelectedTower->Profile.SlowMultiplier < 0.999f)
				{
					SelectedTower->Profile.SlowMultiplier = FMath::Max(
						GetTuningValue(TEXT("SlowMultiplierUpgradeMin"), 0.45f),
						SelectedTower->Profile.SlowMultiplier * GetTuningValue(TEXT("SlowMultiplierUpgradeMultiplier"), 0.94f));
				}
				break;
			}

			if (GetUpgradeLevel(*SelectedTower, UpgradeType) >= GetTuningInt(TEXT("UpgradeMaxLevel"), 3))
			{
				ApplyUpgradeCapstone(*SelectedTower, UpgradeType);
			}

			return true;
		}

		FT66TDMapDefinition MapDefinition;
		FT66TDDifficultyDefinition DifficultyDefinition;
		FT66TDMapLayoutDefinition LayoutDefinition;
		FT66TDStageDefinition StageDefinition;
		TArray<FT66TDHeroDefinition> HeroDefinitions;
		TArray<FT66TDHeroCombatDefinition> HeroCombatDefinitions;
		TArray<FT66TDEnemyArchetypeDefinition> EnemyArchetypeDefinitions;
		TArray<FT66TDThemeModifierRule> ThemeModifierRules;
		TMap<FName, FT66TDHeroDefinition> HeroLookup;
		TMap<FName, FT66TDHeroCombatDefinition> HeroCombatLookup;
		TMap<FName, FT66TDHeroCombatProfile> HeroProfiles;
		TMap<FName, FT66TDEnemyArchetype> EnemyArchetypeLookup;
		TMap<FName, FT66TDThemeModifierRule> ThemeModifierLookup;
		FT66TDBattleTuningMap BattleTuningValues;
		FT66TDHeroBrushMap HeroBrushes;
		FT66TDEnemyBrushMap EnemyBrushes;
		FT66TDEnemyBrushMap BossBrushes;
		TArray<FT66TDPathRuntime> PathRuntimes;
		TArray<FT66TDPlacedTower> Towers;
		TArray<FT66TDActiveEnemy> Enemies;
		TArray<FT66TDQueuedSpawn> PendingSpawns;
		TArray<FT66TDBeamEffect> BeamEffects;
		TSharedPtr<FActiveTimerHandle> ActiveTimerHandle;
		TWeakObjectPtr<UGameInstance> OwningGameInstance;
		ET66TDMatchState MatchState = ET66TDMatchState::AwaitingWave;
		int32 Materials = 0;
		int32 Gold = 0;
		int32 Hearts = 0;
		int32 CurrentWave = 0;
		int32 SelectedPadIndex = INDEX_NONE;
		int32 PreviewPadIndex = INDEX_NONE;
		int32 NextSpawnIndex = 0;
		float TimeUntilNextSpawn = 0.f;
		float SimulationSpeed = 0.f;
		bool bPreviewPlacementValid = false;
		bool bLeaderboardResultSubmitted = false;
		bool bStageResultRecorded = false;
	};
}

UT66TDBattleScreen::UT66TDBattleScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::TDBattle;
	bIsModal = false;
}

void UT66TDBattleScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	EnsureRunSelectionState();
}

void UT66TDBattleScreen::OnScreenDeactivated_Implementation()
{
	ReleaseRetainedSlateState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66TDBattleScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

TSharedRef<SWidget> UT66TDBattleScreen::BuildSlateUI()
{
	EnsureRunSelectionState();

	UGameInstance* GameInstance = GetGameInstance();
	UT66TDDataSubsystem* TDDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDDataSubsystem>() : nullptr;
	UT66TDFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
	UT66TDVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDVisualSubsystem>() : nullptr;
	const FT66TDDifficultyDefinition* SelectedDifficulty = (TDDataSubsystem && FrontendState) ? TDDataSubsystem->FindDifficulty(FrontendState->GetSelectedDifficultyID()) : nullptr;
	const FT66TDMapDefinition* SelectedMap = (TDDataSubsystem && FrontendState) ? TDDataSubsystem->FindMap(FrontendState->GetSelectedMapID()) : nullptr;
	const FT66TDStageDefinition* SelectedStage = (TDDataSubsystem && FrontendState && FrontendState->HasSelectedStage()) ? TDDataSubsystem->FindStage(FrontendState->GetSelectedStageID()) : nullptr;
	if (!SelectedStage && TDDataSubsystem && SelectedMap)
	{
		SelectedStage = TDDataSubsystem->FindStageForMap(SelectedMap->MapID);
	}
	const FT66TDMapLayoutDefinition* SelectedLayout = (TDDataSubsystem && SelectedMap) ? TDDataSubsystem->FindLayout(SelectedMap->MapID) : nullptr;

	if (!TDDataSubsystem || !SelectedDifficulty || !SelectedMap || !SelectedLayout)
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(T66TDUI::ShellFill())
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(24.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66TD.Battle", "UnavailableTitle", "TD battle data is unavailable."))
					.Font(FT66Style::MakeFont(TEXT("Black"), 28))
					.ColorAndOpacity(T66TDUI::BrightText())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(24.f, 0.f, 24.f, 24.f)
				[
					FT66Style::MakeButton(T66TDUI::MakeUtilityButtonParams(
						NSLOCTEXT("T66TD.Battle", "Back", "BACK"),
						FOnClicked::CreateUObject(this, &UT66TDBattleScreen::HandleBackClicked),
						180.f,
						42.f,
						14))
				]
			];
	}

	const TSharedPtr<FSlateBrush>& BackgroundBrush = FindOrLoadMapBrush(SelectedMap->BackgroundRelativePath);
	FT66TDMapDefinition RuntimeMap = *SelectedMap;
	if (SelectedStage && SelectedStage->BossWave > 0)
	{
		RuntimeMap.BossWave = SelectedStage->BossWave;
	}

	TArray<FT66TDHeroDefinition> OrderedHeroes = TDDataSubsystem->GetHeroes();
	OrderedHeroes.Sort([SelectedMap](const FT66TDHeroDefinition& Left, const FT66TDHeroDefinition& Right)
	{
		const bool bLeftFeatured = SelectedMap->FeaturedHeroIDs.Contains(Left.HeroID);
		const bool bRightFeatured = SelectedMap->FeaturedHeroIDs.Contains(Right.HeroID);
		if (bLeftFeatured != bRightFeatured)
		{
			return bLeftFeatured;
		}
		return Left.HeroID.LexicalLess(Right.HeroID);
	});

	HeroSpriteBrushes.Reset();
	EnemySpriteBrushes.Reset();
	BossSpriteBrushes.Reset();

	if (VisualSubsystem)
	{
		for (const FT66TDHeroDefinition& HeroDefinition : OrderedHeroes)
		{
			if (UTexture2D* HeroTexture = VisualSubsystem->LoadHeroTexture(HeroDefinition.HeroID, HeroDefinition.DisplayName))
			{
				HeroSpriteBrushes.Add(HeroDefinition.HeroID, MakeTextureBrush(HeroTexture, FVector2D(64.f, 64.f)));
			}
		}

		TArray<FString> EnemyVisualIDs;
		for (const FT66TDEnemyArchetypeDefinition& ArchetypeDefinition : TDDataSubsystem->GetEnemyArchetypes())
		{
			if (!ArchetypeDefinition.VisualID.IsEmpty())
			{
				EnemyVisualIDs.AddUnique(ArchetypeDefinition.VisualID);
			}
		}
		for (const FString& EnemyVisualID : EnemyVisualIDs)
		{
			if (UTexture2D* EnemyTexture = VisualSubsystem->LoadEnemyTexture(EnemyVisualID))
			{
				EnemySpriteBrushes.Add(EnemyVisualID, MakeTextureBrush(EnemyTexture, FVector2D(48.f, 48.f)));
			}
			if (UTexture2D* BossTexture = VisualSubsystem->LoadBossTexture(EnemyVisualID))
			{
				BossSpriteBrushes.Add(EnemyVisualID, MakeTextureBrush(BossTexture, FVector2D(72.f, 72.f)));
			}
		}
	}

	TSharedPtr<ST66TDBattleBoardWidget> BattleBoard;
	SAssignNew(BattleBoard, ST66TDBattleBoardWidget)
		.MapDefinition(RuntimeMap)
		.DifficultyDefinition(*SelectedDifficulty)
		.LayoutDefinition(*SelectedLayout)
		.StageDefinition(SelectedStage ? *SelectedStage : FT66TDStageDefinition())
		.Heroes(OrderedHeroes)
		.HeroCombatDefinitions(TDDataSubsystem->GetHeroCombatDefinitions())
		.EnemyArchetypes(TDDataSubsystem->GetEnemyArchetypes())
		.BattleTuningValues(TDDataSubsystem->GetBattleTuningValues())
		.ThemeModifierRules(TDDataSubsystem->GetThemeModifierRules())
		.HeroBrushes(HeroSpriteBrushes)
		.EnemyBrushes(EnemySpriteBrushes)
		.BossBrushes(BossSpriteBrushes)
		.OwningGameInstance(TWeakObjectPtr<UGameInstance>(GameInstance));
	BattleBoardRoot = BattleBoard;

	TSharedRef<SVerticalBox> HeroRoster = SNew(SVerticalBox);
	for (const FT66TDHeroDefinition& HeroDefinition : OrderedHeroes)
	{
		const FT66TDHeroCombatProfile Profile = BuildHeroProfile(HeroDefinition, TDDataSubsystem->FindHeroCombatDefinition(HeroDefinition.HeroID));
		HeroRoster->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(ST66TDHeroRosterTile)
			.HeroID(HeroDefinition.HeroID)
			.DisplayName(HeroDefinition.DisplayName)
			.Cost(Profile.Cost)
			.Tint(HeroDefinition.PlaceholderColor)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Transparent)
			.Padding(FMargin(1.f))
			[
				T66TDUI::MakeGeneratedPanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(0.f, 0.f, 10.f, 0.f)
					[
						MakeSpriteWidget(
							HeroSpriteBrushes.FindRef(HeroDefinition.HeroID),
							HeroDefinition.DisplayName.Left(1).ToUpper(),
							HeroDefinition.PlaceholderColor,
							FVector2D(54.f, 54.f),
							16)
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(HeroDefinition.DisplayName))
							.Font(FT66Style::MakeFont(TEXT("Bold"), 15))
							.ColorAndOpacity(T66TDUI::BrightText())
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::Format(
								NSLOCTEXT("T66TD.Battle", "HeroRosterMetaFmt", "{0}  |  {1} materials"),
								FText::FromString(Profile.CombatLabel),
								FText::AsNumber(Profile.Cost)))
							.Font(FT66Style::MakeFont(TEXT("Regular"), 10))
							.ColorAndOpacity(T66TDUI::MutedText())
							.AutoWrapText(true)
						]
					]
					,
					TDBattleRosterRowBrush(),
					T66TDUI::CardFill(),
					FMargin(12.f, 9.f))
			]
		];
	}

	const TAttribute<FText> MaterialsText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetMaterialsText);
	const TAttribute<FText> HeartsText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetHeartsText);
	const TAttribute<FText> WaveText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetWaveText);
	const TAttribute<FText> ThreatText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetThreatText);
	const TAttribute<FText> StatusText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetStatusText);
	const TAttribute<FText> PrimaryActionText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetPrimaryActionText);
	const TAttribute<FText> SpeedText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetSpeedText);
	const TAttribute<FText> SelectedTowerTitle = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetSelectedTowerTitle);
	const TAttribute<FText> SelectedTowerBody = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetSelectedTowerBody);
	const TAttribute<FText> DamageUpgradeText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetDamageUpgradeText);
	const TAttribute<FText> RangeUpgradeText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetRangeUpgradeText);
	const TAttribute<FText> TempoUpgradeText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetTempoUpgradeText);
	const TAttribute<FText> SellSelectedTowerText = TAttribute<FText>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::GetSellSelectedTowerText);
	const TAttribute<bool> CanUpgradeDamage = TAttribute<bool>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::CanUpgradeDamage);
	const TAttribute<bool> CanUpgradeRange = TAttribute<bool>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::CanUpgradeRange);
	const TAttribute<bool> CanUpgradeTempo = TAttribute<bool>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::CanUpgradeTempo);
	const TAttribute<bool> CanSellSelectedTower = TAttribute<bool>::CreateSP(BattleBoard.Get(), &ST66TDBattleBoardWidget::CanSellSelectedTower);

	const FLinearColor BrightText = T66TDUI::BrightText();
	const FLinearColor MutedText = T66TDUI::MutedText();
	const auto MakeDynamicActionButton = [&](const TAttribute<FText>& Label, const FOnClicked& OnClicked, const ET66ButtonType ButtonType, const FVector2D& Size, const int32 FontSize) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(), OnClicked, ButtonType)
			.SetDynamicLabel(Label)
			.SetMinWidth(Size.X)
			.SetHeight(Size.Y)
			.SetFontSize(FontSize)
			.SetPadding(FMargin(12.f, 8.f, 12.f, 6.f))
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(TDBattleButtonBrush(ButtonType))
			.SetTextColor(T66TDUI::BrightText()));
	};

	const auto MakeUpgradeButton = [&](const TAttribute<FText>& Label, const TAttribute<bool>& bEnabled, const FOnClicked& OnClicked, const ET66ButtonType ButtonType) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(), OnClicked, ButtonType)
			.SetDynamicLabel(Label)
			.SetEnabled(bEnabled)
			.SetMinWidth(320.f)
			.SetHeight(38.f)
			.SetFontSize(11)
			.SetPadding(FMargin(12.f, 7.f, 12.f, 5.f))
			.SetUseGlow(false)
			.SetUseDotaPlateOverlay(true)
			.SetDotaPlateOverrideBrush(TDBattleButtonBrush(ButtonType))
			.SetTextColor(T66TDUI::BrightText()));
	};

	FT66ButtonParams BackButtonParams = T66TDUI::MakeUtilityButtonParams(
		NSLOCTEXT("T66TD.Battle", "BackToMaps", "BACK TO MAPS"),
		FOnClicked::CreateUObject(this, &UT66TDBattleScreen::HandleBackClicked),
		260.f,
		44.f,
		10);
	BackButtonParams.SetDotaPlateOverrideBrush(TDBattleButtonBrush(ET66ButtonType::Neutral));

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(T66TDUI::ShellFill())
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				MakeOptionalImage(BackgroundBrush)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.76f))
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(20.f, 20.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(260.f)
				.HeightOverride(44.f)
				[
					FT66Style::MakeButton(BackButtonParams)
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 24.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("%s | %s"), *SelectedMap->DisplayName.ToUpper(), *SelectedDifficulty->DisplayName.ToUpper())))
				.Font(FT66Style::MakeFont(TEXT("Black"), 30))
				.ColorAndOpacity(T66TDUI::AccentGold())
			]
			+ SOverlay::Slot().Padding(FMargin(22.f, 84.f, 22.f, 22.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
				[
					SNew(SBox).WidthOverride(320.f)
					[
						T66TDUI::MakeGeneratedPanel(
							SNew(SVerticalBox)
							+ SVerticalBox::Slot().AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66TD.Battle", "RosterTitle", "HERO ROSTER"))
								.Font(FT66Style::MakeFont(TEXT("Black"), 20))
								.ColorAndOpacity(T66TDUI::AccentGold())
							]
							+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 12.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66TD.Battle", "RosterBody", "Drag any hero onto an open pad. Featured picks for this map are sorted first."))
								.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
								.ColorAndOpacity(MutedText)
								.AutoWrapText(true)
							]
							+ SVerticalBox::Slot().FillHeight(1.f)
							[
								SNew(SScrollBox)
								+ SScrollBox::Slot()
								[
									HeroRoster
								]
							]
							,
							TDBattleRosterPanelBrush(),
							T66TDUI::ShellFill(),
							FMargin(22.f, 20.f))
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 16.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						T66TDUI::MakeGeneratedPanel(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
							[
								SNew(STextBlock)
								.Text(MaterialsText)
								.Font(FT66Style::MakeFont(TEXT("Bold"), 14))
								.ColorAndOpacity(BrightText)
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 18.f, 0.f)
							[
								SNew(STextBlock)
								.Text(HeartsText)
								.Font(FT66Style::MakeFont(TEXT("Bold"), 14))
								.ColorAndOpacity(BrightText)
							]
							+ SHorizontalBox::Slot().AutoWidth()
							[
								SNew(STextBlock)
								.Text(WaveText)
								.Font(FT66Style::MakeFont(TEXT("Bold"), 14))
								.ColorAndOpacity(BrightText)
							]
							,
							TDBattleStatsBarBrush(),
							T66TDUI::PanelFill(),
							FMargin(18.f, 12.f))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
					[
						T66TDUI::MakeGeneratedPanel(
							SNew(SBox)
							.WidthOverride(TDBattleBoardSize.X)
							.HeightOverride(TDBattleBoardSize.Y)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								[
									MakeOptionalImage(BackgroundBrush)
								]
								+ SOverlay::Slot()
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.14f))
								]
								+ SOverlay::Slot()
								[
									BattleBoard.ToSharedRef()
								]
							]
							,
							TDBattleBoardFrameBrush(),
							T66TDUI::PanelFill(),
							FMargin(10.f))
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(42.f)
						[
							T66TDUI::MakeGeneratedPanel(
								SNew(STextBlock)
								.Text(StatusText)
								.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
								.ColorAndOpacity(MutedText)
								.AutoWrapText(true)
								.Justification(ETextJustify::Center)
								,
								TDBattleStatusBarBrush(),
								T66TDUI::PanelFill(),
								FMargin(14.f, 6.f))
						]
					]
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(360.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							T66TDUI::MakeGeneratedPanel(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66TD.Battle", "MatchPanelTitle", "MATCH CONTROL"))
									.Font(FT66Style::MakeFont(TEXT("Black"), 20))
									.ColorAndOpacity(T66TDUI::AccentGold())
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
								[
									MakeDynamicActionButton(
										PrimaryActionText,
										FOnClicked::CreateSP(BattleBoard.ToSharedRef(), &ST66TDBattleBoardWidget::HandlePrimaryActionClicked),
										ET66ButtonType::Success,
										FVector2D(320.f, 54.f),
										15)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
								[
									MakeDynamicActionButton(
										SpeedText,
										FOnClicked::CreateSP(BattleBoard.ToSharedRef(), &ST66TDBattleBoardWidget::HandleSpeedClicked),
										ET66ButtonType::Neutral,
										FVector2D(320.f, 42.f),
										13)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
								[
								SNew(STextBlock)
								.Text(FText::FromString(SelectedMap->EnemyNotes))
								.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
								.ColorAndOpacity(MutedText)
								.AutoWrapText(true)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(ThreatText)
									.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
									.ColorAndOpacity(BrightText)
									.AutoWrapText(true)
								]
								,
								TDBattleMatchPanelBrush(),
								T66TDUI::ShellFill(),
								FMargin(24.f, 22.f))
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
						[
							T66TDUI::MakeGeneratedPanel(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text(SelectedTowerTitle)
									.Font(FT66Style::MakeFont(TEXT("Black"), 20))
									.ColorAndOpacity(BrightText)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(SelectedTowerBody)
									.Font(FT66Style::MakeFont(TEXT("Regular"), 11))
									.ColorAndOpacity(MutedText)
									.AutoWrapText(true)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
								[
									MakeUpgradeButton(
										DamageUpgradeText,
										CanUpgradeDamage,
										FOnClicked::CreateSP(BattleBoard.ToSharedRef(), &ST66TDBattleBoardWidget::HandleDamageUpgradeClicked),
										ET66ButtonType::Primary)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
								[
									MakeUpgradeButton(
										RangeUpgradeText,
										CanUpgradeRange,
										FOnClicked::CreateSP(BattleBoard.ToSharedRef(), &ST66TDBattleBoardWidget::HandleRangeUpgradeClicked),
										ET66ButtonType::Neutral)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
								[
									MakeUpgradeButton(
										TempoUpgradeText,
										CanUpgradeTempo,
										FOnClicked::CreateSP(BattleBoard.ToSharedRef(), &ST66TDBattleBoardWidget::HandleTempoUpgradeClicked),
										ET66ButtonType::Success)
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
								[
									MakeUpgradeButton(
										SellSelectedTowerText,
										CanSellSelectedTower,
										FOnClicked::CreateSP(BattleBoard.ToSharedRef(), &ST66TDBattleBoardWidget::HandleSellSelectedTowerClicked),
										ET66ButtonType::Danger)
								]
								,
								TDBattleUpgradePanelBrush(),
								T66TDUI::ShellFill(),
								FMargin(24.f, 22.f))
						]
					]
				]
			]
		];
}

FReply UT66TDBattleScreen::HandleBackClicked()
{
	NavigateTo(ET66ScreenType::TDDifficultySelect);
	return FReply::Handled();
}

void UT66TDBattleScreen::EnsureRunSelectionState()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66TDDataSubsystem* TDDataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66TDDataSubsystem>() : nullptr;
	UT66TDFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66TDFrontendStateSubsystem>() : nullptr;
	if (!TDDataSubsystem || !FrontendState)
	{
		return;
	}

	if (!FrontendState->HasSelectedDifficulty() && TDDataSubsystem->GetDifficulties().Num() > 0)
	{
		FrontendState->SelectDifficulty(TDDataSubsystem->GetDifficulties()[0].DifficultyID);
	}

	if (!FrontendState->HasSelectedMap())
	{
		const TArray<const FT66TDMapDefinition*> MapsForDifficulty = TDDataSubsystem->GetMapsForDifficulty(FrontendState->GetSelectedDifficultyID());
		if (MapsForDifficulty.Num() > 0)
		{
			FrontendState->SelectMap(MapsForDifficulty[0]->MapID);
		}
	}

	if (const FT66TDStageDefinition* StageDefinition = TDDataSubsystem->FindStageForMap(FrontendState->GetSelectedMapID()))
	{
		FrontendState->SelectStage(StageDefinition->StageID);
	}
}

const TSharedPtr<FSlateBrush>& UT66TDBattleScreen::FindOrLoadMapBrush(const FString& RelativePath)
{
	if (!MapBackgroundBrush.IsValid())
	{
		MapBackgroundBrush = MakeShared<FSlateBrush>();
	}
	EnsureBrush(MapBackgroundBrush, TDBattleBoardSize);

	if (!MapBackgroundTexture.IsValid())
	{
		for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
		{
			if (!FPaths::FileExists(CandidatePath))
			{
				continue;
			}

			if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("TDBattleBackground")))
			{
				MapBackgroundTexture.Reset(Texture);
				MapBackgroundBrush->SetResourceObject(Texture);
				MapBackgroundBrush->ImageSize = FVector2D(FMath::Max(1, Texture->GetSizeX()), FMath::Max(1, Texture->GetSizeY()));
				break;
			}
		}
	}

	return MapBackgroundBrush;
}

void UT66TDBattleScreen::ReleaseRetainedSlateState()
{
	BattleBoardRoot.Reset();
	MapBackgroundBrush.Reset();
	MapBackgroundTexture.Reset();
	HeroSpriteBrushes.Reset();
	EnemySpriteBrushes.Reset();
	BossSpriteBrushes.Reset();
}
