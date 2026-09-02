// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FastSSGI : ModuleRules
{
	public FastSSGI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Projects",
			"RenderCore",
			"Renderer",
			"RHI"
		});

		// FPostProcessingInputs is part of the Scene View Extension API, but its definition
		// resides in Renderer/Internal in UE 5.8.
		PrivateIncludePaths.Add(GetModuleDirectory("Renderer") + "/Internal");
	}
}
