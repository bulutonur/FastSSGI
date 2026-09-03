# FastSSGI

FastSSGI is a runtime Unreal Engine plugin that adds configurable screen-space global illumination without modifying engine source code. It uses a scene view extension for rendering and Unreal's blendable interface for per-view configuration through Post Process Volumes.

The plugin is currently developed against **Unreal Engine 5.8** and requires an SM5-capable rendering platform.

## Features

- Forward Shading compability
- Configurable through bounded or unbound Post Process Volumes
- Correct volume priority, blend radius, blend weight, and weighted-blendable behavior
- Full, half, or quarter-resolution ray marching
- Temporal reprojection and accumulation
- Depth-aware firefly suppression before samples enter temporal history
- Edge-aware À-trous wavelet denoising
- Per-volume indirect intensity and color tint
- Per-view temporal history keyed by view state and stereo index
- Ray-march, temporal, denoised, depth, and generated-normal debug views
- No engine source changes

## Installation

1. Copy `Plugins/FastSSGI` into the `Plugins` directory of an Unreal Engine project.
2. Enable **FastSSGI** in the Plugins window if it is not already enabled.
3. Restart the editor when prompted and compile the project/plugin for the target engine version.

The plugin module loads during `PostConfigInit` so its global shaders can be registered. The scene view extension is created after engine initialization.

## Post Process Volume Setup

FastSSGI settings are stored in a **FastSSGI Post Process Settings** Data Asset and assigned as a Post Process Volume blendable.

1. In the Content Browser, create a **Data Asset**.
2. Select **FastSSGI Post Process Settings** as the Data Asset class.
3. Configure the FastSSGI properties on the new asset.
4. Add or select a **Post Process Volume** in the level.
5. Open **Rendering Features > Post Process Materials** on the volume.
6. Add an element to the weighted blendables array and assign the FastSSGI asset.
7. Configure the volume's priority, blend radius, blend weight, and **Infinite Extent (Unbound)** option as required.

Multiple volumes may overlap. FastSSGI settings follow Unreal's standard post-process blending order and are resolved independently for each rendered view.

> The weighted-blendable element weight is multiplied by the Post Process Volume's spatial blend weight. Keep the element weight at `1.0` for normal volume behavior.

## Settings

### General

| Setting | Default | Description |
| --- | ---: | --- |
| Enabled | On | Enables FastSSGI. When blended toward disabled, indirect intensity fades with the volume weight. |

### Quality

| Setting | Default | Description |
| --- | ---: | --- |
| Resolution | Full | Internal SSGI resolution: Full, Half, or Quarter. Lower resolutions improve performance. |
| Samples | 8 | Number of hemisphere rays per internal-resolution pixel. Exposed range: 1–16. |

### Ray March

| Setting | Default | Description |
| --- | ---: | --- |
| Steps | 12 | Maximum ray-march steps per ray. Exposed range: 2–32. |
| Ray March Radius | 1.5 m | Maximum screen-space ray distance in world space. |
| Intensity | 1.0 | Multiplier applied to the final indirect contribution. |
| Indirect Color | White | Linear color multiplier applied during final composition. It does not affect temporal accumulation or denoising. |

### Temporal

| Setting | Default | Description |
| --- | ---: | --- |
| History Weight | 0.9 | Contribution of valid reprojected history. Range: 0–0.98. Lower values react faster; higher values reduce noise. |

Temporal accumulation reprojects history through Unreal's current-to-previous clip transform and clamps it to the current neighborhood. A depth-aware neighborhood test suppresses isolated high-luminance fireflies before they enter history.

### Denoise

| Setting | Default | Description |
| --- | ---: | --- |
| Denoise Quality | Low | Controls the number and reach of À-trous wavelet iterations. |

Quality levels map to the following filter passes:

| Quality | Iterations | Step widths |
| --- | ---: | --- |
| Low | 1 | 1 |
| Medium | 2 | 1, 2 |
| High | 3 | 1, 2, 4 |

The denoiser uses a reusable normal and logarithmic-depth guide plus luminance edge stopping to reduce noise without indiscriminately blurring across silhouettes and surface discontinuities.

### Debug

| Mode | Value | Output |
| --- | ---: | --- |
| Off | 0 | Normal FastSSGI composition |
| Ray March | 1 | Raw ray-marched indirect lighting |
| Temporal Accumulation | 2 | Firefly-filtered temporal result before À-trous denoising |
| Denoised GI | 3 | Final À-trous-filtered indirect lighting |
| Scene Depth | 4 | Normalized scene-depth visualization |
| Generated Normal | 5 | Normal reconstructed from scene depth |

Debug modes intentionally show untinted GI. **Indirect Color** and **Intensity** are applied only during normal composition.

## Rendering Pipeline

```text
Scene color + scene depth
    -> screen-space ray march
    -> depth-aware firefly suppression
    -> temporal reprojection and neighborhood clamping
    -> normal/log-depth guide generation
    -> 1–3 edge-aware À-trous iterations
    -> indirect color and intensity
    -> scene-color composition
```

The final denoised texture is retained as per-view history for the next frame. History is rejected after camera cuts, resolution changes, or when it becomes stale.

## Performance Guidance

- Start with **Half** resolution, 8 samples, 12 steps, and Low denoise quality.
- Reduce samples first when ray marching is the bottleneck.
- Reduce steps when long rays spend too much time traversing the screen.
- Quarter resolution offers the largest pixel-cost reduction but can lose thin geometry and fine indirect detail.
- Medium and High denoise quality add full-screen 5×5 À-trous passes at the selected internal resolution.
- A high history weight reduces temporal noise but can react more slowly to lighting changes.

Use Unreal Insights, GPU Visualizer, or RenderDoc to evaluate the appropriate settings for the target content and hardware.

## Limitations

- FastSSGI is screen-space: off-screen geometry and lighting are unavailable.
- Disoccluded areas have no valid temporal history and may appear noisier temporarily.
- Thin geometry, large depth discontinuities, and low internal resolutions can produce missing or unstable intersections.
- It adds indirect lighting to the current scene color and is not a replacement for a complete off-screen GI solution.
- The current implementation and renderer-internal include path target Unreal Engine 5.8; other engine versions may require API adjustments.

## Source Layout

```text
Plugins/FastSSGI/
├── Shaders/Private/
│   ├── Common.ush
│   ├── RayMarch.usf
│   ├── Temporal.usf
│   ├── Denoise.usf
│   └── Composite.usf
└── Source/FastSSGI/
    ├── Public/
    │   ├── FastSSGI.h
    │   ├── FastSSGIBlendable.h
    │   └── FastSSGITypes.h
    └── Private/
        ├── FastSSGI.cpp
        ├── FastSSGIBlendable.cpp
        ├── FastSSGIShaders.cpp
        ├── FastSSGIShaders.h
        ├── FastSSGIViewExtension.cpp
        └── FastSSGIViewExtension.h
```
