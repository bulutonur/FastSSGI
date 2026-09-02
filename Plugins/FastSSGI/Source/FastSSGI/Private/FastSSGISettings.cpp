#include "FastSSGISettings.h"

#include "HAL/IConsoleManager.h"

namespace
{
	void SetCVar(const TCHAR* Name, const int32 Value)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			Variable->Set(Value, ECVF_SetByProjectSetting);
		}
	}

	void SetCVar(const TCHAR* Name, const float Value)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			Variable->Set(Value, ECVF_SetByProjectSetting);
		}
	}
}

UFastSSGISettings::UFastSSGISettings()
{
	SectionName = TEXT("FastSSGI");
}

void UFastSSGISettings::PostInitProperties()
{
	Super::PostInitProperties();
	ApplyToConsoleVariables();
}

#if WITH_EDITOR
void UFastSSGISettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyToConsoleVariables();
}
#endif

void UFastSSGISettings::ApplyToConsoleVariables() const
{
	SetCVar(TEXT("r.FastSSGI.Enable"), bEnabled ? 1 : 0);
	SetCVar(TEXT("r.FastSSGI.Resolution"), static_cast<int32>(Resolution));
	SetCVar(TEXT("r.FastSSGI.Samples"), Samples);
	SetCVar(TEXT("r.FastSSGI.Steps"), Steps);
	SetCVar(TEXT("r.FastSSGI.RayMarchRadius"), RayMarchRadius);
	SetCVar(TEXT("r.FastSSGI.Intensity"), Intensity);
	SetCVar(TEXT("r.FastSSGI.HistoryWeight"), HistoryWeight);
	SetCVar(TEXT("r.FastSSGI.BlurRadius"), BlurRadius);
	SetCVar(TEXT("r.FastSSGI.Debug"), static_cast<int32>(DebugMode));
}