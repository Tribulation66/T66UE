# Easy Mob Vertex Animation Batch Instructions

## Goal

Animate the Difficulty 1 enemy roster first, using vertex baked animation as the intended runtime format. Each enemy should keep the live static-mesh look and scale while gaining behavior-specific idle, move, attack cue, hit react, and eventual death motion.

## Difficulty 1 Definition

Difficulty 1 is the current `Easy` enemy/stage set in `Content/Data`.

Stage usage discovered from `Content/Data/Stages.csv`:

- `Stage_01`: `Slime`, `BoneWalker`, `RatPack`, `CaveBat`, `HexSlinger`, `TombSpider`, `StoneSentinel`
- `Stage_02`: adds `MimicLure`
- `Stage_03`: adds `BoneConjurer`
- `Stage_04`: adds `CryptWraith`

The blank stage enemy slots are currently intentional placeholders, not missing mob animation assignments.

## Required Source Checks

Before animating any Easy mob:

1. Confirm the `Enemies.csv` row still exists and still has `DifficultyID=Easy`.
2. Confirm the `CharacterVisuals.csv` row still resolves to the expected static mesh and scale.
3. Compare the live Unreal static mesh against the source GLB before assuming they match.
4. Check the production report for the source GLB's current visual and runtime status.
5. Keep live `CharacterVisuals.csv` unchanged until the VAT path passes temporary-row or temporary-map QA.

The `Roster_v1` Pixal3D GLBs are available for process exploration and in-game QA. Treat them as final only after visual and runtime acceptance.

## Easy Source Map

Current source GLB locations:

| EnemyID | Source GLB | Live Static Mesh | Live Scale |
| --- | --- | --- | --- |
| `Slime` | `Model Generation/Production/Roster_v1/AgentA/Slime/Slime.glb` | `/Game/Characters/Mobs/Slime/SM_Slime.SM_Slime` | `1.834685` |
| `BoneWalker` | `Model Generation/Production/Roster_v1/AgentB/BoneWalker/BoneWalker.glb` | `/Game/Characters/Mobs/BoneWalker/SM_BoneWalker.SM_BoneWalker` | `1.999062` |
| `RatPack` | `Model Generation/Production/Roster_v1/AgentA/RatPack/RatPack.glb` | `/Game/Characters/Mobs/RatPack/SM_RatPack.SM_RatPack` | `1.829114` |
| `CaveBat` | `Model Generation/Production/Roster_v1/AgentB/CaveBat/CaveBat.glb` | `/Game/Characters/Mobs/CaveBat/SM_CaveBat.SM_CaveBat` | `1.824371` |
| `HexSlinger` | `Model Generation/Production/Roster_v1/AgentA/HexSlinger/HexSlinger.glb` | `/Game/Characters/Mobs/HexSlinger/SM_HexSlinger.SM_HexSlinger` | `1.880113` |
| `TombSpider` | `Model Generation/Production/Roster_v1/AgentB/TombSpider/TombSpider.glb` | `/Game/Characters/Mobs/TombSpider/SM_TombSpider.SM_TombSpider` | `1.955138` |
| `StoneSentinel` | `Model Generation/Production/Roster_v1/AgentA/StoneSentinel/StoneSentinel.glb` | `/Game/Characters/Mobs/StoneSentinel/SM_StoneSentinel.SM_StoneSentinel` | `2.089124` |
| `MimicLure` | `Model Generation/Production/Roster_v1/AgentB/MimicLure/MimicLure.glb` | `/Game/Characters/Mobs/MimicLure/SM_MimicLure.SM_MimicLure` | `2.174346` |
| `BoneConjurer` | `Model Generation/Production/Roster_v1/AgentA/BoneConjurer/BoneConjurer.glb` | `/Game/Characters/Mobs/BoneConjurer/SM_BoneConjurer.SM_BoneConjurer` | `2.065492` |
| `CryptWraith` | `Model Generation/Production/Roster_v1/AgentB/CryptWraith/CryptWraith.glb` | `/Game/Characters/Mobs/CryptWraith/SM_CryptWraith.SM_CryptWraith` | `1.973075` |

## Runtime Behavior Caveat

`Enemies.csv` records a small active archetype set for the current demo-era runtime. Ranged subsections were intentionally collapsed into `Archetype=Ranged`; the current runtime family resolver maps these implemented families:

- `Melee`
- `Rush`
- `Ranged`
- `Flying`

Do not claim that animation completes missing gameplay behavior. For example:

