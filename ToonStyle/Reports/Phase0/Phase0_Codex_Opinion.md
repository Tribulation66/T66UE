# Phase 0 Codex Opinion

This is the engineering critique of the ToonStyle plan after reading the live repo state. It is direct by design.

## Unlit Surface Shader Foundation

I agree with the direction for characters and controlled stylized assets, but the plan is currently under-specified for environments. T66 already has a large unlit GLB path and a lit environment path. Moving everything to unlit cel math is plausible, but it means the existing Dungeon torch lights and Default Lit environment material stop being physically meaningful unless the shader explicitly models those lighting cues. That is acceptable if intentional, but it should not be sold as a small material swap.

The bigger immediate problem is anti-aliasing. The project config currently has `r.AntiAliasingMethod=0`. Smoothstep cel transitions help inside the shader, but a native-resolution cel look still needs an AA policy. Phase 1 should not judge toon quality until AA is deliberately set and verified.

## `.ush` Files Plus Thin Material Shells

I agree with putting shader logic in text files. It is the right fit for Codex-driven iteration. However, the repo does not currently have a project shader directory, `.ush` files, `.usf` files, or `.uproject` shader-source registration. The proposed material include path will not work until that is added and verified.

This makes shader-directory registration a Phase 1 prerequisite, not an implementation detail. The first real task should be: register the directory, create a trivial included function, compile a minimal material shell, and prove the include path works in UE5.7 with Substrate enabled.

## No Engine Modification

I strongly agree. This project is already broad and operationally complex. Maintaining an engine fork for cel shading would be a bad tradeoff for a solo developer using AI-driven implementation. UE5.7, Substrate, Interchange, staged-build behavior, and existing import automation already give us enough moving parts.

The no-engine-mod choice does mean the plan must stop relying on claims that belong to custom shading models. If the shader is unlit, it owns the light vector, banding, shadow color, and material response. It does not get full deferred lighting behavior for free.

## Dropping Per-Volume Shadow Color

I agree. Per-volume authoring is a poor fit for procedural dungeons and the repo's current atmosphere architecture. T66 already has the right granularity for this game: theme/floor-level visual specs.

The plan should explicitly reuse the existing theme post-process and atmosphere spec ideas, but it should not pretend there is already material-parameter plumbing there. `FT66ThemeAtmosphereSpec` currently drives actors, fog, post-process, torches, and carry lights. It does not drive material instances. That path must be added.

## Separate Character And Environment Lighting

I agree with the separation, especially because T66 already accepts that torches do not need to shade characters. The risk is asset taxonomy. "Character" in this repo can mean hero actors, QuadRetro static meshes, imported mobs, boss visuals, weapon/projectile meshes, VAT mobs, or display/test assets. These do not all share one material path or runtime setup.

Phase 1 should pick one narrow visual target first. The lowest-risk proof is probably a single imported static character or controlled test mesh, not every hero/mob/environment material at once.

## Inverted Hull Character Outlines

The principle is sound, but the current pipeline is not prepared for it. GG Xrd-style outline control depends on reliable vertex colors, separate normal concepts, and consistent mesh import behavior. T66's current QuadRetro pipeline explicitly disables vertex-color remeshing, recalculates normals, force-flat-shades by default, and hardwires pixelated texture output.

Do not schedule production outline work before the Blender pipeline can preserve the data the outline material needs.

## Environment Post-Process Outlines

This is a reasonable fit for procedural environments, but it has to coexist with the current Retro FX post-process stack. `UT66RetroFXSubsystem` already owns outline-like post-process materials, pixelation stencils, geometry materials, and weighted blendables. A separate ToonStyle outline subsystem could easily fight it.

The safer design is to make ToonStyle post-process ownership explicit: either integrate with the existing Retro FX blendable management or create a clearly separated ToonStyle manager with documented order and weights.

## Six-Phase Sequencing

The phase order needs adjustment.

Before "Phase 1: cel shading material foundation", add a prerequisite proof pass:

1. Decide and verify the AA method for ToonStyle.
2. Register the project shader directory and compile a trivial included `.ush`.
3. Decide whether ToonStyle material parameters use MIDs, a Material Parameter Collection, or both.
4. Pick one character asset and one environment asset as the test surface.
5. Add or plan a non-pixelated branch in the QuadRetro pipeline so the source texture is clean before toon rendering.

Without those, Phase 1 risks becoming a pile of partially connected material work that cannot be fairly evaluated.

## Existing Code Conflicts Or Gaps

The prompt says the existing atmosphere system already pushes material parameters via MIDs. I did not find that in the live atmosphere path. It pushes actor properties and post-process settings. That distinction matters because ToonStyle needs a new parameter-delivery mechanism.

The prompt says SkyAtmosphere has an eclipse/Rayleigh dusk state. Live runtime setup destroys SkyAtmosphere actors. That memory is stale.

The prompt asks about files under `Source/T66/Rendering`; the live files are under `Source/T66/Core`.

The prompt expects 32 `M_GLB_Unlit` dependents; current disk has 63. The migration blast radius is larger than the prompt assumes.

The prompt expects named Blender scripts that are not present. The current pipeline is consolidated under QuadRetro. Future prompts should target the actual script.

## UE5.7-Specific Concerns

Substrate is enabled. Before creating material assets programmatically, verify which material properties and shading-model settings are still safe to set through UE Python in this project.

Forward shading is disabled. Surface ForwardShading material lighting mode should be tested in this exact deferred/Substrate configuration before being adopted as a core requirement.

The project targets DX12/SM6, Nanite, Virtual Shadow Maps, and no static lighting. ToonStyle should be tested in packaged/staged runtime eventually, not only in editor previews.

## Hidden Dependencies

Shader includes depend on `.uproject` registration.

Toon parameters depend on a new delivery path from `FT66ThemeAtmosphereSpec`.

Character cel quality depends on mesh normals and vertex colors, which depend on Blender pipeline changes before material work can fully succeed.

Retro overlay correctness depends on `UT66RetroFXSubsystem` ordering and user-owned saved settings. New work must not silently re-enable retro effects for existing saves.

Git tracking of empty folders depends on adding placeholder files or accepting that empty scaffold folders remain local only.

## Better Alternatives To Consider

Use one throwaway ToonStyle test map or controlled runtime spawn path before touching bulk content.

Use a Material Parameter Collection for global floor/theme cel values if most parameters are shared. Use MIDs only for per-asset or per-character overrides. This fits the existing theme-spec idea while avoiding an expensive actor/component scan every time settings change.

Treat `/Game/Materials/M_GLB_ViewSpaceLit_Character` as a reference candidate before discarding it. The repo already has a pending issue saying it needs a visual lock decision; it may contain useful work or prove what not to do.

Add a non-pixelated output mode to the QuadRetro pipeline before building ToonStyle character materials. That creates clean source textures and makes it easier to isolate whether bad results come from shader math or asset preparation.

## Bottom Line

The high-level pivot is correct: clean toon first, retro overlay later. The fragile part is implementation sequencing. The repo does not yet have shader-source plumbing, material-parameter plumbing, AA policy, or mesh-pipeline data preservation needed for the full plan. Build those foundations first, on one character and one environment asset, then scale.
