#include "LogicFlowEditor.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "Framework/Commands/GenericCommands.h"
#include "EdGraphUtilities.h"
#include "ScopedTransaction.h"
#include "GraphEditorActions.h"
#include "PropertyEditorModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "EdNode_LogicFlowNode.h"
#include "EdNode_LogicFlowEdge.h"
#include "SEdNode_LogicFlowEdge.h"
#include "SEdNode_LogicFlowNode.h"
#include "LogicFlowEditorSchema.h"
#include "ClassViewerFilter.h"
#include "EdGraph_LogicFlow.h"
#include "LogicFlowEditorToolbar.h"
#include "LogicFlowEditorCommands.h"
#include "LogicFlowEdge.h"
#include "LogicFlow.h"
#include "LogicFlowNode.h"

#define LOCTEXT_NAMESPACE "LogicFlowEditor"

const FName LogicFlowEditorAppName = FName(TEXT("LogicFlowEditor"));

class FLFTypeFilter : public IClassViewerFilter
{
public:
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InClass->IsChildOf<ULogicFlowTypeDef>() && !InClass->HasAnyClassFlags(CLASS_Abstract);
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InClass->IsChildOf(ULogicFlowTypeDef::StaticClass()) && !InClass->HasAnyClassFlags(CLASS_Abstract);
	}
};

class FGraphPanelNodeFactory_LogicFlow : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UEdNode_LogicFlowNode* EdNode_GraphNode = Cast<UEdNode_LogicFlowNode>(Node))
		{
			return SNew(SEdNode_LogicFlowNode, EdNode_GraphNode);
		}
		else if (UEdNode_LogicFlowEdge* EdNode_GrpahEdge = Cast<UEdNode_LogicFlowEdge>(Node))
		{
			return SNew(SEdNode_LogicFlowEdge, EdNode_GrpahEdge);
		}

		return nullptr;
	}
};

struct FLogicFlowEditorTabs
{
	static const FName PropertyID;
	static const FName ViewportID;
	static const FName SettingsID;
};
const FName FLogicFlowEditorTabs::PropertyID(TEXT("FlowProperty"));
const FName FLogicFlowEditorTabs::ViewportID(TEXT("Viewport"));
const FName FLogicFlowEditorTabs::SettingsID(TEXT("FlowSettings"));


FLogicFlowEditor::FLogicFlowEditor()
{
	LogicFlowAsset = nullptr;

	GraphPanelNodeFactory = MakeShareable(new FGraphPanelNodeFactory_LogicFlow());
	FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory);
}

FLogicFlowEditor::~FLogicFlowEditor()
{
	if (GraphPanelNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory);
		GraphPanelNodeFactory.Reset();
	}
}

void FLogicFlowEditor::InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULogicFlow* InTreeTemp)
{
	LogicFlowAsset = InTreeTemp;
	CreateEdGraph(LogicFlowAsset.Get(), LogicFlowAsset->EdGraph);

	FGraphEditorCommands::Register();
	FLogicFlowEditorCommands::Register();

	BindCommands();
	CreateCommandList();

	if (!ToolbarBuilder.IsValid())
		ToolbarBuilder = MakeShareable(new FLogicFlowEditorToolbar(SharedThis(this)));

	const TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarBuilder->AddGraphToolbar(ToolbarExtender);

	// Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_LogicFlowEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.65f)
					->AddTab(FLogicFlowEditorTabs::ViewportID, ETabState::OpenedTab)->SetHideTabWell(true)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.7f)
						->AddTab(FLogicFlowEditorTabs::PropertyID, ETabState::OpenedTab)->SetHideTabWell(true)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.3f)
						->AddTab(FLogicFlowEditorTabs::SettingsID, ETabState::OpenedTab)
					)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	FAssetEditorToolkit::InitAssetEditor (
		Mode, InitToolkitHost, LogicFlowEditorAppName, StandaloneDefaultLayout,
		bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, LogicFlowAsset.Get(), false
	);

	RegenerateMenusAndToolbars();
}

void FLogicFlowEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_LogicFlowEditor", "LogicFlow Editor"));

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner
	(
		FLogicFlowEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FLogicFlowEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("Tab_Graph_Canvas", "Canvas"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::Get().GetStyleSetName(), "GraphEditor.EventGraph_16x")
	);

	InTabManager->RegisterTabSpawner
	(
		FLogicFlowEditorTabs::PropertyID, FOnSpawnTab::CreateSP(this, &FLogicFlowEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("Tab_Details", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.Tabs.Details")
	);

	InTabManager->RegisterTabSpawner
	(
		FLogicFlowEditorTabs::SettingsID, FOnSpawnTab::CreateSP(this, &FLogicFlowEditor::SpawnTab_GraphSettings))
		.SetDisplayName(LOCTEXT("Tab_Graph_Settings", "Settings"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::Get().GetStyleSetName(), "LevelEditor.Tabs.Details")
	);
}


void FLogicFlowEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FLogicFlowEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FLogicFlowEditorTabs::PropertyID);
	InTabManager->UnregisterTabSpawner(FLogicFlowEditorTabs::SettingsID);
}

FName FLogicFlowEditor::GetToolkitFName() const
{
	return FName("LogicFlow");
}

FText FLogicFlowEditor::GetBaseToolkitName() const
{
	return LOCTEXT("LogicFlowEditorAppLabel", "Logic Flow Editor");
}

FText FLogicFlowEditor::GetToolkitName() const
{
	return FAssetEditorToolkit::GetToolkitName();
	// const bool bDirtyState = LogicFlowAsset->GetOutermost()->IsDirty();
	//
	// FFormatNamedArguments Args;
	// Args.Add(TEXT("LogicFlowName"), FText::FromString(LogicFlowAsset->GetName()));
	// Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	// return FText::Format(LOCTEXT("LogicFlowEditorToolkitName", "{LogicFlowName}{DirtyState}"), Args);
}

FText FLogicFlowEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(LogicFlowAsset.Get());
}

FLinearColor FLogicFlowEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FLogicFlowEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("LogicFlowEditor");
}

FString FLogicFlowEditor::GetDocumentationLink() const
{
	return TEXT("");
}

void FLogicFlowEditor::SaveAsset_Execute()
{
	UEdGraph_LogicFlow* FlowEdGraph = CastChecked<UEdGraph_LogicFlow>(LogicFlowAsset->EdGraph);
	if (FlowEdGraph)
	{
		FlowEdGraph->UpdateLogicFlowAsset();
	}

	LogicFlowAsset->MarkPackageDirty();

	FAssetEditorToolkit::SaveAsset_Execute();
}

void FLogicFlowEditor::AddReferencedObjects(FReferenceCollector& Collector)
{}

UEdGraph_LogicFlow* FLogicFlowEditor::GetEditingFlowGraph() const
{
	if (LogicFlowAsset.IsValid() && LogicFlowAsset->EdGraph)
	{
		return Cast<UEdGraph_LogicFlow>(LogicFlowAsset->EdGraph);
	}

	return nullptr;
}

bool FLogicFlowEditor::IsDebugging() const
{
	if (LogicFlowAsset.IsValid() && LogicFlowAsset->EdGraph)
	{
		if (const UEdGraph_LogicFlow* FlowGraph = Cast<UEdGraph_LogicFlow>(LogicFlowAsset->EdGraph))
		{
			return FlowGraph->IsDebugging();
		}
	}
	return false;
}

TSharedRef<SDockTab> FLogicFlowEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FLogicFlowEditorTabs::ViewportID);

	CreateViewportWidget(TEXT("Logic Flow Editor"), LogicFlowAsset->EdGraph);

	SAssignNew(ViewportTab, SDockTab).Label(LOCTEXT("ViewportTab_Title", "Viewport"));

	if (ViewportWidgetStack.Num() > 0)
		ViewportTab->SetContent(ViewportWidgetStack[0].ToSharedRef());

	return ViewportTab.ToSharedRef();
}

