
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
		for (uint32 Index = 0; Index < NumChildren; Index++)
		{
			ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(Index).ToSharedRef());
		}
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




