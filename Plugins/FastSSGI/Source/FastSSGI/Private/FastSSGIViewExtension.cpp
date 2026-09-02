#include "FastSSGIViewExtension.h"

#include "FastSSGIShaders.h"
#include "PostProcess/PostProcessInputs.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ScreenPass.h"

namespace
{
	TAutoConsoleVariable<int32> CVarEnable(
		TEXT("r.FastSSGI.Enable"), 1,
		TEXT("Enables Fast SSGI. 0: off, 1: on."), ECVF_RenderThreadSafe);
	TAutoConsoleVariable<int32> CVarResolution(
		TEXT("r.FastSSGI.Resolution"), 1,
		TEXT("Internal resolution. 0: full, 1: half, 2: quarter."), ECVF_RenderThreadSafe);
	TAutoConsoleVariable<int32> CVarSamples(
		TEXT("r.FastSSGI.Samples"), 8,
		TEXT("Number of SSGI rays per pixel (1-32)."), ECVF_RenderThreadSafe);
	TAutoConsoleVariable<int32> CVarSteps(
		TEXT("r.FastSSGI.Steps"), 12,
		TEXT("Ray-march steps per SSGI ray (2-64)."), ECVF_RenderThreadSafe);
	TAutoConsoleVariable<float> CVarRadius(
		TEXT("r.FastSSGI.Radius"), 1.5f,
		TEXT("Maximum 3D SSGI ray distance in meters."), ECVF_RenderThreadSafe);
	TAutoConsoleVariable<float> CVarIntensity(
		TEXT("r.FastSSGI.Intensity"), 1.0f,
		TEXT("Indirect-light intensity."), ECVF_RenderThreadSafe);
	TAutoConsoleVariable<float> CVarHistoryWeight(
		TEXT("r.FastSSGI.HistoryWeight"), 0.9f,
		TEXT("Temporal history weight (0-0.98)."), ECVF_RenderThreadSafe);
	TAutoConsoleVariable<int32> CVarDebug(
		TEXT("r.FastSSGI.Debug"), 0,
		TEXT("Debug output. 0: off, 1: ray march, 2: temporal, 3: denoised, 4: depth, 5: generated normal."), ECVF_RenderThreadSafe);

	FRDGTextureRef CreateTarget(FRDGBuilder& GraphBuilder, FIntPoint Extent, const TCHAR* Name)
	{
		const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
			Extent,
			PF_FloatRGBA,
			FClearValueBinding::Black,
			TexCreate_ShaderResource | TexCreate_RenderTargetable);
		return GraphBuilder.CreateTexture(Desc, Name);
	}

	FRDGTextureRef CreateCompositeTarget(FRDGBuilder& GraphBuilder, const FRDGTextureDesc& SceneColorDesc)
	{
		FRDGTextureDesc Desc = SceneColorDesc;
		Desc.ClearValue = FClearValueBinding::Black;
		Desc.Flags |= TexCreate_ShaderResource | TexCreate_RenderTargetable;
		Desc.Flags &= ~(TexCreate_UAV | TexCreate_FastVRAM | TexCreate_ResolveTargetable);
		return GraphBuilder.CreateTexture(Desc, TEXT("FastSSGI.Composite"));
	}

	template <typename TPixelShader>
	void DrawPass(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		FRDGEventName&& EventName,
		FRDGTextureRef Output,
		FIntRect OutputRect,
		FIntPoint InputExtent,
		typename TPixelShader::FParameters* Parameters)
	{
		const FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(View.GetFeatureLevel());
		TShaderMapRef<FScreenPassVS> VertexShader(ShaderMap);
		TShaderMapRef<TPixelShader> PixelShader(ShaderMap);
		Parameters->RenderTargets[0] = FRenderTargetBinding(Output, ERenderTargetLoadAction::EClear);

		AddDrawScreenPass(
			GraphBuilder,
			MoveTemp(EventName),
			View,
			FScreenPassTextureViewport(Output, OutputRect),
			FScreenPassTextureViewport(InputExtent),
			VertexShader,
			PixelShader,
			Parameters);
	}
}

FFastSSGIViewExtension::FFastSSGIViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
{
}

FFastSSGIViewExtension::~FFastSSGIViewExtension() = default;

