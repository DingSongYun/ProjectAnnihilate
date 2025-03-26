#include "LogicFlowEditorSchema.h"
#include "CoreMinimal.h"
#include "ScopedTransaction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdNode_LogicFlowNode.h"
#include "EdNode_LogicFlowEdge.h"
#include "EdGraph_LogicFlow.h"
#include "ConnectionDrawingPolicy_LogicFlow.h"
#include "..\..\LogicFlow\Public\LogicFlowEdge.h"
#include "LogicFlow.h"
#include "LogicFlowNode.h"

#define LOCTEXT_NAMESPACE "AssetSchema_GenericGraph"


// 循环连接检查器
struct FNodeCycleChecker
{
public:
	bool CheckForLoop(UEdGraphNode* StartNode, UEdGraphNode* EndNode)
	{
		CacheStartNode = StartNode;

		return TraverseInputNodesToRoot(EndNode);
	}

private:
	bool TraverseInputNodesToRoot(UEdGraphNode* Node)
	{
		for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
		{
			UEdGraphPin* MyPin = Node->Pins[PinIndex];

			if (MyPin->Direction == EGPD_Output)
			{
				for (int32 LinkedPinIndex = 0; LinkedPinIndex < MyPin->LinkedTo.Num(); ++LinkedPinIndex)
				{
					if (UEdGraphPin* OtherPin = MyPin->LinkedTo[LinkedPinIndex])
					{
						if (CacheStartNode == OtherPin->GetOwningNode())
							return false;
						else if (!TraverseInputNodesToRoot(OtherPin->GetOwningNode()))
							return false;
					}
				}
			}
		}

		return true;
	}

	UEdGraphNode* CacheStartNode = nullptr;
};

UEdGraphNode* FLogicFlowEditorSchemaAction_NewNode::PerformAction ( class UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate)
	{
		const FScopedTransaction Transaction(LOCTEXT("LogicFlowEditorNewNode", "LogicFlow Editor: New Node"));
		ParentGraph->Modify();
		if (FromPin)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		NodeTemplate->FlowNode->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;

		if (UEdGraph_LogicFlow* FlowGraph = Cast<UEdGraph_LogicFlow>(ParentGraph))
		{
			FlowGraph->UpdateLogicFlowAsset();
		}
	}

	return ResultNode;
}

void FLogicFlowEditorSchemaAction_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}


UEdGraphNode* FLogicFlowEditorSchemaAction_NewEdge::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode /*= true*/)
{
	UEdGraphNode* ResultNode = nullptr;

	if (NodeTemplate != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("FlowEditorNewTransition", "Logic Flow Editor: New Transition"));
		ParentGraph->Modify();
		if (FromPin != nullptr)
			FromPin->Modify();

		NodeTemplate->Rename(nullptr, ParentGraph);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		NodeTemplate->NodePosX = Location.X;
		NodeTemplate->NodePosY = Location.Y;

		NodeTemplate->FlowTransition->SetFlags(RF_Transactional);
		NodeTemplate->SetFlags(RF_Transactional);

		ResultNode = NodeTemplate;

		if (UEdGraph_LogicFlow* FlowGraph = Cast<UEdGraph_LogicFlow>(ParentGraph))
		{
			FlowGraph->UpdateLogicFlowAsset();
		}
	}

	return ResultNode;
}

void FLogicFlowEditorSchemaAction_NewEdge::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(NodeTemplate);
}

//=============================================================================
/** 
 * ULogicFlowEditorSchema
 */
void ULogicFlowEditorSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	static UClass* RootNodeClass = ULogicFlowRootNode::StaticClass();
	UEdGraph_LogicFlow* FlowGraph = Cast<UEdGraph_LogicFlow>(&Graph);

	FGraphNodeCreator<UEdNode_LogicFlowRootNode> NodeCreator(Graph);
	UEdNode_LogicFlowRootNode* RootNode = NodeCreator.CreateNode();
	RootNode->FlowNode = NewObject<ULogicFlowNode>(FlowGraph->GetFlowAsset(), RootNodeClass);
	NodeCreator.Finalize();
	SetNodeMetaData(RootNode, FNodeMetadata::DefaultGraphNode);

	FlowGraph->RootNode = RootNode;

	FlowGraph->NotifyGraphChanged();
}

