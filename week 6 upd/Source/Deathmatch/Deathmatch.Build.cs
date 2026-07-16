// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Deathmatch : ModuleRules
{
	public Deathmatch(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Deathmatch",
			"Deathmatch/TDM",
			"Deathmatch/Variant_Platforming",
			"Deathmatch/Variant_Platforming/Animation",
			"Deathmatch/Variant_Combat",
			"Deathmatch/Variant_Combat/AI",
			"Deathmatch/Variant_Combat/Animation",
			"Deathmatch/Variant_Combat/Gameplay",
			"Deathmatch/Variant_Combat/Interfaces",
			"Deathmatch/Variant_Combat/UI",
			"Deathmatch/Variant_SideScrolling",
			"Deathmatch/Variant_SideScrolling/AI",
			"Deathmatch/Variant_SideScrolling/Gameplay",
			"Deathmatch/Variant_SideScrolling/Interfaces",
			"Deathmatch/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
