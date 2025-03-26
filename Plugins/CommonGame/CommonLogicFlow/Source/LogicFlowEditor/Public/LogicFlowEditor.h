#pragma once

#include "Misc/NotifyHook.h"
#include "GraphEditor.h"
#include "EdGraphUtilities.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/Docking/SDockTab.h"

class ULogicFlow;
class ULogicFlowTypeDef;
class ULogicFlowNode;

class FLogicFlowEditor : public FAssetEditorToolkit, public FNotifyHook, public FGCObject
{
public:
	FLogicFlowEditor();
	virtual ~FLogicFlowEditor();

	void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULogicFlow* InTreeTemp);

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void SaveAsset_Execute() override;
	// End of FAssetEditorToolkit

	TSharedPtr<class FLogicFlowEditorToolbar> GetToolbarBuilder() { return ToolbarBuilder; }
	void RegisterToolbarTab(const TSharedRef<class FTabManager>& TabManager);

	// FSerializableObject interface
	virtual FString GetReferencerName() const;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

	FORCEINLINE ULogicFlow* GetEditingFlowAsset() const { return LogicFlowAsset.Get(); }
	class UEdGraph_LogicFlow* GetEditingFlowGraph() const;

	bool IsDebugging() const;

private:
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_GraphSettings(const FSpawnTabArgs& Args);

	/** 创建新的节点编辑窗口 */
	bool CreateViewportWidget(FString ViewportName, UEdGraph* OwnerGraph);

	void BindCommands();
	void CreateEdGraph(UObject* TheOwner, UEdGraph*& TheGraphPtr);
	void CreateCommandList();
	TSharedPtr<SGraphEditor> GetCurrentGraphEditor() const;
	FGraphPanelSelectionSet GetSelectedNodes() const;

	// Delegates for graph editor commands
	void SelectAllNodes();
	bool CanSelectAllNodes();
	void DeleteNodes(FGraphPanelSelectionSet& SelectedNodes);
	void DeleteSelectedNodes();
	bool CanDeleteNodes();
	void DeleteSelectedDuplicatableNodes();
	void CutSelectedNodes();
	bool CanCutNodes();
	void CopySelectedNodes();
	bool CanCopyNodes();
	void PasteNodes();
	void PasteNodesHere(const FVector2D& Location);
	bool CanPasteNodes();
	void DuplicateNodes();
	bool CanDuplicateNodes();
	void OpenSelectNode();
	bool CanOpenSelectNode();
	// 回到上一层
	void BackToPreviousGraph();

	void OnRenameNode();
	bool CanRenameNodes() const;

	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

	void OnNodeDoubleClicked(UEdGraphNode* Node);

	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

private:
	void CheckGraphNodeType(UEdGraph* CurSaveGraph);

private:
	TWeakObjectPtr<ULogicFlow> LogicFlowAsset = nullptr;

	TSharedPtr<class FLogicFlowEditorToolbar> ToolbarBuilder;

	// 编辑窗口
	TSharedPtr<SDockTab> ViewportTab;
	TArray<TSharedPtr<SGraphEditor>> ViewportWidgetStack;
	TSharedPtr<class IDetailsView> PropertyWidget;
	TSharedPtr<class IDetailsView> EditorSettingsWidget;

	TSharedPtr<FUICommandList> GraphEditorCommands;
	TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory;
};