void ULogicFlowEditorSchema::GetBreakLinkToSubMenuActions(class FMenuBuilder& MenuBuilder, class UEdGraphPin* InGraphPin)
{
	TMap< FString, uint32 > LinkTitleCount;

	for (TArray<class UEdGraphPin*>::TConstIterator Links(InGraphPin->LinkedTo); Links; ++Links)
	{
		UEdGraphPin* Pin = *Links;
		FString TitleString = Pin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView).ToString();
		FText Title = FText::FromString(TitleString);
		if (Pin->PinName != TEXT(""))
		{
			TitleString = FString::Printf(TEXT("%s (%s)"), *TitleString, *Pin->PinName.ToString());

			FFormatNamedArguments Args;
			Args.Add(TEXT("NodeTitle"), Title);
			Args.Add(TEXT("PinName"), Pin->GetDisplayName());
			Title = FText::Format(LOCTEXT("BreakDescPin", "{NodeTitle} ({PinName})"), Args);
		}

		uint32 &Count = LinkTitleCount.FindOrAdd(TitleString);

		FText Description;
		FFormatNamedArguments Args;
		Args.Add(TEXT("NodeTitle"), Title);
		Args.Add(TEXT("NumberOfNodes"), Count);

		if (Count == 0)
		{
			Description = FText::Format(LOCTEXT("BreakDesc", "Break link to {NodeTitle}"), Args);
		}
		else
		{
			Description = FText::Format(LOCTEXT("BreakDescMulti", "Break link to {NodeTitle} ({NumberOfNodes})"), Args);
		}
		++Count;

		MenuBuilder.AddMenuEntry
		(
			Description, Description, FSlateIcon(), 
			FUIAction
			(
				FExecuteAction::CreateUObject(this, &ULogicFlowEditorSchema::BreakSinglePinLink, const_cast<UEdGraphPin*>(InGraphPin), *Links)
			)
		);
	}
}

EGraphType ULogicFlowEditorSchema::GetGraphType(const UEdGraph* TestEdGraph) const
{
	return GT_StateMachine;
}

void ULogicFlowEditorSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	// 获取节点类型信息
	TArray<TSubclassOf<ULogicFlowNode>> NodeTypes;
	if (const ULogicFlowNode_SubFlow* GraphNode = Cast<ULogicFlowNode_SubFlow>(ContextMenuBuilder.CurrentGraph->GetOuter()))
	{
		if (GraphNode->SubFlow)
		{
				NodeTypes.Append(GraphNode->SubFlow->GetAllowedNodeTypes());
		}
	}
	else if (const ULogicFlow* LogicFlow = Cast<ULogicFlow>(ContextMenuBuilder.CurrentGraph->GetOuter()))
	{
		NodeTypes.Append(LogicFlow->GetAllowedNodeTypes());
	}


	if (NodeTypes.Num() > 0)
	{
		const bool bNoParent = (ContextMenuBuilder.FromPin == nullptr);

		const FText AddToolTip = LOCTEXT("NewFlowNodeTooltip", "Add node here");

		for (int32 i = 0; i < NodeTypes.Num(); ++i)
		{
			if (!NodeTypes[i]->HasAnyClassFlags(CLASS_Abstract))
			{
				FString Title = NodeTypes[i]->GetMetaData("DisplayName");
				if (Title.IsEmpty())
				{
					Title = NodeTypes[i]->GetName();
				}
				
				Title.RemoveFromEnd("_C");
				FText Desc = FText::FromString(Title);

				TSharedPtr<FLogicFlowEditorSchemaAction_NewNode> NewNodeAction(new FLogicFlowEditorSchemaAction_NewNode(LOCTEXT("FlowNodeAction", "Flow Node"), Desc, AddToolTip, 0));

				NewNodeAction->NodeTemplate = NewObject<UEdNode_LogicFlowNode>(ContextMenuBuilder.OwnerOfTemporaries);
				NewNodeAction->NodeTemplate->FlowNode = NewObject<ULogicFlowNode>(ContextMenuBuilder.CurrentGraph->GetOuter(), NodeTypes[i]);
				ContextMenuBuilder.AddAction(NewNodeAction);
			}
		}
	}

}

