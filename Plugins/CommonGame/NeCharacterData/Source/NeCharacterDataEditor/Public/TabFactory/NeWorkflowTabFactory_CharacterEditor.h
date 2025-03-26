// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once
#include "NeCharacterEditor.h"
#include "NeCharacterEditorPreviewScene.h"
#include "SNeCharacterEdViewport.h"
#include "SNeCharacterTemplateDetail.h"
#include "SNeCharacterBrowser.h"
#include "SNeCharacterAvatar.h"
#include "AdvancedPreviewSceneModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "NeCharacterEditorPreviewSettings.h"
#include "NeEditorTypes.h"

DECLARE_TAB_SUMMONER_BEGIN(FPreviewViewportSummoner, CharacterEditorTabs::ViewportTab)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FNeCharacterEditor> CharacterEditorPtr = StaticCastSharedPtr<FNeCharacterEditor>(HostingApp.Pin());
	return SNew(SCharacterEdViewportTab, CharacterEditorPtr->GetPreviewScene().ToSharedRef(), CharacterEditorPtr.ToSharedRef(), 0);
}
DECLARE_TAB_SUMMONER_END

DECLARE_TAB_SUMMONER_BEGIN(FPreviewSceneSettingSummoner, CharacterEditorTabs::PrevSceneSetting)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FNeCharacterEditor> CharacterEditorPtr = StaticCastSharedPtr<FNeCharacterEditor>(HostingApp.Pin());
	TSharedRef<FNeCharacterEditorPreviewScene> PrevSceneRef = CharacterEditorPtr->GetPreviewScene().ToSharedRef();

	TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo> DetailsCustomizations;
	TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo> PropertyTypeCustomizations;
	CharacterEditorPtr->GetPreviewSettingsCustomization(DetailsCustomizations, PropertyTypeCustomizations);
	UObject* AdditionalSetting = CharacterEditorPtr->GetAdditionalPreviewSettings();
	FAdvancedPreviewSceneModule& AdvancedPreviewSceneModule = FModuleManager::LoadModuleChecked<FAdvancedPreviewSceneModule>("AdvancedPreviewScene");
	return AdvancedPreviewSceneModule.CreateAdvancedPreviewSceneSettingsWidget
	(
		PrevSceneRef, AdditionalSetting,
		DetailsCustomizations, PropertyTypeCustomizations
	);
}
DECLARE_TAB_SUMMONER_END

DECLARE_TAB_SUMMONER_BEGIN(FCharacterBrowserSummoner, CharacterEditorTabs::CharacterBrowser)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FNeCharacterEditor> CharacterEditorPtr = StaticCastSharedPtr<FNeCharacterEditor>(HostingApp.Pin());
	return SNew(SNeCharacterBrowser, CharacterEditorPtr->GetEditAsset());
}
DECLARE_TAB_SUMMONER_END

DECLARE_TAB_SUMMONER_BEGIN(FCharacterTemplateDetailSummoner, CharacterEditorTabs::CharacterAssetDetail)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FNeCharacterEditor> CharacterEditorPtr = StaticCastSharedPtr<FNeCharacterEditor>(HostingApp.Pin());
	return SNew(SNeCharacterTemplateDetail, CharacterEditorPtr->GetEditAsset(), CharacterEditorPtr);
}
DECLARE_TAB_SUMMONER_END

DECLARE_TAB_SUMMONER_BEGIN(FCharacterAvatarSummoner, CharacterEditorTabs::AvatarInfos)
virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FNeCharacterEditor> CharacterEditorPtr = StaticCastSharedPtr<FNeCharacterEditor>(HostingApp.Pin());
	return SNew(SNeCharacterAvatar, CharacterEditorPtr->GetEditAsset(), CharacterEditorPtr);
}
DECLARE_TAB_SUMMONER_END
