// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.


#include "DocumentationUtilitiesSettings.h"
#include <AssetRegistry/AssetIdentifier.h>
#include <Editor.h>


void UDocumentationUtilities::PostInitProperties()
{
	Super::PostInitProperties();

	if (bIncludeNativeHints)
	{
		TMap<FString, FString> OldNativeLinks = NativeLinks;
		NativeLinks.Reset();

		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			UClass* Class = *ClassIt;
			for (TFieldIterator<FStructProperty> PropertyIt(Class, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
			{
				FStructProperty* Prop = *PropertyIt;
				if (Prop->Struct == FHintStruct::StaticStruct())
				{
					FHintStruct* Value = Prop->ContainerPtrToValuePtr<FHintStruct>(Class->GetDefaultObject());
					
					FString LinkAddress = Value ? Value->LinkAddress : TEXT("");
					if (!LinkAddress.IsEmpty())
					{
						const FString* OldRedirect = OldNativeLinks.Find(LinkAddress);
						NativeLinks.Add(LinkAddress, OldRedirect ? *OldRedirect : TEXT(""));
					}					
				}
			}
		}
		NativeLinks.Remove(Hint.LinkAddress);
	}	

	NativeLinks.KeyStableSort([](const FString& A, const FString& B) { return A < B; });
	DocumentationLinks.KeyStableSort([](const FString& A, const FString& B) { return A < B; });
	ClassDocumentationLinks.KeyStableSort([](const FSoftClassPath& A, const FSoftClassPath& B) { return A.ToString() < B.ToString(); });
	AssetDocumentationLinks.KeyStableSort([](const FSoftObjectPath& A, const FSoftObjectPath& B) { return A.ToString() < B.ToString(); });
}

void UDocumentationUtilities::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (bFindReference)
	{
		TArray<FAssetIdentifier> AssetIdentifiers;
		AssetIdentifiers.Add(FAssetIdentifier(FHintStruct::StaticStruct(), *FindReference));
		FEditorDelegates::OnOpenReferenceViewer.Broadcast(AssetIdentifiers, FReferenceViewerParams());

		bFindReference = false;
	}
}

FString UDocumentationUtilities::ResolveLink(const FString& Link)
{
	if (const UDocumentationUtilities* Settings = GetDefault<UDocumentationUtilities>())
	{
		if (const FString* Redirect = Settings->NativeLinks.Find(Link))
		{
			if (!Redirect->IsEmpty())
			{
				return *Redirect;
			}
		}

		if (const FString* Redirect = Settings->DocumentationLinks.Find(Link))
		{
			if (!Redirect->IsEmpty())
			{
				return *Redirect;
			}			
		}

		if (const FString* Redirect = Settings->ClassDocumentationLinks.Find(Link))
		{
			if (!Redirect->IsEmpty())
			{
				return *Redirect;
			}
		}

		if (const FString* Redirect = Settings->AssetDocumentationLinks.Find(Link))
		{
			if (!Redirect->IsEmpty())
			{
				return *Redirect;
			}
		}
	}

	return Link;
}
