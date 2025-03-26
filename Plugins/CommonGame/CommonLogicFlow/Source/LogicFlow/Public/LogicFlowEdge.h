// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LogicFlowEdge.generated.h"

class ULogicFlowEvalContext;
class ULogicFlowNode;

/**
 * ULogicFlowTransition
 * LogicFlow边
 * 通常情况下，LogicFlow不依靠边驱动，而是通过Node Pin的联通关系进行驱动
 * Edge可用于一些特殊的场景，比如分支结构中需要对边进行条件判定
 */
UCLASS(BlueprintType, Blueprintable)
class COMMONLOGICFLOW_API ULogicFlowTransition : public UObject
{
	GENERATED_BODY()

public:
	/** 该节点的检查条件 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Conditions")
	class UFlowCondition* Condition = nullptr;

	UFUNCTION(BlueprintCallable)
	virtual bool CheckConditions(ULogicFlowEvalContext* InDataCollector);

#if WITH_EDITORONLY_DATA
public:
	// 边信息框的大小
	UPROPERTY(EditDefaultsOnly)
	FVector2D EdgeMessageSize = FVector2D(5.0f, 5.0f);

	bool bDebuggFlowSelect = false;
protected:
	UPROPERTY()
	FText EdgeTitle;

#endif

public:
	/** 边的出发节点 */
	UPROPERTY(VisibleDefaultsOnly)
	ULogicFlowNode* FromNode = nullptr;

	UPROPERTY(VisibleDefaultsOnly)
	int32 FromNodePin = INDEX_NONE;

	/** 边的终止节点 */
	UPROPERTY(VisibleDefaultsOnly)
	ULogicFlowNode* ToNode = nullptr;

#if WITH_EDITOR
public:
	virtual FText GetTransitionTitle() const;

protected:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