TSharedRef<SDockTab> FLogicFlowEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FLogicFlowEditorTabs::PropertyID);

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NotifyHook = this;
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyWidget = PropertyModule.CreateDetailView(DetailsViewArgs);
	PropertyWidget->SetObject(LogicFlowAsset.Get());
	PropertyWidget->OnFinishedChangingProperties().AddSP(this, &FLogicFlowEditor::OnFinishedChangingProperties);

	return SNew(SDockTab)
		//.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("Details_Title", "Property"))
		[
			PropertyWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLogicFlowEditor::SpawnTab_GraphSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FLogicFlowEditorTabs::SettingsID);

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NotifyHook = this;
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	EditorSettingsWidget = PropertyModule.CreateDetailView(DetailsViewArgs);
	EditorSettingsWidget->SetObject(LogicFlowAsset->EdGraph);

	return SNew(SDockTab)
		//.Icon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("EditorSettings_Title", "Logic Flow Editor Setttings"))
		[
			EditorSettingsWidget.ToSharedRef()
		];
}

bool FLogicFlowEditor::CreateViewportWidget(FString ViewportName, UEdGraph* OwnerGraph)
{
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = FText::FromString(ViewportName);

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FLogicFlowEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FLogicFlowEditor::OnNodeDoubleClicked);

	TSharedPtr<SGraphEditor> NewViewPort;

	SAssignNew(NewViewPort, SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(OwnerGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);

	if (NewViewPort.IsValid())
	{
		ViewportWidgetStack.Add(NewViewPort);
		return true;
	}
	else
		return false;

}


void FLogicFlowEditor::BindCommands()
{
	const FLogicFlowEditorCommands& Commands = FLogicFlowEditorCommands::Get();
	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();

	UICommandList->MapAction(Commands.BackToPreviousGraph, FExecuteAction::CreateSP(this, &FLogicFlowEditor::BackToPreviousGraph));

}

void FLogicFlowEditor::CreateEdGraph(UObject* Outter, UEdGraph*& OutGraph)
{
	// 如果该LogicFlow还没有可视化界面，则进行创建
	if (!OutGraph)
	{
		OutGraph = CastChecked<UEdGraph_LogicFlow> (FBlueprintEditorUtils::CreateNewGraph(
			Outter, NAME_None,
			UEdGraph_LogicFlow::StaticClass(),
			ULogicFlowEditorSchema::StaticClass()
		));

		const UEdGraphSchema* Schema = OutGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*OutGraph);
	}
}

void FLogicFlowEditor::CreateCommandList()
{
	if (GraphEditorCommands.IsValid())
		return;

	GraphEditorCommands = MakeShareable(new FUICommandList);

	GraphEditorCommands->MapAction
	(
		FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateRaw(this, &FLogicFlowEditor::SelectAllNodes),
		FCanExecuteAction::CreateRaw(this, &FLogicFlowEditor::CanSelectAllNodes)
	);

	GraphEditorCommands->MapAction
	(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FLogicFlowEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FLogicFlowEditor::CanDeleteNodes)
	);

	GraphEditorCommands->MapAction
	(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FLogicFlowEditor::CopySelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FLogicFlowEditor::CanCopyNodes)
	);

	GraphEditorCommands->MapAction
	(
		FGenericCommands::Get().Cut,
		FExecuteAction::CreateRaw(this, &FLogicFlowEditor::CutSelectedNodes),
		FCanExecuteAction::CreateRaw(this, &FLogicFlowEditor::CanCutNodes)
	);

	GraphEditorCommands->MapAction
	(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FLogicFlowEditor::PasteNodes),
		FCanExecuteAction::CreateRaw(this, &FLogicFlowEditor::CanPasteNodes)
	);

	GraphEditorCommands->MapAction
	(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateRaw(this, &FLogicFlowEditor::DuplicateNodes),
		FCanExecuteAction::CreateRaw(this, &FLogicFlowEditor::CanDuplicateNodes)
	);

	GraphEditorCommands->MapAction
	(
		FGenericCommands::Get().Rename,
		FExecuteAction::CreateSP(this, &FLogicFlowEditor::OnRenameNode),
		FCanExecuteAction::CreateSP(this, &FLogicFlowEditor::CanRenameNodes)
	);
}

