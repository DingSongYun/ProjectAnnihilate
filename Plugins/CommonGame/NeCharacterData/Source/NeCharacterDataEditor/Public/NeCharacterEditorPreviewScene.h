// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewScene.h"
#include "EditorUndoClient.h"
#include "AdvancedPreviewSceneModule.h"

using FDetailCustomizationInfo = FAdvancedPreviewSceneModule::FDetailCustomizationInfo;
using FPropertyTypeCustomizationInfo = FAdvancedPreviewSceneModule::FPropertyTypeCustomizationInfo;

class FNeCharacterEditor;

class FNeCharacterEditorPreviewScene : public FAdvancedPreviewScene, public FEditorUndoClient
{
public:
	FNeCharacterEditorPreviewScene(ConstructionValues CVS);
	virtual ~FNeCharacterEditorPreviewScene();

	//~ Begin: FPreviewScene  interface
	void Tick(float InDeltaTime);
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const { return ETickableTickType::Always; }
	//~ End: FPreviewScene  interface

	/** 创建预览角色 */
	void CreatePreviewActors(class UNeCharacterAsset* CharacterAsset);

	/** 销毁预览角色 */
	void DestroyPreviewActors();

	TArray<AActor*> GetPreviewActors() { return PreviewActors; }

private:
	double LastTickTime;
	TArray<AActor*> PreviewActors;

	// 辅助灯光，只给角色使用
	class UDirectionalLightComponent* SubDirectionalLight;
};
