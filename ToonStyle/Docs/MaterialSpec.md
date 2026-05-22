# ToonStyle Material Spec

Phase: 1C

This document is the text source of truth for the binary ToonStyle material assets created in `/Game/ToonStyle/Materials`.

## Shader Mapping

The project shader source directory is registered during T66 module startup:

- Virtual path: `/Project/ToonStyle`
- Disk path: `C:/UE/T66/ToonStyle/Shaders/Public`

Material Custom nodes include `.ush` files through the Custom node `IncludeFilePaths` property, not inline `#include` text in the node body. This keeps functions outside Unreal's generated Custom expression function wrapper and makes the include dependency visible to cook.

## M_Toon_Character

Path: `/Game/ToonStyle/Materials/M_Toon_Character`

Settings:

- Shading model: Unlit
- Blend mode: Opaque
- Two sided: false
- Output: Emissive Color

Parameters:

- `BaseColorTexture` Texture2D
- `TintTexture` Texture2D
- `InnerLineTexture` Texture2D, default `/Game/ToonStyle/Textures/T_InnerLines_DefaultBlack`
- `InnerLineColor` Vector, default black
- `InnerLineStrength` Scalar, default `0.5`
- `LightDirection` Vector
- `RampStep1` Scalar
- `RampStep2` Scalar
- `ShadeColor` Vector
- `MidtoneColor` Vector
- `LitColor` Vector, default `(0.85, 0.85, 0.85, 1.0)`
- `RimColor` Vector
- `RimPower` Scalar
- `RimStrength` Scalar, default `0.21`

Graph:

- Texture sample `BaseColorTexture`.
- Texture sample `TintTexture`, default neutral grey at the instance level.
- Texture sample `InnerLineTexture`, default black no-op texture for assets without baked line content.
- `InnerLineTexture.R` feeds the inner-line mask.
- Pixel normal world space into `N`.
- Camera vector world space into `V`.
- Vertex color R into `ThresholdOffset`.
- Custom node includes `/Project/ToonStyle/ToonShadingCommon.ush`.
- Custom node calls `ToonCharacterShade(...)`.
- `TintTexture` is floored with `max(TintColor, float3(0.4, 0.4, 0.4))` before the shader function call.
- Character diffuse contribution is `(BaseColor * TintColor * 1.5 * Ramp) + (RimColor * Rim)`.
- Inner line composition is alpha-blended overlay inside `ToonCharacterShade`: `lerp(LitOutput, InnerLineColor, saturate(InnerLineMask * InnerLineStrength))`. A black default texture therefore evaluates to `LineAlpha=0` and is a visible no-op.
- Custom output goes to Emissive Color.

## M_Toon_Environment

Path: `/Game/ToonStyle/Materials/M_Toon_Environment`

Settings:

- Shading model: Unlit
- Blend mode: Opaque
- Two sided: false
- Output: Emissive Color

Parameters:

- `BaseColorTexture` Texture2D
- `UVTileU` Scalar
- `UVTileV` Scalar
- `bUseWorldSpaceUVs` Static Switch, default `false`
- `ProjectionAxes` Vector, default `(1, 1, 0, 0)`
- `WorldSpaceTileSize` Scalar, default `100.0`
- `LightDirection` Vector
- `RampStep1` Scalar
- `RampStep2` Scalar
- `EnvShadeColor` Vector
- `EnvMidtoneColor` Vector
- `EnvLitColor` Vector

Graph:

- Texture coordinate U and V are split, multiplied independently by `UVTileU` and `UVTileV`, then appended back into tiled UVs.
- Static switch `bUseWorldSpaceUVs=false` preserves the legacy mesh-UV path above. This is the default so existing test-room material instances remain unchanged.
- Static switch `bUseWorldSpaceUVs=true` uses planar world-space UVs from `AbsoluteWorldPosition` divided by `WorldSpaceTileSize`.
- `ProjectionAxes` selects the projection plane for world-space UVs:
  - `(1, 1, 0)` = floor/ceiling projection onto XY, ignoring Z.
  - `(1, 0, 1)` = XZ wall projection, ignoring Y. Use for walls that run along X with normals along Y.
  - `(0, 1, 1)` = YZ wall projection, ignoring X. Use for walls that run along Y with normals along X.
