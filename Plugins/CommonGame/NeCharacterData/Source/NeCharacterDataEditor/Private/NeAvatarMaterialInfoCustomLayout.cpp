// Copyright NetEase Games, Inc. All Rights Reserved.

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "NeAvatarMaterialInfo.h"
#include "PropertyCustomizationHelpers.h"
#include "CustomLayout/NeCustomLayout_AvatarMaterialInfo.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Editor/MaterialEditor/Public/MaterialEditorModule.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"

void FNeCustomLayout_AvatarMaterialInfo::CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyHandle = InPropertyHandle;
	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();

	FAvatarMaterialInfo* MaterialInfoPtr = GetStruct(PropertyHandle);

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(FText::FromString(MaterialInfoPtr->SlotName.ToString()))
		]
	];
}

void FNeCustomLayout_AvatarMaterialInfo::CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FAvatarMaterialInfo* pStructData = GetStruct(PropertyHandle);
	//IDetailGroup& SlotGroup = ChildBuilder.AddGroup(TEXT("TestGroup"), FText::FromString(TEXT("TestGroup")));

	
	auto Icon = FAppStyle::GetBrush("EditableComboBox.Add");
	FDetailWidgetRow& NewRow = ChildBuilder.AddCustomRow(FText::FromString(TEXT("AddParameter")));
	NewRow
		.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SComboButton)
			.ButtonContent()
		[
			SNew(SImage)
			.Image(Icon)
		]
		.MenuContent()
		[
			GetComboContent_MaterialParameters(GetStruct(PropertyHandle)->Material.LoadSynchronous())
		]
		]
		]
	.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(pStructData->SlotName.ToString()))
		]
		];
	//Material Index
	{
		TSharedPtr<IPropertyHandle> ChildPropertyHandle = InPropertyHandle->GetChildHandle(TEXT("MaterialIndex"));
		IDetailPropertyRow& PropertyRow = ChildBuilder.AddProperty(ChildPropertyHandle.ToSharedRef());
		TSharedPtr<SWidget> NameWidget, ValueWidget;
		PropertyRow.GetDefaultWidgets(NameWidget, ValueWidget);
		PropertyRow.CustomWidget()
			.NameContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				NameWidget.ToSharedRef()
			]
			]
		.ValueContent()
			[
				ValueWidget.ToSharedRef()
			];
	}
	//Material
	{
		TSharedPtr<IPropertyHandle> ChildPropertyHandle = InPropertyHandle->GetChildHandle(TEXT("Material"));

		FSimpleDelegate OnValueChangeDelegate = FSimpleDelegate::CreateSP(this, &FNeCustomLayout_AvatarMaterialInfo::OnMaterialValuePostChange);
		ChildPropertyHandle->SetOnPropertyValueChanged(OnValueChangeDelegate);

		IDetailPropertyRow& PropertyRow = ChildBuilder.AddProperty(ChildPropertyHandle.ToSharedRef());
		TSharedPtr<SWidget> NameWidget, ValueWidget;
		PropertyRow.GetDefaultWidgets(NameWidget, ValueWidget);
		PropertyRow.CustomWidget()
			.NameContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.AutoWidth()
			[
				NameWidget.ToSharedRef()
			]
			]
		.ValueContent()
			[
				ValueWidget.ToSharedRef()
			];
	}

	//float parameter
	{
		TSharedPtr<IPropertyHandle> ChildPropertyHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAvatarMaterialInfo, MaterialParamsScalar));
		for (int32 i = 0; i < pStructData->MaterialParamsScalar.Num(); i++)
		{
			TSharedPtr<IPropertyHandle> ChildSubPropertyHandle = ChildPropertyHandle->GetChildHandle(i);

			if (ChildSubPropertyHandle.IsValid())
			{
				TSharedPtr<IPropertyHandle> ValueHandle = ChildSubPropertyHandle->GetChildHandle(TEXT("Value"));
				IDetailPropertyRow& PropertyRow = ChildBuilder.AddProperty(ValueHandle.ToSharedRef());
				PropertyRow.DisplayName(FText::FromString(pStructData->MaterialParamsScalar[i].ParamName.ToString()));
				TSharedPtr<SWidget> NameWidget, ValueWidget;
				PropertyRow.GetDefaultWidgets(NameWidget, ValueWidget);
				PropertyRow.CustomWidget()
					.NameContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						PropertyCustomizationHelpers::MakeDeleteButton(
							FSimpleDelegate::CreateSP(this, &FNeCustomLayout_AvatarMaterialInfo::RemoveParameter, pStructData->MaterialParamsScalar[i].ParamName))
					]
				+ SHorizontalBox::Slot()
						+ SHorizontalBox::Slot()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						NameWidget.ToSharedRef()
					]
					]
				.ValueContent()
					[
						ValueWidget.ToSharedRef()
					];

			}
		}
	}

	//vector parameter
	{
		TSharedPtr<IPropertyHandle> ChildPropertyHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAvatarMaterialInfo, MaterialParamsVector));
		for (int32 i = 0; i < pStructData->MaterialParamsVector.Num(); i++)
		{
			TSharedPtr<IPropertyHandle> ChildSubPropertyHandle = ChildPropertyHandle->GetChildHandle(i);

			if (ChildSubPropertyHandle.IsValid())
			{
				TSharedPtr<IPropertyHandle> ValueHandle = ChildSubPropertyHandle->GetChildHandle(TEXT("Value"));
				IDetailPropertyRow& PropertyRow = ChildBuilder.AddProperty(ValueHandle.ToSharedRef());
				PropertyRow.DisplayName(FText::FromString(pStructData->MaterialParamsVector[i].ParamName.ToString()));
				TSharedPtr<SWidget> NameWidget, ValueWidget;
				PropertyRow.GetDefaultWidgets(NameWidget, ValueWidget);
				PropertyRow.CustomWidget()
					.NameContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						PropertyCustomizationHelpers::MakeDeleteButton(
							FSimpleDelegate::CreateSP(this, &FNeCustomLayout_AvatarMaterialInfo::RemoveParameter, pStructData->MaterialParamsVector[i].ParamName))
					]
						+ SHorizontalBox::Slot()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						NameWidget.ToSharedRef()
					]
					]
				.ValueContent()
					[
						ValueWidget.ToSharedRef()
					];

			}
		}
	}

	//texture parameter
	{
		TSharedPtr<IPropertyHandle> ChildPropertyHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAvatarMaterialInfo, MaterialParamsTexture));
		for (int32 i = 0; i < pStructData->MaterialParamsTexture.Num(); i++)
		{
			TSharedPtr<IPropertyHandle> ChildSubPropertyHandle = ChildPropertyHandle->GetChildHandle(i);

			if (ChildSubPropertyHandle.IsValid())
			{
				TSharedPtr<IPropertyHandle> ValueHandle = ChildSubPropertyHandle->GetChildHandle(TEXT("Value"));
				IDetailPropertyRow& PropertyRow = ChildBuilder.AddProperty(ValueHandle.ToSharedRef());
				PropertyRow.DisplayName(FText::FromString(pStructData->MaterialParamsTexture[i].ParamName.ToString()));
				TSharedPtr<SWidget> NameWidget, ValueWidget;
				PropertyRow.GetDefaultWidgets(NameWidget, ValueWidget);
				PropertyRow.CustomWidget()
					.NameContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						PropertyCustomizationHelpers::MakeDeleteButton(
							FSimpleDelegate::CreateSP(this, &FNeCustomLayout_AvatarMaterialInfo::RemoveParameter, pStructData->MaterialParamsTexture[i].ParamName))
					]
						+ SHorizontalBox::Slot()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.AutoWidth()
					[
						NameWidget.ToSharedRef()
					]
					]
				.ValueContent()
					[
						ValueWidget.ToSharedRef()
					];

			}
		}
	}
}

