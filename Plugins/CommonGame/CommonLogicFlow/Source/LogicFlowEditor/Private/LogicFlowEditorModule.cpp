// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "LogicFlowEditorModule.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_LogicFlow.h"

#define LOCTEXT_NAMESPACE "LogicFlowEditorModule"

void FCommonLogicFlowEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTypActions_LogicFlow = MakeShareable(new FAssetTypeActions_LogicFlow());
	AssetTools.RegisterAssetTypeActions(AssetTypActions_LogicFlow.ToSharedRef());
	// AssetTypActions_LogicFlowTypeDef = MakeShareable(new FAssetTypeActions_LogicFlowTypeDef());
	// AssetTools.RegisterAssetTypeActions(AssetTypActions_LogicFlowTypeDef.ToSharedRef());
}

void FCommonLogicFlowEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(AssetTypActions_LogicFlow.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypActions_LogicFlowTypeDef.ToSharedRef());
	}
	AssetTypActions_LogicFlow = nullptr;
	AssetTypActions_LogicFlowTypeDef = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonLogicFlowEditorModule, CommonLogicFlowEditor)