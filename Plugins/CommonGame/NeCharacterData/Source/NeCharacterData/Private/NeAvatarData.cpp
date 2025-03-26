// Copyright NetEase Games, Inc. All Rights Reserved.

#include "NeAvatarData.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(NeAvatarData)

void FVisualSceneComponent::CopyDataFromOther(const USceneComponent* OtherComp)
{
	if (!OtherComp)
		return;

	ComponentName = OtherComp->GetFName();

	AttachParentPath = OtherComp->GetAttachParent() ? OtherComp->GetAttachParent()->GetFName() : NAME_None;
	AttachSocket.SocketName = OtherComp->GetAttachSocketName();
	RelativeTransform = OtherComp->GetRelativeTransform();

	bHiddenInGame = OtherComp->bHiddenInGame;
}

void FVisualSceneComponent::CopyDataToOther(USceneComponent* OtherComp, AActor* CompOwner) const
{
	if (!OtherComp)
		return;

	OtherComp->ComponentTags.Empty();
	OtherComp->ComponentTags.Append(CustomTags);

	if (OtherComp != CompOwner->GetRootComponent())
	{
		USceneComponent* AttachComp;
		if (AttachParentPath.IsNone())
		{
			AttachComp = CompOwner->GetRootComponent();
		}
		else
		{
			AttachComp = Cast<USceneComponent>(AttachParentPath.ResolveComponent(CompOwner));
			if (AttachComp == nullptr)
				AttachComp = CompOwner->GetRootComponent();
		}
		if (AttachComp)
		{
			OtherComp->AttachToComponent(AttachComp, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket.SocketName);
			OtherComp->SetRelativeTransform(RelativeTransform);
		}
	}

	OtherComp->SetHiddenInGame(bHiddenInGame);
}

#if WITH_EDITORONLY_DATA
void FVisualSceneComponent::SetPreviewActor(AActor* PreviewActor)
{
	if (AttachParentPath.HostActor != PreviewActor)
		AttachParentPath.HostActor = PreviewActor;
	AttachSocket.SetHostComponent(PreviewActor, AttachParentPath);
}

#endif

void UAvatarMakeData::PreLoad(UObject* WorldContext)
{
}

void UAvatarMakeData::MakeAvatar(class ACharacter* Character)
{
	Character->GetMesh()->SetSkeletalMesh(nullptr);
	Character->GetMesh()->SetAnimInstanceClass(nullptr);
}

void UAvatarMakeData::ModifyMeshMaterial(UMeshComponent* MeshComponent, const TArray<FAvatarMaterialInfo>& NewMaterialInfo)
{
	if (MeshComponent == nullptr || NewMaterialInfo.Num() == 0)
		return;
	const int32 TotalMaterialSlot = MeshComponent->GetNumMaterials();
	for (const auto& ChangeData : NewMaterialInfo)
	{
		if (ChangeData.MaterialIndex < 0 || ChangeData.MaterialIndex >= TotalMaterialSlot)
			continue;
		if (ChangeData.Material.LoadSynchronous() != nullptr)
		{
			UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(ChangeData.Material.LoadSynchronous(), MeshComponent);
			for (auto& MatParam : ChangeData.MaterialParamsScalar)
				if(MatParam.bEnable)
					MaterialInstance->SetScalarParameterValue(MatParam.ParamName, MatParam.Value);
			for (auto& MatParam : ChangeData.MaterialParamsVector)
				if (MatParam.bEnable)
					MaterialInstance->SetVectorParameterValue(MatParam.ParamName, FVector4(MatParam.Value));
			for (auto& MatParam : ChangeData.MaterialParamsTexture)
				if (MatParam.bEnable)
					MaterialInstance->SetTextureParameterValue(MatParam.ParamName, MatParam.Value.LoadSynchronous());
			MeshComponent->SetMaterial(ChangeData.MaterialIndex, MaterialInstance);
		}
	}
}

