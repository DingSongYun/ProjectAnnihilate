// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "LogicFlowTypeDef.h"
#include "LogicFlowEdge.h"
#include "LogicFlowEvalContext.h"
#include "LogicFlowEvaluator.h"
#include "LogicFlowNode.h"

ULogicFlowTypeDef::ULogicFlowTypeDef(const FObjectInitializer& Initializer) : Super(Initializer)
{
	LogicFlowType = ULogicFlow::StaticClass();
	EvaluatorType = ULogicFlowEvaluator::StaticClass();
	EvalContextType = ULogicFlowEvalContext::StaticClass();

#if WITH_EDITORONLY_DATA
	NodeTypes.Add(ULogicFlowNode::StaticClass());
	NodeTypes.Add(ULogicFlowNode_Branch::StaticClass());
	NodeTypes.Add(ULogicFlowNode_Select::StaticClass());
	NodeTypes.Add(ULogicFlowNode_SubFlow::StaticClass());
	NodeTypes.Add(ULogicFlowNode_GenericValue::StaticClass());
	EdgeType = ULogicFlowTransition::StaticClass();
#endif
}
