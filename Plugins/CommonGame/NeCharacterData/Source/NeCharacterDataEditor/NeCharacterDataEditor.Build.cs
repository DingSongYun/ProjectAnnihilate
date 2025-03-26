// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NeCharacterDataEditor : ModuleRules
{
	public NeCharacterDataEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"NeCharacterData",
				"NeGameFramework",
				"NeEditorFramework",
				"AdvancedPreviewScene",
				"Core",
				"EditorStyle",
				"InputCore",
				"LevelEditor",
				"GraphEditor",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"Persona",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorFramework",
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
