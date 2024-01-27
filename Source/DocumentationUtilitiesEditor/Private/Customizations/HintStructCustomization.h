// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IPropertyHandle;
enum class EHintSource : uint8;

class FHintStructCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FHintStructCustomization);
	}

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

protected:
	FText GetHint(TSharedRef<IPropertyHandle> StructHandle, TSharedRef<IPropertyHandle> ManualValueHandle, TSharedRef<IPropertyHandle> ModeHandle, TArray<UObject*> OuterChain) const;

	void SetLink(FString NewLink, bool bTryLock = true);
	void SetLinkAndLock(FString NewLink) { SetLink(NewLink, true); }
	FString GetLink() const;
	FString GetLinkAddress() const;

	FText GetLinkText() const;
	bool LinkReadOnly() const;
	void ToggleLock();

	void CopyLink();
	void PasteLink();

	void CopyLinkAddress();

	void AddressChanged(const FText& NewValue, ETextCommit::Type ChangeType);

	TSharedRef<SWidget> CreateLinkOptions();

private:
	//TSharedPtr<IPropertyHandle> HintStructHandle;

	TSharedPtr<IPropertyHandle> LinkAddressPathHandle;
	TSharedPtr<IPropertyHandle> LinkAddressHandle;
};
