#pragma once

#include "Modules/ModuleManager.h"

class FFastSSGIViewExtension;
	
class FFastSSGIModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void OnPostEngineInit();
	FDelegateHandle PostEngineInitHandle;
	TSharedPtr<FFastSSGIViewExtension, ESPMode::ThreadSafe> ViewExtension;

};