#if WITH_EDITOR
void UAvatarMakeData::CopyAvatarMaterialParameter(const TSoftObjectPtr<USkeletalMesh>& SkeletalMesh, TArray<FAvatarMaterialInfo>& Materials)
{
	Materials.Empty();
	if (USkeletalMesh* MeshAsset = SkeletalMesh.LoadSynchronous())
	{
		const TArray<FSkeletalMaterial>& AssetMaterialArray = MeshAsset->GetMaterials();
		for (int32 i = 0; i < AssetMaterialArray.Num(); ++i)
		{
			const FSkeletalMaterial& AssetMaterial = AssetMaterialArray[i];
			FAvatarMaterialInfo MaterialData;
			MaterialData.SlotName = AssetMaterialArray[i].MaterialSlotName;
			MaterialData.MaterialIndex = i;
			MaterialData.Material = AssetMaterial.MaterialInterface;
			UpdateMaterialParameter(MaterialData);
			Materials.Add(MaterialData);
		}
	}
}

void UAvatarMakeData::UpdateMaterialParameter(FAvatarMaterialInfo& MaterialData)
{
	MaterialData.MaterialParamsScalar.Empty();
	MaterialData.MaterialParamsVector.Empty();
	MaterialData.MaterialParamsTexture.Empty();
	//材质参数修改方案从扫描全部参数改为手动添加参数，因此下面代码注释掉
	/*
	if (UMaterialInterface* MaterialInterface = MaterialData.Material.LoadSynchronous())
	{
		UMaterial* Material = MaterialInterface->GetMaterial();
		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(MaterialData.Material.LoadSynchronous(), nullptr);

		TArray<FMaterialParameterInfo> VisibleExpressions;
		IMaterialEditorModule* MaterialEditorModule = &FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
		if (MaterialEditorModule && MaterialInstance)
			MaterialEditorModule->GetVisibleMaterialParameters(Material, MaterialInstance, VisibleExpressions);

		TArray<FMaterialParameterInfo> ParameterInfo;
		TArray<FGuid> ParameterIds;

		Material->GetAllScalarParameterInfo(ParameterInfo, ParameterIds);
		for (auto& Param : ParameterInfo)
		{
			if (!VisibleExpressions.Contains(FMaterialParameterInfo(Param.Name)))
				continue;
			FCharTemplateMaterialParamScalar ParamTemp{ Param.Name,0 };
			Material->GetScalarParameterValue(Param, ParamTemp.Value);
			MaterialData.MaterialParamsScalar.Add(ParamTemp);
		}
		Material->GetAllVectorParameterInfo(ParameterInfo, ParameterIds);
		for (auto& Param : ParameterInfo)
		{
			if (!VisibleExpressions.Contains(FMaterialParameterInfo(Param.Name)))
				continue;
			FCharTemplateMaterialParamVector ParamTemp{ Param.Name,0 };
			Material->GetVectorParameterValue(Param, ParamTemp.Value);
			MaterialData.MaterialParamsVector.Add(ParamTemp);
		}
		Material->GetAllTextureParameterInfo(ParameterInfo, ParameterIds);
		for (auto& Param : ParameterInfo)
		{
			if (!VisibleExpressions.Contains(FMaterialParameterInfo(Param.Name)))
				continue;
			FCharTemplateMaterialParamTexture ParamTemp{ Param.Name,0 };
			UTexture* TextureInstance = nullptr;
			Material->GetTextureParameterValue(Param, TextureInstance);
			ParamTemp.Value = TextureInstance;
			MaterialData.MaterialParamsTexture.Add(ParamTemp);
		}
	}
	*/
}

#endif

UAvatarMakeSimple::UAvatarMakeSimple(const FObjectInitializer& Initializer) : Super(Initializer)
{
}

void UAvatarMakeSimple::PreLoad(UObject* WorldContext)
{

}

