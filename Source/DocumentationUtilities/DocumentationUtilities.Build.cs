// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

using UnrealBuildTool;

public class DocumentationUtilities : ModuleRules
{
	public DocumentationUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;	
		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
			}
			);
	}
}
