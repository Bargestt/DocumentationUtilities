
#include "HintStructCustomization.h"
#include "HintStruct.h"
#include "DocumentationUtilitiesSettings.h"

#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <PropertyCustomizationHelpers.h>

#include <HAL/PlatformProcess.h>
#include <HAL/PlatformApplicationMisc.h>

#include <Dialogs/Dialogs.h>
#include <Framework/Notifications/NotificationManager.h>
#include <Widgets/Notifications/SNotificationList.h>


#define LOCTEXT_NAMESPACE "HintStructCustomization"


void FHintStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TArray<UObject*> Outers;
	PropertyHandle->GetOuterObjects(Outers);

	FText HintText = GetHint(
			PropertyHandle, 
			PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, HintText)).ToSharedRef(), 
			PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, HintSource)).ToSharedRef(),
			Outers);	

	FText HintTooltipText = GetHint(
		PropertyHandle,
		PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, TooltipText)).ToSharedRef(),
		PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, TooltipSource)).ToSharedRef(),
		Outers);

	FString Link;
	PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, LinkAddress))->GetValue(Link);
	
	FString Address = UDocumentationUtilities::ResolveLink(Link);

	HeaderRow	
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(100)
	.MaxDesiredWidth(4096)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Left)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(0, 2)
			[
				SNew(STextBlock)
				.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
				.AutoWrapText(true)
				.Text(HintText)
				.ToolTipText(HintTooltipText)
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
		[
			SNew(SBox)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.WidthOverride(22)
			.HeightOverride(22)
			.ToolTipText(FText::FromString(Address))
			.Visibility(Link.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
			[	
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked_Lambda([Link]()
				{
					FHintStructCustomization::OpenLink(Link);
					return FReply::Handled();
				})
				.ContentPadding(0)
				[			
					SNew(SImage)				
					.Image(FAppStyle::GetBrush("Icons.Help"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
		]
	];

	if (!Link.IsEmpty() || !Address.IsEmpty())
	{
		HeaderRow
		.AddCustomContextMenuAction(FUIAction(
			FExecuteAction::CreateLambda([Link]()
			{
				FPlatformApplicationMisc::ClipboardCopy(*Link);
			})),
			LOCTEXT("CopyLink", "Copy Link"),
			LOCTEXT("CopyLinkTooltip", "Copy link value"),
			FSlateIcon()
		)
		.AddCustomContextMenuAction(FUIAction(
			FExecuteAction::CreateLambda([Address]()
			{
				FPlatformApplicationMisc::ClipboardCopy(*Address);
			})),
			LOCTEXT("CopyAddress", "Copy Address"),
			LOCTEXT("CopyAddressTooltip", "Copy final link address"),
			FSlateIcon()
		);
	}
}


void FHintStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	if (PropertyHandle->HasMetaData(TEXT("HideChildren")))
	{
		return;
	}

	bool bIsCDO = false;
	{
		TArray<UObject*> Outers;
		PropertyHandle->GetOuterObjects(Outers);
		bIsCDO = Outers.Num() > 0 && Outers[0]->HasAnyFlags(RF_ClassDefaultObject);
	}

	if (bIsCDO)
	{
		uint32 NumChildren = 0;
		PropertyHandle->GetNumChildren(NumChildren);
		for (uint32 Index = 0; Index < NumChildren - 1; Index++)
		{
			ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(Index).ToSharedRef());
		}

		LinkHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, LinkAddress));


 		bool bLinkError = false;		
		{			
			FString Link; 
			if (LinkHandle->GetValue(Link) == FPropertyAccess::Success && !Link.IsEmpty())
			{
				FString Address = UDocumentationUtilities::ResolveLink(Link);
				bLinkError = !(Address.StartsWith(TEXT("http")) || Address.StartsWith(TEXT("https")));
			}
		}
		
		ChildBuilder.AddProperty(LinkHandle.ToSharedRef()).CustomWidget()
		.NameContent()
		[
			LinkHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.MinDesiredWidth(125.0f)
				[
					LinkHandle->CreatePropertyValueWidget()
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0)
			[
				
				SNew(SComboButton)
				.ButtonStyle( FAppStyle::Get(), "SimpleButton" )
				.HasDownArrow(false)	
				.ToolTipText(bLinkError ? LOCTEXT("LinkError", "Failed to resolve link") : LOCTEXT("PinkLink", "Pick link from settings"))
				.ButtonContent()
				[
					SNew(SBox)
					.WidthOverride(16)
					.HeightOverride(16)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::Get().GetBrush("NoBorder"))
						.ColorAndOpacity(bLinkError ? FLinearColor::Red : FLinearColor::White)
						.Padding(0)
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush("Symbols.SearchGlass"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
				.OnGetMenuContent(this, &FHintStructCustomization::CreateLinkOptions)
			]
		];
	}
}

FText FHintStructCustomization::GetHint(TSharedRef<IPropertyHandle> StructHandle, TSharedRef<IPropertyHandle> ManualValueHandle, TSharedRef<IPropertyHandle> ModeHandle, TArray<UObject*> OuterChain) const
{
	FText Hint;

	EHintSource Mode = EHintSource::MAX;
	{
		uint8 ModeByte;
		FPropertyAccess::Result Result = ModeHandle->GetValue(ModeByte);
		if (Result == FPropertyAccess::Success)
		{
			Mode = (EHintSource)ModeByte;
		}
		else if (Result == FPropertyAccess::MultipleValues)
		{
			Hint = LOCTEXT("MultipleValues", "Multiple Values");
		}
	}
	UObject* Outer = (OuterChain.Num() > 0) ? OuterChain[0] : nullptr;


	static_assert((uint8)EHintSource::MAX == 5, "Enum changed, update this func");
	switch (Mode)
	{
	case EHintSource::PropertyValue:
		ManualValueHandle->GetValueAsDisplayText(Hint);
		break;
	case EHintSource::PropertyTooltip:
		Hint = StructHandle->GetToolTipText();
		break;
	case EHintSource::ClassTooltip:
		if (Outer)
		{
			Hint = Outer->GetClass()->GetToolTipText();
		}		
		break;
	case EHintSource::FirstValidTooltip:
		if (Outer)
		{
			for (const UClass* Class = Outer->GetClass(); Class != nullptr; Class = Class->GetSuperClass())
			{
				Hint = Class->GetToolTipText();
				if (!Hint.IsEmpty() && !Hint.EqualToCaseIgnored(Class->GetDisplayNameText()))
				{
					break;
				}
			}
		}
		break;
	case EHintSource::NativeClassTooltip:
		if (Outer)
		{
			for (const UClass* Class = Outer->GetClass(); Class != nullptr; Class = Class->GetSuperClass())
			{
				if (Class->HasAnyClassFlags(CLASS_Native))
				{
					Hint = Class->GetToolTipText();
					break;
				}
			}
		}
		break;
	case EHintSource::MAX:
		//Some error happened
		break;
	}

	return Hint;
}


void FHintStructCustomization::SetLink(FString NewLink)
{
	if (LinkHandle.IsValid() && LinkHandle->IsValidHandle())
	{
		LinkHandle->SetValue(NewLink);
	}
}

TSharedRef<SWidget> FHintStructCustomization::CreateLinkOptions()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	const UDocumentationUtilities* Settings = GetDefault<UDocumentationUtilities>();
	if (Settings)
	{						
		TArray<UObject*> Outers;
		LinkHandle->GetOuterObjects(Outers);
		if (Outers.Num() > 0 && Outers[0] != nullptr)
		{
			UObject* Outer = Outers[0];
			UClass* OuterClass = Cast<UClass>(Outer);
			if (OuterClass == nullptr)
			{
				OuterClass = Outer->GetClass();
			}

			{
				FString OptionValue = Outer->GetPathName();

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_Self", "Self"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}

			{
				FString OptionValue = Outer->GetName();

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_SelfShort", "Self Short"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}

			{
				FString OptionValue = OuterClass->GetStructPathName().ToString();

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_Class", "Class"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}

			{
				FString OptionValue = OuterClass->GetName();

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_ClassShort", "Class Short"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}
		}



		MenuBuilder.BeginSection("Native", LOCTEXT("SectionNative", "Native"));						
		for (auto& KeyValuePair : Settings->NativeLinks)
		{
			FString LinkKey = KeyValuePair.Key;
			FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, LinkKey));

			MenuBuilder.AddMenuEntry(FText::FromString(LinkKey), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Doc", LOCTEXT("SectionDoc", "Doc"));						
		for (auto& KeyValuePair : Settings->DocumentationLinks)
		{
			FString LinkKey = KeyValuePair.Key;
			FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, LinkKey));

			MenuBuilder.AddMenuEntry(FText::FromString(LinkKey), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Class", LOCTEXT("SectionClass", "Class"));						
		for (auto& KeyValuePair : Settings->ClassDocumentationLinks)
		{
			FString LinkKey = KeyValuePair.Key.ToString();
			FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, LinkKey));

			MenuBuilder.AddMenuEntry(FText::FromString(LinkKey), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("Asset", LOCTEXT("SectionAsset", "Asset"));						
		for (auto& KeyValuePair : Settings->AssetDocumentationLinks)
		{
			FString LinkKey = KeyValuePair.Key.ToString();
			FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLink, LinkKey));

			MenuBuilder.AddMenuEntry(FText::FromString(LinkKey), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

void FHintStructCustomization::OpenLink(const FString& Link)
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

#undef LOCTEXT_NAMESPACE




