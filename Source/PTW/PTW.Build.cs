// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;

public class PTW : ModuleRules
{
	public PTW(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// PrivateIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.Add(ModuleDirectory);

		// 1. Core & Basic Modules (기본 엔진 모듈)
		PublicDependencyModuleNames.AddRange(new string[] 
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"CoreOnline"
		});

		// 2. GAS (Gameplay Ability System) 필수 모듈
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		// 3. UMG & UI 모듈
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"UMG",
			"MoviePlayer"
		});

		// 4. Networking & Online Subsystem (스팀/세션 연동용)
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"HTTP",
			"WebSockets",
		});

		// 5. Niagara 이펙트 모듈
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Niagara"
		});

		// 6. ModularGameplay (모듈 친화 모듈)
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"ModularGameplay",
			"ModularCoreFramework"
		});

		// Private Dependencies (내부적으로만 사용하는 모듈)
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"Json",
			"JsonUtilities",
			"NetCore",
		});


		string GameLift_ClientApi_Endpoint = "";
		switch (Target.Configuration)
		{
			// case UnrealTargetConfiguration.Shipping:
			// 	GameLift_ClientApi_Endpoint = "https://wfefdk5ctj.execute-api.ap-northeast-2.amazonaws.com";
			// 	break;
			default:
				GameLift_ClientApi_Endpoint = "https://wfefdk5ctj.execute-api.ap-northeast-2.amazonaws.com/stage_1";
				break;
		}
		
		string WebSocket_ClientApi_Endpoint = "";
		switch (Target.Configuration)
		{
			default:
				WebSocket_ClientApi_Endpoint = "wss://neepxqhon2.execute-api.ap-northeast-2.amazonaws.com/Dev";
				break;
		}
		
		PublicDefinitions.Add(string.Format("GAMELIFT_CLIENTAPI_ENDPOINT=\"{0}\"", GameLift_ClientApi_Endpoint));
		PublicDefinitions.Add(string.Format("WEBSOCKET_CLIENTAPI_ENDPOINT=\"{0}\"", WebSocket_ClientApi_Endpoint));
	}
}
