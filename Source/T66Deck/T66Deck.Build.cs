using UnrealBuildTool;
using System.IO;

public class T66Deck : ModuleRules
{
	public T66Deck(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"SlateCore",
			"UMG",
			"T66"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"ApplicationCore",
			"Json",
			"JsonUtilities"
		});

		void AddLooseDeckRuntimeDependency(string RelativeProjectPath)
		{
			RuntimeDependencies.Add("$(ProjectDir)/" + RelativeProjectPath.Replace('\\', '/'), StagedFileType.NonUFS);
		}

		AddLooseDeckRuntimeDependency("Content/Deck/Data/...");
		AddLooseDeckRuntimeDependency("SourceAssets/Deck/...");
		AddLooseDeckRuntimeDependency("UI/screens/minigames/chadpocalypse_deckbuilder/reference/chadpocalypse_deckbuilder_main_menu_mockup_1920x1080_imagegen_20260503_v2.png");
		AddLooseDeckRuntimeDependency("UI/screens/minigames/chadpocalypse_deckbuilder/reference/chadpocalypse_deckbuilder_gameplay_mockup_1920x1080_imagegen_20260503_v2.png");

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "UI", "Screens"));
	}
}
