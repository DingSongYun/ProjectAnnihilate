// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "Widgets/SCompoundWidget.h"
#include "IContentBrowserSingleton.h"
#include "Editor.h"

class SNeCharacterBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNeCharacterBrowser)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakObjectPtr<UObject> InTemplateAsset)
	{
		if (!InTemplateAsset.IsValid())
			return;
		UClass* TemplateClass = InTemplateAsset->GetClass();
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		FAssetPickerConfig Config;
		FARFilter Filter;
		Filter.bRecursiveClasses = true;
		Filter.ClassPaths.Add(TemplateClass->GetClassPathName());
		Config.Filter = Filter;
		Config.InitialAssetViewType = EAssetViewType::Column;
		Config.bAddFilterUI = false;
		Config.bShowPathInColumnView = false;
		Config.bSortByPathInColumnView = false;
		Config.bShowTypeInColumnView = false;
		Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SNeCharacterBrowser::OnRequestOpenAsset);
		Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SNeCharacterBrowser::OnGetAssetContextMenu);
		Config.bFocusSearchBoxWhenOpened = false;

		this->ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(Config)
			]
		];
	}

	void OnRequestOpenAsset(const FAssetData& AssetData)
	{
		if (UObject* ObjectToEdit = AssetData.GetAsset())
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ObjectToEdit);
		}
	}

	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
	{
		return SNullWidget::NullWidget;
	}
};
