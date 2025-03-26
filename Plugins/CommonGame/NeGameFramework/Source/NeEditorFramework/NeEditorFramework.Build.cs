// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NeEditorFramework : ModuleRules
{
	public NeEditorFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"SlateCore",
				"Slate",
				"SequencerWidgets",
				"StructUtils",
				"BlueprintGraph",
				"EditorWidgets",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"InputCore",
				"AssetTools",
				"LevelEditor",
				"SequencerWidgets",
				"EditorWidgets",
				"TimeManagement",
				"StructUtils",
				"AdvancedPreviewScene",
				// ... add private dependencies that you statically link with here ...
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
