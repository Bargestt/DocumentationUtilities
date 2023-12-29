// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HintStruct.h"
#include <Engine/DataAsset.h>
#include "DocumentationUtilitiesSettings.generated.h"



/**
 * Additional settings for documentation links
 * 
 */
UCLASS(Config = Editor, defaultconfig, meta = (DisplayName = "Documentation"))
class DOCUMENTATIONUTILITIESEDITOR_API UDocumentationUtilities : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Documentation", meta = (HideChildren))
	FHintStruct Hint = FHintStruct().Hint(EHintSource::ClassTooltip).Link(TEXT("https://github.com/Bargestt/DocumentationUtilities"));

	UPROPERTY(EditAnywhere, Category = "Documentation", meta = (InlineEditConditionToggle))
	bool bFindReference;

	/** Find link references */
	UPROPERTY(EditAnywhere, Category = "Documentation", meta = (EditCondition = "!bFindReference"))
	FString FindReference;


	/** 	 
	 * Use maps below to redirect documentation links
	 * 
	 * Maps are checked in order: Native, Documentation, Class, Asset
	 * Empty values are ignored
	 */
	UPROPERTY(EditAnywhere, Category = "Redirectors", meta = (HideChildren))
	FHintStruct LinksHelp = FHintStruct().Hint(EHintSource::PropertyTooltip);

	UPROPERTY(config, EditAnywhere, Category = "Redirectors", meta = (ConfigRestartRequired = true))
	bool bIncludeNativeHints;

	/** Documentation links created in C++ */
	UPROPERTY(config, EditAnywhere, Category = "Redirectors", meta = (ReadOnlyKeys, EditCondition = "bIncludeNativeHints", EditConditionHides))
	TMap<FString, FString> NativeLinks;	

	UPROPERTY(config, EditAnywhere, Category = "Redirectors")
	TMap<FString, FString> DocumentationLinks;

	UPROPERTY(config, EditAnywhere, Category = "Redirectors", meta = (ForceInlineRow, AllowAbstract = true))
	TMap<FSoftClassPath, FString> ClassDocumentationLinks;

	UPROPERTY(config, EditAnywhere, Category = "Redirectors", meta = (ForceInlineRow, DisplayThumbnail = false))
	TMap<FSoftObjectPath, FString> AssetDocumentationLinks;


public:
	virtual void PostInitProperties() override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	static FString ResolveLink(const FString& Link);
	static bool HasLinkRedirector(const FString& Link);

};

