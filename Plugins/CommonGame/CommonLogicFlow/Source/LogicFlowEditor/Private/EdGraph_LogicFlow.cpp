// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "EdGraph_LogicFlow.h"
#include "EdNode_LogicFlowNode.h"
#include "EdNode_LogicFlowEdge.h"
#include "..\..\LogicFlow\Public\LogicFlowEdge.h"
#include "GraphEditAction.h"
#include "LogicFlow.h"
#include "LogicFlowNode.h"

UEdGraph_LogicFlow::UEdGraph_LogicFlow()
{
}

void UEdGraph_LogicFlow::Clear()
{
	Nodes.Empty();
}

void UEdGraph_LogicFlow::PostEditUndo()
{
	Super::PostEditUndo();

	NotifyGraphChanged();
}

void UEdGraph_LogicFlow::NotifyGraphChanged()
{
	UpdateLogicFlowAsset();
	Super::NotifyGraphChanged();
}

void UEdGraph_LogicFlow::NotifyGraphChanged( const FEdGraphEditAction& Action )
{
	if (((int32)Action.Action & EEdGraphActionType::GRAPHACTION_AddNode) == 0) // Add Node时Node数据还不正确，暂不更新资源
	{
		UpdateLogicFlowAsset();
	}
	Super::NotifyGraphChanged(Action);
}

void UEdGraph_LogicFlow::UpdateLogicFlowAsset()
{
	UpdateLogicFlowFromGraph(GetFlowAsset(), *this);
}

bool UEdGraph_LogicFlow::ShouldShowDebug(ULogicFlowNode* FlowNode) const
{
	if (FlowNode == nullptr) return false;
	if (!IsDebugging()) return false;

	for (const FFlowEvalRecordItem& Item : ObjectBeingDebugged->EvalRecord)
	{
		if (Item.EvalNode == FlowNode)
		{
			return true;
		}
	}

	return false;
}

bool UEdGraph_LogicFlow::ShouldShowDebug(ULogicFlowTransition* FlowTransition) const
{
	if (FlowTransition == nullptr) return false;

	if (!IsDebugging()) return false;

	for (const FFlowEvalRecordItem& Item : ObjectBeingDebugged->EvalRecord)
	{
		if (Item.EvalTransition == FlowTransition)
		{
			return true;
		}
	}

	return false;
}

