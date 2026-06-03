# Phase 0 Pending Questions

These are the questions that should be answered before drafting or executing the Phase 1 prompt.

## 1. Should Phase 1 update project AA settings first?

Context: `Config/DefaultEngine.ini` currently has `r.AntiAliasingMethod=0`. ToonStyle is supposed to be clean native-resolution cel shading with smooth bands. Shader-side smoothstep helps, but project AA is still central to the result.

## 2. Is Phase 1 allowed to edit `T66.uproject` for shader-source registration?

Context: no project shader directory is registered today. `.ush` includes under `/Project/ToonStyle/...` will not work until the project registers a shader source directory. `T66.uproject` is also already dirty in the current worktree, so Phase 1 needs an explicit instruction on how to handle that file.

## 3. Should ToonStyle use a Material Parameter Collection, MaterialInstanceDynamics, or both?

Context: the current atmosphere system does not push material parameters. It drives actor, fog, post-process, torch, and carry-light properties. A new material delivery path is required.

## 4. What is the first controlled visual target?

Context: "characters" can mean hero actors, QuadRetro static mesh characters, mobs, bosses, weapon/projectile meshes, VAT enemies, or display-only test assets. Phase 1 should name one character asset and one environment asset instead of trying to migrate the whole game.

## 5. Should `M_GLB_ViewSpaceLit_Character` be evaluated before creating a new character master?

Context: `Content/Materials/pending_issues_Materials.md` says this material exists but still needs a visual lock decision. It may be useful reference work or a known dead end.

## 6. Should new-save Retro FX defaults change during ToonStyle work?

Context: `FT66RetroFXSettings` defaults the master gate to true while most effect intensities are zero. User preference from prior work is that unrelated work should not re-enable Retro FX. Phase 1 should decide whether to leave defaults alone, force ToonStyle test profiles off, or make a deliberate settings migration later.

## 7. Should the QuadRetro pipeline gain a non-pixelated output mode before material work?

Context: the current pipeline hardwires pixelated texture creation and material assignment. ToonStyle needs clean source textures. A skip-pixelation branch would make shader evaluation much cleaner.

## 8. What vertex color channel layout should Phase 3 target?

Context: the plan says one channel for threshold offset and three for outline controls. The current pipeline does not preserve/author those channels. The exact layout should be named before any Blender or material implementation.

## 9. Does Pablo want empty ToonStyle scaffold folders tracked in Git?

Context: Git does not track empty directories. This pass created `Shaders/`, `Materials/`, `Source/`, and `BlenderScripts/` locally, but only markdown files will be staged unless placeholder files are added in a later pass.

## 10. Should ToonStyle get its own folder router?

Context: `ToonStyle/` did not exist before Phase 0, so there is no `ToonStyle_AGENTS.md` or pending issue file. If this becomes a long-lived workspace, adding a folder router would align it with the repo's folder instruction discovery rule.

## 11. Is Pixal3D still research-only for this initiative?

Context: Pixal3D is separate from runtime pixelation and separate from the Trellis/QuadRetro production pipeline. Prior process docs treat it as research-only until licensing and quality are cleared.

## 12. Should Phase 1 treat `Gameplay/World/T66_LIGHTING_REFERENCE.md` as stale and update it?

Context: the doc says the runtime baseline is neutral and stripped, but live source now applies theme atmosphere, fog, post-process, ambient cubemap, carry-light overrides, and Dungeon torches. This is a documentation drift that could mislead future agents.
