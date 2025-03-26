#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "LogicFlowEvalContext.h"

#include "LogicFlowCondition.generated.h"

//=============================================================================
/** 
 * UFlowCondition
 * LogicFlow迁移条件
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class COMMONLOGICFLOW_API UFlowCondition : public UObject
{
	GENERATED_BODY()
	
public:
	// 结果取非
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logic")
	bool bResultBeNot = false;

	// 与运算/或运算
	UPROPERTY(EditAnywhere, Category = "Logic")
	bool bLogicAnd = false;

public:
	// 外界调用的条件检查函数
	UFUNCTION(BlueprintCallable)
	virtual bool CheckCondition(ULogicFlowEvalContext* EvalContext) { return bResultBeNot ? !ReceiveCheckCondition(EvalContext) : ReceiveCheckCondition(EvalContext); }

protected:
	/** 蓝图重载的条件检查函数 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName="CheckCondition")
	bool ReceiveCheckCondition(ULogicFlowEvalContext* EvalContext);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="GetDisplayText"))
	FText ReceiveGetConditionDisplayText() const;

	/** TODO: To delete */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="CopyData"))
	void ReceiveCopyData(UFlowCondition* OtherCondition);

#if WITH_EDITOR
public:
	UFUNCTION(BlueprintCallable)
	virtual void CopyData(UFlowCondition* OtherCondition);

	/** Condition的描述 */
	virtual FText GetConditionDisplayText() const;

#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FText DisplayText = FText::GetEmpty();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FString LineBreakString = TEXT("\n");
#endif
};

//=============================================================================
/** 
 * UFlowConditionGroup
 * LogicFlow迁移条件
 */
UCLASS(NotBlueprintType, NotBlueprintable)
class COMMONLOGICFLOW_API UFlowConditionGroup : public UFlowCondition
{
	GENERATED_BODY()

public:
	// 该条件组的条件信息
	UPROPERTY(EditAnywhere, Instanced, Category = "Logic")
	TArray<UFlowCondition*> Conditions;

public:
	// 外界调用的条件检查函数
	virtual bool CheckCondition(ULogicFlowEvalContext* InDataCollector) override;


#if WITH_EDITOR
public:
	virtual void CopyData(UFlowCondition* OtherCondition) override;

	virtual FText GetConditionDisplayText() const override;

#endif

};