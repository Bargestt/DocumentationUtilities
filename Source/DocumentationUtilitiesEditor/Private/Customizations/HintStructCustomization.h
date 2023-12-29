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

	void SetLink(FString NewLink);

	TSharedRef<SWidget> CreateLinkOptions();

	static void OpenLink(const FString& Link);

private:
	TSharedPtr<IPropertyHandle> LinkHandle;
};
