// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Lagproject : ModuleRules
{
	public Lagproject(ReadOnlyTargetRules Target) : base(Target)
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
			"Lagproject",
			"Lagproject/Variant_Platforming",
			"Lagproject/Variant_Platforming/Animation",
			"Lagproject/Variant_Combat",
			"Lagproject/Variant_Combat/AI",
			"Lagproject/Variant_Combat/Animation",
			"Lagproject/Variant_Combat/Gameplay",
			"Lagproject/Variant_Combat/Interfaces",
			"Lagproject/Variant_Combat/UI",
			"Lagproject/Variant_SideScrolling",
			"Lagproject/Variant_SideScrolling/AI",
			"Lagproject/Variant_SideScrolling/Gameplay",
			"Lagproject/Variant_SideScrolling/Interfaces",
			"Lagproject/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
