// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "LogicFlowTypeDef.generated.h"

//=============================================================================
/**
 * ULogicFlowTypeDef
 * LogicFlow定义信息
 */
UCLASS(Blueprintable, BlueprintType)
class COMMONLOGICFLOW_API ULogicFlowTypeDef : public UObject
{
	GENERATED_BODY()

public:
	ULogicFlowTypeDef(const FObjectInitializer& Initializer = FObjectInitializer::Get());

	/** LogicFlow类型, 默认未通用LogicFlow */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=DesicionTree)
	TSubclassOf<class ULogicFlow> LogicFlowType;

	/** 执行器类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=DesicionTree)
	TSubclassOf<class ULogicFlowEvaluator> EvaluatorType;

	/**
	 * 执行器上下文类型
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=DesicionTree)
	TSubclassOf<class ULogicFlowEvalContext> EvalContextType;

#if WITH_EDITORONLY_DATA
public:
	// 节点类型
	UPROPERTY(EditDefaultsOnly, Category = "Edit Setting")
	TArray<TSubclassOf<class ULogicFlowNode> > NodeTypes;

	// 边类型
	UPROPERTY(EditDefaultsOnly, Category = "Edit Setting")
	TSubclassOf<class ULogicFlowTransition> EdgeType;
#endif
};
