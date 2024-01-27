// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "DocumentationUtilitiesEditor.h"
#include "Modules/ModuleManager.h"
#include <PropertyEditorModule.h>


#include "HintStruct.h"
#include "Customizations/HintStructCustomization.h"
#include "Customizations/HintLinkCustomization.h"

#include <ToolMenus.h>
#include <ContentBrowserMenuContexts.h>

#include <Dialogs/Dialogs.h>
#include <Framework/Notifications/NotificationManager.h>
#include <Widgets/Notifications/SNotificationList.h>



#define LOCTEXT_NAMESPACE "DocumentationUtilities"



void IDocumentationUtilitiesEditorModule::OpenLink(FString Link)
{
	FString Address = UDocumentationUtilities::ResolveLink(Link);
	if (Address.StartsWith(TEXT("http")) || Address.StartsWith(TEXT("https")))
	{
		FText Message = LOCTEXT("OpeningURLMessage", "You are about to open an external URL. This will open your web browser. Do you want to proceed?");
		FText URLDialog = LOCTEXT("OpeningURLTitle", "Open external link");

		FSuppressableWarningDialog::FSetupInfo Info(Message, URLDialog, "Hint_SuppressOpenURLWarning");
		Info.ConfirmText = LOCTEXT("OpenURL_yes", "Yes");
		Info.CancelText = LOCTEXT("OpenURL_no", "No");
		FSuppressableWarningDialog OpenURLWarning(Info);

		if (OpenURLWarning.ShowModal() != FSuppressableWarningDialog::Cancel)
		{
			FPlatformProcess::LaunchURL(*Address, nullptr, nullptr);
		}
	}
	else
	{
		const FText ErrorText = LOCTEXT("OpenURL_BadAddress", "Failed to open link: Bad address");
		FNotificationInfo Info(ErrorText);
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}	
}

bool IDocumentationUtilitiesEditorModule::IsLinkValid(FString Link)
{
	FString Address = UDocumentationUtilities::ResolveLink(Link);
	return Address.StartsWith(TEXT("http")) || Address.StartsWith(TEXT("https"));
}

class FDocumentationUtilitiesEditorModule : public IDocumentationUtilitiesEditorModule
{
public:
	virtual void StartupModule() override
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		{
			PropertyModule.RegisterCustomPropertyTypeLayout(FHintStruct::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHintStructCustomization::MakeInstance));
			PropertyModule.RegisterCustomPropertyTypeLayout(FDocumentationHintLink::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHintLinkCustomization::MakeInstance));
		}
		

		if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu"))
		{		
			FToolMenuSection& Section = Menu->FindOrAddSection("DocumentationUtilities");

			Section.AddDynamicEntry("AssetDocumentation", 
				FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
				{
					const UContentBrowserAssetContextMenuContext* Context = InSection.FindContext<UContentBrowserAssetContextMenuContext>();
					if (!Context)
					{
						return;
					}
					
					TArray<TTuple<FString, FString>> ClassDocs;
					TArray<TTuple<FString, FString>> AssetDocs;

					const UDocumentationUtilities* Settings = GetDefault<UDocumentationUtilities>();

					for (const FAssetData& AssetData : Context->SelectedAssets)
					{
						{
							FString Path = AssetData.GetObjectPathString();
							FString Address = UDocumentationUtilities::ResolveLink(Path);
							bool bIsValidLink = !Address.IsEmpty() && Address != Path;
							if (bIsValidLink || Settings->bShowUndocumentedLinks)
							{
								AssetDocs.Add(MakeTuple(Path, bIsValidLink ? Address : TEXT("")));
							}
						}

						{
							FString Path = AssetData.AssetClassPath.ToString();
							FString Address = UDocumentationUtilities::ResolveLink(Path);
							bool bIsValidLink = !Address.IsEmpty() && Address != Path;
							if (bIsValidLink || Settings->bShowUndocumentedLinks)
							{
								ClassDocs.Add(MakeTuple(Path, bIsValidLink ? Address : TEXT("")));
							}
						}


						if (ClassDocs.Num() + AssetDocs.Num() >= Settings->MaxContentBrowserLinks)
						{
							break;
						}
					}

					FCanExecuteAction DeniedAction = FCanExecuteAction::CreateLambda([]() { return false; });

					for (const auto& Pair : ClassDocs)
					{
						const FString& Path = Pair.Get<0>();
						const FString& Link = Pair.Get<1>();
						
						FString Name;
						Path.Split(TEXT("."), nullptr, &Name, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

						InSection.AddMenuEntry(
							*FString::Printf(TEXT("OpenDocs_Class_%s"), *Path),
							FText::Format(FText::FromString(Settings->ClassDocumentationLink), FText::FromString(Name)),
							FText::Format(LOCTEXT("OpenDocs_ToolTip", "Click to open documentation\n{0}"), FText::FromString(Path)),
							FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation" ),
							FUIAction(
								FExecuteAction::CreateStatic(&IDocumentationUtilitiesEditorModule::OpenLink, Link),
								Link.IsEmpty() ? DeniedAction : FCanExecuteAction())
							);
					}

					for (const auto& Pair : AssetDocs)
					{
						const FString& Path = Pair.Get<0>();
						const FString& Link = Pair.Get<1>();	
						
						FString Name;
						Path.Split(TEXT("."), nullptr, &Name, ESearchCase::CaseSensitive, ESearchDir::FromEnd);

						InSection.AddMenuEntry(
							*FString::Printf(TEXT("OpenDocs_Asset_%s"), *Path),
							FText::Format(FText::FromString(Settings->AssetDocumentationLink), FText::FromString(Name)),
							FText::Format(LOCTEXT("OpenDocs_ToolTip", "Click to open documentation\n{0}"), FText::FromString(Path)),
							FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation" ),
							FUIAction(
								FExecuteAction::CreateStatic(&IDocumentationUtilitiesEditorModule::OpenLink, Link),
								Link.IsEmpty() ? DeniedAction : FCanExecuteAction())
							);
					}
				}));
		}
	}
	virtual void ShutdownModule() override
	{
		if (UToolMenus* ToolMenus = UToolMenus::TryGet())
		{
			if (UToolMenu* Menu = ToolMenus->FindMenu("ContentBrowser.AssetContextMenu"))
			{
				Menu->RemoveSection("DocumentationUtilities");
			}
		}		

		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyModule.UnregisterCustomPropertyTypeLayout(FHintStruct::StaticStruct()->GetFName());
			PropertyModule.UnregisterCustomPropertyTypeLayout(FDocumentationHintLink::StaticStruct()->GetFName());
		}
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDocumentationUtilitiesEditorModule, DocumentationUtilitiesEditor)