FAvatarMaterialInfo* FNeCustomLayout_AvatarMaterialInfo::GetStruct(TSharedPtr<IPropertyHandle> InPropertyHandle)
{
	void* RawData = nullptr;
	InPropertyHandle->GetValueData(RawData);
	if (RawData)
	{
		return static_cast<FAvatarMaterialInfo*>(RawData);
	}
	return nullptr;
}
void FNeCustomLayout_AvatarMaterialInfo::OnMaterialValuePostChange()
{
	PropertyUtilities->ForceRefresh();
}

TSharedRef<SWidget> FNeCustomLayout_AvatarMaterialInfo::GetComboContent_MaterialParameters(UMaterialInterface* MaterialInterface)
{
	FMenuBuilder MenuBuilder(false, nullptr);
	
	if (MaterialInterface)
	{
		UMaterial* Material = MaterialInterface->GetMaterial();
		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(MaterialInterface, nullptr);
		TArray<FMaterialParameterInfo> VisibleExpressions;
		IMaterialEditorModule* MaterialEditorModule = &FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
		bool bCollectedVisibleParameters = false;
		if (MaterialEditorModule && MaterialInstance)
		{
			MaterialEditorModule->GetVisibleMaterialParameters(Material, MaterialInstance, VisibleExpressions);
			bCollectedVisibleParameters = true;
		}

		TMap<FString, FUIAction> MatParamAction;

		// Collect scalar parameter
		{
			TArray<FMaterialParameterInfo> ParameterInfo;
			TArray<FGuid> ParameterIds;
			Material->GetAllScalarParameterInfo(ParameterInfo, ParameterIds);
			for (int32 i = 0; i < ParameterInfo.Num(); ++i)
			{
				if (!GetStruct(PropertyHandle)->ContainsParameter(ParameterInfo[i].Name)
					&& VisibleExpressions.Contains(FMaterialParameterInfo(ParameterInfo[i].Name)))
				{
					FName ScalarParameterName = ParameterInfo[i].Name;
					float ParameterValue;
					Material->GetScalarParameterValue(ParameterInfo[i], ParameterValue);
					FUIAction AddParameterMenuAction(FExecuteAction::CreateSP(this, &FNeCustomLayout_AvatarMaterialInfo::AddScalarParameter, ScalarParameterName, ParameterValue));
					MatParamAction.Add(TEXT("(float)") + ScalarParameterName.ToString(), AddParameterMenuAction);
				}
			}
		}

		// Collect vector parameter
		{
			TArray<FMaterialParameterInfo> ParameterInfo;
			TArray<FGuid> ParameterIds;
			Material->GetAllVectorParameterInfo(ParameterInfo, ParameterIds);
			for (int32 i = 0; i < ParameterInfo.Num(); ++i)
			{
				if (!GetStruct(PropertyHandle)->ContainsParameter(ParameterInfo[i].Name)
					&& VisibleExpressions.Contains(FMaterialParameterInfo(ParameterInfo[i].Name)))
				{
					FName VectorParameterName = ParameterInfo[i].Name;
					FLinearColor ParameterValue;
					Material->GetVectorParameterValue(ParameterInfo[i], ParameterValue);
					FUIAction AddParameterMenuAction(FExecuteAction::CreateSP(this, &FNeCustomLayout_AvatarMaterialInfo::AddVectorParameter, VectorParameterName, ParameterValue));
					MatParamAction.Add(TEXT("(vector)") + VectorParameterName.ToString(), AddParameterMenuAction);
				}
			}
		}

		// Collect vector parameter
		{
			TArray<FMaterialParameterInfo> ParameterInfo;
			TArray<FGuid> ParameterIds;
			Material->GetAllTextureParameterInfo(ParameterInfo, ParameterIds);
			for (int32 i = 0; i < ParameterInfo.Num(); ++i)
			{
				if (!GetStruct(PropertyHandle)->ContainsParameter(ParameterInfo[i].Name)
					&& VisibleExpressions.Contains(FMaterialParameterInfo(ParameterInfo[i].Name)))
				{
					FName TextureParameterName = ParameterInfo[i].Name;
					UTexture* ParameterValue;
					Material->GetTextureParameterValue(ParameterInfo[i], ParameterValue);
					FUIAction AddParameterMenuAction(FExecuteAction::CreateSP(this, &FNeCustomLayout_AvatarMaterialInfo::AddTextureParameter, TextureParameterName, ParameterValue));
					MatParamAction.Add(TEXT("(texture)") + TextureParameterName.ToString() , AddParameterMenuAction);
				}
			}
		}

		TArray<FString> MatNames;
		MatParamAction.GetKeys(MatNames);
		//MatNames.Sort([](const FName& LHS, const FName& RHS) { return LHS.LexicalLess(RHS); });
		for (auto&  MatName : MatNames)
		{
			MenuBuilder.AddMenuEntry(FText::FromString(MatName), FText(), FSlateIcon(), MatParamAction[MatName]);
		}
	}
	
	return MenuBuilder.MakeWidget();
}