TSharedPtr<SGraphEditor> FLogicFlowEditor::GetCurrentGraphEditor() const
{
	return ViewportWidgetStack.Last();
}

FGraphPanelSelectionSet FLogicFlowEditor::GetSelectedNodes() const
{
	FGraphPanelSelectionSet CurrentSelection;
	TSharedPtr<SGraphEditor> FocusedGraphEd = GetCurrentGraphEditor();
	if (FocusedGraphEd.IsValid())
	{
		CurrentSelection = FocusedGraphEd->GetSelectedNodes();
	}

	return CurrentSelection;
}

void FLogicFlowEditor::SelectAllNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrentGraphEditor();
	if (CurrentGraphEditor.IsValid())
		CurrentGraphEditor->SelectAllNodes();
}

bool FLogicFlowEditor::CanSelectAllNodes()
{
	return true;
}

void FLogicFlowEditor::DeleteNodes(FGraphPanelSelectionSet& SelectedNodes)
{
	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		UEdGraphNode* EdNode = Cast<UEdGraphNode>(*NodeIt);
		if (!EdNode || !EdNode->CanUserDeleteNode())
			continue;

		// 如果是LogicFlow节点，则有可能要断开节点之间的链接关系
		if (UEdNode_LogicFlowNode* EdNode_Node = Cast<UEdNode_LogicFlowNode>(EdNode))
		{
			EdNode_Node->Modify();

			if (const UEdGraphSchema* Schema = EdNode_Node->GetSchema())
				Schema->BreakNodeLinks(*EdNode_Node);

			EdNode_Node->DestroyNode();
		}
		else
		{
			EdNode->Modify();
			EdNode->DestroyNode();
		}
	}
}

void FLogicFlowEditor::DeleteSelectedNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrentGraphEditor();
	if (!CurrentGraphEditor.IsValid())
		return;

	const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());

	CurrentGraphEditor->GetCurrentGraph()->Modify();

	FGraphPanelSelectionSet SelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	DeleteNodes(SelectedNodes);
}

bool FLogicFlowEditor::CanDeleteNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanUserDeleteNode())
			return true;
	}

	return false;
}

void FLogicFlowEditor::DeleteSelectedDuplicatableNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrentGraphEditor();
	if (!CurrentGraphEditor.IsValid())
		return;


	const FGraphPanelSelectionSet OldSelectedNodes = CurrentGraphEditor->GetSelectedNodes();
	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
			CurrentGraphEditor->SetNodeSelection(Node, true);
	}

	DeleteSelectedNodes();

	CurrentGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
			CurrentGraphEditor->SetNodeSelection(Node, true);
	}
}

void FLogicFlowEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	DeleteSelectedDuplicatableNodes();
}

bool FLogicFlowEditor::CanCutNodes()
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FLogicFlowEditor::CopySelectedNodes()
{
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (!Node)
		{
			SelectedIter.RemoveCurrent();
			continue;
		}

		// 如果要复制一条边，要检查它的连接点是否需要被复制
		if (UEdNode_LogicFlowEdge* EdNode_Edge = Cast<UEdNode_LogicFlowEdge>(*SelectedIter))
		{
			UEdNode_LogicFlowNode* StartNode = EdNode_Edge->GetStartNode();
			UEdNode_LogicFlowNode* EndNode = EdNode_Edge->GetEndNode();

			if (!SelectedNodes.Contains(StartNode) || !SelectedNodes.Contains(EndNode))
			{
				SelectedIter.RemoveCurrent();
				continue;
			}
		}

		Node->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FLogicFlowEditor::CanCopyNodes()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if (Node && Node->CanDuplicateNode())
			return true;
	}

	return false;
}

