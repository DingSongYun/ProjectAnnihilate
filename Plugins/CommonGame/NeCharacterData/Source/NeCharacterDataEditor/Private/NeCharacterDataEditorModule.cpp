// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeCharacterDataEditorModule.h"
#include "ISettingsModule.h"
#include "NeAvatarMaterialInfo.h"
#include "NeCharacterDataSettings.h"
#include "PropertyEditorModule.h"
#include "AssetTypeActions/NeAssetTypeActions_Character.h"
#include "CustomLayout/NeCustomLayout_AvatarMaterialInfo.h"
#include "AssetToolsModule.h"
#include "NeEditorTypes.h"
#include "Thumbnail/NeCharacterAssetThumbnailRenderer.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#define LOCTEXT_NAMESPACE "FNeCharacterDataEditorModule"

void FNeCharacterDataEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	REG_ASSET_TYPE_SECTION_BEGIN(AssetTools, AssetTypeActions);
	REG_ASSET_TYPE_ACTIONS(FNeAssetTypeActions_CharacterAsset, EAssetTypeCategories::Gameplay);
	REG_ASSET_TYPE_SECTION_END()

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	REG_CUSTOM_PROP_LAYOUT_GENERIC(FAvatarMaterialInfo, FNeCustomLayout_AvatarMaterialInfo);

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project", "Plugins",
			"Character Asset", LOCTEXT("CharacterAsset", "CharacterAsset"),
			LOCTEXT("CharacterAsset_SettingDesc", "Settings for character asset editor"),
			GetMutableDefault<UCharacterDataSettings>());
	}

	// Thumbnail
	UThumbnailManager::Get().UnregisterCustomRenderer(UNeCharacterAsset::StaticClass());
	UThumbnailManager::Get().RegisterCustomRenderer(UNeCharacterAsset::StaticClass(), UNeCharacterAssetThumbnailRenderer::StaticClass());
}

void FNeCharacterDataEditorModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Character Asset");
	}

	if (const FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();
		for (const TSharedPtr<IAssetTypeActions>& TypeAction : AssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(TypeAction.ToSharedRef());
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNeCharacterDataEditorModule, NeCharacterDataEditor)

