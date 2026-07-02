// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TDMGame : ModuleRules
{
	public TDMGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TDMGame",
			"TDMGame/Variant_Platforming",
			"TDMGame/Variant_Platforming/Animation",
			"TDMGame/Variant_Combat",
			"TDMGame/Variant_Combat/AI",
			"TDMGame/Variant_Combat/Animation",
			"TDMGame/Variant_Combat/Gameplay",
			"TDMGame/Variant_Combat/Interfaces",
			"TDMGame/Variant_Combat/UI",
			"TDMGame/Variant_SideScrolling",
			"TDMGame/Variant_SideScrolling/AI",
			"TDMGame/Variant_SideScrolling/Gameplay",
			"TDMGame/Variant_SideScrolling/Interfaces",
			"TDMGame/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
