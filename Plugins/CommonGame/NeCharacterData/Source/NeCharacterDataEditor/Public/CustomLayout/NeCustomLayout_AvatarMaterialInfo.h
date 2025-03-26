// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "Widgets/SWidget.h"

class FNeCustomLayout_AvatarMaterialInfo : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FNeCustomLayout_AvatarMaterialInfo());
	}

	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	// End of IPropertyTypeCustomization interface

	struct FAvatarMaterialInfo* GetStruct(TSharedPtr<IPropertyHandle> InPropertyHandle);
	TSharedRef<SWidget> GetComboContent_MaterialParameters(class UMaterialInterface* MaterialInterface);

	void AddScalarParameter(FName ParameterName, float Value);
	void AddVectorParameter(FName ParameterName, FLinearColor Value);
	void AddTextureParameter(FName ParameterName, class UTexture* Value);
	void RemoveParameter(FName ParameterName);

	void OnMaterialValuePostChange();
protected:
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;
};
