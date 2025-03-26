// Copyright 2022 SongYun Ding. All Rights Reserved.

#include "LogicFlowNodeBlueprint.h"

int32 ULogicFlowNodeBlueprint::Evaluate(ULogicFlowEvalContext* InContext)
{
	return ReceiveEvaluate(InContext);
}

uint8 ULogicFlowNodeBlueprint::GetOutPinNums() const
{
	return ReceiveGetOutPinNums();
}

TArray<FName> ULogicFlowNodeBlueprint::GetOutPinNames() const
{
	return ReceiveGetOutPinNames();
}
