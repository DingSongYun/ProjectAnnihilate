// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCharacterEditor.h"
#include "Modes/NeCharacterEdMode.h"
#include "TabFactory/NeWorkflowTabFactory_CharacterEditor.h"

FNeCharacterEdMode::FNeCharacterEdMode(const FName& InModeName, TSharedRef<class FNeCharacterEditor> InHostingApp)
	: FApplicationMode(InModeName)
	, HostingAppPtr(InHostingApp)
{
	check(HostingAppPtr.IsValid());
}

void FNeCharacterEdMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FWorkflowCentricApplication> HostingApp = HostingAppPtr.Pin();
	HostingApp->RegisterTabSpawners(InTabManager.ToSharedRef());
	HostingApp->PushTabFactories(TabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

void FNeCharacterEdMode::AddTabFactory(FCreateWorkflowTabFactory FactoryCreator)
{
	if (FactoryCreator.IsBound())
	{
		TabFactories.RegisterFactory(FactoryCreator.Execute(HostingAppPtr.Pin()));
	}
}

void FNeCharacterEdMode::RemoveTabFactory(FName TabFactoryID)
{
	TabFactories.UnregisterFactory(TabFactoryID);
}


FCharacterEdMode_TemplateAsset::FCharacterEdMode_TemplateAsset(TSharedRef<FNeCharacterEditor> InHostingApp) : FNeCharacterEdMode(CharacterEditorModes::DataMode, InHostingApp)
{
	// 创建Mode所需的Tabs
	CreateModeTabs(HostingAppPtr.Pin().ToSharedRef(), TabFactories);

	// 基础布局
	TabLayout = FTabManager::NewLayout("Standalone_CharacterEd_Template_Layout_v1.2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewSplitter()
				->SetSizeCoefficient(1.f)
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.15f)
					->SetHideTabWell(false)
					->AddTab(CharacterEditorTabs::CharacterBrowser, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->SetHideTabWell(false)
					->AddTab(CharacterEditorTabs::ViewportTab, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.25f)
					->SetHideTabWell(false)
					->AddTab(CharacterEditorTabs::CharacterAssetDetail, ETabState::OpenedTab)
				)
			)
		);
}


void FCharacterEdMode_TemplateAsset::CreateModeTabs(const TSharedRef<FNeCharacterEditor> HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories)
{
	OutTabFactories.RegisterFactory(FCharacterTemplateDetailSummoner::Create(HostingAppPt));
	OutTabFactories.RegisterFactory(FCharacterBrowserSummoner::Create(HostingAppPt));
}


FCharacterEdMode_AvatarMake::FCharacterEdMode_AvatarMake(TSharedRef<FNeCharacterEditor> InHostingApp) : FNeCharacterEdMode(CharacterEditorModes::AvatarMode, InHostingApp)
{
	// 创建Mode所需的Tabs
	CreateModeTabs(HostingAppPtr.Pin().ToSharedRef(), TabFactories);

	// 基础布局
	TabLayout = FTabManager::NewLayout("Standalone_CharacterEd_AvatarMake_Layout_v1.1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->Split
			(
				FTabManager::NewSplitter()
				->SetSizeCoefficient(1.f)
				->SetOrientation(Orient_Horizontal)
				// ->Split
				// (
				// 	FTabManager::NewStack()
				// 	->SetSizeCoefficient(0.2f)
				// 	->SetHideTabWell(false)
				// 	->AddTab(CharacterEditorTabs::ProtoTypeBrowser, ETabState::OpenedTab)
				// )
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->SetHideTabWell(false)
					->AddTab(CharacterEditorTabs::ViewportTab, ETabState::OpenedTab)
				)
				// ->Split
				// (
				// 	FTabManager::NewStack()
				// 	->SetSizeCoefficient(0.3f)
				// 	->SetHideTabWell(false)
				// 	->AddTab(CharacterEditorTabs::CharacterPrototypeDetail, ETabState::OpenedTab)
				// )
				/*->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.1f)
					->SetHideTabWell(false)
					->AddTab(CharacterEditorTabs::AvatarInfos, ETabState::OpenedTab)
				)*/

			)
		);
}


void FCharacterEdMode_AvatarMake::CreateModeTabs(const TSharedRef<FNeCharacterEditor> HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories)
{
	// OutTabFactories.RegisterFactory(FCharacterPrototypeBrowser::Create(HostingAppPt));
	// OutTabFactories.RegisterFactory(FCharacterPrototypeDetailSummoner::Create(HostingAppPt));
	//OutTabFactories.RegisterFactory(FCharacterAvatarSummoner::Create(HostingAppPt));
}