void FNeCustomLayout_AvatarMaterialInfo::AddScalarParameter(FName ParameterName, float Value)
{
	TSharedPtr<IPropertyHandle> SlotHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAvatarMaterialInfo, MaterialParamsScalar));
	SlotHandle->NotifyPreChange();
	GetStruct(PropertyHandle)->AddParameter(ParameterName, Value);
	SlotHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
	SlotHandle->NotifyFinishedChangingProperties();
	PropertyUtilities->ForceRefresh();
}

void FNeCustomLayout_AvatarMaterialInfo::AddVectorParameter(FName ParameterName, FLinearColor Value)
{
	TSharedPtr<IPropertyHandle> SlotHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAvatarMaterialInfo, MaterialParamsVector));
	SlotHandle->NotifyPreChange();
	GetStruct(PropertyHandle)->AddParameter(ParameterName, Value);
	SlotHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
	SlotHandle->NotifyFinishedChangingProperties();
	PropertyUtilities->ForceRefresh();
}

void FNeCustomLayout_AvatarMaterialInfo::AddTextureParameter(FName ParameterName, UTexture* Value)
{
	TSharedPtr<IPropertyHandle> SlotHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAvatarMaterialInfo, MaterialParamsTexture));
	SlotHandle->NotifyPreChange();
	GetStruct(PropertyHandle)->AddParameter(ParameterName, Value);
	SlotHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
	SlotHandle->NotifyFinishedChangingProperties();
	PropertyUtilities->ForceRefresh();
}
void FNeCustomLayout_AvatarMaterialInfo::RemoveParameter(FName ParameterName)
{
	//TSharedPtr<IPropertyHandle> SlotHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FAvatarMaterialInfo, MaterialParamsTexture));
	PropertyHandle->NotifyPreChange();
	GetStruct(PropertyHandle)->RemoveParameter(ParameterName);
	PropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayRemove);
	PropertyHandle->NotifyFinishedChangingProperties();
	PropertyUtilities->ForceRefresh();
}