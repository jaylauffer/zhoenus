// Copyright 2024 Run Rong Games. All Rights Reserved.

using UnrealBuildTool;

public class Zhoenus : ModuleRules
{
	public Zhoenus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

	                PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "EngineSettings", "InputCore", "UMG", "Slate", "SlateCore", "Niagara", "MetasoundEngine", "AudioExtensions", "EnhancedInput", "CommonUI", "MediaAssets", "RenderCore", "AssetRegistry" });
		//PrivateDependecyModuleNames.AddRange(new string[] { "Metasound" });
	}
}
