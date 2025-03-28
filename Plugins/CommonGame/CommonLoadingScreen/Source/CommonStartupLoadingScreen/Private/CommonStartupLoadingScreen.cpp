// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "MoviePlayer.h"
#include "PreLoadScreenManager.h"
#include "Misc/App.h"
#include "CommonPreLoadScreen.h"

#define LOCTEXT_NAMESPACE "FCommonLoadingScreenModule"

class FCommonStartupLoadingScreenModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	bool IsGameModule() const override;

private:
	void OnPreLoadScreenManagerCleanUp();

	TSharedPtr<FCommonPreLoadScreen> PreLoadingScreen;
	TSharedPtr<FCommonPreLoadScreen> EarlyStartupScreen;
	TSharedPtr<FCommonPreLoadScreen> SplashScreen;
};


void FCommonStartupLoadingScreenModule::StartupModule()
{
	// No need to load these assets on dedicated servers.
	// Still want to load them in commandlets so cook catches them
	if (!IsRunningDedicatedServer())
	{
		SplashScreen = MakeShared<FCommonPreLoadScreen>();
		SplashScreen->Init();

		EarlyStartupScreen = MakeShared<FCommonPreLoadScreen>();
		EarlyStartupScreen->Init();

		PreLoadingScreen = MakeShared<FCommonPreLoadScreen>();
		PreLoadingScreen->Init();

		if (!GIsEditor && FApp::CanEverRender() && FPreLoadScreenManager::Get())
		{
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(SplashScreen);
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(EarlyStartupScreen);
			FPreLoadScreenManager::Get()->RegisterPreLoadScreen(PreLoadingScreen);
			
			FPreLoadScreenManager::Get()->OnPreLoadScreenManagerCleanUp.AddRaw(this, &FCommonStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp);
		}
	}
}

void FCommonStartupLoadingScreenModule::OnPreLoadScreenManagerCleanUp()
{
	//Once the PreLoadScreenManager is cleaning up, we can get rid of all our resources too
	SplashScreen.Reset();
	EarlyStartupScreen.Reset();
	PreLoadingScreen.Reset();
	ShutdownModule();
}

void FCommonStartupLoadingScreenModule::ShutdownModule()
{

}

bool FCommonStartupLoadingScreenModule::IsGameModule() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonStartupLoadingScreenModule, CommonStartupLoadingScreen)