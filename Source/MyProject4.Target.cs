// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MyProject4Target : TargetRules
{
	public MyProject4Target( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "MyProject4" } );
		bUsesSteam = true;

		GlobalDefinitions.Add("UE4_PRODUCT_STEAMGAMEDESC=\"Spacewar\"");
		GlobalDefinitions.Add("UE4_PRODUCT_STEAMPRODUCTNAME=\"Spacewar\"");
		GlobalDefinitions.Add("UE4_PRODUCT_STEAMGAMEDIR=\"Spacewar\"");

		GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPINGID=\"480\"");
	}
}
