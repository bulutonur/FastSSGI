#include "FastSSGI.h"
#include "FastSSGISettings.h"
#include "FastSSGIViewExtension.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FFastSSGIModule"

void FFastSSGIModule::StartupModule()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FastSSGI"));
	checkf(Plugin.IsValid(), TEXT("FastSSGI plugin descriptor could not be found."));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/FastSSGI"), Plugin->GetBaseDir() / TEXT("Shaders"));
	
	// PostConfigInit is required for global shader mappings, but it is too early to create
	// UObject class default objects. Load the settings after UEngine and packages are initialized.
	PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddRaw(
		this,
		&FFastSSGIModule::OnPostEngineInit);
}

void FFastSSGIModule::ShutdownModule()
{
	if (PostEngineInitHandle.IsValid())
	{
		FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
		PostEngineInitHandle.Reset();
	}
	ViewExtension.Reset();

}

void FFastSSGIModule::OnPostEngineInit()
{
	check(GEngine);
	check(GEngine->ViewExtensions.IsValid());

	// Construct settings after CVars have registered so config values are pushed to render-safe CVars.
	GetMutableDefault<UFastSSGISettings>();
	ViewExtension = FSceneViewExtensions::NewExtension<FFastSSGIViewExtension>();

	FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
	PostEngineInitHandle.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFastSSGIModule, FastSSGI)