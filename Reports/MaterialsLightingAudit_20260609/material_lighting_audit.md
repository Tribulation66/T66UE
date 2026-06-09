# T66 Materials & Lighting Audit — + Blender review models & Hero 1 material

**Date:** 2026-06-09
**Scope:** Read-only audit. Answers the questions: which models are in the Blender review scene, how many materials/master materials the game uses, how many lighting presets exist, and which material Hero 1 currently uses.
**Engine:** UE 5.7. Project `C:\UE\T66`.

---

## 1. Blender review scene — the 5 models (×2 lightings = 10)

Open scene: `Model Generation/Runs/Pixal3D/HeroChadStacy_SourceAssets_20260609_0536/Blender/HeroChadStacy_with_current_hero1_male.blend` (verified live via Blender MCP — it was the currently-open file; 113 MB; `is_dirty: true` so it had unsaved changes).

10 mesh objects = **5 unique subjects, each in two lighting variants** (`_UnrealUnlit_ReviewRoot` vs `_PreviousBlenderLight_ReviewRoot`):

| # | Model | Unreal-unlit mesh | Previous-Blender-light mesh |
|---|-------|-------------------|------------------------------|
| 1 | **CurrentHero1Male** | `geometry_0` | `geometry_0.005` |
| 2 | **Hero1Stacy** | `geometry_0.001` | `geometry_0.006` |
| 3 | **Hero2Chad** | `geometry_0.002` | `geometry_0.007` |
| 4 | **IdolProjectile** | `geometry_0.003` | `geometry_0.008` |
| 5 | **WeaponProjectile** | `geometry_0.004` | `geometry_0.009` |

Scene also contains the documented Unreal-unlit preview rig (`T66_UnrealFriendSlopPreview` → `T66_Key_Softbox_L/R`, `T66_Top_Soft_Fill`, `T66_Front_Fill`, currently hidden) plus `ReviewSun`/`ReviewFill` for the "previous Blender light" comparison. Engine EEVEE, workspace Layout, camera `ReviewCamera`.

**Interpretation:** this is a side-by-side of the **current FriendSlop direction** (`CurrentHero1Male`) vs the **legacy ToonStyle direction** (`Hero1Stacy`) plus `Hero2Chad` and the idol/weapon projectiles, each under unlit vs lit — i.e. the lit-vs-unlit decision the rubber-material migration still owes (see §4/§5).

---

## 2. Materials — count and the master materials that matter

### Raw asset count (filename glob, not deduped)
- Masters `M_*`: ~150
- Instances `MI_*`: ~351 (mostly per-character generated skins)
- Functions `MF_*`: ~9
- **Total ≈ 510** — but the vast majority are instances + third-party packs (Stylized_VFX_StPack ~130, UE5RFX) + legacy (ToonStyle ~52, archived Pixal3D snapshot ~32).

### Master materials and what uses each (code-grounded, file:line)

| Master material (`/Game/Materials/…`) | Used by | Status |
|---|---|---|
| **M_GLB_Unlit** | **Master for FriendSlop hero/character skeletal meshes** via per-character `MI_` instances (e.g. `MI_SK_Hero_1_Chad_PhysicsFirst`) — runtime-confirmed §6; also GLB props `T66TowerLighting.cpp:218`; GLB util `T66VisualUtil.cpp:28`; RetroFX GLB base `T66RetroFXSubsystem.cpp:36` | Active (key character master) |
| **M_Character_Unlit** | character base const `T66CharacterVisualSubsystem.cpp:32`; RetroFX `T66RetroFXSubsystem.cpp:33` | Active |
| **M_FBX_Unlit** | FBX base `T66CharacterVisualSubsystem.cpp:33`; RetroFX `T66RetroFXSubsystem.cpp:35` | Active |
| **M_Environment_Lit** | world/terrain + hazards: `T66GameInstance.cpp:2096`, `T66MainMapTerrain.cpp:878`, `T66TerrainThemeAssets.cpp:62`, `T66TowerThemeVisuals.cpp:13`, `T66MiasmaBoundary.cpp:61`, lava `T66LavaShared.h:8` | Active |
| **M_Environment_Unlit** | RetroFX env base `T66RetroFXSubsystem.cpp:34` | Active (RetroFX) |
| **MI_GLB_Unlit_Character_Shared** | QuadRetro/mob static meshes `T66CharacterVisualSubsystem.cpp:34`; Pixal test shared `T66GameMode.cpp:1020` | Active (mobs/test) |
| **M_*_Unlit_RetroGeometry** (Char/Env/FBX/GLB) | RetroFX "retro geometry" swap `T66RetroFXSubsystem.cpp:37–40` | Active (RetroFX) |
| **M_T66_OutlinePostProcess, M_RetroChromaticAberrationPostProcess, MI_T66_PS1_*** | RetroFX post-process `T66RetroFXSubsystem.cpp:51–52` + PS1 variants | Active (RetroFX) |
| **M_GLB_ViewSpaceLit_Character** | listed by asset scan; **no runtime code reference found** | Asset-only / unverified |
| **M_FriendSlop_Rubber** (+ _Unlit, _ClearCoat) | **ZERO code references** — asset exists, not wired into runtime | **Migration target, not yet wired** |

