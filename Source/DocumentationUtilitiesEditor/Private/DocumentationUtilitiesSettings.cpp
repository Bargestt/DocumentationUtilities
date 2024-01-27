// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.


#include "DocumentationUtilitiesSettings.h"
#include <UObject/ObjectSaveContext.h>
#include <ProfilingDebugging/ScopedTimers.h>

bool FDocumentationHintLink::ExportTextItem(FString& ValueStr, FDocumentationHintLink const& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
	// Custom export to remove redundant data

	FDocumentationHintLink Default;
	Default.Type = EDocumentationLinkType::MAX; //force to always appear in diff

	FDocumentationHintLink Export;
	Export.Type = this->Type;
	Export.StringKey = this->GetLinkKey();
	Export.Value = this->Value;

	FDocumentationHintLink::StaticStruct()->ExportText(ValueStr, &Export, &Default, Parent, PortFlags, ExportRootScope, false);
	return true;
}

bool FDocumentationHintLink::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	FDocumentationHintLink::StaticStruct()->ImportText(Buffer, this, Parent, PortFlags, ErrorText, TEXT("FDocumentationHintLink"), false);

	switch (Type)
	{		
	case EDocumentationLinkType::Asset: AssetKey = StringKey; StringKey.Empty(); break;
	case EDocumentationLinkType::Class: ClassKey = StringKey; StringKey.Empty(); break;
	case EDocumentationLinkType::String: break;
	case EDocumentationLinkType::Native: break;	
	}

	return true;
}



UDocumentationUtilities::UDocumentationUtilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxContentBrowserLinks = 4;

	ClassDocumentationLink = TEXT("Class Documentation - {0}");
	AssetDocumentationLink = TEXT("Asset Documentation - {0}");

	bCollectNativeHints = true;

	bLinksPicker_ShowNative = true;
	bLinksPicker_ShowString = true;
	bLinksPicker_ShowAsset = true;
	bLinksPicker_ShowClass = true;	
}

void UDocumentationUtilities::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	Links.StableSort([](const FDocumentationHintLink& A, const FDocumentationHintLink& B)
	{
		return A.GetLinkKey() < B.GetLinkKey();
	});
}

void UDocumentationUtilities::PostInitProperties()
{
	Super::PostInitProperties();

	if (bCollectNativeHints)
	{
		FScopedDurationTimeLogger(TEXT("DocumentationUtilities::IncludeNativeHints"));

		TMap<FString, FString> OldNativeLinks = CollectLinksOfType(EDocumentationLinkType::Native);
		NativeLinks.Reset();

		TMap<FString, FString> TempMap;
		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* Class = *ClassIt;
			const UObject* CDO = Class->GetDefaultObject();

			for (TFieldIterator<FStructProperty> PropertyIt(Class, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
			{
				FStructProperty* Prop = *PropertyIt;
				if (Prop->Struct == FHintStruct::StaticStruct())
				{
					const FHintStruct* Value = Prop->ContainerPtrToValuePtr<FHintStruct>(CDO);
					
					FString LinkKey = Value ? Value->GetLink() : TEXT("");
					if (!LinkKey.IsEmpty())
					{
						const FString* OldLink = OldNativeLinks.Find(LinkKey);
						TempMap.Add(LinkKey, OldLink ? *OldLink : TEXT(""));
					}					
				}
			}
		}

		//TempMap.Remove(Hint.GetLink());
		TempMap.KeyStableSort([](const FString& A, const FString& B) { return A < B; });
		for (const auto& Pair : TempMap)
		{
			FDocumentationHintLink NativeLink;
			NativeLink.Type = EDocumentationLinkType::Native;
			NativeLink.StringKey = Pair.Key;
			NativeLink.Value = Pair.Value;

			NativeLinks.Add(NativeLink);
		}
	}
}

const FDocumentationHintLink* UDocumentationUtilities::FindLinkByKey(const FString& Link)
{
	if (const UDocumentationUtilities* Settings = GetDefault<UDocumentationUtilities>())
	{
		auto Lambda = [Link](const FDocumentationHintLink& Entry) { return Entry.GetLinkKey() == Link; };

		if (const FDocumentationHintLink* Redirect = Settings->Links.FindByPredicate(Lambda))
		{
			if (Redirect->HasValue())
			{
				return Redirect;
			}			
		}

		if (const FDocumentationHintLink* Redirect = Settings->NativeLinks.FindByPredicate(Lambda))
		{
			if (Redirect->HasValue())
			{
				return Redirect;
			}			
		}
	}
	return nullptr;
}

bool UDocumentationUtilities::HasLinkRedirector(const FString& Link)
{
	return FindLinkByKey(Link) != nullptr;
}

FString UDocumentationUtilities::ResolveLink(const FString& Link)
{
	const FDocumentationHintLink* Redirector = FindLinkByKey(Link);
	return Redirector ? Redirector->Value : Link;
}

TMap<FString, FString> UDocumentationUtilities::CollectLinksOfType(EDocumentationLinkType Type) const
{
	TMap<FString, FString> Map;

	const TArray<FDocumentationHintLink>& Source = (Type == EDocumentationLinkType::Native) ? NativeLinks : Links;
	for (const FDocumentationHintLink& Link : Source)
	{
		if (Link.IsValid() && Link.Type == Type)
		{
			Map.Add(Link.GetLinkKey(), Link.Value);			
		}
	}

	return Map;
}

