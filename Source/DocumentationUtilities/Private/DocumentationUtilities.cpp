// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#include "DocumentationUtilities.h"

#define LOCTEXT_NAMESPACE "FDocumentationUtilitiesModule"


void FDocumentationUtilitiesModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FDocumentationUtilitiesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDocumentationUtilitiesModule, DocumentationUtilities)