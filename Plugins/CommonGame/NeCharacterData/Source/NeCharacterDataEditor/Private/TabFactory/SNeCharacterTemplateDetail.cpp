#include "TabFactory/SNeCharacterTemplateDetail.h"
#include "NeCharacterAsset.h"

void SNeCharacterTemplateDetail::Construct(const FArguments& InArgs, TWeakObjectPtr<UObject> InTemplateAsset, const TSharedPtr<FNeCharacterEditor>& InHostEdPtr)
{
	TemplateAsset = InTemplateAsset;
	EditorPtr = InHostEdPtr;
	check(TemplateAsset.IsValid());

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
	TSharedRef<IDetailsView> DetailView = EditModule.CreateDetailView(DetailsViewArgs);
	DetailView->SetObject(TemplateAsset.Get());

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(3.0f, 2.0f)
		[
			DetailView
		]
	];
}

void SNeCharacterTemplateDetail::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	// const FName PropName = PropertyChangedEvent.Property->GetFName();
	EditorPtr->RefreshPreviewActor();
}
