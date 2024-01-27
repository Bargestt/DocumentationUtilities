// Copyright (C) Vasily Bulgakov. 2024. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"


class IDocumentationUtilitiesEditorModule : public IModuleInterface
{
public:
	/**  */
	static void OpenLink(FString Link);

	/** Check link can result in action */
	static bool IsLinkValid(FString Link);
};


