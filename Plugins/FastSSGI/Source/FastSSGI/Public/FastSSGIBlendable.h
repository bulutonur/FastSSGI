#pragma once

#include "CoreMinimal.h"
#include "Engine/BlendableInterface.h"
#include "Engine/DataAsset.h"
#include "FastSSGITypes.h"
#include "FastSSGIBlendable.generated.h"

/** Render-thread-safe copy of the FastSSGI settings blended for one view. */
struct FFastSSGIBlendableData
{
	float Enabled = 1.0f;
	float Resolution = 0.0f;
	float Samples = 8.0f;
	float Steps = 12.0f;
	float RayMarchRadius = 1.5f;
	float Intensity = 1.0f;
	float HistoryWeight = 0.9f;
	float BlurQuality = 0.0f;
	float BlurRadius = 1.0f;
	float DebugMode = 0.0f;

	static const FName& GetFName();
	void SetBaseValues();
	void Override(const FFastSSGIBlendableData& Values, float Weight);
};

/**
 * FastSSGI settings that can be assigned to a Post Process Volume's
 * Rendering Features > Post Process Materials array.
 */
UCLASS(BlueprintType, meta=(DisplayName="FastSSGI Post Process Settings"))
class FASTSSGI_API UFastSSGIBlendable final : public UDataAsset, public IBlendableInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quality")
	EFastSSGIResolution Resolution = EFastSSGIResolution::Full;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quality", meta=(ClampMin="1", ClampMax="16", UIMin="1", UIMax="16"))
	int32 Samples = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ray March", meta=(ClampMin="2", ClampMax="32", UIMin="4", UIMax="32"))
	int32 Steps = 12;

	/** Maximum 3D ray distance in meters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ray March", meta=(ClampMin="0.1", ClampMax="10.0", UIMin="0.1", UIMax="10.0", Units="m"))
	float RayMarchRadius = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ray March", meta=(ClampMin="0.0", ClampMax="10.0"))
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Temporal", meta=(ClampMin="0.0", ClampMax="0.98"))
	float HistoryWeight = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Blur")
	EFastSSGIBlurQuality BlurQuality = EFastSSGIBlurQuality::Low;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Blur", meta=(ClampMin="0.5", ClampMax="10.0", UIMin="0.5", UIMax="10.0"))
	float BlurRadius = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	EFastSSGIDebugMode DebugMode = EFastSSGIDebugMode::Off;

	virtual void OverrideBlendableSettings(FSceneView& View, float Weight) const override;

private:
	FFastSSGIBlendableData GetValues() const;
};