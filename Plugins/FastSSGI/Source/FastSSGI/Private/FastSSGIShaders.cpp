#include "FastSSGIShaders.h"

IMPLEMENT_GLOBAL_SHADER(FFastSSGIRayMarchPS, "/Plugin/FastSSGI/Private/RayMarch.usf", "RayMarchPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FFastSSGITemporalPS, "/Plugin/FastSSGI/Private/Temporal.usf", "TemporalPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FFastSSGIDenoiseGuidePS, "/Plugin/FastSSGI/Private/Denoise.usf", "DenoiseGuidePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FFastSSGIAtrousDenoisePS, "/Plugin/FastSSGI/Private/Denoise.usf", "AtrousDenoisePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FFastSSGICompositePS, "/Plugin/FastSSGI/Private/Composite.usf", "CompositePS", SF_Pixel);