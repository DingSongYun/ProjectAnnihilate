// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeCharacterEditor.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "NeCharacterAsset.h"
#include "NeCharacterEditorPreviewScene.h"
#include "NeCharacterEditorPreviewSettings.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Blueprint.h"
#include "GameFramework/WorldSettings.h"
#include "Modes/NeCharacterEdMode.h"
#include "Modules/ModuleManager.h"
#include "TabFactory/NeWorkflowTabFactory_CharacterEditor.h"
#include "Toolkits/IToolkitHost.h"
#include "Widgets/SNeCharacterEditorToolbar.h"


const FName CHARACTER_EDITOR_NAME = FName(TEXT("CharacterEditor"));
const FName CHARACTER_EDITOR_APP_IDENTIFIER = FName(TEXT("CharacterEditorApp"));
const FName CharacterEditorModes::AvatarMode = FName(TEXT("AvatarMode"));
const FName CharacterEditorModes::DataMode = FName(TEXT("TemplateMode"));
const FName CharacterEditorTabs::ViewportTab = FName(TEXT("WGViewport"));
const FName CharacterEditorTabs::PrevSceneSetting = FName(TEXT("Preview Setting"));
const FName CharacterEditorTabs::CharacterBrowser = FName(TEXT("WGAssetBrowser"));
const FName CharacterEditorTabs::CharacterAssetDetail = FName(TEXT("WGAssetDetail"));
const FName CharacterEditorTabs::AvatarInfos = FName(TEXT("AvatarInfos"));

DEFINE_LOG_CATEGORY_STATIC(LogCharacterEditor, Log, All);

#define LOCTEXT_NAMESPACE "CharacterEditor"


void FCharacterEditorCommands::RegisterCommands()
{
	UI_COMMAND(m_ShowLockable, "Show Lock Point", "Draw Debug Point At Lockable Component Location.", EUserInterfaceActionType::Button, FInputChord(EKeys::L));
	UI_COMMAND(m_RefreshActor, "Refresh Actor", "Refresh Actor By Current Avatar Message.", EUserInterfaceActionType::Button, FInputChord(EKeys::R));
}

TSharedPtr<ICharacterEditor> ICharacterEditor::CreateEditor(UObject* InAsset, const TSharedPtr< IToolkitHost >& InitToolkitHost)
{
	TSharedPtr<FNeCharacterEditor> EditorPtr(new FNeCharacterEditor());
	EditorPtr->InitializeEditor(InAsset, CharacterEditorModes::DataMode, InitToolkitHost);
	return EditorPtr;
}

FNeCharacterEditor::FNeCharacterEditor() : CurrentEditAsset(nullptr)
{
	if (UEditorEngine* Editor = Cast<UEditorEngine>(GEngine))
		Editor->RegisterForUndo(this);

	PreviewSettings = NewObject<UNeCharacterEditorPreviewSetting>();
	PreviewSettings->AddToRoot();

}

FNeCharacterEditor::~FNeCharacterEditor()
{
	if (UEditorEngine* Editor = Cast<UEditorEngine>(GEngine))
		Editor->UnregisterForUndo(this);

	PreviewSettings->RemoveFromRoot();
	PreviewSettings = nullptr;

}

//~ Begin: ICharacterEditor interface
void FNeCharacterEditor::InitializeEditor(UObject* InAsset, const FName& InitMode, const TSharedPtr< IToolkitHost >& InitToolkitHost)
{
	CurrentEditAsset = InAsset;
	check(CurrentEditAsset.IsValid());

	CurrentEditorMode = InitMode;

	// 创建工具栏
	if (!m_CharacterEditorToolbar.IsValid())
		m_CharacterEditorToolbar = MakeShareable(new FNeCharacterEditorToolbar());

	// 初始化编辑器
	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor
	(
		EToolkitMode::Standalone, InitToolkitHost, CHARACTER_EDITOR_APP_IDENTIFIER,
		FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea()),
		bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InAsset
	);

	// 创建场景
	CreatePreviewScene();

	// 注册Modes
	if (InitMode.IsEqual(CharacterEditorModes::DataMode))
		AddApplicationMode(CharacterEditorModes::DataMode, MakeShareable(new FCharacterEdMode_TemplateAsset(SharedThis(this))));
	else if (InitMode.IsEqual(CharacterEditorModes::AvatarMode))
		AddApplicationMode(CharacterEditorModes::AvatarMode, MakeShareable(new FCharacterEdMode_AvatarMake(SharedThis(this))));

	// 设置Mode
	SetCurrentMode(InitMode);

	FCharacterEditorCommands::Register();
	BindCommands();

	// 工具栏
	//ExtendToolbar();

	RegenerateMenusAndToolbars();
}

