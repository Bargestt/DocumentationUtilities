// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

using UnrealBuildTool;

public class DocumentationUtilitiesEditor : ModuleRules
{
	public DocumentationUtilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DocumentationUtilities"
			}
		);			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
				"Engine",
                "Slate",
                "SlateCore",
                "PropertyEditor",

				"DeveloperSettings",

                "UnrealEd",
				"ApplicationCore",
				"ToolMenus",
                "ContentBrowser"
            }
		);
	}
}