void FFastSSGIViewExtension::PrePostProcessPass_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessingInputs& Inputs)
{
	check(IsInRenderingThread());
	if (CVarEnable.GetValueOnRenderThread() == 0 || !Inputs.SceneTextures || !View.Family)
	{
		return;
	}

	FRDGTextureRef SceneColor = (*Inputs.SceneTextures)->SceneColorTexture;
	FRDGTextureRef SceneDepth = (*Inputs.SceneTextures)->SceneDepthTexture;
	if (!SceneColor || !SceneDepth)
	{
		return;
	}

	// During a viewport resize, the view rect and scene texture allocation can be updated on
	// different frames. Keep every draw and copy inside the texture that is valid this frame.
	const FIntPoint SceneColorExtent = SceneColor->Desc.Extent;
	const FIntRect RequestedSceneRect = View.UnscaledViewRect;
	const FIntRect SceneRect(
		FMath::Clamp(RequestedSceneRect.Min.X, 0, SceneColorExtent.X),
		FMath::Clamp(RequestedSceneRect.Min.Y, 0, SceneColorExtent.Y),
		FMath::Clamp(RequestedSceneRect.Max.X, 0, SceneColorExtent.X),
		FMath::Clamp(RequestedSceneRect.Max.Y, 0, SceneColorExtent.Y));
	if (SceneRect.IsEmpty())
	{
		return;
	}

	const int32 ResolutionMode = FMath::Clamp(CVarResolution.GetValueOnRenderThread(), 0, 2);
	const int32 Divisor = 1 << ResolutionMode;
	const FIntPoint InternalExtent(
		FMath::DivideAndRoundUp(SceneRect.Width(), Divisor),
		FMath::DivideAndRoundUp(SceneRect.Height(), Divisor));
	const FIntRect InternalRect(FIntPoint::ZeroValue, InternalExtent);
	const FVector2f SceneExtent(static_cast<float>(SceneColorExtent.X), static_cast<float>(SceneColorExtent.Y));
	const FVector4f SceneUVScaleBias(
		static_cast<float>(SceneRect.Width()) / SceneExtent.X,
		static_cast<float>(SceneRect.Height()) / SceneExtent.Y,
		static_cast<float>(SceneRect.Min.X) / SceneExtent.X,
		static_cast<float>(SceneRect.Min.Y) / SceneExtent.Y);

	FRDGTextureRef RayMarch = CreateTarget(GraphBuilder, InternalExtent, TEXT("FastSSGI.RayMarch"));
	auto* RayParameters = GraphBuilder.AllocParameters<FFastSSGIRayMarchPS::FParameters>();
	RayParameters->View = View.ViewUniformBuffer;
	RayParameters->SceneUVScaleBias = SceneUVScaleBias;
	RayParameters->OutputInvSize = FVector2f(1.0f / InternalExtent.X, 1.0f / InternalExtent.Y);
	RayParameters->SampleCount = FMath::Clamp(CVarSamples.GetValueOnRenderThread(), 1, 32);
	RayParameters->StepCount = FMath::Clamp(CVarSteps.GetValueOnRenderThread(), 2, 64);
	RayParameters->Radius = FMath::Clamp(CVarRadius.GetValueOnRenderThread(), 0.1f, 10.0f) * 100.0f;
	RayParameters->DebugMode = FMath::Clamp(CVarDebug.GetValueOnRenderThread(), 0, 5);
	RayParameters->SceneColorTexture = SceneColor;
	RayParameters->SceneColorSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp>::GetRHI();
	RayParameters->SceneDepthTexture = SceneDepth;
	RayParameters->SceneDepthSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
	DrawPass<FFastSSGIRayMarchPS>(GraphBuilder, View, RDG_EVENT_NAME("CustomSSGI Ray March (%d rays x %d steps)", RayParameters->SampleCount, RayParameters->StepCount), RayMarch, InternalRect, InternalExtent, RayParameters);

	auto AddBlurPass = [&](FRDGTextureRef Input, const FVector2f Direction, const TCHAR* Name)
	{
		FRDGTextureRef Output = CreateTarget(GraphBuilder, InternalExtent, Name);
		auto* Parameters = GraphBuilder.AllocParameters<FFastSSGIBlurPS::FParameters>();
		Parameters->Direction = Direction;
		Parameters->InvSize = FVector2f(1.0f / InternalExtent.X, 1.0f / InternalExtent.Y);
		Parameters->InputTexture = Input;
		Parameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
		DrawPass<FFastSSGIBlurPS>(GraphBuilder, View, RDG_EVENT_NAME("%s", Name), Output, InternalRect, InternalExtent, Parameters);
		return Output;
	};
/*
	auto AddDenoisePass = [&](FRDGTextureRef Input, const TCHAR* Name)
	{
		FRDGTextureRef Output = CreateTarget(GraphBuilder, InternalExtent, Name);
		auto* Parameters = GraphBuilder.AllocParameters<FCustomSSGIDenoisePS::FParameters>();
		Parameters->View = View.ViewUniformBuffer;
		Parameters->SceneUVScaleBias = SceneUVScaleBias;
		Parameters->InvSize = FVector2f(1.0f / InternalExtent.X, 1.0f / InternalExtent.Y);
		Parameters->OutputInvSize = FVector2f(1.0f / InternalExtent.X, 1.0f / InternalExtent.Y);
		Parameters->DenoiseStrength = FMath::Clamp(CVarDenoiseStrength.GetValueOnRenderThread(), 1.0f, 5.0f);
		Parameters->InputTexture = Input;
		Parameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
		Parameters->SceneDepthTexture = SceneDepth;
		Parameters->SceneDepthSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
		DrawPass<FCustomSSGIDenoisePS>(GraphBuilder, View, RDG_EVENT_NAME("%s", Name), Output, InternalRect, InternalExtent, Parameters);
		return Output;
	};

	FRDGTextureRef BlurredRayMarch = RayMarch;
	if (CVarRayMarchBlur.GetValueOnRenderThread() != 0)
	{
		const int32 BlurRadius = FMath::Clamp(CVarRayMarchBlurRadius.GetValueOnRenderThread(), 1, 5);
		const float BlurSigma = FMath::Clamp(CVarRayMarchBlurSigma.GetValueOnRenderThread(), 0.25f, 5.0f);
		FRDGTextureRef BlurHorizontal = AddBlurPass(RayMarch, FVector2f(1.0f, 0.0f), BlurRadius, BlurSigma, TEXT("CustomSSGI Ray March Blur Horizontal"));
		BlurredRayMarch = AddBlurPass(BlurHorizontal, FVector2f(0.0f, 1.0f), BlurRadius, BlurSigma, TEXT("CustomSSGI Ray March Blur Vertical"));
	}
*/
	
	FRDGTextureRef BlurHorizontal = AddBlurPass(RayMarch, FVector2f(1.0f, 0.0f), TEXT("FastSSGI Blur Horizontal"));
	FRDGTextureRef BlurVertical = AddBlurPass(BlurHorizontal, FVector2f(0.0f, 1.0f), TEXT("FastSSGI Blur Vertical"));
	
	const uint64 ViewKey = HashCombineFast(
		PointerHash(View.State),
		GetTypeHash(View.StereoViewIndex));
	TUniquePtr<FViewHistory>& HistoryEntry = Histories.FindOrAdd(ViewKey);
	if (!HistoryEntry)
	{
		HistoryEntry = MakeUnique<FViewHistory>();
	}
	FViewHistory& History = *HistoryEntry;
	const uint64 CurrentFrame = static_cast<uint64>(View.Family->FrameNumber);
	const bool bHistorySizeMatches = History.Texture.IsValid() && History.Extent == InternalExtent;
	// FrameNumber belongs to the view family, not to this extension. Editor viewports and
	// multiple view families can therefore skip values between two renders of the same view.
	// Requiring an exact +1 discarded otherwise valid history and repeatedly restarted accumulation.
	const bool bHistoryRecent = CurrentFrame > History.LastFrame && CurrentFrame - History.LastFrame <= 120;
	const bool bHasHistory = bHistorySizeMatches && bHistoryRecent && !View.bCameraCut;
	FRDGTextureRef HistoryTexture = bHasHistory
		? GraphBuilder.RegisterExternalTexture(History.Texture, TEXT("FastSSGI.History"))
		: BlurVertical;

	FRDGTextureRef Temporal = CreateTarget(GraphBuilder, InternalExtent, TEXT("FastSSGI.Temporal"));
	auto* TemporalParameters = GraphBuilder.AllocParameters<FFastSSGITemporalPS::FParameters>();
	TemporalParameters->View = View.ViewUniformBuffer;
	TemporalParameters->SceneUVScaleBias = SceneUVScaleBias;
	TemporalParameters->InvSize = FVector2f(1.0f / InternalExtent.X, 1.0f / InternalExtent.Y);
	TemporalParameters->HistoryWeight = FMath::Clamp(CVarHistoryWeight.GetValueOnRenderThread(), 0.0f, 0.98f);
	TemporalParameters->HasHistory = bHasHistory ? 1u : 0u;
	TemporalParameters->CurrentTexture = BlurVertical;
	TemporalParameters->HistoryTexture = HistoryTexture;
	TemporalParameters->InputSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp>::GetRHI();
	TemporalParameters->SceneDepthTexture = SceneDepth;
	TemporalParameters->SceneDepthSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
	DrawPass<FFastSSGITemporalPS>(GraphBuilder, View, RDG_EVENT_NAME("FastSSGI Temporal Accumulation"), Temporal, InternalRect, InternalExtent, TemporalParameters);

	//FRDGTextureRef Denoised = AddDenoisePass(Temporal, TEXT("FastSSGI Denoise Horizontal"));

	// Persist the actual final denoise output. QueueTextureExtraction extends this RDG texture's
	// lifetime and assigns the pooled target after graph execution, so no intermediate copy is needed.
	// The next frame feeds this exact texture through temporal accumulation and both denoise passes.
	History.Texture.SafeRelease();
	History.Extent = InternalExtent;
	History.LastFrame = CurrentFrame;
	GraphBuilder.QueueTextureExtraction(Temporal, &History.Texture);

	FRDGTextureRef Composite = CreateCompositeTarget(GraphBuilder, SceneColor->Desc);
	auto* CompositeParameters = GraphBuilder.AllocParameters<FFastSSGICompositePS::FParameters>();
	CompositeParameters->View = View.ViewUniformBuffer;
	CompositeParameters->SceneUVScaleBias = SceneUVScaleBias;
	CompositeParameters->Intensity = FMath::Max(CVarIntensity.GetValueOnRenderThread(), 0.0f);
	CompositeParameters->DebugMode = FMath::Clamp(CVarDebug.GetValueOnRenderThread(), 0, 5);
	CompositeParameters->SceneColorTexture = SceneColor;
	// Generated-normal debug output is authored by RayMarchPS but is not ray-marched radiance, so keep it unfiltered.
	CompositeParameters->RayMarchTexture = CompositeParameters->DebugMode == 5 ? RayMarch : BlurVertical;
	CompositeParameters->TemporalTexture = Temporal;
	CompositeParameters->DenoisedTexture = HistoryTexture;
	CompositeParameters->SceneDepthTexture = SceneDepth;
	CompositeParameters->BilinearSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp>::GetRHI();
	CompositeParameters->PointSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp>::GetRHI();
	DrawPass<FFastSSGICompositePS>(GraphBuilder, View, RDG_EVENT_NAME("FastSSGI %s", CompositeParameters->DebugMode == 0 ? TEXT("Composite") : TEXT("Debug")), Composite, SceneRect, InternalExtent, CompositeParameters);

	FRHICopyTextureInfo CopyInfo;
	CopyInfo.SourcePosition = FIntVector(SceneRect.Min.X, SceneRect.Min.Y, 0);
	CopyInfo.DestPosition = FIntVector(SceneRect.Min.X, SceneRect.Min.Y, 0);
	CopyInfo.Size = FIntVector(SceneRect.Width(), SceneRect.Height(), 1);
	AddCopyTexturePass(GraphBuilder, Composite, SceneColor, CopyInfo);

	// Remove stale editor/split-screen view histories without touching resources still in flight.
	for (auto It = Histories.CreateIterator(); It; ++It)
	{
		if (It.Value() && CurrentFrame > It.Value()->LastFrame + 120)
		{
			It.RemoveCurrent();
		}
	}
}