// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LogicFlowTypeDef.h"
#include "LogicFlow.generated.h"

class ULogicFlowNode;
class ULogicFlowTransition;
class ULogicFlowEvaluator;
class ULogicFlowEvalContext;

//=============================================================================
/** 
 * ULogicFlowEvalHandler
 * LogicFlow执行句柄
 */
USTRUCT(BlueprintType, Blueprintable)
struct COMMONLOGICFLOW_API FLogicFlowEvalHandler
{
	GENERATED_BODY()

public:
	void Execute() const;

	bool IsValid() const { return LogicFlow && Evaluator; }
	
public:
	UPROPERTY(BlueprintReadOnly)
	class ULogicFlowEvaluator* Evaluator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	class ULogicFlowEvalContext* EvalContext = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UObject* CallerObject = nullptr;

	UPROPERTY(BlueprintReadOnly)
	ULogicFlow* LogicFlow = nullptr;
};

//=============================================================================
/** 
 * ULogicFlow
 * 通用逻辑执行流
 */
UCLASS(BlueprintType, Blueprintable)
class COMMONLOGICFLOW_API ULogicFlow : public UObject
{
	GENERATED_BODY()
	
public:
	// /**
	//  * 创建LogicFlow得执行句柄
	//  *
	//  * @param Caller				调用发起得UObject
	//  * @param FlowAsset				LogicFlow
	//  * @param ParentContext			使用特定Context创建Handler，一般子Flow的Handler时会使用父Flow的Context来创建Handler
	//  */
	// UFUNCTION(BlueprintCallable, Category="LogicFlow", meta=(AdvancedDisplay="3"))
	// static FLogicFlowEvalHandler MakeHandler(UObject* Caller, ULogicFlow* FlowAsset, class ULogicFlowEvalContext* ParentContext = nullptr);
	//
	//
	// /** 判断 Hander得有效性 */
	// UFUNCTION(BlueprintPure, Category="LogicFlow|Handler", meta = (DisplayName="IsValid", DefaultToSelf = Handler))
	// static bool FlowHandler_IsValid(const FLogicFlowEvalHandler& Handler) { return Handler.IsValid(); }

	/**
	 * 创建LogicFlow得执行上下文
	 *
	 * @param Caller				调用发起得UObject
	 * @param FlowAsset				LogicFlow
	 * @param ParentContext			使用特定Context创建Handler，一般子Flow的Handler时会使用父Flow的Context来创建Handler
	 */
	UFUNCTION(BlueprintCallable, Category="LogicFlow", meta=(AdvancedDisplay="3"))
	static ULogicFlowEvalContext* MakeEvalContext(UObject* Caller, ULogicFlow* FlowAsset, class ULogicFlowEvalContext* ParentContext = nullptr);

	/** Handler执行 */
	UFUNCTION(BlueprintCallable, Category="LogicFlow|Handler", meta = (DisplayName="Execute", DefaultToSelf = Handler))
	static void FlowHandler_Execute(const FLogicFlowEvalHandler& Handler) { return Handler.Execute(); }
	
	UFUNCTION(BlueprintCallable)
	virtual void PreLoad(UObject* WorldContext) {}

	const ULogicFlowTypeDef* GetTypeDefinition() const
	{
		return FlowTypeDefinition.Get() ? FlowTypeDefinition->GetDefaultObject<ULogicFlowTypeDef>() : nullptr;
	}

	void SetTypeDefinition(TSubclassOf<ULogicFlowTypeDef> InDef) { FlowTypeDefinition = InDef; }

public:
	/** LogicFlow的根节点数组 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TArray<ULogicFlowNode*> RootNodes;

	/** LogicFlow节点 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<ULogicFlowNode*> FlowNodes;
	
	/** LogicFlow的边信息,大部分情况下不会有实体对象的边 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<ULogicFlowTransition*> FlowEdges;

protected:
	/** LogicFlow类型信息 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ULogicFlowTypeDef> FlowTypeDefinition;

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY()
	class UEdGraph* EdGraph = nullptr;
#endif

#if WITH_EDITOR
	friend class ULogicFlowAssetFactory;
public:

	/**
	 * 当前LogicFlow允许得节点类型
	 */
	TArray<TSubclassOf<ULogicFlowNode>> GetAllowedNodeTypes() const;

	/**
	 * LogicFlow允许得边类型
	 */
	TSubclassOf<ULogicFlowTransition> GetAllowedEdgeType() const;
#endif
};