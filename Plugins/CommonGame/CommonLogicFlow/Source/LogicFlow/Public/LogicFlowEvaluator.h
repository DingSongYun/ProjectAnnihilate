// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LogicFlow.h"
#include "LogicFlowEvaluator.generated.h"

class ULogicFlowNode;
class ULogicFlowEvalContext;

//=============================================================================
/**
 * ULogicFlowEvaluator
 */
UCLASS(Abstract)
class COMMONLOGICFLOW_API ULogicFlowEvaluator : public UObject
{
	GENERATED_BODY()

public:
	ULogicFlowEvaluator(const FObjectInitializer& Initializer = FObjectInitializer::Get());

	virtual ~ULogicFlowEvaluator() override;

	virtual void Execute(ULogicFlowEvalContext* InContext, ULogicFlowNode* ExecutionFromNode = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual ULogicFlowNode* TravelFlow(ULogicFlowEvalContext* InContext, ULogicFlowNode* StartNode);

public:
	/** 是否创建单独得运行时实例, 否则会使用共享得执行器实例进行LogicFlow计算 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=DesicionTree)
	uint8 bCreateInstanced : 1;
};