TWeakObjectPtr<UObject> FNeCharacterEditor::GetEditAsset()
{ 
	return CurrentEditAsset;
}

AActor* FNeCharacterEditor::GetPreviewActor(int32 Index) const
{
	if (PreviewScene)
	{
		if (TArray<AActor*> PreActors = PreviewScene->GetPreviewActors(); PreActors.IsValidIndex(Index))
		{
			return PreActors[Index];
		}
	}

	return nullptr;
}

TArray<AActor*> FNeCharacterEditor::GetPreviewActors() const
{
	if (PreviewScene)
		return PreviewScene->GetPreviewActors();

	return TArray<AActor*>{};
}

void FNeCharacterEditor::CreateEditorTabs(const TSharedRef<FNeCharacterEditor>& HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories)
{
	OutTabFactories.RegisterFactory(FPreviewViewportSummoner::Create(HostingAppPt));
	OutTabFactories.RegisterFactory(FPreviewSceneSettingSummoner::Create(HostingAppPt));
}

//~ End: ICharacterEditor interface

void FNeCharacterEditor::ExtendToolbar()
{
	if (ToolbarExtender.IsValid())
	{
		RemoveToolbarExtender(ToolbarExtender);
		ToolbarExtender.Reset();
	}

	ToolbarExtender = MakeShareable(new FExtender);

	m_CharacterEditorToolbar->SetupToolbar(ToolbarExtender, SharedThis(this));
	m_CharacterEditorToolbar->AddHelperToolbar(ToolbarExtender);

	AddToolbarExtender(ToolbarExtender);
}

void FNeCharacterEditor::BindCommands()
{
	const FCharacterEditorCommands& Commands = FCharacterEditorCommands::Get();

	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();

	UICommandList->MapAction(Commands.m_ShowLockable, FExecuteAction::CreateSP(this, &FNeCharacterEditor::ShowLockPoint));

	UICommandList->MapAction(Commands.m_RefreshActor, FExecuteAction::CreateSP(this, &FNeCharacterEditor::RefreshPreviewActor));
}

class UNeCharacterEditorPreviewSetting* FNeCharacterEditor::GetAdditionalPreviewSettings() const
{
	return PreviewSettings;
}

void FNeCharacterEditor::GetPreviewSettingsCustomization
(
	TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>& DetailsCustomizations,
	TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>& PropertyTypeCustomizations
) const
{
#if 0
	TSharedPtr<FNeCharacterEdMode> CurrentMode = StaticCastSharedPtr<FNeCharacterEdMode>(CurrentAppModePtr);
	if (CurrentMode.IsValid())
		CurrentMode->GetPreviewSettingsCustomization(DetailsCustomizations, PropertyTypeCustomizations);
#endif

	DetailsCustomizations.Add
	(
		{ 
			UNeCharacterEditorPreviewSetting::StaticClass(), 
			FOnGetDetailCustomizationInstance::CreateStatic(&FCharacterPrevSettingsCustomization::MakeInstance, this) 
		}
	);
}

void FNeCharacterEditor::CreatePreviewActor() const
{
	if (PreviewScene)
	{
		PreviewScene->CreatePreviewActors(Cast<UNeCharacterAsset>(CurrentEditAsset.Get()));
	}
}

void FNeCharacterEditor::CreatePreviewScene()
{
	if (!PreviewScene.IsValid())
	{
		PreviewScene = MakeShareable
		(
			new FNeCharacterEditorPreviewScene
			(
				FPreviewScene::ConstructionValues()
				.AllowAudioPlayback(true)
				.ShouldSimulatePhysics(true)
				.SetEditor(true)
			)
		);
		PreviewScene->GetWorld()->GetWorldSettings()->SetIsTemporarilyHiddenInEditor(false);

		// Post init preview scene
		CreatePreviewActor();
	}
}

