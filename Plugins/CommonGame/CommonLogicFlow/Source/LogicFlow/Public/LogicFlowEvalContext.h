// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LogicFlowEvalContext.generated.h"

class ULogicFlow;
class ULogicFlowNode;
class ULogicFlowEvaluator;
class ULogicFlowTransition;

/** 记录flow的执行过程 */
class FFlowEvalRecordItem
{
public:
	FFlowEvalRecordItem(ULogicFlowNode* InNode): EvalNode(InNode), EvalTransition(nullptr) {}
	FFlowEvalRecordItem(ULogicFlowTransition* InTransition): EvalNode(nullptr), EvalTransition(InTransition) {}
public:
	TWeakObjectPtr<ULogicFlowNode> EvalNode = nullptr;
	TWeakObjectPtr<ULogicFlowTransition> EvalTransition = nullptr;
};

UCLASS(Blueprintable, BlueprintType)
class COMMONLOGICFLOW_API ULogicFlowEvalContext : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UObject* InOwnerObject, ULogicFlow* InLogicFlow);

	UFUNCTION(BlueprintPure)
	double GetEvalResultNumeric() const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE ULogicFlowNode* GetEvalResultNode() const { return LastExecutionReachedNode; }

	UFUNCTION(BlueprintCallable)
	void Execute();

	template<typename T>
	T* GetEvalResultNode() const
	{
		return Cast<T>(LastExecutionReachedNode);
	}

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveInitialize();

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> OwnerObject = nullptr;

	/** 逻辑流图 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULogicFlow> LogicFlow = nullptr;

	/** 如果是子节点，需要ParentContext来填充 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULogicFlowEvalContext> ParentContext = nullptr;

	/** 执行器 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULogicFlowEvaluator> Evaluator = nullptr;

	/** 上一次执行到的节点 */
	UPROPERTY(BlueprintReadWrite, Category=DesicionTree)
	TObjectPtr<ULogicFlowNode> LastExecutionReachedNode = nullptr;

public:
#if WITH_EDITORONLY_DATA
	TArray<FFlowEvalRecordItem> EvalRecord;
#endif
};