void ULogicFlowEditorSchema::GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	Super::GetContextMenuActions(Menu, Context);
}

const FPinConnectionResponse ULogicFlowEditorSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	UEdNode_LogicFlowNode* Node = Cast<UEdNode_LogicFlowNode>(A->GetOwningNode());
	UEdNode_LogicFlowNode* EndNode = Cast<UEdNode_LogicFlowNode>(B->GetOwningNode());

	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorSameNode", "Both are on the same node"));
	}

	if ((A->Direction == EGPD_Input) && (B->Direction == EGPD_Input))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorInput", "Can't connect input node to input node"));
	}
	else if ((A->Direction == EGPD_Output) && (B->Direction == EGPD_Output))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorOutput", "Can't connect output node to output node"));
	}

	// 先判断是否允许有环
	{
		// 检查是否出现环路
		FNodeCycleChecker CycleChecker;
		if (!CycleChecker.CheckForLoop(A->GetOwningNode(), B->GetOwningNode()))
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinErrorCycle", "Can't create a graph cycle"));
	}

	UEdNode_LogicFlowNode* EdNode_A = Cast<UEdNode_LogicFlowNode>(A->GetOwningNode());
	UEdNode_LogicFlowNode* EdNode_B = Cast<UEdNode_LogicFlowNode>(B->GetOwningNode());

	if (!EdNode_A || !EdNode_B)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("PinError", "Not a valid UGenericGraphEdNode"));


	FText ErrorMessage;
	if (A->Direction == EGPD_Input)
	{
		if (!EdNode_A->FlowNode->CanCreateConnection(EdNode_B->FlowNode, ErrorMessage))
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}
	else
	{
		if (!EdNode_B->FlowNode->CanCreateConnection(EdNode_A->FlowNode, ErrorMessage))
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, ErrorMessage);
	}

	if (true)
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, LOCTEXT("PinConnect", "Connect nodes with edge"));
	else
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("PinConnect", "Connect nodes"));
}

bool ULogicFlowEditorSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const
{
	UEdNode_LogicFlowNode* NodeA = Cast<UEdNode_LogicFlowNode>(A->GetOwningNode());
	UEdNode_LogicFlowNode* NodeB = Cast<UEdNode_LogicFlowNode>(B->GetOwningNode());

	if (!NodeA || !NodeB)
	{
		return false;
	}

	FVector2D InitPos((NodeA->NodePosX + NodeB->NodePosX) / 2, (NodeA->NodePosY + NodeB->NodePosY) / 2);

	// 获取边类型信息
	TSubclassOf<ULogicFlowTransition> EdgeType;
	if (ULogicFlowNode_SubFlow* GraphNode = Cast<ULogicFlowNode_SubFlow>(NodeA->GetGraph()->GetOuter()))
	{
		EdgeType = GraphNode->SubFlow->GetAllowedEdgeType();
	}
	else if (const ULogicFlow* LogicFlow = Cast<ULogicFlow>(NodeA->GetGraph()->GetOuter()))
	{
		EdgeType = LogicFlow->GetAllowedEdgeType();
	}

	if (EdgeType)
	{
		FLogicFlowEditorSchemaAction_NewEdge Action;
		Action.NodeTemplate = NewObject<UEdNode_LogicFlowEdge>(NodeA->GetGraph());
		Action.NodeTemplate->SetEdge(NewObject<ULogicFlowTransition>(NodeA->GetGraph()->GetOuter(), EdgeType));
		UEdNode_LogicFlowEdge* EdgeNode = Cast<UEdNode_LogicFlowEdge>(Action.PerformAction(NodeA->GetGraph(), nullptr, InitPos, false));

		UEdGraphPin* InputPin = nullptr;
		UEdGraphPin* OutputPin = nullptr;
		if (A->Direction == EGPD_Output)
		{
			InputPin = B;
			OutputPin = A;
		}
		else
		{
			InputPin = A;
			OutputPin = B;
		}

		OutputPin->Modify();
		OutputPin->BreakAllPinLinks();
		EdgeNode->Pins[0]->Modify();
		EdgeNode->Pins[0]->LinkedTo.Empty();
		EdgeNode->Pins[0]->MakeLinkTo(OutputPin);

		InputPin->Modify();
		EdgeNode->Pins[1]->Modify();
		EdgeNode->Pins[1]->LinkedTo.Empty();
		EdgeNode->Pins[1]->MakeLinkTo(InputPin);
	}

	return true;
}