**Bottom line:** ~510 assets, but only ~8 master materials actually matter, and per `ART_DIRECTION.md` the target is to collapse to **one shared rubber master** (color the only per-object variable) — a decision still TBD.

**Active vs legacy:** FriendSlop (current 3D direction) + Stylized VFX + RetroFX are active; ToonStyle (~52, TestRoom-only) and the archived Pixal3D snapshot (~32) are legacy.

---

## 3. Hero 1's current material (data-table + code grounded)

Resolution chain:
1. `T66HeroBase.cpp:1012` → `GetHeroVisualID("Hero_1", Chad body, skin)` → **VisualID `Hero_1_Chad`** (`T66CharacterVisualSubsystem.cpp:1459`).
2. `CharacterVisuals.csv` row `Hero_1_Chad`:
   - SkeletalMesh: `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst`
   - No StaticMesh / OutlineMesh / PixelatedTexture; anims `AM_Hero_1_Chad_PhysicsFirst_{Walk,Idle,Jump,Leap}`
3. `ApplyCharacterVisual` → `T66ApplySafeCharacterMaterialOverrides` (`T66CharacterVisualSubsystem.cpp:560–638`): takes the skeletal mesh's **own embedded imported material** (`SourceMaterial->GetBaseMaterial()`, line 579) and rebuilds it as a runtime **`UMaterialInstanceDynamic`** (line 609), rebinding the generated Pixal3D base-color/diffuse texture and forcing unlit-leaning params (Brightness set, Tint/BaseColor white, spec/emissive weights 0, Opacity 1 — lines 615–637).

**Answer (runtime-confirmed, see §6):** Hero 1 (Chad) = the **FriendSlop raw Pixal3D "PhysicsFirst" skeletal mesh**. Its one authored material slot is **`MI_SK_Hero_1_Chad_PhysicsFirst`** — a `MaterialInstanceConstant` whose **base/master material is `M_GLB_Unlit`** — bound to base-color texture `T_Hero_1_Chad_PhysicsFirst_BaseColor`. At runtime `T66ApplySafeCharacterMaterialOverrides` (when it rebuilds) creates a DMI from that base (`GetBaseMaterial()` → `M_GLB_Unlit`) and rebinds the base-color texture with unlit params.

So Hero 1's **master material is `M_GLB_Unlit`** (via a per-character instance). It is **not** using `M_FriendSlop_Rubber` (the rubber migration target, still unwired), `MI_GLB_Unlit_Character_Shared` (that's the mob/QuadRetro path), or ToonStyle.

> Correction note: an earlier code-only reading described this as "not M_GLB_Unlit." The headless runtime dump (§6) shows the embedded instance is in fact parented to `M_GLB_Unlit`, so `M_GLB_Unlit` IS Hero 1's master. This is why the runtime check was worth running.

Contrast in the same data table: `Hero_1_Stacy` = `AnimatedToonStyle` + `Pixal3DToonStyle` + outline (legacy) — i.e. the Blender scene's `Hero1Stacy` is the old direction, `CurrentHero1Male` is `Hero_1_Chad` FriendSlop.

> **Runtime confirmation:** see §6 (headless editor dump of the actual material slots on `SK_Hero_1_Chad_PhysicsFirst`).

> **Side finding (stale script):** `Scripts/VerifyFriendSlopHero1SkeletalVisualAndExit.py` still expects the older path `FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop`, but the live CSV now points to `FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst`. That verifier would fail as-is and should be updated (or retired) — flagged, not changed.

---

## 4. Lighting — presets and what uses them

**5 theme presets** (`FT66ThemeAtmosphereSpec`, enum `ET66TowerGameplayLevelTheme`) + 1 internal neutral fallback, in `T66ThemeAtmosphereData.cpp`:

| Preset | Status | Notes |
|---|---|---|
| **Dungeon** | ✅ fully authored | full sky/fog/color-grade/torch/cel (see §5) |
| **Forest / Ocean / Martian / Hell** | ⚠️ placeholders | only cel-shading colors + light direction overridden; sky/fog/color-grade return neutral |
| **Neutral** | fallback | no fog, neutral grade |

Confirmed against `Source/T66/Gameplay/pending_issues_Gameplay.md` ("Atmosphere Iteration 01 is explicitly Dungeon-only").

