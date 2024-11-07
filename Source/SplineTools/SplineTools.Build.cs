// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SplineTools : ModuleRules
{
    public SplineTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "SplineTools", "UMG", "SplineTools" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore"});

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("EditorScriptingUtilities");
        }
    }
}
