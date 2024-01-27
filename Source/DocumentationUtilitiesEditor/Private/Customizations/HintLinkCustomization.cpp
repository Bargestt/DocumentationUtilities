
#include "HintLinkCustomization.h"
#include "DocumentationUtilitiesEditor.h"
#include "DocumentationUtilitiesSettings.h"

#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <PropertyCustomizationHelpers.h>
#include <Widgets/Layout/SWidgetSwitcher.h>

#include <AssetRegistry/AssetIdentifier.h>
#include <Editor.h>

#include <HAL/PlatformApplicationMisc.h>




#define LOCTEXT_NAMESPACE "HintLinkCustomization"


void FHintLinkCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyHandle> TypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDocumentationHintLink, Type));
	auto GetWidgetIndex = [TypeHandle]() -> int32
	{
		if (TypeHandle.IsValid() && TypeHandle->IsValidHandle())
		{
			uint8 Data;
			if (TypeHandle->GetValue(Data) == FPropertyAccess::Success)
			{
				switch ((EDocumentationLinkType)Data)
				{		
				case EDocumentationLinkType::Asset: return 2;
				case EDocumentationLinkType::Class: return 3;
				case EDocumentationLinkType::String: return 1;
				case EDocumentationLinkType::Native: return 1;
				}
			}
		}
		return 0;
	};


	HeaderRow	
	.NameContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(5.0f, 0.0f)
		[
			SNew(SBox)
			.MinDesiredWidth(125.0f)
			[
				TypeHandle->CreatePropertyValueWidget()
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SBox).MinDesiredWidth(4096)
			[
				SNew(SWidgetSwitcher)
				.WidgetIndex(MakeAttributeLambda(GetWidgetIndex))
				+ SWidgetSwitcher::Slot()
				[
					SNew(STextBlock)
					.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
					.Text(LOCTEXT("ErrorType", "Error"))
				]
				+ SWidgetSwitcher::Slot()
				[
					PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDocumentationHintLink, StringKey))->CreatePropertyValueWidget()
				]
				+ SWidgetSwitcher::Slot()
				[
					PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDocumentationHintLink, AssetKey))->CreatePropertyValueWidget()
				]
				+ SWidgetSwitcher::Slot()
				[
					PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDocumentationHintLink, ClassKey))->CreatePropertyValueWidget()
				]
			]
		]
	]
	.ValueContent()
	.MinDesiredWidth(125)
	.MaxDesiredWidth(4096)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Left)
	[
		PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDocumentationHintLink, Value))->CreatePropertyValueWidget()
	]
	.AddCustomContextMenuAction(FUIAction(
			FExecuteAction::CreateLambda([StructHandle = TSharedPtr<IPropertyHandle>(PropertyHandle)]()
			{
				if (StructHandle.IsValid() && StructHandle->IsValidHandle())
				{
					const FDocumentationHintLink* LinkPtr = nullptr;
					{
						TArray<void*> StructPtrs;
						StructHandle->AccessRawData(StructPtrs);
						LinkPtr = (StructPtrs.Num() == 1) ? reinterpret_cast<FDocumentationHintLink*>(StructPtrs[0]) : nullptr;
					}

					if (LinkPtr)
					{
						const FString Reference = LinkPtr->GetLinkKey();

						TArray<FAssetIdentifier> AssetIdentifiers;
						AssetIdentifiers.Add(FAssetIdentifier(FHintStruct::StaticStruct(), *Reference));
						FEditorDelegates::OnOpenReferenceViewer.Broadcast(AssetIdentifiers, FReferenceViewerParams());
					}
				}
			})),
			LOCTEXT("FindReferences", "Find References"),
			LOCTEXT("FindReferencesTooltip", "Find References"),
			FSlateIcon()
		)
	.AddCustomContextMenuAction(FUIAction(
			FExecuteAction::CreateLambda([StructHandle = TSharedPtr<IPropertyHandle>(PropertyHandle)]()
			{
				if (StructHandle.IsValid() && StructHandle->IsValidHandle())
				{
					const FDocumentationHintLink* LinkPtr = nullptr;
					{
						TArray<void*> StructPtrs;
						StructHandle->AccessRawData(StructPtrs);
						LinkPtr = (StructPtrs.Num() == 1) ? reinterpret_cast<FDocumentationHintLink*>(StructPtrs[0]) : nullptr;
					}

					if (LinkPtr)
					{
						FPlatformApplicationMisc::ClipboardCopy(*LinkPtr->GetLinkKey());						
					}
				}
			})),
			LOCTEXT("CopyKey", "Copy Key"),
			LOCTEXT("CopyKeyTooltip", "Copy Key"),
			FSlateIcon()
		);
}


void FHintLinkCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{

}


#undef LOCTEXT_NAMESPACE




