// Copyright (c) 2016 - 2018 Extra Life Studios, LLC. All rights reserved.

#pragma once

#include "Framework/MultiBox/MultiBoxBuilder.h"

class FNeCharacterEditor;

class FNeCharacterEditorToolbar : public TSharedFromThis<FNeCharacterEditorToolbar>
{
public:
	void SetupToolbar(TSharedPtr<FExtender> Extender, TSharedPtr<FNeCharacterEditor> InCharacterEditor);

	void AddHelperToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillHelperToolbar(FToolBarBuilder& ToolbarBuilder);

private:
	TWeakPtr<FNeCharacterEditor> CharacterEditor;
};