void UAvatarMakeSimple::MakeAvatar(ACharacter* Character)
{
	if (USkeletalMeshComponent* SkeletalMeshComponent = Character->GetMesh())
	{
		const USkeletalMesh* LoadedSkeletalMesh = SkeletalMesh.LoadSynchronous();
		if (LoadedSkeletalMesh)
		{
			SkeletalMeshComponent->SetSkeletalMesh(SkeletalMesh.LoadSynchronous());
			//SkeletalMeshComponent->SetAnimInstanceClass(AnimationClass);
			SkeletalMeshComponent->SetRelativeLocation(MeshOffset);
			SkeletalMeshComponent->SetRelativeScale3D(MeshScale);
			SkeletalMeshComponent->SetRelativeRotation(MeshRotation);
			Character->CacheInitialMeshOffset(SkeletalMeshComponent->GetRelativeLocation(), SkeletalMeshComponent->GetRelativeRotation());
		}

		// 修改部位材质
		if (bModifyMaterial)
		{
			ModifyMeshMaterial(SkeletalMeshComponent, Materials);
		}
	}
}

#if WITH_EDITOR
void UAvatarMakeSimple::SetPreviewContext(AActor* Actor)
{
}


TArray<FName> UAvatarMakeSimple::GetAllMeshName()
{
	TArray<FName> Results;

	return Results;
}

void UAvatarMakeSimple::PreEditChange(FProperty* PropertyAboutToChange)
{
	MaterialFlag.Empty();
	for (const FAvatarMaterialInfo& MaterialData : Materials)
	{
		MaterialFlag.Add(MaterialData.Material);
	}
}
void UAvatarMakeSimple::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == FName(TEXT("SkeletalMesh")))
	{
		//改变了Mesh，重新扫描材质信息
		bModifyMaterial = false; //默认不开启材质修改
		CopyAvatarMaterialParameter(SkeletalMesh, Materials);
		if (const USkeletalMesh* SkeletalMeshAsset = SkeletalMesh.LoadSynchronous())
		{
			MeshOffset.Z = - SkeletalMeshAsset->GetBounds().BoxExtent.Z;
		}
	}
	else if (PropertyChangedEvent.GetPropertyName() == FName(TEXT("Material")))
	{
		for (int i = 0; i < Materials.Num(); ++i)
		{
			if (Materials[i].Material != MaterialFlag[i])
			{
				//改变了材质，重新扫描材质参数信息
				UpdateMaterialParameter(Materials[i]);
				break;
			}
		}
	}
}

#endif







void UpdateAvatarPartPreviewMesh(FAvatarPart& Part)
{
#if WITH_EDITOR
	
#endif
}


void FVisualPrimitiveComponent::CopyDataFromOther(const UPrimitiveComponent* OtherComp)
{
	FVisualSceneComponent::CopyDataFromOther(OtherComp);
}

