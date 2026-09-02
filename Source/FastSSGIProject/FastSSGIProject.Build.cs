// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FastSSGIProject : ModuleRules
{
	public FastSSGIProject(ReadOnlyTargetRules Target) : base(Target)
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
			"FastSSGIProject",
			"FastSSGIProject/Variant_Horror",
			"FastSSGIProject/Variant_Horror/UI",
			"FastSSGIProject/Variant_Shooter",
			"FastSSGIProject/Variant_Shooter/AI",
			"FastSSGIProject/Variant_Shooter/UI",
			"FastSSGIProject/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
