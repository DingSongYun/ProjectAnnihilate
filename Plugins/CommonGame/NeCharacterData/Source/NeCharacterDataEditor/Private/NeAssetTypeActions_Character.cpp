// Copyright NetEase Games, Inc. All Rights Reserved.
#include "NeCharacterDataSettings.h"
#include "NeCharacterEditor.h"
#include "AssetTypeActions/NeAssetTypeActions_Character.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_Character"

/**********************************************/
// FNeAssetTypeActions_CharacterAsset
/**********************************************/
FText FNeAssetTypeActions_CharacterAsset::GetName() const
{
	return LOCTEXT("FNeAssetTypeActions_CharacterAsset", "Ne Character Asset");
}
#undef LOCTEXT_NAMESPACE
