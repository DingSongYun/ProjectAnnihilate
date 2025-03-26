// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCommonLogicFlowEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	TSharedPtr<class IAssetTypeActions> AssetTypActions_LogicFlow;
	TSharedPtr<class IAssetTypeActions> AssetTypActions_LogicFlowTypeDef;
};
