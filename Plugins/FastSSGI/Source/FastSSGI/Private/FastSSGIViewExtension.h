#pragma once

#include "SceneViewExtension.h"

struct IPooledRenderTarget;
struct FPostProcessingInputs;

class FFastSSGIViewExtension final : public FSceneViewExtensionBase
{
public:
	explicit FFastSSGIViewExtension(const FAutoRegister& AutoRegister);
	virtual ~FFastSSGIViewExtension() override;

	virtual void PrePostProcessPass_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessingInputs& Inputs) override;

private:
	struct FViewHistory
	{
		TRefCountPtr<IPooledRenderTarget> Texture;
		FIntPoint Extent = FIntPoint::ZeroValue;
		uint64 LastFrame = 0;
	};

	// Heap allocation keeps QueueTextureExtraction's destination address stable if the map grows for another view.
	TMap<uint64, TUniquePtr<FViewHistory>> Histories;
};