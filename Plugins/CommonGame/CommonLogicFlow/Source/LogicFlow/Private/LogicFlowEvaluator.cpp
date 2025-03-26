// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "LogicFlowEvaluator.h"
#include "LogicFlowEdge.h"
#include "LogicFlowEvalContext.h"
#include "LogicFlowNode.h"

ULogicFlowEvaluator::ULogicFlowEvaluator(const FObjectInitializer& Initializer)
	: Super(Initializer)
	, bCreateInstanced(false)
{
}

ULogicFlowEvaluator::~ULogicFlowEvaluator()
{}

void ULogicFlowEvaluator::Execute(ULogicFlowEvalContext* InContext, ULogicFlowNode* ExecutionFromNode)
{
	if (!InContext) return ;

#if WITH_EDITOR
	InContext->EvalRecord.Empty();
#endif


	TArray<ULogicFlowNode*> RootList;
	if (ExecutionFromNode != nullptr)
	{
		RootList.Add(ExecutionFromNode);
	}
	else if (InContext->LogicFlow)
	{
		RootList.Append(InContext->LogicFlow->RootNodes);
	}

	ULogicFlowNode* ResultNode = nullptr;
	for (int32 i = 0; i < RootList.Num(); ++i)
	{
		if (RootList[i])
		{
			// 遍历该根节点的LogicFlow
			ResultNode = TravelFlow(InContext, RootList[i]);
			if (ResultNode)
			{
				InContext->LastExecutionReachedNode = ResultNode;
			}
		}
	}
}

ULogicFlowNode* ULogicFlowEvaluator::TravelFlow(ULogicFlowEvalContext* InContext, ULogicFlowNode* StartNode)
{
	if (!StartNode) return nullptr;

#if WITH_EDITOR
	InContext->EvalRecord.Add(StartNode);
#endif
	const int32 OutPinIndex = StartNode->Evaluate(InContext);
	if (OutPinIndex <= INDEX_NONE)
	{
		return StartNode;
	}

	TArray<ULogicFlowTransition*>& EdgeList = InContext->LogicFlow->FlowEdges;
	ULogicFlowNode* ResultNode = StartNode;

	// 遍历从该节点出发的边
	for (int32 i = 0; i < StartNode->Transitions.Num(); ++i)
	{
		const int32 TransitionIndex = StartNode->Transitions[i];
		if (!EdgeList.IsValidIndex(TransitionIndex)) continue;
		
		ULogicFlowTransition* Transition = EdgeList[TransitionIndex];
		if (!Transition || Transition->FromNodePin != OutPinIndex + 1) continue;

		const bool bSucc = Transition->CheckConditions(InContext);
		
#if WITH_EDITOR
	InContext->EvalRecord.Add(Transition);
#endif

		if (!bSucc) continue;

		if (ULogicFlowNode* EndNode = TravelFlow(InContext, Transition->ToNode))
		{
			ResultNode = EndNode;
			break;
		}
	}

	return ResultNode;
}