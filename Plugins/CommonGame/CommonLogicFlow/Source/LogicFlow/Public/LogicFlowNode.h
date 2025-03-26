// Copyright 2022 SongYun Ding. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LogicFlowCondition.h"
#include "LogicFlowEvalContext.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "LogicFlowNode.generated.h"

//=============================================================================
/** 
 * ULogicFlowNode
 * LogicFlow节点
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable)
class COMMONLOGICFLOW_API ULogicFlowNode : public UObject, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

public:
	inline static int PIN_OUT_INDEX_DEFAULT = 0;

	/**
	 * 执行节点
	 * @return index: out pin 的index
	 */
	virtual int32 Evaluate(ULogicFlowEvalContext* InContext) { return PIN_OUT_INDEX_DEFAULT; }

	/** 当前节点OutPin的数目 */
	virtual uint8 GetOutPinNums() const { return !bLeaf ? 1 : 0; }
	virtual TArray<FName> GetOutPinNames() const;

	virtual void GrabDebugSnapshot(struct FVisualLogEntry* Snapshot) const override;

#if WITH_EDITOR
public:
	virtual FText GetNodeTitle() const;
	virtual FLinearColor GetNodeColor() const;
	virtual FText GetNodeDescription() const;
	virtual bool CanCreateConnection(ULogicFlowNode* Other, FText& ErrorMessage) { return true; }

	/** Get customize pin display name */
	virtual FText GetPinDisplayName(int32 Index) const { return FText::GetEmpty(); }

protected:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	/** 从该节点出发的边的索引 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Flow")
	TArray<int32> Transitions;

	UPROPERTY(EditDefaultsOnly, Category = "Flow", AdvancedDisplay)
	uint8 bLeaf : 1;

#if WITH_EDITORONLY_DATA
public:
	UPROPERTY()
	FText NodeTitle = FText::GetEmpty();

	UPROPERTY(EditDefaultsOnly)
	bool bCustomizePinName = false;
#endif
};

//========================基础节点=====================================================

/** 
 * ULogicFlowRootNode
 * LogicFlow子树根节点
 */
UCLASS(NotBlueprintable, BlueprintType)
class COMMONLOGICFLOW_API ULogicFlowRootNode : public ULogicFlowNode
{
	GENERATED_BODY()

	virtual TArray<FName> GetOutPinNames() const { return {TEXT("")}; }

#if WITH_EDITOR
public:
	virtual FText GetNodeTitle() const override { return FText::FromString("Start"); }
#endif
};

/**
 * ULogicFlowNode_Branch
 * 分支节点
 */
UCLASS(NotBlueprintable, BlueprintType, Category="Flow", DisplayName="分支节点")
class COMMONLOGICFLOW_API ULogicFlowNode_Branch : public ULogicFlowNode
{
	GENERATED_BODY()

public:
	inline static int PIN_OUT_INDEX_YES = 0;
	inline static int PIN_OUT_INDEX_NO = 1;

	//~BEGIN: ULogicFlowNode interface
	virtual int32 Evaluate(ULogicFlowEvalContext* InContext) override;
	virtual uint8 GetOutPinNums() const override { return 2; }
	virtual TArray<FName> GetOutPinNames() const override;
	//~END: ULogicFlowNode interface

#if WITH_EDITOR
	virtual FLinearColor GetNodeColor() const override;
	virtual FText GetNodeTitle() const override;
#endif

protected:
	/** 检查条件 */
	UFUNCTION(BlueprintCallable)
	virtual bool CheckCondition(ULogicFlowEvalContext* InDataCollector);

protected:
	/** 该节点的检查条件 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Branch")
	TObjectPtr<UFlowCondition> Condition;
};

/**
 * ULogicFlowNode_Select
 * 选择节点
 */
UCLASS(NotBlueprintable, BlueprintType, Category="Flow", DisplayName="选择节点")
class COMMONLOGICFLOW_API ULogicFlowNode_Select : public ULogicFlowNode
{
	GENERATED_UCLASS_BODY()

public:
	//~BEGIN: ULogicFlowNode interface
	virtual int32 Evaluate(ULogicFlowEvalContext* InContext) override;
	virtual uint8 GetOutPinNums() const override { return GetOutPinNames().Num() + 1; }
	virtual TArray<FName> GetOutPinNames() const override;
	//~END: ULogicFlowNode interface

	FName GetOutputPinNameOfIndex(int32 Index) const;

	void AddNewSelection()
	{
		Conditions.AddDefaulted();
	}

	void RemoveSelection(int Index)
	{
		if (Conditions.IsValidIndex(Index))
		{
			Conditions.RemoveAt(Index);
		}
	}

#if WITH_EDITOR
	virtual FLinearColor GetNodeColor() const override;
	virtual FText GetNodeTitle() const override;
	virtual FText GetPinDisplayName(int32 Index) const;
#endif

public:
	/** 该节点的检查条件 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Branch", EditFixedSize)
	TArray<TObjectPtr<UFlowCondition>> Conditions;
};

//=============================================================================
/** 
 * ULogicFlowNode_SubFlow
 * LogicFlow子树节点
 */
UCLASS(NotBlueprintable, NotBlueprintType, DisplayName="子流图节点")
class COMMONLOGICFLOW_API ULogicFlowNode_SubFlow : public ULogicFlowNode
{
	GENERATED_BODY()

public:
	ULogicFlowNode_SubFlow(): Super()
	{
		bLeaf = true;
	}

public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Instanced)
	class ULogicFlow* SubFlow = nullptr;
};

//=============================================================================
/**
 * ULogicFlowNode_GenericValue
 */
UCLASS(NotBlueprintable, NotBlueprintType, DisplayName="数值")
class COMMONLOGICFLOW_API ULogicFlowNode_GenericValue : public ULogicFlowNode
{
	GENERATED_BODY()

public:
	ULogicFlowNode_GenericValue(): Super()
	{
		bLeaf = true;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Generic Value")
	double Value = 0;
};