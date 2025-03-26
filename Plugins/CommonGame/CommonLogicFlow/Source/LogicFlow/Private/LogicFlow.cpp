// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "LogicFlow.h"
#include "LogicFlowEdge.h"
#include "LogicFlowEvaluator.h"
#include "LogicFlowNode.h"

void FLogicFlowEvalHandler::Execute() const
{
	check(LogicFlow && Evaluator);

	Evaluator->Execute(EvalContext);
}

// FLogicFlowEvalHandler ULogicFlow::MakeHandler(UObject* Caller, ULogicFlow* FlowAsset, ULogicFlowEvalContext* ParentContext)
// {
// 	check(FlowAsset);
// 	FLogicFlowEvalHandler Handler;
// 	Handler.LogicFlow = FlowAsset;
// 	Handler.CallerObject = Caller;
// 	if (FlowAsset)
// 	{
// 		if (const ULogicFlowTypeDef* Def = FlowAsset->GetTypeDefinition())
// 		{
// 			if (ParentContext != nullptr)
// 			{
// 				Handler.EvalContext = ParentContext;
// 			}
// 			else if (const UClass* DataCollectorClass = Def->EvalContextType.Get())
// 			{
// 				Handler.EvalContext = NewObject<ULogicFlowEvalContext>(Caller, DataCollectorClass);
// 				Handler.EvalContext->LogicFlow = FlowAsset;
// 			}
//
// 			// Create Evaluator
// 			const UClass* EvaluatorClass = Def->EvaluatorType.Get() ? Def->EvaluatorType.Get() : ULogicFlowEvaluator::StaticClass();
// 			ULogicFlowEvaluator* EvaluatorCDO = EvaluatorClass->GetDefaultObject<ULogicFlowEvaluator>();
// 			Handler.Evaluator = EvaluatorCDO->bCreateInstanced ? NewObject<ULogicFlowEvaluator>(Caller, EvaluatorClass) : EvaluatorCDO;
// 			check(Handler.Evaluator);
// 		}
// 	}
// 	return Handler;
// }

ULogicFlowEvalContext* ULogicFlow::MakeEvalContext(UObject* Caller, ULogicFlow* FlowAsset, ULogicFlowEvalContext* ParentContext)
{
	ULogicFlowEvalContext* EvalContext = nullptr;
	const ULogicFlowTypeDef* TypeDefinition = FlowAsset ? FlowAsset->GetTypeDefinition() : nullptr;
	if (FlowAsset && TypeDefinition)
	{
		const UClass* ContextClass = TypeDefinition->EvalContextType.Get();
		if (ContextClass == nullptr)
		{
			ContextClass = ULogicFlowEvalContext::StaticClass();
		}
		EvalContext = NewObject<ULogicFlowEvalContext>(Caller, ContextClass);
		if (ParentContext != nullptr)
		{
			EvalContext->ParentContext = ParentContext;
		}
		EvalContext->Initialize(Caller, FlowAsset);

		// Create Evaluator
		const UClass* EvaluatorClass = TypeDefinition->EvaluatorType.Get() ? TypeDefinition->EvaluatorType.Get() : ULogicFlowEvaluator::StaticClass();
		ULogicFlowEvaluator* EvaluatorCDO = EvaluatorClass->GetDefaultObject<ULogicFlowEvaluator>();
		EvalContext->Evaluator = EvaluatorCDO->bCreateInstanced ? NewObject<ULogicFlowEvaluator>(Caller, EvaluatorClass) : EvaluatorCDO;
		check(EvalContext->Evaluator);
	}

	return EvalContext;
}

#if WITH_EDITOR
TArray<TSubclassOf<ULogicFlowNode>> ULogicFlow::GetAllowedNodeTypes() const
{
	if (const ULogicFlowTypeDef* Def = GetTypeDefinition())
	{
		return Def->NodeTypes;
	}
	return {};
}

TSubclassOf<ULogicFlowTransition> ULogicFlow::GetAllowedEdgeType() const
{
	if (const ULogicFlowTypeDef* Def = GetTypeDefinition())
	{
		return Def->EdgeType;
	}
	return nullptr;
}

#endif