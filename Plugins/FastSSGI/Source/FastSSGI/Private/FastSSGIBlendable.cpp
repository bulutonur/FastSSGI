#include "FastSSGIBlendable.h"

#include "HAL/IConsoleManager.h"
#include "SceneView.h"

namespace
{
	float GetConsoleVariableValue(const TCHAR* Name, const float DefaultValue)
	{
		if (const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			return Variable->GetFloat();
		}
		return DefaultValue;
	}
}

const FName& FFastSSGIBlendableData::GetFName()
{
	static const FName Name(TEXT("FFastSSGIBlendableData"));
	return Name;
}

void FFastSSGIBlendableData::SetBaseValues()
{
	Enabled = GetConsoleVariableValue(TEXT("r.FastSSGI.Enable"), 1.0f);
	Resolution = GetConsoleVariableValue(TEXT("r.FastSSGI.Resolution"), 0.0f);
	Samples = GetConsoleVariableValue(TEXT("r.FastSSGI.Samples"), 8.0f);
	Steps = GetConsoleVariableValue(TEXT("r.FastSSGI.Steps"), 12.0f);
	RayMarchRadius = GetConsoleVariableValue(TEXT("r.FastSSGI.RayMarchRadius"), 1.5f);
	Intensity = GetConsoleVariableValue(TEXT("r.FastSSGI.Intensity"), 1.0f);
	HistoryWeight = GetConsoleVariableValue(TEXT("r.FastSSGI.HistoryWeight"), 0.9f);
	BlurQuality = GetConsoleVariableValue(TEXT("r.FastSSGI.BlurQuality"), 0.0f);
	BlurRadius = GetConsoleVariableValue(TEXT("r.FastSSGI.BlurRadius"), 1.5f);
	DebugMode = GetConsoleVariableValue(TEXT("r.FastSSGI.Debug"), 0.0f);
}

void FFastSSGIBlendableData::Override(const FFastSSGIBlendableData& Values, const float Weight)
{
	Enabled = FMath::Lerp(Enabled, Values.Enabled, Weight);
	Resolution = FMath::Lerp(Resolution, Values.Resolution, Weight);
	Samples = FMath::Lerp(Samples, Values.Samples, Weight);
	Steps = FMath::Lerp(Steps, Values.Steps, Weight);
	RayMarchRadius = FMath::Lerp(RayMarchRadius, Values.RayMarchRadius, Weight);
	Intensity = FMath::Lerp(Intensity, Values.Intensity, Weight);
	HistoryWeight = FMath::Lerp(HistoryWeight, Values.HistoryWeight, Weight);
	BlurQuality = FMath::Lerp(BlurQuality, Values.BlurQuality, Weight);
	BlurRadius = FMath::Lerp(BlurRadius, Values.BlurRadius, Weight);
	DebugMode = FMath::Lerp(DebugMode, Values.DebugMode, Weight);
}

void UFastSSGIBlendable::OverrideBlendableSettings(FSceneView& View, const float Weight) const
{
	check(IsInGameThread());
	check(Weight > 0.0f && Weight <= 1.0f);

	FBlendableManager& Manager = View.FinalPostProcessSettings.BlendableManager;
	FFastSSGIBlendableData& FinalValues = Manager.GetSingleFinalData<FFastSSGIBlendableData>();
	FinalValues.Override(GetValues(), Weight);
}

FFastSSGIBlendableData UFastSSGIBlendable::GetValues() const
{
	FFastSSGIBlendableData Values;
	Values.Enabled = bEnabled ? 1.0f : 0.0f;
	Values.Resolution = static_cast<float>(Resolution);
	Values.Samples = static_cast<float>(Samples);
	Values.Steps = static_cast<float>(Steps);
	Values.RayMarchRadius = RayMarchRadius;
	Values.Intensity = Intensity;
	Values.HistoryWeight = HistoryWeight;
	Values.BlurQuality = static_cast<float>(BlurQuality);
	Values.BlurRadius = BlurRadius;
	Values.DebugMode = static_cast<float>(DebugMode);
	return Values;
}