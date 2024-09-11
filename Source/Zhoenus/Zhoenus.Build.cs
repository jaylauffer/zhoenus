// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Zhoenus : ModuleRules
{
	public Zhoenus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "EngineSettings", "InputCore", "UMG", "Slate", "SlateCore", "Niagara", "ReplicationGraph", "NetworkPrediction", "MetasoundEngine", "AudioExtensions", "EnhancedInput" });
		//PrivateDependecyModuleNames.AddRange(new string[] { "Metasound" });
	}
}
