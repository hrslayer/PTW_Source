using UnrealBuildTool;
using System;

public class PTWDedicatedServer : ModuleRules
{
	public PTWDedicatedServer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[] 
			{
				ModuleDirectory
			}
		);

		// 0. 기본 게임 모듈 
		PublicDependencyModuleNames.Add("PTW");
		
		// 1. Core & Basic Modules (기본 엔진 모듈)
		PublicDependencyModuleNames.AddRange(new string[] 
		{
            "Core",
            "CoreUObject",
            "Engine",
			"AIModule",
			"NavigationSystem",
			"CoreOnline",
		});

        // 2. GAS (Gameplay Ability System) 필수 모듈
		PublicDependencyModuleNames.AddRange(new string[] {
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		// 3. Networking & Online Subsystem (스팀/세션 연동용)
		PublicDependencyModuleNames.AddRange(new string[] {
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"HTTP",
			"WebSockets",
			"GameLiftServerSDK",
		});

        // 4. ModularGameplay (모듈 친화 모듈)
        PublicDependencyModuleNames.AddRange(new string[] {
			"ModularGameplay",
		});
        
        // Private Dependencies (내부적으로만 사용하는 모듈)
        PrivateDependencyModuleNames.AddRange(new string[] {
			"Json",
			"JsonUtilities",
			"NetCore" 
		});
        
        
		string GameLift_ServerApi_Endpoint = "";
		switch (Target.Configuration)
		{
			case UnrealTargetConfiguration.Shipping:
				GameLift_ServerApi_Endpoint = "https://jhbm9idps8.execute-api.ap-northeast-2.amazonaws.com/Dev";
				break;
			default:
				GameLift_ServerApi_Endpoint = "https://jhbm9idps8.execute-api.ap-northeast-2.amazonaws.com/Dev";
				break;
		}

		PublicDefinitions.Add(string.Format("GAMELIFT_SERVERAPI_ENDPOINT=\"{0}\"", GameLift_ServerApi_Endpoint));
	}
}
