// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAvatarMaterialInfo.h"
#include "Engine/Texture.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAvatarMaterialInfo)

bool FAvatarMaterialInfo::ContainsParameter(const FName& Name)
{
	for (auto It = MaterialParamsScalar.CreateIterator(); It; ++It)
		if (It->ParamName == Name)
			return true;
	for (auto It = MaterialParamsVector.CreateIterator(); It; ++It)
		if (It->ParamName == Name)
			return true;
	for (auto It = MaterialParamsTexture.CreateIterator(); It; ++It)
		if (It->ParamName == Name)
			return true;
	return false;
}

void FAvatarMaterialInfo::AddParameter(const FName& Name, float Value)
{
	if (!MaterialParamsScalar.ContainsByPredicate(
		[Name](const FAvatarMatParam_Scalar& Parameter) { return Parameter.ParamName == Name; }
		))
	{
		MaterialParamsScalar.Add({ Name, 1,Value });
	}
}

void FAvatarMaterialInfo::AddParameter(const FName& Name, const FLinearColor& Value)
{
	if (!MaterialParamsVector.ContainsByPredicate(
		[Name](const FAvatarMatParam_Vector& Parameter){ return Parameter.ParamName == Name; }
		))
	{
		MaterialParamsVector.Add({ Name, 1,Value });
	}
}

void FAvatarMaterialInfo::AddParameter(const FName& Name, UTexture* Value)
{
	if (!MaterialParamsTexture.ContainsByPredicate(
		[Name](const FAvatarMatParam_Texture& Parameter) { return Parameter.ParamName == Name; }
		))
	{
		MaterialParamsTexture.Add({ Name, 1,Value });
	}
}

void FAvatarMaterialInfo::RemoveParameter(const FName& Name)
{
	for (auto It = MaterialParamsScalar.CreateIterator(); It; ++It)
	{
		if (It->ParamName == Name)
		{
			It.RemoveCurrent();
			return;
		}
	}
	for (auto It = MaterialParamsVector.CreateIterator(); It; ++It)
	{
		if (It->ParamName == Name)
		{
			It.RemoveCurrent();
			return;
		}
	}
	for (auto It = MaterialParamsTexture.CreateIterator(); It; ++It)
	{
		if (It->ParamName == Name)
		{
			It.RemoveCurrent();
			return;
		}
	}
}