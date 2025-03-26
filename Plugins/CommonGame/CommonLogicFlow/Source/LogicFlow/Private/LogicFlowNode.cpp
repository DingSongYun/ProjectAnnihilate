// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "LogicFlowNode.h"
#include "LogicFlowEdge.h"

TArray<FName> ULogicFlowNode::GetOutPinNames() const
{
	if (bLeaf) return {};

	return { TEXT("Out") };
}

void ULogicFlowNode::GrabDebugSnapshot(struct FVisualLogEntry* Snapshot) const
{
}

#if WITH_EDITOR
FText ULogicFlowNode::GetNodeTitle() const
{
	return NodeTitle.IsEmpty() ? GetClass()->GetDisplayNameText() : NodeTitle;
}

FLinearColor ULogicFlowNode::GetNodeColor() const
{
	return bLeaf ? FLinearColor(144./255., 230./255., 162./255.) : FLinearColor(0.08f, 0.08f, 0.08f);
}

FText ULogicFlowNode::GetNodeDescription() const
{
	return FText::FromString(GetClass()->GetDescription());
}

void ULogicFlowNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

/**
 * ULogicFlowNode_Branch
 */
int32 ULogicFlowNode_Branch::Evaluate(ULogicFlowEvalContext* InContext)
{
	return CheckCondition(InContext) ? PIN_OUT_INDEX_YES : PIN_OUT_INDEX_NO;
}

TArray<FName> ULogicFlowNode_Branch::GetOutPinNames() const
{
	return {TEXT("是"), TEXT("否")};
}

#if WITH_EDITOR
FLinearColor ULogicFlowNode_Branch::GetNodeColor() const
{
	return FLinearColor(120. / 255., 150. / 255., 230. / 255.);
}

FText ULogicFlowNode_Branch::GetNodeTitle() const
{
	return FText::FromString(TEXT("条件"));
}
#endif

bool ULogicFlowNode_Branch::CheckCondition(ULogicFlowEvalContext* InDataCollector)
{
	if (Condition)
	{
		return Condition->CheckCondition(InDataCollector);
	}

	return true;
}

/**
 * ULogicFlowNode_Select
 */
ULogicFlowNode_Select::ULogicFlowNode_Select(const FObjectInitializer& Initializer) : Super(Initializer)
{
	Conditions.Empty();

#if WITH_EDITORONLY_DATA
	bCustomizePinName = true;
#endif
}

int32 ULogicFlowNode_Select::Evaluate(ULogicFlowEvalContext* InContext)
{
	for (int Index = 0, Num = Conditions.Num(); Index < Num; ++Index)
	{
		if (const TObjectPtr<UFlowCondition>& Condition = Conditions[Index]; Condition && Condition->CheckCondition(InContext))
		{
			return Index;
		}
	}

	return Conditions.Num() + 1;
}

TArray<FName> ULogicFlowNode_Select::GetOutPinNames() const
{
	TArray<FName> OutPinNames;
	for (int i = 0, Num = Conditions.Num(); i < Num; ++i)
	{
		OutPinNames.Add(GetOutputPinNameOfIndex(i));
	}

	OutPinNames.Add(TEXT("Out"));

	return OutPinNames;
}

FName ULogicFlowNode_Select::GetOutputPinNameOfIndex(int32 Index) const
{
#if WITH_EDITOR
	if (Conditions.IsValidIndex(Index))
	{
		if (const TObjectPtr<UFlowCondition> Condition = Conditions[Index])
		{
			const FText DisplayText = Condition->GetConditionDisplayText();
			if (!DisplayText.IsEmpty())
			{
				return *DisplayText.ToString();
			}
		}
	}
#endif

	return *FString::FromInt(Index + 1);
}

#if WITH_EDITOR

FLinearColor ULogicFlowNode_Select::GetNodeColor() const
{
	return FLinearColor(122. / 255., 130. / 255., 230. / 255.);
}

FText ULogicFlowNode_Select::GetNodeTitle() const
{
	return FText::FromString(TEXT("选择"));
}

FText ULogicFlowNode_Select::GetPinDisplayName(int32 Index) const
{
	int32 OutputIndex = Index - 1;
	if (Conditions.IsValidIndex(OutputIndex))
	{
		return FText::FromName(GetOutputPinNameOfIndex(OutputIndex));
	}
	return Super::GetPinDisplayName(Index);
}
#endif
