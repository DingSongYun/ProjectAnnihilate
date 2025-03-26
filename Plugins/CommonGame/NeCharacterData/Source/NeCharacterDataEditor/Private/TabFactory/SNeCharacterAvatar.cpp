// Copyright NetEase Games, Inc. All Rights Reserved.

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "DetailLayoutBuilder.h"
#include "ISinglePropertyView.h"
#include "IStructureDetailsView.h"
#include "NeAvatarData.h"
#include "NeCharacterEditor.h"
#include "PropertyEditorModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Modules/ModuleManager.h"
#include "TabFactory/SNeCharacterAvatar.h"
#include "Widgets/Input/SComboButton.h"

void SNeCharacterAvatar::Construct(const FArguments& InArgs, TWeakObjectPtr<UObject> InPrototypeAsset, const TSharedPtr<FNeCharacterEditor>& InHostEdPtr)
{
	PrototypeAsset = InPrototypeAsset;
	check(PrototypeAsset.IsValid());

	EditorPtr = InHostEdPtr;
	// 当角色发生改变时，需要通知该面板
	if (EditorPtr.IsValid())
		EditorPtr->OnRefreshActor.AddSP(this, &SNeCharacterAvatar::PrepareMeshLink);

	if (!GetAvatarInfo())
		return;

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding( 3.0f, 2.0f )
		[
			CreateAvatarInfoTypePicker()
		]
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding( 3.0f, 2.0f )
		[
			SAssignNew(DetailContainer, SBorder)
		]
	];

	SetDetailStruct(FAvatarInfo::StaticStruct(), (uint8*)GetAvatarInfo());

	PrepareMeshLink();
}

void SNeCharacterAvatar::SetDetailStruct(const UStruct* InScriptStruct, uint8* InData)
{
	if (!StructureDetailView.IsValid())
	{
		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		{
			DetailsViewArgs.bAllowSearch = true;
			DetailsViewArgs.bHideSelectionTip = true;
			DetailsViewArgs.bLockable = false;
			DetailsViewArgs.bSearchInitialKeyFocus = true;
			DetailsViewArgs.NotifyHook = this;
			DetailsViewArgs.bShowOptions = false;
			DetailsViewArgs.bShowModifiedPropertiesOption = false;
		}

		FStructureDetailsViewArgs StructDetailsViewArgs;
		{
			StructDetailsViewArgs.bShowAssets = true;
			StructDetailsViewArgs.bShowClasses = true;
			StructDetailsViewArgs.bShowInterfaces = true;
			StructDetailsViewArgs.bShowObjects = true;
		}
		StructureDetailView = EditModule.CreateStructureDetailView(DetailsViewArgs, StructDetailsViewArgs, nullptr);
	}

	StructureDetailView->SetStructureData(MakeShareable(new FStructOnScope(InScriptStruct, InData)));
	DetailContainer->ClearContent();
	DetailContainer->SetContent(StructureDetailView->GetWidget().ToSharedRef());
}

void SNeCharacterAvatar::SetDetailObject(UObject* InObject)
{
	if (!DetailView.IsValid())
	{
		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		{
			DetailsViewArgs.bAllowSearch = true;
			DetailsViewArgs.bHideSelectionTip = true;
			DetailsViewArgs.bLockable = false;
			DetailsViewArgs.bSearchInitialKeyFocus = true;
			DetailsViewArgs.NotifyHook = this;
			DetailsViewArgs.bShowOptions = false;
			DetailsViewArgs.bShowModifiedPropertiesOption = false;
		}
		DetailView = EditModule.CreateDetailView(DetailsViewArgs);
	}
	DetailView->SetObject(InObject);
	DetailContainer->ClearContent();
	DetailContainer->SetContent(DetailView.ToSharedRef());
}

void SNeCharacterAvatar::SetDetailProperty(UObject* InObject, FName InPropertyName)
{
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FSinglePropertyParams PropertyParams;
	{
		PropertyParams.NotifyHook = this;
	}
	PropertyView = EditModule.CreateSingleProperty(InObject, InPropertyName, PropertyParams);
	DetailContainer->ClearContent();
	DetailContainer->SetContent(PropertyView.ToSharedRef());
}

class FAvatarInfoFilter : public IClassViewerFilter
{
public:
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		return  InClass->IsChildOf(UAvatarMakeData::StaticClass()) 
			&& !InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const class IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< class FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return InUnloadedClassData->IsChildOf(UAvatarMakeData::StaticClass()) 
			&& !InUnloadedClassData->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated | CLASS_Abstract);;
	}
};

TSharedRef<SWidget> SNeCharacterAvatar::CreateAvatarInfoTypePicker()
{
	FClassViewerInitializationOptions InitOptions;
	InitOptions.Mode = EClassViewerMode::ClassPicker;
	InitOptions.bShowNoneOption = true;
	InitOptions.bShowObjectRootClass = false;
	InitOptions.bShowUnloadedBlueprints = true;
	InitOptions.bEnableClassDynamicLoading = true;
	InitOptions.bExpandRootNodes = true;
	InitOptions.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	// InitOptions.ClassFilter = MakeShareable(new FAvatarInfoFilter());
	InitOptions.ClassFilters.Add(MakeShareable(new FAvatarInfoFilter()));
	InitOptions.bShowBackgroundBorder = false;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	TSharedRef<SWidget> ClassPicker = ClassViewerModule.CreateClassViewer
	(
		InitOptions,
		FOnClassPicked::CreateLambda
		(
			[this](UClass* InClass)
			{
				UAvatarMakeData** MakeData = &this->GetAvatarInfo()->MakeData;
				UClass* MakeDataClass = *MakeData ? (*MakeData)->GetClass() : nullptr;

				// 切换了Avatar类型，需要重新生成预览角色
				if (InClass && MakeDataClass != InClass)
				{
					*MakeData = InClass ? NewObject<UAvatarMakeData>(this->PrototypeAsset.Get(), InClass) : nullptr;
					this->OnChangeAvatarType();
				}

				FSlateApplication::Get().DismissAllMenus();
			}
		)
	);
	return
		SNew(SComboButton)
		.ContentPadding(2)
		.MenuContent()
		[
			ClassPicker
		]
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &SNeCharacterAvatar::GetAvatarInfoTypeName)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];
}

FText SNeCharacterAvatar::GetAvatarInfoTypeName() const
{
	UAvatarMakeData* MakeData = GetAvatarInfo()->MakeData;
	return MakeData ? MakeData->GetClass()->GetDisplayNameText() : FText::FromString("");
}

void SNeCharacterAvatar::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
}

void SNeCharacterAvatar::OnChangeAvatarType()
{
	if (!EditorPtr.IsValid())
		return;

	EditorPtr->RefreshPreviewActor();

}

void SNeCharacterAvatar::PrepareMeshLink()
{
	if (!GetAvatarInfo())
		return;

	UAvatarMakeData* MakeData = GetAvatarInfo()->MakeData;

	if (MakeData && EditorPtr.IsValid())
		MakeData->SetPreviewContext(EditorPtr->GetPreviewActor());

}

void SNeCharacterAvatar::RefreshCharacterAvatar()
{
	if (!EditorPtr.IsValid())
		return;
}

FAvatarInfo* SNeCharacterAvatar::GetAvatarInfo() const
{
	//if (UCharacterPrototype* CurPrototype = Cast<UCharacterPrototype>(PrototypeAsset))
	//	return &CurPrototype->AvatarInfo;

	return NULL;
}


