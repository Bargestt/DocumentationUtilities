// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HintStruct.generated.h"


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

//
// Kept in runtime module to avoid bunch of packaging warnings
//



/** 
 * Setup and display hint
 * 
 * Can forcefully hide settings using meta: HideChildren
 */
USTRUCT(BlueprintType)
struct DOCUMENTATIONUTILITIES_API FHintStruct
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(EditAnywhere)
	EHintSource HintSource;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "HintSource == EHintSource::PropertyValue", EditConditionHides))
	FString HintText;


	UPROPERTY(EditAnywhere)
	EHintSource TooltipSource;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "TooltipSource == EHintSource::PropertyValue", EditConditionHides))
	FString TooltipText;


	UPROPERTY(EditAnywhere)
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

	FHintStruct& Link(const UObject* Object)
	{
		LinkAddress = Object ? Object->GetPathName() : TEXT("");
		return *this;
	}

	FHintStruct& Link(const UClass* Object)
	{
		LinkAddress = Object ? Object->GetPathName() : TEXT("");
		return *this;
	}

	FHintStruct& Link(const UScriptStruct* Struct)
	{
		LinkAddress = Struct ? Struct->GetStructPathName().ToString() : TEXT("");
		return *this;
	}


	void PostSerialize(const FArchive& Ar)
	{
		if (Ar.IsSaving())
		{
			if (!LinkAddress.IsEmpty())
			{
				Ar.MarkSearchableName(FHintStruct::StaticStruct(), *LinkAddress);
			}
			Ar.MarkSearchableName(FHintStruct::StaticStruct(), NAME_None);
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