- Final UV samples `BaseColorTexture`.
- Pixel normal world space into `N`.
- Custom node includes `/Project/ToonStyle/ToonShadingCommon.ush`.
- Custom node calls `ToonEnvironmentShade(...)`.
- Custom output goes to Emissive Color.

## M_Toon_Character_Outline

Path: `/Game/ToonStyle/Materials/M_Toon_Character_Outline`

Settings:

- Shading model: Unlit
- Blend mode: Opaque
- Two sided: false
- Output: Emissive Color, World Position Offset

Parameters:

- `OutlineColor` Vector
- `OutlineBaseWidth` Scalar
- `OutlineReferenceDistance` Scalar
- `OutlineFOVTanHalf` Scalar
- `OutlineReferenceFOVTanHalf` Scalar
- `OutlineDepthOffsetScalar` Scalar, default `2.5`
- `OutlineWidth` Scalar, legacy compatibility alias set by C++ but not consumed by the production graph

Graph:

- The outline mesh has reversed face winding authored by the Phase 1C Blender pipeline. The material is one-sided opaque, so normal backface culling handles the inverted-hull visibility without `TwoSidedSign` or an opacity mask.
- `CameraDistance` is computed in the material graph as `Length(WorldPosition - CameraPositionWS)`.
- `EffectiveOutlineWidth = OutlineBaseWidth * VertexColor.G * (CameraDistance / OutlineReferenceDistance) * (OutlineFOVTanHalf / OutlineReferenceFOVTanHalf)`.
- `VertexNormalWS * EffectiveOutlineWidth` provides the normal-direction inverted-hull extrusion. The outline mesh preserves geometric normals because it is duplicated before face-normal transfer.
- `ViewDirToCamera` is computed as `normalize(CameraPositionWS - WorldPosition)`.
- `DepthOffsetWPO = -ViewDirToCamera * VertexColor.B * OutlineDepthOffsetScalar`.
- The final WPO is `NormalExtrusionWPO + DepthOffsetWPO`.
- `VertexColor.A` is not consumed in WPO. The mask behavior is folded into `VertexColor.G`; authored `G=0` means no outline extrusion at that vertex.
- The WPO path is graph-only; Custom HLSL nodes are not used for world/camera position math.
- WPO output goes to World Position Offset.
- `OutlineColor` goes to Emissive Color.

## Vertex Color Channel Layout

- **R**: lighting threshold offset, AO-derived and used by `M_Toon_Character`.
- **G**: outline width multiplier. Mask functionality is folded into this channel, so `G=0` means no outline extrusion at that vertex.
- **B**: outline depth offset, modulating the backface push-away amount described in Motomura slide 55.
- **A**: unused in production.

This layout differs from Arc System Works' canonical assignments, which use A for width. Arc's UE4 pipeline modified the engine source to support multi-pass shaders (Motomura inverse-hull lecture slide 32) and likely retains alpha through the WPO-stage derivative path via those modifications. T66 does not modify engine source. UE5.7's stock derivative-aware shading path strips alpha from material expressions feeding into WPO; `Saved/Codex/ToonStyle/Phase1C/Diagnostic2/Phase1C_SM6_Diagnostic_Report.md` Investigation 2 captured the literal V1/V2 cook failures. The channel assignments therefore differ while the channel purposes match Arc's canonical three-control inverted-hull architecture: lighting threshold, outline width, and outline depth offset.

Notes:

- Phase 1C replaces the Phase 1B two-sided masked workaround with the agreed reversed-winding outline mesh path.
- Phase 1C R1 restores camera-distance and FOV outline scaling after Diagnostic2 proved the prior cooked SM6 failure was not a general rejection of graph `WorldPosition` / `CameraPositionWS` WPO math.
- Phase 1C R1 keeps world/camera math out of project Custom nodes and uses the D3-proven graph-distance path.

