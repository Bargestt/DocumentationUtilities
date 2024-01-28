// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HintStruct.h"
#include <Engine/DataAsset.h>
#include "DocumentationUtilitiesSettings.generated.h"


/** */
UENUM()
enum class EDocumentationLinkType : uint8
{	
	String,
	Asset,
	Class,
	Native UMETA(Hidden),
	MAX UMETA(Hidden)
};

/**  */
USTRUCT()
struct FDocumentationHintLink
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Link", meta = (EditCondition = "Type != EDocumentationLinkType::Native"))
	EDocumentationLinkType Type;

	UPROPERTY(EditAnywhere, Category = "Link", meta = (EditCondition = "Type != EDocumentationLinkType::Native"))
	FString StringKey;

	UPROPERTY(EditAnywhere, Category = "Link", meta = (EditCondition = "Type != EDocumentationLinkType::Native", DisplayThumbnail = false))
	TSoftObjectPtr<UObject> AssetKey;

	UPROPERTY(EditAnywhere, Category = "Link", meta = (EditCondition = "Type != EDocumentationLinkType::Native"))
	TSoftClassPtr<UObject> ClassKey;

	UPROPERTY(EditAnywhere, Category = "Link")
	FString Value;


public:
	FString GetLinkKey() const
	{
		switch (Type)
		{		
		case EDocumentationLinkType::Asset: return AssetKey.ToString();
		case EDocumentationLinkType::Class: return ClassKey.ToString();
		case EDocumentationLinkType::String: break;
		case EDocumentationLinkType::Native: break;		
		}
		return StringKey;
	}

	bool IsValid() const 
	{ 		
		if (!Value.IsEmpty())
		{		
			switch (Type)
			{		
			case EDocumentationLinkType::Asset: return !AssetKey.IsNull();
			case EDocumentationLinkType::Class: return !ClassKey.IsNull();
			case EDocumentationLinkType::String: return !StringKey.IsEmpty();
			case EDocumentationLinkType::Native: return !StringKey.IsEmpty();		
			}
		}
		return false;		
	}

	bool HasValue() const { return !Value.IsEmpty(); }


	bool ExportTextItem(FString& ValueStr, FDocumentationHintLink const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;
	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);
};

template<>
struct TStructOpsTypeTraits< FDocumentationHintLink > : public TStructOpsTypeTraitsBase2< FDocumentationHintLink >
{
	enum
	{		
		WithExportTextItem = true,
		WithImportTextItem = true
	};
};



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
	FHintStruct Hint = FHintStruct().Hint(EHintSource::ClassTooltip).Link(TEXT("https://github.com/Bargestt/DocumentationUtilities/wiki"));


	/** 
	 * Documentation links in content browser
	 * This adds prompts using AssetPath and AssetClassPath. 
	 * To document Blueprints: use Asset links instead of class links
	 */
	UPROPERTY(EditAnywhere, Category = "Documentation: Content Browser", meta = (HideChildren))
	FHintStruct ContentBrowserHint = FHintStruct().Hint(EHintSource::PropertyTooltip).Tooltip(EHintSource::PropertyTooltip);

	/** Maximum number of displayed documentation links in content browser */
	UPROPERTY(EditAnywhere, Category = "Documentation: Content Browser")
	int32 MaxContentBrowserLinks;

	/** Content browser context menu will show links for undocumented assets */
	UPROPERTY(EditAnywhere, Category = "Documentation: Content Browser")
	bool bShowUndocumentedLinks;

	/** Format string of how the Action will be displayed in context menu. Use arg {0} to display asset name */
	UPROPERTY(EditAnywhere, Category = "Documentation: Content Browser")
	FString ClassDocumentationLink;

	/** Format string of how the Action will be displayed in context menu. Use arg {0} to display asset name */
	UPROPERTY(EditAnywhere, Category = "Documentation: Content Browser")
	FString AssetDocumentationLink;



	/** Display last text segment only. Converts /Game/Folder/Asset.Asset to Asset */
	UPROPERTY(config, EditAnywhere, Category = "Documentation: Links Picker", meta = (DisplayName = "Short Names"))
	bool bLinksPicker_ShortNames;

	UPROPERTY(config, EditAnywhere, Category = "Documentation: Links Picker", meta = (DisplayName = "Show Native"))
	bool bLinksPicker_ShowNative;

	UPROPERTY(config, EditAnywhere, Category = "Documentation: Links Picker", meta = (DisplayName = "Show String"))
	bool bLinksPicker_ShowString;

	UPROPERTY(config, EditAnywhere, Category = "Documentation: Links Picker", meta = (DisplayName = "Show Asset"))
	bool bLinksPicker_ShowAsset;

	UPROPERTY(config, EditAnywhere, Category = "Documentation: Links Picker", meta = (DisplayName = "Show Class"))
	bool bLinksPicker_ShowClass;



	/** 
	 * Define your Link actions here or override existing ones
	 * Use context menu to search Link references
	 * 
	 * Start Link with 'http' or 'https' to open URL in your web browser
	 * Start Link with '/' to open asset editor for the Path specified, add prefix 'Edit:' to open in edit mode
	 */
	UPROPERTY(EditAnywhere, Category = "Documentation: Links", meta = (HideChildren))
	FHintStruct LinksHint = FHintStruct().Hint(EHintSource::PropertyTooltip).Tooltip(EHintSource::PropertyTooltip);

	/** Collect Links from native classes */
	UPROPERTY(config, EditAnywhere, Category = "Documentation: Links", meta = (ConfigRestartRequired = true))
	bool bCollectNativeHints;

	UPROPERTY(config, EditAnywhere, EditFixedSize, Category = "Documentation: Links")
	TArray<FDocumentationHintLink> NativeLinks;

	UPROPERTY(config, EditAnywhere, Category = "Documentation: Links")
	TArray<FDocumentationHintLink> Links;


public:
	UDocumentationUtilities(const FObjectInitializer& ObjectInitializer);
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual void PostInitProperties() override;

public:
	static const FDocumentationHintLink* FindLinkByKey(const FString& Link);
	static bool HasLinkRedirector(const FString& Link);
	static FString ResolveLink(const FString& Link);

	TMap<FString, FString> CollectLinksOfType(EDocumentationLinkType Type) const;

};

