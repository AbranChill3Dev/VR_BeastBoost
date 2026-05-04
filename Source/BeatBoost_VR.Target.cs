// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class BeatBoost_VRTarget : TargetRules
{
	public BeatBoost_VRTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "BeatBoost_VR" } );

        bOverrideBuildEnvironment = true;
        AdditionalCompilerArguments = "/wd4668 /wd4067";
    }
}
