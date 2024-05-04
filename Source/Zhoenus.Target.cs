// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ZhoenusTarget : TargetRules
{
	public ZhoenusTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("Zhoenus");
		DefaultBuildSettings = BuildSettingsVersion.Latest;
	}
}
