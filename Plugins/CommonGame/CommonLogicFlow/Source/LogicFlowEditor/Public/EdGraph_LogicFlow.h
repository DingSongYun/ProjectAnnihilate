// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph_LogicFlow.generated.h"

class ULogicFlow;

/** 流图绘制方式 */
UENUM(BlueprintType)
enum class EFlowDrawDirection : uint8
{
	Horizontal,
	Vertical
};

UENUM(BlueprintType)
enum class EFlowConnectionStyle : uint8
{
	Default,
	Straight
};

UCLASS()
class UEdGraph_LogicFlow : public UEdGraph
{
	GENERATED_BODY()

public:
	UEdGraph_LogicFlow();

	//~BEGIN: UEdGraph interface
	virtual void NotifyGraphChanged() override;
	virtual void NotifyGraphChanged( const FEdGraphEditAction& Action ) override;
	virtual void PostEditUndo() override;
	//~END: UEdGraph interface

	/**
	 * 更新资源
	 */
	void UpdateLogicFlowAsset();

	/** 获取LogicFlow资源*/
	class ULogicFlow* GetFlowAsset() const;

	FORCEINLINE bool IsDebugging() const { return ObjectBeingDebugged.IsValid(); }
	bool ShouldShowDebug(class ULogicFlowNode* FlowNode) const;
	bool ShouldShowDebug(class ULogicFlowTransition* FlowTransition) const;

protected:
	void Clear();
	static void UpdateLogicFlowFromGraph(ULogicFlow* InLogicFlow, UEdGraph_LogicFlow& Graph);
	void CheckGraphNodeType(ULogicFlow* LogicFlow, TArray<class UEdGraphNode*>& ErrorNodes) const;

public:
	UPROPERTY()
	TObjectPtr<class UEdNode_LogicFlowNode> RootNode;

	UPROPERTY(EditAnywhere)
	EFlowDrawDirection FlowDrawDirection = EFlowDrawDirection::Horizontal;

	UPROPERTY(EditAnywhere)
	EFlowConnectionStyle ConnectionStyle = EFlowConnectionStyle::Default;

#if WITH_EDITOR
	//~=============================================================================
	// Debugging
public:
	TWeakObjectPtr<class ULogicFlowEvalContext> ObjectBeingDebugged = nullptr;
	TWeakObjectPtr<UWorld> WorldBeingDebugged = nullptr;
#endif
};