- `MimicLure` can receive an explosion wind-up cue, but the current runtime still falls back through rush behavior until an exploder class exists.
- `StoneSentinel` and `BoneConjurer` can receive stone-sentry or caster-flavored animation cues, but they are still normal `Ranged` runtime enemies unless a future ranged-design pass explicitly reintroduces ranged subsections.
- `CryptWraith` can receive a stutter/glide visual, but the current runtime still falls back through melee behavior until a stutterer class exists.

Track this distinction in QA notes and in `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md` if implementation discovers new gaps.

## Per-Enemy Animation Direction

### Slime

- Gameplay row: `FamilyID=Melee`, `Archetype=Melee`, `Feeling=MowDown`
- Body read: low blob with no legs
- Idle: low breathing squash, small mouth/lip wobble, weight settled on floor
- Move: drag/pulse loop with belly contact, front edge stretching forward and body catching up
- AttackCue: quick compression, forward snap, mouth opening
- HitReact: ripple through body and quick deflate/reinflate
- Death: flatten, melt, or pop into a low puddle
- Reject if it walks on invisible legs or bounces so high it stops reading as floor-dragging

#### 2026-05-21 Accepted Move Direction

- Baseline preview: `Model Generation/Rigging and Animation/Runs/Slime_MoveTowardBouncyStutterPreview_V2_20260521/Slime_MoveTowardCamera_preview.mp4`
- Source front: `+Y`
- Review camera: front-facing on `+Y`
- Preview render: native Blender MP4, 72 frames, 15 fps, unlit/emissive material
- Timing: every frame should visibly change; use stepped/constant-style motion for the crunchy low-frame read
- Preview travel: move toward camera for review, while runtime actor translation remains gameplay-owned
- Motion note: stronger bounce is desired, but the body must still read as a sticky ground blob instead of a hidden biped

### BoneWalker

- Gameplay row: `FamilyID=Melee`, `Archetype=Melee`, `Feeling=Pressure`
- Body read: stripped skeleton humanoid
- Idle: loose clatter, uneven shoulder/head settling
- Move: stiff biped shamble with small foot lifts and clear ground contact
- AttackCue: arm/torso wind-up, jaw/head snap
- HitReact: bone-rattle recoil
- Death: collapse downward, not a clean player-style fall
- Reject if it uses polished player locomotion without skeletal looseness

### RatPack

- Gameplay row: `FamilyID=Rush`, `Archetype=Rush`, `Feeling=MowDown`
- Body read: a fused mass of small rats moving as one threat
- Idle: clustered jitter, tails/heads twitching in offset timing
- Move: low scuttle with internal rat offsets and mass-level forward intent
- AttackCue: surge/lunge from the whole pack
- HitReact: pack bunches inward and scatters back into shape
- Death: mass breaks low or collapses, depending on source mesh separability
- Reject if it becomes one large mammal stride instead of a swarm-like clump

### CaveBat

- Gameplay row: `FamilyID=Flying`, `Archetype=Flying`, `Feeling=Pressure`
- Runtime reference: `AT66FlyingEnemy` already has hover bob and flying movement
- Body read: airborne bat with active wings
- Idle: hover flap with body bob synced visually to runtime hover
- Move: stronger wing beats, slight body pitch, feet tucked
- AttackCue: wing flare and forward body dip
- HitReact: wing stumble and quick recovery
- Death: fold wings and fall only when runtime death state can show it
- Reject if it touches the floor or reads like a walking mesh

### HexSlinger

- Gameplay row: `FamilyID=Ranged`, `Archetype=Ranged`, `Feeling=DodgeThreat`
- Body read: small hooded caster
- Idle: robe sway, hands/staff held ready, subtle magic pulse if material supports it
- Move: short shuffle or glide-step; do not over-athleticize it
- AttackCue: casting anticipation with arm/staff extension and recoil
- HitReact: hood/robe snap back
- Death: robe collapse or kneel/fall
- Reject if ranged attack timing is unreadable from gameplay camera

### TombSpider

- Gameplay row: `FamilyID=Melee`, `Archetype=Melee`, `Feeling=MiniBossFeel`, `Rarity=Rare`
- Body read: larger heavy spider
- Idle: abdomen breathing, front leg taps, low stance
- Move: alternating four-leg group crawl with abdomen lag
- AttackCue: front-leg lift, body rear-back, bite/drop
- HitReact: abdomen recoil and leg spread
- Death: legs curl inward or body drops
- Reject if legs skate, clip through the body, or all move in the same phase

