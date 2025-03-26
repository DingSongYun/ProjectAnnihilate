#include "LogicFlowCondition.h"


#if WITH_EDITOR
void UFlowCondition::CopyData(UFlowCondition* OtherCondition)
{
	// 调用蓝图的数据拷贝接口
	ReceiveCopyData(OtherCondition);

	if (OtherCondition)
	{
		bResultBeNot = OtherCondition->bResultBeNot;
		bLogicAnd = OtherCondition->bLogicAnd;
	}
}

FText UFlowCondition::GetConditionDisplayText() const
{
	if (!DisplayText.IsEmpty())
	{
		return DisplayText;
	}
	return ReceiveGetConditionDisplayText();
}
#endif

bool UFlowConditionGroup::CheckCondition(ULogicFlowEvalContext* InDataCollector)
{
	bool Result = true;
	for (int32 i = 0; i < Conditions.Num(); ++i)
	{
		if (!Conditions[i])
			continue;

		if (Conditions[i]->bLogicAnd && Result == false) continue;

		bool CurReuslt = Conditions[i]->CheckCondition(InDataCollector);

		if (i == 0)
			Result = CurReuslt;
		else
		{
			if (Conditions[i]->bLogicAnd)
				Result &= CurReuslt;
			else
				Result |= CurReuslt;
		}
	}

	return bResultBeNot ? !Result : Result;
}

#if WITH_EDITOR
void UFlowConditionGroup::CopyData(UFlowCondition* OtherCondition)
{
	UFlowConditionGroup* OtherGroup= Cast<UFlowConditionGroup>(OtherCondition);
	if (OtherGroup == nullptr) return ;
	
	Conditions.Empty();
	for (int32 i = 0; i < OtherGroup->Conditions.Num(); ++i)
	{
		if (OtherGroup->Conditions[i])
		{
			UFlowCondition* NewCondition = NewObject<UFlowCondition>(this, OtherGroup->Conditions[i]->GetClass());
			if (NewCondition)
			{
				NewCondition->CopyData(OtherGroup->Conditions[i]);
				Conditions.Add(NewCondition);
			}
		}
	}
}

FText UFlowConditionGroup::GetConditionDisplayText() const
{
	if (!DisplayText.IsEmpty())
	{
		return DisplayText;
	}
	FString Result;
	for (int32 i = 0; i < Conditions.Num(); ++i)
	{
		if (Conditions[i])
		{
			Result += Conditions[i]->GetConditionDisplayText().ToString();
			Result += TEXT("\n");
		}
	}

	return FText::FromString(Result);
}

#endif