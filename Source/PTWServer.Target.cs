using UnrealBuildTool;
using System.Collections.Generic;

public class PTWServerTarget : TargetRules
{
	public PTWServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		
		ExtraModuleNames.Add("PTW");
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			ExtraModuleNames.Add("PTWDedicatedServer");
		}

		bUseLoggingInShipping = true;
		bUseChecksInShipping = true;
	}
}
