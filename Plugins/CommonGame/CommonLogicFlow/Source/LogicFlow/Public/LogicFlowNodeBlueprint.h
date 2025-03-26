// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LogicFlowNode.h"
#include "LogicFlowNodeBlueprint.generated.h"

/**
 * ULogicFlowNodeBlueprint
 */
UCLASS(Blueprintable)
class COMMONLOGICFLOW_API ULogicFlowNodeBlueprint : public ULogicFlowNode
{
	GENERATED_BODY()

public:
	virtual int32 Evaluate(ULogicFlowEvalContext* InContext) override;
	virtual uint8 GetOutPinNums() const override;
	virtual TArray<FName> GetOutPinNames() const override;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	int32 ReceiveEvaluate(ULogicFlowEvalContext* InContext);
	UFUNCTION(BlueprintImplementableEvent)
	int32 ReceiveGetOutPinNums() const;
	UFUNCTION(BlueprintImplementableEvent)
	TArray<FName> ReceiveGetOutPinNames() const;
};
