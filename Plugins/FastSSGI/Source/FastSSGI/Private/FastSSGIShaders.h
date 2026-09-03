#pragma once

#include "GlobalShader.h"
#include "SceneView.h"
#include "ScreenPass.h"
#include "ShaderParameterStruct.h"

class FFastSSGIShader : public FGlobalShader
{
public:
	FFastSSGIShader() = default;
	FFastSSGIShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
};

class FFastSSGIRayMarchPS final : public FFastSSGIShader
{
	DECLARE_GLOBAL_SHADER(FFastSSGIRayMarchPS);
	SHADER_USE_PARAMETER_STRUCT(FFastSSGIRayMarchPS, FFastSSGIShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER(FVector4f, SceneUVScaleBias)
		SHADER_PARAMETER(FVector2f, OutputInvSize)
		SHADER_PARAMETER(int32, SampleCount)
		SHADER_PARAMETER(int32, StepCount)
		SHADER_PARAMETER(float, RayMarchRadius)
		SHADER_PARAMETER(int32, DebugMode)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColorTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SceneColorSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SceneDepthSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};

class FFastSSGITemporalPS final : public FFastSSGIShader
{
	DECLARE_GLOBAL_SHADER(FFastSSGITemporalPS);
	SHADER_USE_PARAMETER_STRUCT(FFastSSGITemporalPS, FFastSSGIShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER(FVector4f, SceneUVScaleBias)
		SHADER_PARAMETER(FVector2f, InvSize)
		SHADER_PARAMETER(float, HistoryWeight)
		SHADER_PARAMETER(uint32, HasHistory)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, CurrentTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SceneDepthSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};

class FFastSSGIDenoiseGuidePS final : public FFastSSGIShader
{
	DECLARE_GLOBAL_SHADER(FFastSSGIDenoiseGuidePS);
	SHADER_USE_PARAMETER_STRUCT(FFastSSGIDenoiseGuidePS, FFastSSGIShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER(FVector4f, SceneUVScaleBias)
		SHADER_PARAMETER(FVector2f, InvSize)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, PointSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};

class FFastSSGIAtrousDenoisePS final : public FFastSSGIShader
{
	DECLARE_GLOBAL_SHADER(FFastSSGIAtrousDenoisePS);
	SHADER_USE_PARAMETER_STRUCT(FFastSSGIAtrousDenoisePS, FFastSSGIShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector2f, InvSize)
		SHADER_PARAMETER(int32, StepWidth)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GuideTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, PointSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};

class FFastSSGICompositePS final : public FFastSSGIShader
{
	DECLARE_GLOBAL_SHADER(FFastSSGICompositePS);
	SHADER_USE_PARAMETER_STRUCT(FFastSSGICompositePS, FFastSSGIShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER(FVector4f, SceneUVScaleBias)
		SHADER_PARAMETER(float, Intensity)
		SHADER_PARAMETER(int32, DebugMode)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColorTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, RayMarchTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, TemporalTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, DenoisedTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepthTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, BilinearSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, PointSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};