### StoneSentinel

- Gameplay row: `FamilyID=Ranged`, `Archetype=Ranged`, `Feeling=DodgeThreat`, `Rarity=Rare`
- Runtime caveat: this is a normal ranged runtime enemy for now; stone-sentry presentation is animation flavor, not a separate turret class
- Body read: heavy stone statue/sentry
- Idle: almost still, weighty breathing/stone settling
- Move: if runtime moves it, use a slow heavy slide or stomp; otherwise prefer idle/aim/recoil clips
- AttackCue: aim, glow, recoil, or stone arm lift
- HitReact: chip-like shake, not flexible flesh
- Death: crack, slump, or heavy crumble
- Reject if it runs like a normal ranged humanoid

### MimicLure

- Gameplay row: `FamilyID=Rush`, `Archetype=Exploder`, `Feeling=Disruptor`, `StageTag=Late`
- Runtime caveat: exploder behavior is not implemented yet; current runtime falls back through rush behavior
- Body read: chest/lure threat with sudden snap
- Idle: lid breathing, tongue/teeth twitch, bait-like stillness
- Move: awkward skitter or short lunge, depending on source silhouette
- AttackCue: lid flare, bite snap, or explosion wind-up anticipation
- HitReact: lid slam and recoil
- Death: collapse open; explosion visual only after runtime supports it
- Reject if it hides the tell for its disruptive role

### BoneConjurer

- Gameplay row: `FamilyID=Ranged`, `Archetype=Ranged`, `Feeling=Specialist`, `StageTag=Late`
- Runtime caveat: this is a normal ranged runtime enemy for now; conjuring/casting presentation is animation flavor, not a separate summoner class
- Body read: skeletal or robed caster
- Idle: chant loop, hand hover, robe/bone sway
- Move: controlled shuffle or glide
- AttackCue: cast/summon wind-up with clear hands/staff silhouette
- HitReact: interrupted chant recoil
- Death: robe/bones collapse
- Reject if the conjure/cast pose is hidden from gameplay camera

### CryptWraith

- Gameplay row: `FamilyID=Melee`, `Archetype=Stutterer`, `Feeling=Disruptor`, `StageTag=Late`
- Runtime caveat: stutterer behavior is not implemented yet; current runtime falls back through melee behavior
- Body read: shrouded gliding threat
- Idle: cloth ripple, suspended body drift, tiny phase flickers if material supports it
- Move: glide with intermittent stutter in the local mesh, while actor movement stays gameplay-owned
- AttackCue: sudden forward silhouette tear or arm/claw extension
- HitReact: wisp-like distortion and snap back
- Death: dissolve/collapse only after runtime material state supports it
- Reject if it becomes a normal footstep walk

## Batch Execution Order

Run the first batch in this order:

1. `Slime`: proves non-skeletal blob deformation and ground drag.
2. `CaveBat`: proves flying/wing deformation and hover compatibility.
3. `BoneWalker`: proves simple humanoid enemy VAT without using player rigging.
4. `RatPack`: proves swarm-like multi-part offset motion.
5. `TombSpider`: proves many-leg crawl and contact QA.
6. `HexSlinger`: proves ranged/caster cues.
7. `StoneSentinel`: proves heavy low-motion statue/ranged-sentry presentation.
8. `MimicLure`: proves rush/disruptor anticipation without claiming exploder gameplay.
9. `BoneConjurer`: proves specialist caster presentation without claiming summoner gameplay.
10. `CryptWraith`: proves stutter/glide presentation without claiming stutterer gameplay.

This order gives early coverage of the hardest authoring categories before scaling to all ten.

## Required Batch Evidence

For each Easy mob, record:

- source mesh path
- live static mesh path
- live row scale
- Blender source file path
- VAT bake source path
- Unreal VAT output path
- contact sheets from front, side, three-quarter, and gameplay camera
- crowd/phase test screenshot or capture once runtime VAT exists
- temporary runtime row or test-map evidence
- performance evidence when many copies are present
- standalone evidence if playable content changes

## 2026-05-14 Batch Output

Current Easy VAT output paths:

- Source inspection report: `Saved/EasyMobSourceInspection_20260514.json`
- Blender source: `Model Generation/Rigging and Animation/Runs/Easy_Mob_VAT_20260514/Easy_Mob_VAT_Source.blend`
- Blender manifest: `Model Generation/Rigging and Animation/Runs/Easy_Mob_VAT_20260514/easy_mob_vat_manifest.json`
- Per-mob preview frames: `Model Generation/Rigging and Animation/Runs/Easy_Mob_VAT_20260514/PreviewFrames/<EnemyID>/`
- All-mob contact sheet index: `Model Generation/Rigging and Animation/Runs/Easy_Mob_VAT_20260514/PreviewFrames/Easy_Mobs_AllClips_AllViews_Index_Contact_Sheet.png`
- Unreal VAT root: `/Game/Characters/MobsVAT/<EnemyID>/`
- Runtime CSV: `Content/Data/MobVertexAnimations.csv`
- Runtime data table: `/Game/Data/DT_MobVertexAnimations.DT_MobVertexAnimations`
- Unreal import report: `Saved/EasyMobVATImportReport.json`
- Unreal verification report: `Saved/EasyMobVATVerifyReport.json`
- Staged gameplay smoke log: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.log`
- Staged gameplay smoke screenshot: `Saved/StandaloneLogs/EasyMobVAT_GameplaySmoke_WithAudio.png`

The current frame layout is shared by all ten Easy mobs:

- `Idle`: frames `0-59`
- `Move`: frames `60-99`
- `AttackCue`: frames `100-129`
- `HitReact`: frames `130-149`
- `Death`: frames `150-194`
- sample rate: `30`
- rows per frame: `4`

Current runtime behavior:

- `AT66EnemyBase` checks `DT_MobVertexAnimations` first for the mob ID.
- If a row is enabled, the enemy uses the VAT static mesh/material and hides the skeletal mesh component.
- If no enabled VAT row exists, the original `CharacterVisuals.csv` static visual path remains the fallback.
- Actor movement, collision, touch damage, hit reaction state, death handling, and pooling remain gameplay-owned.
- VAT clip playback is currently per-enemy dynamic material instance playback. A later crowd-scaling pass can move this data to instanced rendering.

## 2026-05-21 Slime Movement Preview Output

Current accepted movement preview:

```text
Model Generation/Rigging and Animation/Runs/Slime_MoveTowardBouncyStutterPreview_V2_20260521/Slime_MoveTowardCamera_preview.mp4
```

Associated review assets:

- `Model Generation/Rigging and Animation/Runs/Slime_MoveTowardBouncyStutterPreview_V2_20260521/Slime_MoveTowardCamera_bouncy_stutter_v2_contact_sheet.png`
- `Model Generation/Rigging and Animation/Runs/Slime_MoveTowardBouncyStutterPreview_V2_20260521/Slime_MoveTowardCamera_bouncy_stutter_v2_autocrop_first16.png`

This preview is accepted as the current Slime movement direction, not as final imported VAT content. The next Slime pass should apply the same style standard to idle before attack work.

Current accepted warnings:

- UE Python cannot reliably read the data asset `FVector3f` bounds for this plugin path, so verification treats material instance `MinBBox`/`SizeBBox` as authoritative and fails if CSV values do not match them.
- Commandlet Python UV inspection can report zero channels for generated VAT static meshes. Run `Tools/verify_easy_mob_vat_in_unreal.py` through the full editor wrapper for authoritative UV2 verification; a passing full-editor report should show LOD `0` with `3` UV channels.
- The runtime assets are wired for in-game QA, but future release review must confirm visual and runtime quality before treating them as final mob art.

Current staged smoke evidence:

- The staged standalone launched without `-nosound`, initialized XAudio2, and registered OGG/Vorbis successfully.
- The QA capture spawned all ten Easy mobs in one scene with representative clips: `Slime=Idle`, `BoneWalker=Move`, `RatPack=AttackCue`, `CaveBat=HitReact`, `HexSlinger=Death`, `TombSpider=Idle`, `StoneSentinel=Move`, `MimicLure=AttackCue`, `BoneConjurer=HitReact`, `CryptWraith=Death`.
- The staged log had no matches for `M_EasyMobVAT`, `invalid ShaderMap`, `Default Material will be used`, `Failed to compile Material`, `Fatal`, `Critical error`, `Assertion failed`, `libogg`, or `libvorbis`.
- The taskbar `T66 Standalone.lnk` target was verified as `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Promotion Rule

Do not replace a live Easy mob visual with the VAT version until:

- source mesh match is proven
- scale match is proven
- behavior-specific animation is accepted from all QA angles
- temporary runtime wiring works
- many-copy playback has no obvious performance or synchronization issue
- the relevant runtime caveats are either fixed or explicitly documented as visual-only
