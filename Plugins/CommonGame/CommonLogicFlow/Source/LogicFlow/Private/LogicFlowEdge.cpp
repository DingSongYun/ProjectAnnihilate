#include "LogicFlowEdge.h"
#include "LogicFlowCondition.h"

bool ULogicFlowTransition::CheckConditions(ULogicFlowEvalContext* InDataCollector)
{
	if (Condition)
	{
		return Condition->CheckCondition(InDataCollector);
	}

	return true;
}

#if WITH_EDITOR
FText ULogicFlowTransition::GetTransitionTitle() const
{
	return Condition ? Condition->GetConditionDisplayText() : FText::FromString(TEXT(""));
}

void ULogicFlowTransition::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif