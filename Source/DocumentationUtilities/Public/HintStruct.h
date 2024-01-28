// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HintStruct.generated.h"



// 
// 
// Intentionally kept in runtime module for multiple reasons
// 
// While moving struct to Editor module will automatically strip it from builds
// It will produce multiple problems
// - Using them in code will require conditional includes and conditional dependency in Build.cs
// - Packaged Blueprints have no Editor-only properties, 
//   they will keep the property, report error, and replace property value with FFallbackStruct.
//   FHintStruct is stripped down do empty so this is identical situation, but without error messages
// 
// 
// You can setup default value in declaration
// 
// #if WITH_EDITORONLY_DATA
// UPROPERTY(EditAnywhere, Category = "")
// FHintStruct Hint = FHintStruct().Hint(EHintSource::PropertyTooltip).Tooltip(EHintSource::ClassTooltip).Link(this);
// #endif


/** */
UENUM()
enum class EHintSource : uint8
{		
	/** Set manually */
	PropertyValue,
	/** Use tooltip assigned to this property */
	PropertyTooltip,
	/** Use class description */
	ClassTooltip,
	/** Find first non-empty class description */
	FirstValidTooltip,
	/** Find native class and use it's description */
	NativeClassTooltip,

	MAX UMETA(Hidden)
};

/** 
 * Setup and display hint
 * Also provides documentation link
 * 
 * Can forcefully hide settings using meta: HideChildren
 */
USTRUCT(BlueprintType)
struct DOCUMENTATIONUTILITIES_API FHintStruct
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(EditAnywhere, Category = "Hint")
	EHintSource HintSource;

	UPROPERTY(EditAnywhere, Category = "Hint", meta = (EditCondition = "HintSource == EHintSource::PropertyValue", EditConditionHides))
	FString HintText;


	UPROPERTY(EditAnywhere, Category = "Hint")
	EHintSource TooltipSource;

	UPROPERTY(EditAnywhere, Category = "Hint", meta = (EditCondition = "TooltipSource == EHintSource::PropertyValue", EditConditionHides))
	FString TooltipText;


	UPROPERTY(EditAnywhere, Category = "Hint")
	FSoftObjectPath LinkAddressPath;

	UPROPERTY(EditAnywhere, Category = "Hint")
	FString LinkAddress;

public:
	FHintStruct()
		: HintSource(EHintSource::PropertyValue)
		, HintText(TEXT("Info"))
		, TooltipSource(EHintSource::ClassTooltip)
		, TooltipText(TEXT(""))
		, LinkAddress(TEXT(""))
	{ }

	FHintStruct(EHintSource InHintSource, 
				const FString& InHint = TEXT("Info"), 
				EHintSource InTooltipSource = EHintSource::ClassTooltip, 
				const FString& InTooltip = TEXT(""),
				const FString& InLink = TEXT(""))
		: HintSource(InHintSource)
		, HintText(InHint)
		, TooltipSource(InTooltipSource)
		, TooltipText(InTooltip)
		, LinkAddress(InLink)
	{

	}


	FHintStruct& Hint(const EHintSource& Source, const FString& PropertyValue = TEXT(""))
	{
		HintSource = Source;
		HintText = PropertyValue;
		return *this;
	}

	FHintStruct& Hint(const FString& PropertyValue = TEXT(""))
	{
		HintSource = EHintSource::PropertyValue;
		HintText = PropertyValue;
		return *this;
	}


	FHintStruct& Tooltip(const EHintSource& Source, const FString& PropertyValue = TEXT(""))
	{
		TooltipSource = Source;
		TooltipText = PropertyValue;
		return *this;
	}

	FHintStruct& Tooltip(const FString& PropertyValue)
	{
		TooltipSource = EHintSource::PropertyValue;
		TooltipText = PropertyValue;
		return *this;
	}



	FHintStruct& Link(const FString& InLink = TEXT(""))
	{
		LinkAddress = InLink;
		return *this;
	}

	FHintStruct& Link(const UObject* Object, bool bLinkObjectPath = true)
	{				 
		LinkAddress = Object ? Object->GetPathName() : TEXT("");
		if (bLinkObjectPath)
		{
			LinkAddressPath = LinkAddress;
			LinkAddress.Reset();
		}
		return *this;
	}

	FHintStruct& Link(const UClass* Object, bool bLinkObjectPath = true)
	{
		LinkAddress = Object ? Object->GetPathName() : TEXT("");
		if (bLinkObjectPath)
		{
			LinkAddressPath = LinkAddress;
			LinkAddress.Reset();
		}
		return *this;
	}

	FHintStruct& Link(const UScriptStruct* Struct)
	{
		LinkAddress = Struct ? Struct->GetStructPathName().ToString() : TEXT("");
		return *this;
	}
	
	bool HasLink() const { return !LinkAddressPath.IsNull() || !LinkAddress.IsEmpty(); }
	bool IsObjectLink() const { return !LinkAddressPath.IsNull(); }
	FString GetLink() const { return LinkAddressPath.IsNull() ? LinkAddress : LinkAddressPath.ToString(); }

	void PostSerialize(const FArchive& Ar)
	{
		if (Ar.IsSaving() && HasLink())
		{
			Ar.MarkSearchableName(FHintStruct::StaticStruct(), *GetLink());
		}
	}
#endif // WITH_EDITORONLY_DATA
};

#if WITH_EDITORONLY_DATA
template<>
struct TStructOpsTypeTraits< FHintStruct > : public TStructOpsTypeTraitsBase2< FHintStruct >
{
	enum
	{
		WithPostSerialize = true,
	};
}; 
#endif // WITH_EDITORONLY_DATA