void UEdGraph_LogicFlow::UpdateLogicFlowFromGraph(class ULogicFlow* InLogicFlow, UEdGraph_LogicFlow& Graph)
{
	TArray<ULogicFlowNode*> InvalidNodes = InLogicFlow->FlowNodes;
	TArray<ULogicFlowTransition*> InvalidEdges = InLogicFlow->FlowEdges;
	
	InLogicFlow->RootNodes.Empty();
	InLogicFlow->FlowEdges.Empty();
	InLogicFlow->FlowNodes.Empty();

	// 1. Add Root node
	if (IsValid(Graph.RootNode))
	{
		ULogicFlowNode* RootNode = Graph.RootNode->FlowNode;
		check(RootNode);
		InLogicFlow->RootNodes.Add(RootNode);
		InLogicFlow->FlowNodes.Add(RootNode);
		InvalidNodes.Remove(RootNode);
	}

	TArray<UEdGraphNode*> SortNodes;
	for (auto Node : Graph.Nodes)
	{
		SortNodes.Add(Node.Get());
	}

	SortNodes.StableSort([](UEdGraphNode& A, UEdGraphNode& B) { return  A.NodePosX < B.NodePosX; });

	// 2. Build nodes
	for (int32 i = 0; i < SortNodes.Num(); ++i)
	{
		// Skip Root Node
		if (SortNodes[i] == Graph.RootNode)
		{
			if (ULogicFlowNode* RootNode = Graph.RootNode->FlowNode)
			{
				RootNode->Transitions.Empty();
			}
		}
		else
		if (UEdNode_LogicFlowNode* EdNode = Cast<UEdNode_LogicFlowNode>(SortNodes[i]))
		{
			if (ensureMsgf(IsValid(EdNode->FlowNode), TEXT("Invalid Flow Node.")))
			{
				ULogicFlowNode* FlowNode = InLogicFlow->FlowNodes.Add_GetRef(EdNode->FlowNode);
				FlowNode->Transitions.Empty();
				InvalidNodes.Remove(FlowNode);
			}
		}
	}
	
	// 3. Build transitions
	for (int32 i = 0; i < SortNodes.Num(); ++i)
	{
		if (UEdNode_LogicFlowEdge* EdNodeTransition = Cast<UEdNode_LogicFlowEdge>(SortNodes[i]))
		{
			check(IsValid(EdNodeTransition->FlowTransition));
			ULogicFlowTransition* FlowTransitionNode = InLogicFlow->FlowEdges.Add_GetRef(EdNodeTransition->FlowTransition);
			int32 Index = InLogicFlow->FlowEdges.Num() - 1;
			InvalidEdges.Remove(FlowTransitionNode);

			if (UEdGraphPin* EdFromPin = EdNodeTransition->GetStartPin())
			{
				const UEdNode_LogicFlowNode* FlowEdNode = Cast<UEdNode_LogicFlowNode>(EdFromPin->GetOwningNode());
				if (const int32 PinIndex = FlowEdNode->GetPinIndex(EdFromPin); PinIndex > INDEX_NONE)
				{
					ULogicFlowNode* FromNode = FlowEdNode->FlowNode;
					FromNode->Transitions.Add(Index);
					FlowTransitionNode->FromNode = FromNode;
					FlowTransitionNode->FromNodePin = PinIndex;
				}
			}

			const UEdNode_LogicFlowNode* EdEndNode = EdNodeTransition->GetEndNode();
			if (ULogicFlowNode* ToNode = EdEndNode ? EdEndNode->FlowNode : nullptr)
			{
				FlowTransitionNode->ToNode = ToNode;
			}

			InvalidEdges.Remove(FlowTransitionNode);
		}
	}

	for (ULogicFlowNode* Element : InvalidNodes) { if (Element) Element->MarkAsGarbage(); }
	for (ULogicFlowTransition* Element : InvalidEdges) { if (Element) Element->MarkAsGarbage(); }
}

class ULogicFlow* UEdGraph_LogicFlow::GetFlowAsset() const
{
	return Cast<ULogicFlow>(GetOuter());
}

void UEdGraph_LogicFlow::CheckGraphNodeType(ULogicFlow* LogicFlow, TArray<class UEdGraphNode*>& OutErrorNodes) const
{
	check(LogicFlow);
	const TArray<TSubclassOf<ULogicFlowNode>>& NodeTypes = LogicFlow->GetAllowedNodeTypes();
	const UClass* EdgeType = LogicFlow->GetAllowedEdgeType().Get();

	// 删除类型不对的节点
	for ( int32 i = 0; i < Nodes.Num(); ++i )
	{
		if (UEdNode_LogicFlowNode* CurEdNode = Cast<UEdNode_LogicFlowNode>(Nodes[i]))
		{
			// 检查节点类型
			if (!NodeTypes.Contains(CurEdNode->FlowNode->GetClass()))
			{
				OutErrorNodes.Add(CurEdNode);
				continue;
			}

			// 如果是子图表，递归检查子图的节点类型
			if (ULogicFlowNode_SubFlow* SubTreeNode = Cast<ULogicFlowNode_SubFlow>(CurEdNode->FlowNode))
			{
				UEdGraph_LogicFlow* SubGraph = Cast<UEdGraph_LogicFlow>(SubTreeNode->SubFlow->EdGraph);
				CheckGraphNodeType(SubGraph->GetFlowAsset(), OutErrorNodes);
			}
		}

		// 检查边类型
		if (UEdNode_LogicFlowEdge* Edge = Cast<UEdNode_LogicFlowEdge>(Nodes[i]))
		{
			if (EdgeType != Edge->FlowTransition->GetClass())
			{
				OutErrorNodes.Add(Edge);
			}
		}
	}
}
