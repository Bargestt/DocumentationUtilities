// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "DocumentationUtilitiesEditor.h"
#include "Modules/ModuleManager.h"
#include <PropertyEditorModule.h>


#include "HintStruct.h"
#include "Customizations/HintStructCustomization.h"


#define LOCTEXT_NAMESPACE "FDocumentationUtilitiesEditorModule"


class FDocumentationUtilitiesEditorModule : public IDocumentationUtilitiesEditorModule
{
public:
	virtual void StartupModule() override
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		{
			PropertyModule.RegisterCustomPropertyTypeLayout(FHintStruct::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FHintStructCustomization::MakeInstance));
		}
	}
	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyModule.UnregisterCustomPropertyTypeLayout(FHintStruct::StaticStruct()->GetFName());
		}
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDocumentationUtilitiesEditorModule, DocumentationUtilitiesEditor)