**What applies/consumes them** (`T66WorldVisualSetup`):
- `EnsureAtmosphereForWorld()` (`:586`) → `T66ApplyAtmosphereSkyLight()` (`:347`), `T66ApplyAtmosphereFog()` (`:428`), `T66ApplyThemePostProcess()` (`:478`, color grading on a PostProcessVolume), `ApplyAtmosphereToHeroCarryLights()` (`:601`), `ApplyToonCelAtmosphereToRegisteredMaterials()` (`:697`).
- `EnsureNeutralVisualSetupForWorld()` (`:531`) strips legacy lighting actors first.
- **`T66TowerLighting`** consumes the spec for procedural torch + hero carry-light placement — **Dungeon only** (`T66TowerLighting.cpp:325–334`); boss floors skip torches.

**2 global (non-theme) lighting systems:**
- **`UT66PerActorLightDirection`** — per-actor directional-light override for toon shading (`T66WorldVisualSetup.cpp:49`).
- **`T66TowerLighting`** torch/carry-light placement (above).

Net: **5 theme presets (only Dungeon real) + 2 global lighting systems.**

---

## 5. Exact Dungeon atmosphere spec values

`T66ThemeAtmosphereData.cpp:47–81` (cel block `:26–45`):

```
SkyLight        Color=White  Intensity=0.0        (sky light off; ambient cubemap primary)
AmbientCubemap  /Engine/MapTemplates/Sky/DaylightAmbientCubemap
                Intensity=2.5  Tint=(0.45, 0.60, 0.90)        cool blue ambient
Fog             Density=0.018  HeightFalloff=0.2
                Inscattering=(0.18, 0.26, 0.45)  Start=400  Cutoff=20000   blue-purple
ColorGrade      Shadows/Mid/Highlights tint = (1,1,1)
                Saturation=0.95  Contrast=1.0  Gain=1.0        slightly desaturated
Torch           Intensity=500  AttenRadius=600  Color=(1.0, 0.30, 0.05)    warm orange
                Falloff=2.0  VertOffset=450  Spacing=1800  MinSep=1400  MaxPerFloor=200
CarryLight      Intensity=0.0 (disabled in dungeon)  Color=(1.0,0.7,0.4)  AttenRadius=350
Cel (toon)      LightDir=(-0.4, 0.6, -0.7)  LightColor=White  Ramp=[0.0, 0.5]
                Shade=(0.35,0.38,0.50)  Mid=(0.70,0.70,0.72)  Lit=(0.85,0.85,0.85)
                Rim=(1.0,0.95,0.85)  RimPower=4.0  RimStrength=0.21  Outline=Black width 1.5
                Env: Shade=(0.30,0.32,0.42)  Mid=(0.55,0.58,0.62)  Lit=(0.85,0.85,0.90)
```

The 4 placeholder themes keep neutral sky/fog/color-grade and only override the cel `LightDirection` + shade/mid/lit/rim/env colors (`:83–143`).

---

## 6. Runtime confirmation — Hero 1 material slots (headless editor dump)

Headless dump via `Scripts/DumpHero1MaterialAndExit.py` (`UnrealEditor-Cmd -run=pythonscript -NullRHI`, exit 0). Raw JSON: `Saved/CombatTest/Hero1_Material_RuntimeDump.json`.

```json
skeletal_mesh: /Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/SK_Hero_1_Chad_PhysicsFirst
materials[0]:
  slot_name:          Material_0
  material_path:      /Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/Materials/MI_SK_Hero_1_Chad_PhysicsFirst
  material_class:     MaterialInstanceConstant
  base_material_path: /Game/Materials/M_GLB_Unlit            <-- master
  dependencies:       M_GLB_Unlit, T_Hero_1_Chad_PhysicsFirst_BaseColor
```

**Confirmed:** Hero 1's single material slot is the per-character instance `MI_SK_Hero_1_Chad_PhysicsFirst`, parented to master **`M_GLB_Unlit`**, carrying the generated base-color texture. No rubber master in the chain.

_(Note: `Scripts/DumpHero1MaterialAndExit.py` is a task-specific throwaway — keep as a durable "dump hero material" helper or delete per the Script Lifecycle rule; left in place pending your call.)_

---

## Appendix — sources

- Blender: live MCP summary of the open `.blend` (path-info + datablocks + objects).
- Materials: `Glob` counts on `Content/**` + `Grep` of `Source/T66` for material paths; `ART_DIRECTION.md`.
- Hero 1: `Content/Data/CharacterVisuals.csv` row `Hero_1_Chad`; `T66CharacterVisualSubsystem.cpp` (`GetHeroVisualID` :1459, `T66ApplySafeCharacterMaterialOverrides` :560); `T66HeroBase.cpp:1007–1036`.
- Lighting: `T66ThemeAtmosphereData.cpp`, `T66WorldVisualSetup.cpp`, `T66TowerLighting.cpp`, `T66PerActorLightDirection.*`; `pending_issues_Gameplay.md`.
