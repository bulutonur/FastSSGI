#pragma once

#include "CoreMinimal.h"
#include "FastSSGITypes.generated.h"

UENUM(BlueprintType)
enum class EFastSSGIResolution : uint8
{
	Full UMETA(DisplayName = "Full Resolution"),
	Half UMETA(DisplayName = "Half Resolution"),
	Quarter UMETA(DisplayName = "Quarter Resolution")
};

UENUM(BlueprintType)
enum class EFastSSGIBlurQuality : uint8
{
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High")
};

UENUM(BlueprintType)
enum class EFastSSGIDebugMode : uint8
{
	Off,
	RayMarch UMETA(DisplayName = "Ray March"),
	Temporal UMETA(DisplayName = "Temporal Accumulation"),
	Denoised UMETA(DisplayName = "Denoised GI"),
	Depth UMETA(DisplayName = "Scene Depth"),
	GeneratedNormal UMETA(DisplayName = "Generated Normal")
};