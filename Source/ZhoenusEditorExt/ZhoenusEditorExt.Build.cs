// Copyright 2026 Run Rong Games. All Rights Reserved.

using UnrealBuildTool;

public class ZhoenusEditorExt : ModuleRules
{
	public ZhoenusEditorExt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AssetRegistry",
				"MaterialEditor",
				"Niagara",
				"NiagaraEditor",
				"UnrealEd",
			});
	}
}