void FNeCharacterEditor::ShowLockPoint()
{
	bShowLockPoint = !bShowLockPoint;
}

void FNeCharacterEditor::RefreshPreviewActor() const
{
	CreatePreviewActor();

	OnRefreshActor.Broadcast();
}

void FNeCharacterEditor::SetEditingObject(UObject* Object)
{
	RemoveEditingObject(GetEditingObject());
	AddEditingObject(Object);
}

void FNeCharacterEditor::UpdatePreviewComponentName(const FName& OldName, const FName& NewName) const
{
	const AActor* PreviewChar = GetPreviewActor();
	if (!PreviewChar)
		return;

	TArray<USceneComponent*> SceneComps;
	PreviewChar->GetComponents<USceneComponent>(SceneComps);
	for (int32 i = 0; i < SceneComps.Num(); ++i)
	{
		if (SceneComps[i])
		{
			for (int32 j = 0; j < SceneComps[i]->ComponentTags.Num(); ++j)
			{
				FString TagName = SceneComps[i]->ComponentTags[j].ToString();
				if (TagName.Contains(OldName.ToString()))
				{
					SceneComps[i]->ComponentTags[j] = FName(TEXT("Name_") + NewName.ToString());
					return;
				}
			}
		}
	}
}

void FNeCharacterEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_CharacterEditor", "Character Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	TabFactories.Clear();
	CreateEditorTabs(SharedThis(this), TabFactories);

	PushTabFactories(TabFactories);
	//for (auto FactoryIt = TabFactories.CreateIterator(); FactoryIt; ++FactoryIt)
	//	FactoryIt.Value()->RegisterTabSpawner(TabManager.ToSharedRef(), nullptr);
}

void FNeCharacterEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	TabFactories.Clear();
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

FName FNeCharacterEditor::GetToolkitFName() const
{
	return CHARACTER_EDITOR_NAME;
}

FText FNeCharacterEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "CharacterEditor");
}

FText FNeCharacterEditor::GetTabSuffix() const
{
	const bool bDirtyState = IsValid(CurrentEditAsset.Get()) ? CurrentEditAsset->GetOutermost()->IsDirty() : false;
	return bDirtyState ? LOCTEXT("TabSuffixAsterix", "*") : FText::GetEmpty();
}

FString FNeCharacterEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "CharacterEditor").ToString();
}

FLinearColor FNeCharacterEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

bool FNeCharacterEditor::OnRequestClose(EAssetEditorCloseReason InCloseReason)
{
	return FWorkflowCentricApplication::OnRequestClose(InCloseReason);
}

void FNeCharacterEditor::SetCurrentMode(FName NewMode)
{
	CurrentEditorMode = NewMode;

	FWorkflowCentricApplication::SetCurrentMode(CurrentEditorMode);
}

void FNeCharacterEditor::SaveAsset_Execute()
{
	TArray<UPackage*> PackagesToSave;
	PackagesToSave.AddUnique(CurrentEditAsset->GetOutermost());

	FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, false);
}

void FNeCharacterEditor::PostUndo(bool bSuccess)
{
	OnPostUndo.Broadcast();
}

void FNeCharacterEditor::PostRedo(bool bSuccess)
{
	OnPostRedo.Broadcast();
}

void FNeCharacterEditor::UpdatePrevDetailObject(const FDetailObjectScope& NewObject)
{
	PrevDetailObject = NewObject;
	if (OnDetailObjectChanged.IsBound())
		OnDetailObjectChanged.Broadcast(PrevDetailObject);
}

void FNeCharacterEditor::UpdatePrevDetailStruct(const TSharedPtr<FStructOnScope>& NewStruct)
{
	PrevDetailStruct = NewStruct;
	if (OnDetailStructDelegate.IsBound())
		OnDetailStructDelegate.Broadcast(PrevDetailStruct);
}

void FNeCharacterEditor::RefreshDetails() const
{
	if (OnNeedFreshDetailsDelegate.IsBound())
		OnNeedFreshDetailsDelegate.Broadcast();
}

#undef LOCTEXT_NAMESPACE