class FConnectionDrawingPolicy* ULogicFlowEditorSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const
{
	return new FConnectionDrawingPolicy_LogicFlow(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, Cast<UEdGraph_LogicFlow>(InGraphObj));
}

FLinearColor ULogicFlowEditorSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FColor::White;
}

void ULogicFlowEditorSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	UEdNode_LogicFlowNode* Node = Cast<UEdNode_LogicFlowNode>(&TargetNode);

	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakNodeLinks", "Break Node Links"));

	Super::BreakNodeLinks(TargetNode);
}

void ULogicFlowEditorSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	UEdNode_LogicFlowNode* Node = Cast<UEdNode_LogicFlowNode>(TargetPin.GetOwningNode());

	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakPinLinks", "Break Pin Links"));

	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
}

void ULogicFlowEditorSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	UEdNode_LogicFlowNode* Node = Cast<UEdNode_LogicFlowNode>(SourcePin->GetOwningNode());

	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}

UEdGraphPin* ULogicFlowEditorSchema::DropPinOnNode(UEdGraphNode* InTargetNode, const FName& InSourcePinName, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection) const
{
	UEdNode_LogicFlowNode* EdNode = Cast<UEdNode_LogicFlowNode>(InTargetNode);
	switch (InSourcePinDirection)
	{
	case EGPD_Input:
		return EdNode->GetOutputPin();
	case EGPD_Output:
		return EdNode->GetInputPin();
	default:
		return nullptr;
	}
}

bool ULogicFlowEditorSchema::SupportsDropPinOnNode(UEdGraphNode* InTargetNode, const FEdGraphPinType& InSourcePinType, EEdGraphPinDirection InSourcePinDirection, FText& OutErrorMessage) const
{
	return Cast<UEdNode_LogicFlowNode>(InTargetNode) != nullptr;
}

#if WITH_EDITORONLY_DATA
FText ULogicFlowEditorSchema::GetPinDisplayName(const UEdGraphPin* Pin) const
{
	UEdNode_LogicFlowNode* EdFlowNode = Cast<UEdNode_LogicFlowNode>(Pin->GetOwningNode());
	if (EdFlowNode && EdFlowNode->FlowNode && EdFlowNode->FlowNode->bCustomizePinName)
	{
		const int32 PinIndex = EdFlowNode->GetPinIndex(const_cast<UEdGraphPin*>(Pin));
		if (PinIndex > INDEX_NONE)
		{
			FText DisplayName = EdFlowNode->FlowNode->GetPinDisplayName(PinIndex);
			if (!DisplayName.IsEmpty())
			{
				return DisplayName;
			}
		}
	}
	return Super::GetPinDisplayName(Pin);
}
#endif


#undef LOCTEXT_NAMESPACE
