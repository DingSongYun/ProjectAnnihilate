// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewSceneModule.h"
#include "EditorUndoClient.h"
#include "TickableEditorObject.h"
#include "Runtime/Slate/Public/Framework/Commands/Commands.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "NeCharacterEditor.generated.h"

class NeCharacterAsset;

DECLARE_MULTICAST_DELEGATE(FUserRefreshPreviewActor);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDetailObjectDelegate, const struct FDetailObjectScope& /*NewObject*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDetailStructDelegate, TSharedPtr<class FStructOnScope> /*NewStruct*/);
DECLARE_DELEGATE_TwoParams(FOnDetailPropertyChangedDelegate, const FPropertyChangedEvent& /*PropertyChangedEvent*/, FProperty* /*PropertyThatChanged*/)

UENUM(BlueprintType)
enum class ECharacterEdPattern : uint8
{
	Character,
	OnlyCamera,
};

/** Mode Identifiers */
namespace CharacterEditorModes
{
	extern const FName AvatarMode;
	extern const FName DataMode;
};

/** Tab Identifiers */
namespace CharacterEditorTabs
{
	// 视口标签
	extern const FName ViewportTab;
	// 预览场景设置标签
	extern const FName PrevSceneSetting;
	// CharacterTemplate资源编辑标签
	extern const FName CharacterAssetDetail;
	// 角色列表
	extern const FName CharacterBrowser;
	// 角色外观编辑
	extern const FName AvatarInfos;

};

struct FDetailObjectScope
{
	FDetailObjectScope() {}
	FDetailObjectScope(TWeakObjectPtr<UObject> InObject, const FOnGetDetailCustomizationInstance& InDetailLayoutDelegate)
		: Object(InObject), DetailLayoutDelegate(InDetailLayoutDelegate)
	{}
	FDetailObjectScope(const FDetailObjectScope& Other) 
	{
		Object = Other.Object;
		DetailLayoutDelegate = Other.DetailLayoutDelegate;
		OnPropertyChanged = Other.OnPropertyChanged;
	}

	TWeakObjectPtr<UObject> Object;
	FOnGetDetailCustomizationInstance DetailLayoutDelegate;
	FOnDetailPropertyChangedDelegate OnPropertyChanged;
};

class FCharacterEditorCommands final : public TCommands<FCharacterEditorCommands>
{
public:
	FCharacterEditorCommands() : TCommands<FCharacterEditorCommands>
	(
		TEXT("CharacterEditor"), 
		NSLOCTEXT("Contexts", "CharacterEditor", "Character Editor"), 
		NAME_None, FAppStyle::GetAppStyleSetName()
	) {}
	
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> m_ShowLockable;
	TSharedPtr<FUICommandInfo> m_RefreshActor;
};


/*****************************************************/
// ICharacterEditor
// 编辑器接口，外部模块都应该只通过这个接口进行相关操作
/*****************************************************/
class NECHARACTERDATAEDITOR_API ICharacterEditor : public FWorkflowCentricApplication
{
public:
	/** 静态函数，外部打开编辑器都走此接口 */
	static TSharedPtr<ICharacterEditor> CreateEditor(UObject* InAsset, const TSharedPtr< IToolkitHost >& InitToolkitHost);

	/** 编辑器初始化 */
	virtual void InitializeEditor(UObject* InAsset, const FName& InitMode, const TSharedPtr< IToolkitHost >& InitToolkitHost) = 0;

	/** 获取当前正在编辑的资源 */
	virtual TWeakObjectPtr<UObject> GetEditAsset() = 0;

	/** 获取一个预览角色， 可能为nullptr */
	virtual AActor* GetPreviewActor(int32 Index = 0) const = 0;

	/** 获取所有的预览角色 */
	virtual TArray<AActor*> GetPreviewActors() const = 0;
protected:
	/** 创建预览场景 */
	virtual void CreatePreviewScene() = 0;

public:
	/** 预览配置 */
	virtual class UNeCharacterEditorPreviewSetting* GetAdditionalPreviewSettings() const = 0;

	virtual void GetPreviewSettingsCustomization
	(
		TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>& DetailsCustomizations,
		TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>& PropertyTypeCustomizations
	) const = 0;
};


