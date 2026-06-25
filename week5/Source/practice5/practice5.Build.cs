// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class practice5 : ModuleRules
{
	public practice5(ReadOnlyTargetRules Target) : base(Target)
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
			"practice5",
			"practice5/Variant_Platforming",
			"practice5/Variant_Platforming/Animation",
			"practice5/Variant_Combat",
			"practice5/Variant_Combat/AI",
			"practice5/Variant_Combat/Animation",
			"practice5/Variant_Combat/Gameplay",
			"practice5/Variant_Combat/Interfaces",
			"practice5/Variant_Combat/UI",
			"practice5/Variant_SideScrolling",
			"practice5/Variant_SideScrolling/AI",
			"practice5/Variant_SideScrolling/Gameplay",
			"practice5/Variant_SideScrolling/Interfaces",
			"practice5/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
