// Copyright NetEase Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "NeCharacterEditor.h"
#include "NeCharacterEditorPreviewSettings.generated.h"

/**
 * UNeCharacterEditorPreviewSetting
 */
UCLASS(config=Editor, BlueprintType, Blueprintable)
class UNeCharacterEditorPreviewSetting : public UObject
{
	GENERATED_BODY()
public:
	UNeCharacterEditorPreviewSetting(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{}

	UPROPERTY(EditAnywhere, transient, BlueprintReadWrite, Category = "Viewport")
	FVector DefaultViewLocation = FVector(300.f, 100.f, 0.f);

	UPROPERTY(EditAnywhere, transient, BlueprintReadWrite, Category = "Viewport")
	FRotator DefaultViewRotation;

	UPROPERTY(EditAnywhere, transient, BlueprintReadWrite, Category = "Viewport")
	bool bDefaultViewTransformRelateCharacter = true;
};

/**
 * FCharacterPrevSettingsCustomization
 */
class FCharacterPrevSettingsCustomization : public IDetailCustomization
{
public:
	FCharacterPrevSettingsCustomization(FNeCharacterEditor* EditorPtr) : CharacterEd(EditorPtr), PrevSetting(nullptr) {}

	static TSharedRef<IDetailCustomization> MakeInstance(const FNeCharacterEditor* EditorPtr)
	{
		return MakeShareable(new FCharacterPrevSettingsCustomization(const_cast<FNeCharacterEditor*>(EditorPtr)));
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override
	{
		for (const TWeakObjectPtr<UObject>&Obj : DetailBuilder.GetSelectedObjects())
		{
			if (UNeCharacterEditorPreviewSetting* Anim = Cast<UNeCharacterEditorPreviewSetting>(Obj.Get()))
			{
				PrevSetting = Anim;
				break;
			}
		}
		check(PrevSetting.IsValid());
	}

private:
	FNeCharacterEditor* CharacterEd = nullptr;
	TWeakObjectPtr<UNeCharacterEditorPreviewSetting> PrevSetting = nullptr;
};