/*****************************************************/
// FCharacterEditor
// 角色编辑器
/*****************************************************/
class FNeCharacterEditor : public ICharacterEditor, public FEditorUndoClient
{
public:
	FNeCharacterEditor();
	virtual ~FNeCharacterEditor() override;

	//~ Begin: ICharacterEditor interface
	virtual void InitializeEditor(UObject* InAsset, const FName& InitMode, const TSharedPtr< IToolkitHost >& InitToolkitHost) override;
	virtual TWeakObjectPtr<UObject> GetEditAsset() override;
	virtual AActor* GetPreviewActor(int32 Index = 0) const override;
	virtual TArray<AActor*> GetPreviewActors() const override;
	virtual void CreatePreviewScene() final override;
	//~ End: ICharacterEditor interface

	virtual class UNeCharacterEditorPreviewSetting* GetAdditionalPreviewSettings() const override;

	virtual void GetPreviewSettingsCustomization
	(
		TArray<FAdvancedPreviewSceneModule::FDetailCustomizationInfo>& DetailsCustomizations,
		TArray<FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo>& PropertyTypeCustomizations
	) const override;

	void RefreshPreviewActor() const;

	void SetEditingObject(UObject* Object);

	TSharedPtr<class FNeCharacterEditorPreviewScene> GetPreviewScene() { return PreviewScene; }

	USceneComponent* GetCachedSceneComponent(const FName& InCompName);

	void UpdatePreviewComponentName(const FName& OldName, const FName& NewName) const;

protected:
	void CreatePreviewActor() const;

	void ShowLockPoint();

private:
	void ExtendToolbar();

	// 绑定命令
	void BindCommands();

	void CreateEditorTabs(const TSharedRef<FNeCharacterEditor>& HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories);

public:
	FName CurrentEditorMode = NAME_None;

	FUserRefreshPreviewActor OnRefreshActor;

private:
	// 当前正在编辑的资源
	TWeakObjectPtr<UObject> CurrentEditAsset;

	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;

	TSharedPtr<class FNeCharacterEditorToolbar> m_CharacterEditorToolbar;

	/** Editor生命周期的Tabs */
	FWorkflowAllowedTabSet TabFactories;

	/** 预览场景 */
	TSharedPtr<class FNeCharacterEditorPreviewScene> PreviewScene;

	/** 预览设置 */
	class UNeCharacterEditorPreviewSetting* PreviewSettings;

	bool bShowLockPoint = false;

	//~==============================================================================================
	// Common Opts

public:
	virtual bool IsPrimaryEditor() const override { return true; }

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	virtual FName GetToolkitFName() const override;

	virtual FText GetBaseToolkitName() const override;

	virtual FText GetTabSuffix() const override;

	virtual FString GetWorldCentricTabPrefix() const override;

	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	virtual bool OnRequestClose(EAssetEditorCloseReason InCloseReason) override;

	virtual void SetCurrentMode(FName NewMode) override;

public:
	/** 点击保存时触发 */
	virtual void SaveAsset_Execute() override;

	/** 禁用掉'SaveAs' */
	virtual bool CanSaveAssetAs() const override { return false; }

	//~==============================================================================================
	// Undo & Redo

public:
	virtual void PostUndo(bool bSuccess) override;

	virtual void PostRedo(bool bSuccess) override;

public:
	FSimpleMulticastDelegate OnPostUndo;

	FSimpleMulticastDelegate OnPostRedo;

	//~==============================================================================================
	// Details

public:
	void UpdatePrevDetailObject(const FDetailObjectScope& NewObject);

	void UpdatePrevDetailStruct(const TSharedPtr<FStructOnScope>& NewStruct);

	void RefreshDetails() const;

public:
	FOnDetailObjectDelegate OnDetailObjectChanged;

	FOnDetailStructDelegate OnDetailStructDelegate;

	FSimpleMulticastDelegate OnNeedFreshDetailsDelegate;

private:
	FDetailObjectScope PrevDetailObject;

	TSharedPtr<FStructOnScope> PrevDetailStruct;
};
