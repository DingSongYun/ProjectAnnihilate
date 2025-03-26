// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IAssetTools.h"

class FNeCharacterDataEditorModule final : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	TArray<TSharedPtr<IAssetTypeActions>> AssetTypeActions;
};
