#include "FastSSGIShaders.h"

IMPLEMENT_GLOBAL_SHADER(FFastSSGIRayMarchPS, "/Plugin/FastSSGI/Private/RayMarch.usf", "RayMarchPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FFastSSGITemporalPS, "/Plugin/FastSSGI/Private/Temporal.usf", "TemporalPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FFastSSGIBlurPS, "/Plugin/FastSSGI/Private/Blur.usf", "BlurPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FFastSSGICompositePS, "/Plugin/FastSSGI/Private/Composite.usf", "CompositePS", SF_Pixel);