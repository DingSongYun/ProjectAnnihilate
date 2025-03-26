// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "LogicFlowEvalContext.h"

#include "LogicFlowEvaluator.h"
#include "LogicFlowNode.h"

void ULogicFlowEvalContext::Initialize(UObject* InOwnerObject, ULogicFlow* InLogicFlow)
{
	check(InLogicFlow);
	OwnerObject = InOwnerObject;
	LogicFlow = InLogicFlow;

	ReceiveInitialize();
}

double ULogicFlowEvalContext::GetEvalResultNumeric() const
{
	if (ULogicFlowNode_GenericValue* ValueNode = GetEvalResultNode<ULogicFlowNode_GenericValue>())
	{
		return ValueNode->Value;
	}

	return 0.;
}

void ULogicFlowEvalContext::Execute()
{
	if (Evaluator)
	{
		Evaluator->Execute(this);
	}
}