void FVisualPrimitiveComponent::CopyDataToOther(UPrimitiveComponent* OtherComp, AActor* CompOwner) const
{
	check(OtherComp);
	FVisualSceneComponent::CopyDataToOther(OtherComp, CompOwner);
	//所有的ornament默认都没有碰撞
	OtherComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void FVisualStaticMeshComponent::CopyDataFromOther(const UStaticMeshComponent* OtherComp)
{
	if (!OtherComp)
		return;

	FVisualPrimitiveComponent::CopyDataFromOther(OtherComp);

	StaticMesh = OtherComp->GetStaticMesh();
}

void FVisualStaticMeshComponent::CopyDataToOther(UStaticMeshComponent* OtherComp, AActor* CompOwner) const
{
	if (!OtherComp)
		return;

	FVisualPrimitiveComponent::CopyDataToOther(OtherComp, CompOwner);

	if (UStaticMesh* CurMesh = StaticMesh.LoadSynchronous())
		OtherComp->SetStaticMesh(CurMesh);
	else
		OtherComp->SetHiddenInGame(true);

	for (TMap<int32, TSoftObjectPtr<UMaterialInterface>>::TConstIterator It(OverrideMaterials); It; ++It)
	{
		if (It->Key >= OtherComp->GetNumMaterials())
			continue;

		UMaterialInterface* CurMatInterface = It->Value.IsNull() ? nullptr : It->Value.LoadSynchronous();
		if (!CurMatInterface)
			continue;

		OtherComp->SetMaterial(It->Key, CurMatInterface);
	}
}



void FVisualSkeletalMeshComponent::CopyDataFromOther(const USkeletalMeshComponent* OtherComp)
{
	if (!OtherComp)
		return;

	FVisualPrimitiveComponent::CopyDataFromOther(OtherComp);

	SkeletalMesh = OtherComp->GetSkinnedAsset();
	AnimationMode = OtherComp->GetAnimationMode();
	//AnimationClass = OtherComp->GetAnimClass();
}

void FVisualSkeletalMeshComponent::CopyDataToOther(USkeletalMeshComponent* OtherComp, AActor* CompOwner) const
{
	if (!OtherComp)
		return;

	FVisualPrimitiveComponent::CopyDataToOther(OtherComp, CompOwner);

	if (USkeletalMesh* CurMesh = SkeletalMesh.LoadSynchronous())
		OtherComp->SetSkeletalMesh(CurMesh);
	else
		OtherComp->SetHiddenInGame(true);

	OtherComp->SetAnimationMode(AnimationMode);

	OtherComp->SetBoundsScale(BoundsScale);

	// if (AnimationMode == EAnimationMode::AnimationBlueprint)
	// 	OtherComp->SetAnimClass(AnimationClass);
	// else if(AnimationAsset != nullptr)
	// 	OtherComp->SetAnimation(AnimationAsset);

	for (TMap<int32, TSoftObjectPtr<UMaterialInterface>>::TConstIterator It(OverrideMaterials); It; ++It)
	{
		if (It->Key >= OtherComp->GetNumMaterials())
			continue;

		UMaterialInterface* CurMatInterface = It->Value.IsNull() ? nullptr : It->Value.LoadSynchronous();
		if (!CurMatInterface)
			continue;

		OtherComp->SetMaterial(It->Key, CurMatInterface);
	}

	// 默认关闭骨骼的Overlap
	OtherComp->bUpdateOverlapsOnAnimationFinalize = false;
}

void FVisualParticleSystemComponent::CopyDataFromOther(const UParticleSystemComponent* OtherComp)
{
	if (!OtherComp)
		return;

	FVisualPrimitiveComponent::CopyDataFromOther(OtherComp);

	ParticleSystem = OtherComp->Template;
}

void FVisualParticleSystemComponent::CopyDataToOther(UParticleSystemComponent* OtherComp, AActor* CompOwner) const
{
	if (!OtherComp)
		return;

	FVisualPrimitiveComponent::CopyDataToOther(OtherComp, CompOwner);

	if (UParticleSystem* Particle = ParticleSystem.LoadSynchronous())
		OtherComp->SetTemplate(Particle);
}

#if WITH_EDITOR
void FAvatarPart::OnPreEditChange()
{
	MaterialFlag.Empty();
	for (const FAvatarMaterialInfo& MaterialData : Materials)
	{
		MaterialFlag.Add(MaterialData.Material);
	}
}

void FAvatarPart::OnPostEditChange(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == FName(TEXT("SkeletalMesh")))
	{
		//改变了Mesh，重新扫描材质信息
		UAvatarMakeData::CopyAvatarMaterialParameter(SkeletalMesh, Materials);
	}
	else if (PropertyChangedEvent.GetPropertyName() == FName(TEXT("Material")))
	{
		for (int i = 0; i < Materials.Num(); ++i)
		{
			if (Materials[i].Material != MaterialFlag[i])
			{
				//改变了材质，重新扫描材质参数信息
				UAvatarMakeData::UpdateMaterialParameter(Materials[i]);
				break;
			}
		}
	}
}
#endif
