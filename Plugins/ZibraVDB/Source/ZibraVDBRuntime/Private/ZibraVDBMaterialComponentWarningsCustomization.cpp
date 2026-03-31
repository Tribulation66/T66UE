// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#if WITH_EDITOR

#include "ZibraVDBMaterialComponentWarningsCustomization.h"

#include "Components/SkyAtmosphereComponent.h"
#include "DetailLayoutBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Text/STextBlock.h"
#include "ZibraVDBMaterialComponent.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

bool IsSkyAtmosphereActorActive(const UWorld* World)
{
	bool bSkyAtmospherePresent = false;
	for (TObjectIterator<USkyAtmosphereComponent> It; It; ++It)
	{
		USkyAtmosphereComponent* SkyAtmosphere = *It;
		if (SkyAtmosphere->GetOwner() && World->ContainsActor(SkyAtmosphere->GetOwner()) && IsValid(SkyAtmosphere) &&
			SkyAtmosphere->IsVisibleInEditor() && !SkyAtmosphere->GetOwner()->IsHiddenEd())
		{
			bSkyAtmospherePresent = true;
			break;
		}
	}
	return bSkyAtmospherePresent;
}

TSharedRef<IDetailCustomization> FZibraVDBMaterialComponentWarningsCustomization::MakeInstance()
{
	return MakeShared<FZibraVDBMaterialComponentWarningsCustomization>();
}

void FZibraVDBMaterialComponentWarningsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const TSharedPtr<IPropertyHandle> ProjectedShadowsWarningPropertyHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBMaterialComponent, ProjectedShadowsWarning));

	const TSharedPtr<IPropertyHandle> ReceivedShadowsWarningPropertyHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBMaterialComponent, ReceivedShadowsWarning));

	const TSharedPtr<IPropertyHandle> AmbientLightingWarningPropertyHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBMaterialComponent, AmbientLightingWarning));

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	if (ObjectsBeingCustomized.IsEmpty())
	{
		return;
	}
	const UWorld* World = ObjectsBeingCustomized[0]->GetWorld();

	bool bTranslucencyShadowsEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.TranslucentPerObject.ProjectEnabled"))->GetBool();

	bool bLumenHardwareRaytracingEnabled =
		IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Lumen.HardwareRayTracing"))->GetValueOnGameThread() != 0 &&
		!IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DistanceFields.SupportEvenIfHardwareRayTracingSupported"))->GetValueOnGameThread();

	bool bGDFEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AOGlobalDistanceField"))->GetBool() &&
		IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.GenerateMeshDistanceFields"))->GetValueOnGameThread() != 0 &&
		IConsoleManager::Get().FindConsoleVariable(TEXT("r.DistanceFieldAO"))->GetBool()
		&& !bLumenHardwareRaytracingEnabled;

	bool bSkyAtmosphereEnabled =
		IsSkyAtmosphereActorActive(World) && IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.SkyAtmosphere"))->GetValueOnGameThread();

	if (!bTranslucencyShadowsEnabled)
	{
		DetailBuilder.EditDefaultProperty(ProjectedShadowsWarningPropertyHandle)
			->CustomWidget()
			.WholeRowContent()
				[SNew(SVerticalBox) +
					SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0,
						0)[SNew(STextBlock)
							   .Text(FText::FromString(
								   "To enable projected shadows enable \"Support Volumetric Translucent Self Shadowing\" in project settings"))
							   .Justification(ETextJustify::Left)
							   .Font(IDetailLayoutBuilder::GetDetailFont())
							   .ColorAndOpacity(FSlateColor(FLinearColor::Yellow))]];
	}
	else
		DetailBuilder.HideProperty(ProjectedShadowsWarningPropertyHandle);

	if (!bGDFEnabled)
	{
		DetailBuilder.EditDefaultProperty(ReceivedShadowsWarningPropertyHandle)
			->CustomWidget()
			.WholeRowContent()[SNew(SVerticalBox) +
							   SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0,
								   0)[SNew(STextBlock)
										  .Text(LOCTEXT("GDFDisabledNoReceivedShadows", "Global Distance Field is disabled, received shadows won't work"))
										  .Justification(ETextJustify::Left)
										  .Font(IDetailLayoutBuilder::GetDetailFont())
										  .ColorAndOpacity(FSlateColor(FLinearColor::Yellow))]];
	}
	else
		DetailBuilder.HideProperty(ReceivedShadowsWarningPropertyHandle);

	if (!bSkyAtmosphereEnabled)
	{
		DetailBuilder.EditDefaultProperty(AmbientLightingWarningPropertyHandle)
			->CustomWidget()
			.WholeRowContent()[SNew(SVerticalBox) +
							   SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0,
								   0)[SNew(STextBlock)
										  .Text(LOCTEXT("NoSkyAtmosphereNoAmbientLighting", "There is no Sky Atmosphere, ambient lighting won't work"))
										  .Justification(ETextJustify::Left)
										  .Font(IDetailLayoutBuilder::GetDetailFont())
										  .ColorAndOpacity(FSlateColor(FLinearColor::Yellow))]];
	}
	else
		DetailBuilder.HideProperty(AmbientLightingWarningPropertyHandle);

	UZibraVDBMaterialComponent* MaterialComponent = Cast<UZibraVDBMaterialComponent>(ObjectsBeingCustomized[0]);

	const TSharedPtr<IPropertyHandle> DifferentVoxelSizeWarningPropertyHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBMaterialComponent, bDifferentVoxelSizeWarning));

	if (MaterialComponent && MaterialComponent->bShowWarningDifferentVoxelSize)
	{
		DetailBuilder.EditDefaultProperty(DifferentVoxelSizeWarningPropertyHandle)
			->CustomWidget()
			.WholeRowContent()[SNew(SVerticalBox) +
							   SVerticalBox::Slot().AutoHeight().Padding(
								   0, 5, 0, 0)[SNew(STextBlock)
												   .Text(FText::FromString(
													   "Channels have different voxel size and won't be rendered properly."))
												   .Justification(ETextJustify::Left)
												   .Font(IDetailLayoutBuilder::GetDetailFont())
												   .ColorAndOpacity(FSlateColor(FLinearColor::Yellow))]];
	}
	else
		DetailBuilder.HideProperty(DifferentVoxelSizeWarningPropertyHandle);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 506
	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBMaterialComponent, VDBFirstPersonPrimitiveType));
#endif

	const TSharedPtr<IPropertyHandle> UseHeterogeniousVolumePropertyHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBMaterialComponent, bUseHeterogeneousVolume));
	TWeakPtr<IPropertyUtilities> PropertyUtilities = DetailBuilder.GetPropertyUtilities();
	if (UseHeterogeniousVolumePropertyHandle)
	{
		UseHeterogeniousVolumePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda(
			[PropertyUtilities]()
			{
				if (PropertyUtilities.IsValid())
				{
					PropertyUtilities.Pin()->ForceRefresh();
				}
			}));
	}
}

#endif

#undef LOCTEXT_NAMESPACE
