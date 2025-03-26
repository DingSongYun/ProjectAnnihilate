// Copyright NetEase Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

/*******************************************************/
// FNeCharacterEdMode
/*******************************************************/
class FNeCharacterEdMode : public FApplicationMode
{
public:
	FNeCharacterEdMode(const FName& InModeName, TSharedRef<class FNeCharacterEditor> InHostingApp);
	virtual ~FNeCharacterEdMode() {}
	// ~Begin: FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) final override;
	virtual void AddTabFactory(FCreateWorkflowTabFactory FactoryCreator) final override;
	virtual void RemoveTabFactory(FName TabFactoryID) final override;
	// ~End: FApplicationMode interface

	/** 创建当前Mode的Tab */
	virtual void CreateModeTabs(const TSharedRef<FNeCharacterEditor> HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories) = 0;

protected:
	TWeakPtr<class FNeCharacterEditor> HostingAppPtr;
	FWorkflowAllowedTabSet TabFactories;
};




/*******************************************************/
// FCharacterEdMode_TemplateAsset
/*******************************************************/
class FCharacterEdMode_TemplateAsset : public FNeCharacterEdMode
{
public:
	FCharacterEdMode_TemplateAsset(TSharedRef<class FNeCharacterEditor> InHostingApp);
	virtual void CreateModeTabs(const TSharedRef<FNeCharacterEditor> HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories) override;
};




/*******************************************************/
// FCharacterEdMode_AvatarMake
/*******************************************************/
class FCharacterEdMode_AvatarMake : public FNeCharacterEdMode
{
public:
	FCharacterEdMode_AvatarMake(TSharedRef<class FNeCharacterEditor> InHostingApp);
	virtual void CreateModeTabs(const TSharedRef<FNeCharacterEditor> HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories) override;
};








/*******************************************************/
// FCharacterEdMode_FacialAnim
/*******************************************************/
/*class FCharacterEdMode_FacialAnim : public FNeCharacterEdMode
{
public:
	FCharacterEdMode_FacialAnim(TSharedRef<class FCharacterEditor> InHostingApp);
	virtual void CreateModeTabs(const TSharedRef<FCharacterEditor> HostingAppPt, FWorkflowAllowedTabSet& OutTabFactories) override;
};*/
