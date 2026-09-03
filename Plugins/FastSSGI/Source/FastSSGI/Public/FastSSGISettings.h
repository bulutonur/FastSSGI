#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FastSSGITypes.h"
#include "FastSSGISettings.generated.h"

/** Default controls for the Forward Shading compatible custom SSGI renderer. */
UCLASS(Config=Engine, DefaultConfig, meta=(DisplayName="FastSSGI"))
class FASTSSGI_API UFastSSGISettings final : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFastSSGISettings();

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(Config, EditAnywhere, Category="General")
	bool bEnabled = true;

	UPROPERTY(Config, EditAnywhere, Category="Quality")
	EFastSSGIResolution Resolution = EFastSSGIResolution::Full;

	UPROPERTY(Config, EditAnywhere, Category="Quality", meta=(ClampMin="1", ClampMax="16", UIMin="1", UIMax="16"))
	int32 Samples = 8;

	UPROPERTY(Config, EditAnywhere, Category="Ray March", meta=(ClampMin="2", ClampMax="32", UIMin="4", UIMax="32"))
	int32 Steps = 12;

	/** Maximum 3D ray distance in meters. */
	UPROPERTY(Config, EditAnywhere, Category="Ray March", meta=(ClampMin="0.1", ClampMax="10.0", UIMin="0.1", UIMax="10.0", Units="m"))
	float RayMarchRadius = 1.5f;

	UPROPERTY(Config, EditAnywhere, Category="Ray March", meta=(ClampMin="0.0", ClampMax="10.0"))
	float Intensity = 1.0f;
	
	UPROPERTY(Config, EditAnywhere, Category="Temporal", meta=(ClampMin="0.0", ClampMax="0.98"))
	float HistoryWeight = 0.9f;

	UPROPERTY(Config, EditAnywhere, Category="Blur")
	EFastSSGIBlurQuality BlurQuality = EFastSSGIBlurQuality::Low;
	
	UPROPERTY(Config, EditAnywhere, Category="Blur", meta=(ClampMin="0.5", ClampMax="10.0", UIMin="0.5", UIMax="10.0"))
	float BlurRadius = 1.0f;
	
	UPROPERTY(Config, EditAnywhere, Category="Debug")
	EFastSSGIDebugMode DebugMode = EFastSSGIDebugMode::Off;

private:
	void ApplyToConsoleVariables() const;
};