void FLogicFlowEditor::PasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrentGraphEditor();

	if (CurrentGraphEditor.IsValid())
		PasteNodesHere(CurrentGraphEditor->GetPasteLocation());
}

void FLogicFlowEditor::PasteNodesHere(const FVector2D& Location)
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrentGraphEditor();
	if (!CurrentGraphEditor.IsValid())
		return;

	UEdGraph* EdGraph = CurrentGraphEditor->GetCurrentGraph();
	if (!EdGraph)
		return;

	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
	EdGraph->Modify();

	CurrentGraphEditor->ClearSelectionSet();

	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, TextToImport, PastedNodes);

	// 创建新的数据节点对象，从旧的对象中获取数据并进行复制
	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;

		// 处理节点
		if (UEdNode_LogicFlowNode* TmpNode = Cast<UEdNode_LogicFlowNode>(Node))
		{
			TmpNode->FlowNode = DuplicateObject(TmpNode->FlowNode, LogicFlowAsset.Get());
		}
		// 处理边
		else if (UEdNode_LogicFlowEdge* TmpEdge = Cast<UEdNode_LogicFlowEdge>(Node))
		{
			TmpEdge->FlowTransition = DuplicateObject(TmpEdge->FlowTransition, LogicFlowAsset.Get());
		}
	}

	// 计算所有复制节点的平均位置
	FVector2D AvgNodePosition(0.0f, 0.0f);
	float InvNumNodes = 1.0f / float(PastedNodes.Num());
	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}
	AvgNodePosition.X *= InvNumNodes;
	AvgNodePosition.Y *= InvNumNodes;

	// 将复制出来的节点放到正确的位置上
	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		CurrentGraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

		Node->SnapToGrid(16);

		Node->CreateNewGuid();
	}

	// 标记相关文件需要保存
	CurrentGraphEditor->NotifyGraphChanged();
	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}
}

bool FLogicFlowEditor::CanPasteNodes()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrentGraphEditor();
	if (!CurrentGraphEditor.IsValid())
		return false;

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(CurrentGraphEditor->GetCurrentGraph(), ClipboardContent);
}

void FLogicFlowEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FLogicFlowEditor::CanDuplicateNodes()
{
	return CanCopyNodes();
}

void FLogicFlowEditor::OpenSelectNode()
{
	FGraphPanelSelectionSet CurSelectInfo = GetSelectedNodes();
	if (UEdNode_LogicFlowNode* EdGraphNode = Cast<UEdNode_LogicFlowNode>(CurSelectInfo.Array()[0]))
	{
		if (ULogicFlowNode_SubFlow* GraphNode = Cast<ULogicFlowNode_SubFlow>(EdGraphNode->FlowNode))
		{
			ULogicFlow* OuterAsset = GraphNode->GetTypedOuter<ULogicFlow>();
			if (GraphNode->SubFlow != nullptr)
			{
				GraphNode->SubFlow = NewObject<ULogicFlow>(GraphNode, OuterAsset->GetClass());
				GraphNode->SubFlow->SetTypeDefinition(OuterAsset->GetTypeDefinition()->GetClass());
			}

			ULogicFlow* SubTree = GraphNode->SubFlow;
			if (SubTree)
			{
				CreateEdGraph(GraphNode, SubTree->EdGraph);
				CreateViewportWidget(TEXT("Graph Node Editor"), SubTree->EdGraph);
				if ( ViewportWidgetStack.Num() > 0)
				{
					ViewportTab->SetContent(ViewportWidgetStack.Last().ToSharedRef());
				}
			}
		}
	}
}

bool FLogicFlowEditor::CanOpenSelectNode()
{
	FGraphPanelSelectionSet CurSelectInfo = GetSelectedNodes();
	if (CurSelectInfo.Num() == 1)
	{
		if (UEdNode_LogicFlowNode* GraphNode = Cast<UEdNode_LogicFlowNode>(CurSelectInfo.Array()[0]))
		{
			if (GraphNode->FlowNode->GetClass()->IsChildOf<ULogicFlowNode_SubFlow>())
				return true;
		}
	}

	return false;
}

