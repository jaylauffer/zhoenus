// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ZhoenusEditorTarget : TargetRules
{
	public ZhoenusEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("Zhoenus");

		bCompileChaos = true;
		bUseChaos = true;
	}
}
