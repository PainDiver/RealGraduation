// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyProject4 : ModuleRules
{
	public MyProject4(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore","UMG", "OnlineSubsystem", "﻿OnlineSubsystemSteam﻿", "OnlineSubsystemUtils","SteamWorks","AIModule", "SlateCore", "Slate", "Networking", "Sockets", "CableComponent","Http","Json","JsonUtilities", "GameplayCameras" });

		PrivateDependencyModuleNames.AddRange(new string[] { "CableComponent" });

		PrivateIncludePathModuleNames.AddRange(new string[] { "CableComponent" });


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
