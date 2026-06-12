The revision is complete and compiles. Here's the summary.

## Hero 1 Bounce Projectile Travel Fix — Revision complete

**Blocker fixed:** the moving Bounce carrier's visible silhouette is now the **authored `NS_Hero1AxeBounce_MeshSlash`** Niagara slash, not the temporary cube.

**What changed (carrier only — damage/targeting/per-link contexts untouched):**
- `AT66HeroProjectile::SetPrimaryCarrierNiagara(...)` (new) attaches the authored Niagara to a root-attached `PrimaryCarrierVFXComponent` and **hides the cube/profile meshes**. Visual-only; no damage/collision authority moved in.
- `PerformBounce` resolves the `Hero1Axe_Bounce_Base` / `AttackCategory=Bounce` binding (via existing `ResolveCombatVFXBinding`) and feeds the bound system + `VisualScaleMultiplier` + `BasePlaybackSeconds` into staging.
- `StageBounceProjectileChain` / `SpawnBounceLinkProjectile` now carry the authored system through and attach it per link; the cube path runs **only** as a dev fallback if the binding fails to resolve.
- Added a **presentation-only minimum link travel of 0.32s** (= binding `BasePlaybackSeconds`) so a short ~150uu link (~0.06s raw) reads on screen. Clamps visual speed + the derived next-link launch offset only — damage timing is resolved before staging and is unchanged.

**Sequencing preserved:** one authored slash hero→primary, then one authored slash primary→second, one in flight per segment. No third projectile; the `hero1axebouncevfxbinding` harness was already correct (Primary + ChainSecond only) and was not touched.

**Verification:**
- Focused compile `T66Editor Win64 Development` — **PASS** (`Result: Succeeded`; only the pre-existing unrelated C4996 warning).
- Binding validator **not run / not required** — no CSV/DataTable/asset edit and every guarded symbol it asserts is still present.

**Proof command for Codex:**
```
UnrealEditor-Cmd.exe T66.uproject -ExecCmds="t66.capture hero1axebouncevfxbinding" -unattended -nop4 -nosplash
```
Expect `CombatVFXBounceLinkProjectile ... Carrier=...NS_Hero1AxeBounce_MeshSlash` for LinkIndex 0 then a delayed LinkIndex 1, `TravelSeconds>=0.32`, and capture showing the red/blue slash (not a cube) travelling each segment.

Files changed: `T66HeroProjectile.h/.cpp`, `T66CombatComponent.h/.cpp`, `Hero1AxeBounceMechanismPacket.md`, `pending_issues_Gameplay.md`, and the report. This is an Operator work artifact, not a visual-fidelity greenlight — final acceptance still needs Unreal-owned capture and Pablo approval.
