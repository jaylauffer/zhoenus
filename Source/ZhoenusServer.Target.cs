
using UnrealBuildTool;
using System.Collections.Generic;

public class ZhoenusServerTarget : TargetRules
{
	public ZhoenusServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		ExtraModuleNames.Add("Zhoenus");
		bCompileChaos = true;
		bUseChaos = true;
	}
}