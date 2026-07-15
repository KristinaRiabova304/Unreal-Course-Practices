// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Multi : ModuleRules
{
	public Multi(ReadOnlyTargetRules Target) : base(Target)
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
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Multi",
			"Multi/Variant_Platforming",
			"Multi/Variant_Platforming/Animation",
			"Multi/Variant_Combat",
			"Multi/Variant_Combat/AI",
			"Multi/Variant_Combat/Animation",
			"Multi/Variant_Combat/Gameplay",
			"Multi/Variant_Combat/Interfaces",
			"Multi/Variant_Combat/UI",
			"Multi/Variant_SideScrolling",
			"Multi/Variant_SideScrolling/AI",
			"Multi/Variant_SideScrolling/Gameplay",
			"Multi/Variant_SideScrolling/Interfaces",
			"Multi/Variant_SideScrolling/UI",
			"Multi/Online"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
