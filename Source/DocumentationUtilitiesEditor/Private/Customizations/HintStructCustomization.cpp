// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "HintStructCustomization.h"
#include "DocumentationUtilitiesEditor.h"
#include "HintStruct.h"
#include "DocumentationUtilitiesSettings.h"

#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <PropertyCustomizationHelpers.h>

#include <HAL/PlatformProcess.h>
#include <HAL/PlatformApplicationMisc.h>




#define LOCTEXT_NAMESPACE "HintStructCustomization"


void FHintStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	LinkAddressPathHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, LinkAddressPath));
	LinkAddressHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FHintStruct, LinkAddress));


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
			.ToolTipText(FText::FromString(GetLinkAddress()))
			.Visibility(GetLink().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
			[	
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked_Lambda([this]()
				{
					IDocumentationUtilitiesEditorModule::OpenLink(GetLink());
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

	HeaderRow
	.AddCustomContextMenuAction(FUIAction(
		FExecuteAction::CreateSP(this, &FHintStructCustomization::CopyLink)),
		LOCTEXT("CopyLink", "Copy Link"),
		LOCTEXT("CopyLinkTooltip", "Copy link value"),
		FSlateIcon()
	)
	.AddCustomContextMenuAction(FUIAction(
		FExecuteAction::CreateSP(this, &FHintStructCustomization::CopyLinkAddress)),
		LOCTEXT("CopyAddress", "Copy Address"),
		LOCTEXT("CopyAddressTooltip", "Copy final link address"),
		FSlateIcon()
	);
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
		// Show all but last two
		uint32 NumChildren = 0;
		PropertyHandle->GetNumChildren(NumChildren);
		for (uint32 Index = 0; Index < NumChildren - 2; Index++)
		{
			ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(Index).ToSharedRef());
		}		

 		bool bLinkError = false;
		{			
			FString Link = GetLink(); 
			if (!Link.IsEmpty() && Link != TEXT("None"))
			{
				bLinkError = !IDocumentationUtilitiesEditorModule::IsLinkValid(Link);
			}
		}
		

		ChildBuilder.AddCustomRow(LOCTEXT("LinkAddress", "LinkAddress"))
		.CopyAction(FUIAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::CopyLink)))
		.PasteAction(FUIAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::PasteLink)))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.Text(LOCTEXT("LinkAdressLabel", "Link Address"))
			.ToolTipText(LOCTEXT("LinkAdressTooltip", "Set link address using name from Settings, Asset, Class or enter http link directly"))
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.MinDesiredWidth(125.0f)
				[
					SNew(SEditableTextBox)							
					.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
					.Text(this, &FHintStructCustomization::GetLinkText)
					.ToolTipText(this, &FHintStructCustomization::GetLinkText)
					.OnTextCommitted(FOnTextCommitted::CreateSP(this, &FHintStructCustomization::AddressChanged))
					.IsReadOnly(this, &FHintStructCustomization::LinkReadOnly)
					.ClearKeyboardFocusOnCommit(false)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0)
			[
				SNew(SButton)
				.ButtonStyle( FAppStyle::Get(), "SimpleButton" )
				.ToolTipText_Lambda([this]()
				{
					return this->LinkReadOnly() ? LOCTEXT("Unbind", "Unbind Link from the object") : LOCTEXT("Bind", "Try bind Link to the object");
				})
				.OnClicked_Lambda([this]()
				{
					this->ToggleLock();
					return FReply::Handled();
				})
				.ContentPadding(0.f)
				[
					SNew(SImage)
					.Image_Lambda([this]()
					{						
						return this->LinkReadOnly() ? FAppStyle::Get().GetBrush("Icons.Lock") : FAppStyle::Get().GetBrush("Icons.Unlock");
					})
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0)
			[
				
				SNew(SComboButton)
				.ButtonStyle( FAppStyle::Get(), "SimpleButton" )
				.HasDownArrow(false)	
				.ToolTipText(bLinkError ? 
					LOCTEXT("LinkError", "This link will have no effect.\nEnter different string or add action in settings") : 
					LOCTEXT("PinkLink", "Pick link from settings"))
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


void FHintStructCustomization::SetLink(FString NewLink, bool bTryLock/* = true*/)
{
	if (LinkAddressPathHandle.IsValid() && LinkAddressHandle.IsValid())
	{
		FScopedTransaction Transaction(LOCTEXT("SetLink", "Set Link"));

		FString Path;

		LinkAddressPathHandle->SetValueFromFormattedString(TEXT("None"));
		if (bTryLock)
		{
			LinkAddressPathHandle->SetValueFromFormattedString(NewLink);
			LinkAddressPathHandle->GetValueAsFormattedString(Path);
		}

		bool bHasPath = !Path.IsEmpty() && Path != TEXT("None");
		LinkAddressHandle->SetValueFromFormattedString(bHasPath ? TEXT("") : NewLink);
	}	
}

FString FHintStructCustomization::GetLink() const
{
	FString Path;	
	if (LinkAddressPathHandle->GetValueAsFormattedString(Path) == FPropertyAccess::Success && !Path.IsEmpty() && Path != TEXT("None"))
	{
		return Path;
	}

	Path.Reset();
	if (LinkAddressHandle->GetValueAsFormattedString(Path) == FPropertyAccess::Success)
	{
		return Path;
	}

	return TEXT("");
}

FString FHintStructCustomization::GetLinkAddress() const
{
	FString Link = GetLink();	
	FString Address = UDocumentationUtilities::ResolveLink(Link);
	return Address;
}

FText FHintStructCustomization::GetLinkText() const
{
	return FText::FromString(GetLink());
}

bool FHintStructCustomization::LinkReadOnly() const
{
	FString Path;
	bool bIsEditable = LinkAddressPathHandle->GetValueAsFormattedString(Path) == FPropertyAccess::Success && Path == TEXT("None");
	return !bIsEditable;
}

void FHintStructCustomization::ToggleLock()
{
	FString Path;
	if (LinkAddressPathHandle->GetValueAsFormattedString(Path) == FPropertyAccess::Success)
	{
		if (Path != TEXT("None"))
		{
			FScopedTransaction Transaction(LOCTEXT("UnlockLink", "Unlock Link"));

			LinkAddressPathHandle->SetValueFromFormattedString(TEXT("None"));
			LinkAddressHandle->SetValue(Path);
		}
		else
		{
			FScopedTransaction Transaction(LOCTEXT("LockLink", "Lock Link"));
						
			Path.Reset();

			FString NewLink;
			LinkAddressHandle->GetValueAsFormattedString(NewLink);

			LinkAddressPathHandle->SetValueFromFormattedString(TEXT("None"));
			LinkAddressPathHandle->SetValueFromFormattedString(NewLink);
			LinkAddressPathHandle->GetValueAsFormattedString(Path);

			bool bHasPath = !Path.IsEmpty() && Path != TEXT("None");
			LinkAddressHandle->SetValueFromFormattedString(bHasPath ? TEXT("") : NewLink);
		}
	}
}

void FHintStructCustomization::CopyLink()
{
	FString Data = GetLink();
	FPlatformApplicationMisc::ClipboardCopy(*Data);
}

void FHintStructCustomization::PasteLink()
{
	FString Data;
	FPlatformApplicationMisc::ClipboardPaste(Data);
	SetLink(Data, true);
}

void FHintStructCustomization::CopyLinkAddress()
{
	FString Data = GetLinkAddress();
	FPlatformApplicationMisc::ClipboardCopy(*Data);
}

void FHintStructCustomization::AddressChanged(const FText& NewValue, ETextCommit::Type ChangeType)
{
	SetLink(NewValue.ToString(), false);
}

TSharedRef<SWidget> FHintStructCustomization::CreateLinkOptions()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	const UDocumentationUtilities* Settings = GetDefault<UDocumentationUtilities>();
	if (Settings)
	{						
		TArray<UObject*> Outers;
		LinkAddressHandle->GetOuterObjects(Outers);
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

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_Self", "Self"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}

			{
				FString OptionValue = Outer->GetName();

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_SelfShort", "Self Short"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}

			{
				FString OptionValue = OuterClass->GetStructPathName().ToString();

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_Class", "Class"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}

			{
				FString OptionValue = OuterClass->GetName();

				FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, OptionValue));
				MenuBuilder.AddMenuEntry(LOCTEXT("LinkOption_ClassShort", "Class Short"), FText::FromString(OptionValue), FSlateIcon(), LinkAction);
			}
		}



		TMap<FString, FString> LinkOptions;


		if (Settings->bLinksPicker_ShowNative)
		{
			LinkOptions = Settings->CollectLinksOfType(EDocumentationLinkType::Native);
			if (LinkOptions.Num() > 0)
			{
				MenuBuilder.BeginSection("Native", LOCTEXT("SectionNative", "Native"));	
				for (auto& KeyValuePair : LinkOptions)
				{
					FString LinkKey = KeyValuePair.Key;
					FString DisplayName = LinkKey;
					if (Settings->bLinksPicker_ShortNames)
					{
						LinkKey.Split(TEXT("."), nullptr, &DisplayName);
					}	

					FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, LinkKey));
					MenuBuilder.AddMenuEntry(FText::FromString(DisplayName), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
				}
				MenuBuilder.EndSection();
			}
		}


		if (Settings->bLinksPicker_ShowString)
		{
			LinkOptions = Settings->CollectLinksOfType(EDocumentationLinkType::String);
			if (LinkOptions.Num() > 0)
			{
				MenuBuilder.BeginSection("String", LOCTEXT("SectionString", "String"));
				for (auto& KeyValuePair : LinkOptions)
				{
					FString LinkKey = KeyValuePair.Key;
					FString DisplayName = LinkKey;
					if (Settings->bLinksPicker_ShortNames)
					{
						LinkKey.Split(TEXT("."), nullptr, &DisplayName);
					}					

					FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, LinkKey));
					MenuBuilder.AddMenuEntry(FText::FromString(DisplayName), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
				}
				MenuBuilder.EndSection();
			}
		}


		if (Settings->bLinksPicker_ShowClass)
		{
			LinkOptions = Settings->CollectLinksOfType(EDocumentationLinkType::Class);
			if (LinkOptions.Num() > 0)
			{
				MenuBuilder.BeginSection("Class", LOCTEXT("SectionClass", "Class"));
				for (auto& KeyValuePair : LinkOptions)
				{
					FString LinkKey = KeyValuePair.Key;
					FString DisplayName = LinkKey;
					if (Settings->bLinksPicker_ShortNames)
					{
						LinkKey.Split(TEXT("."), nullptr, &DisplayName);
					}	

					FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, LinkKey));
					MenuBuilder.AddMenuEntry(FText::FromString(DisplayName), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
				}
				MenuBuilder.EndSection();
			}
		}


		if (Settings->bLinksPicker_ShowAsset)
		{
			LinkOptions = Settings->CollectLinksOfType(EDocumentationLinkType::Asset);
			if (LinkOptions.Num() > 0)
			{			
				MenuBuilder.BeginSection("Asset", LOCTEXT("SectionAsset", "Asset"));
				for (auto& KeyValuePair : LinkOptions)
				{
					FString LinkKey = KeyValuePair.Key;
					FString DisplayName = LinkKey;
					if (Settings->bLinksPicker_ShortNames)
					{
						LinkKey.Split(TEXT("."), nullptr, &DisplayName);
					}	

					FUIAction LinkAction(FExecuteAction::CreateSP(this, &FHintStructCustomization::SetLinkAndLock, LinkKey));
					MenuBuilder.AddMenuEntry(FText::FromString(DisplayName), FText::FromString(KeyValuePair.Value), FSlateIcon(), LinkAction);
				}
				MenuBuilder.EndSection();
			}
		}
	}

	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE




