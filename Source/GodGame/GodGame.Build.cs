// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.
using UnrealBuildTool;

public class GodGame : ModuleRules
{
    public GodGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            
        });
        
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
        // Public include path list.
        PublicIncludePaths.AddRange(
	        new string[]
	        {
	        });
		
        // Private include path list.
        PrivateIncludePaths.AddRange(
	        new string[]
	        {
	        });
				
        // Public module dependency list.
        PublicDependencyModuleNames.AddRange(
	        new string[]
	        {
		        "Core",
		        "CoreUObject",
		        "Engine",
		        "InputCore",
		        "GameplayTags",
		        "DeveloperSettings",
		        "Projects",
		        "JoyCore"
	        });
			
        // Private module dependency list.
        PrivateDependencyModuleNames.AddRange(
	        new string[]
	        {
	        });
		
        // Dynamically-loaded module list.
        DynamicallyLoadedModuleNames.AddRange(
	        new string[]
	        {
	        });
    }
}