void FLogicFlowEditor::BackToPreviousGraph()
{
	if (ViewportWidgetStack.Num() > 1)
	{
		ViewportWidgetStack.RemoveAt(ViewportWidgetStack.Num() - 1);
		ViewportTab->SetContent(ViewportWidgetStack.Last().ToSharedRef());
	}
	else if (ViewportWidgetStack.Num() > 0)
		ViewportTab->SetContent(ViewportWidgetStack.Last().ToSharedRef());
}

void FLogicFlowEditor::OnRenameNode()
{
	TSharedPtr<SGraphEditor> CurrentGraphEditor = GetCurrentGraphEditor();
	if (CurrentGraphEditor.IsValid())
	{
		const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UEdGraphNode* SelectedNode = Cast<UEdGraphNode>(*NodeIt);
			if (SelectedNode != nullptr && SelectedNode->bCanRenameNode)
			{
				CurrentGraphEditor->IsNodeTitleVisible(SelectedNode, true);
				break;
			}
		}
	}
}

bool FLogicFlowEditor::CanRenameNodes() const
{
	return true;
}

void FLogicFlowEditor::OnSelectedNodesChanged(const TSet<UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	for (UObject* SelectionEntry : NewSelection)
	{
		if (UEdNode_LogicFlowNode* EdNode = Cast<UEdNode_LogicFlowNode>(SelectionEntry))
		{
			// Selection.Add(EdNode);
			Selection.Add(EdNode->FlowNode);
		}
		else if (UEdNode_LogicFlowEdge* EdEdge = Cast<UEdNode_LogicFlowEdge>(SelectionEntry))
		{
			// Selection.Add(EdEdge);
			Selection.Add(EdEdge->FlowTransition);
		}
	}

	if (Selection.Num() == 0)
	{
		Selection.Add(LogicFlowAsset.Get());
	}

	PropertyWidget->SetObjects(Selection);
}

void FLogicFlowEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	if (CanOpenSelectNode())
		OpenSelectNode();
}

void FLogicFlowEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	return;
}

void FLogicFlowEditor::CheckGraphNodeType(UEdGraph* CurSaveGraph)
{
	if (!CurSaveGraph)
		return;

	const TArray<TSubclassOf<ULogicFlowNode>>& NodeTypes = LogicFlowAsset->GetAllowedNodeTypes();
	const UClass* EdgeType = LogicFlowAsset->GetAllowedEdgeType();

	// 删除类型不对的节点
	FGraphPanelSelectionSet WillDeleteNodes;
	for (int32 i = 0; i < CurSaveGraph->Nodes.Num(); ++i)
	{
		if (UEdNode_LogicFlowNode* CurEdNode = Cast<UEdNode_LogicFlowNode>(CurSaveGraph->Nodes[i]))
		{
			// 检查节点类型
			if (!NodeTypes.Contains(CurEdNode->FlowNode->GetClass()))
			{
				WillDeleteNodes.Add(CurEdNode);
				continue;
			}

			// 如果是子图表，递归检查子图的节点类型
			if (ULogicFlowNode_SubFlow* CurNode = Cast<ULogicFlowNode_SubFlow>(CurEdNode->FlowNode))
				CheckGraphNodeType(CurNode->SubFlow->EdGraph);
		}

		// 检查边类型
		if (UEdNode_LogicFlowEdge* CurEdEdge = Cast<UEdNode_LogicFlowEdge>(CurSaveGraph->Nodes[i]))
		{
			if (EdgeType != CurEdEdge->FlowTransition->GetClass()) WillDeleteNodes.Add(CurEdEdge);
		}
	}
	DeleteNodes(WillDeleteNodes);
}

void FLogicFlowEditor::RegisterToolbarTab(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

FString FLogicFlowEditor::GetReferencerName() const
{
	return "LogicFlowEditor";
}

#undef LOCTEXT_NAMESPACE
