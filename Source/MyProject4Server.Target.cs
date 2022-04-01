// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class MyProject4ServerTarget : TargetRules
{
	public MyProject4ServerTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "MyProject4" } );

        bUsesSteam = true;

        GlobalDefinitions.Add("UE4_PRODUCT_STEAMGAMEDESC=\"Spacewar\"");
        GlobalDefinitions.Add("UE4_PRODUCT_STEAMPRODUCTNAME=\"Spacewar\"");
        GlobalDefinitions.Add("UE4_PRODUCT_STEAMGAMEDIR=\"Spacewar\"");
        GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPINGID=480");
    }
}
