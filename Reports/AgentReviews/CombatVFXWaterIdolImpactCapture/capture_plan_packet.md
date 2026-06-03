# Combat VFX Water Idol Impact Capture Plan

## Working Goal

Capture and deliver an Unreal-owned gameplay video showing the Water idol impact structure in action, using the repo's established video capture process without implementing new gameplay changes.

## User Request

Use our video capture process to capture and send a video of the Water idol impact proof in action.

## Applicable Instructions

- Root process: `C:\UE\T66\AGENTS.md`
- Gameplay process: `C:\UE\T66\Gameplay\GAMEPLAY_AGENTS.md`
- Combat VFX process index: `C:\UE\T66\Gameplay\Combat\VFX_PROCESS_INDEX.md`
- Video/screenshot accepted process: use Unreal-owned capture paths, specifically `Scripts\CaptureT66GameplayVideo.ps1` for gameplay/VFX video proof.

## Plan

- Do not edit gameplay or content.
- Use the existing gameplay capture mode `hero1axeaoewateridolimpact`.
- Capture only the Water idol proof, not the Earth neutral control, because the user asked for the effect in action.
- Exact current script support was verified in `C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1`:
  - parameter: `-Hero1AxeProofIdol`
  - switch: `-UseHero1AxePreviewStaging`
  - switch: `-EvidenceBundle`
  - parameter: `-EvidenceSelectedFrames`
  - capture mode branch: `hero1axeaoewateridolimpact`
  - the script automatically appends `T66.Combat.ImpactSourceVerbose 1` for this capture mode when not already present.
- The command is copied from the last validated proof wrapper source, `C:\UE\T66\Scripts\RunHero1AxeAOEWaterIdolImpactProof.ps1`, and the final validated proof run under `C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpactProof_20260528_104400`.
- Run this exact capture command:

```powershell
& .\Scripts\CaptureT66GameplayVideo.ps1 `
  -CaptureMode hero1axeaoewateridolimpact `
  -Output 'C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528\WaterIdolImpact.mp4' `
  -FrameDir 'C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528\frames' `
  -ResX 1280 `
  -ResY 720 `
  -FrameCount 72 `
  -FrameRate 12 `
  -CaptureIntervalSeconds 0.08 `
  -DelaySeconds 5.0 `
  -Hero1AxeHitboxFireDelay 7.6 `
  -Hero1AxeHitboxVFXLeadSeconds 0.12 `
  -Hero1AxeProofIdol Idol_Water `
  -UseHero1AxePreviewStaging `
  -EvidenceBundle `
  -EvidenceRoot 'C:\UE\T66\Saved\VideoCaptures\Hero1AxeAOE_WaterIdolImpact_UserVideo_20260528\evidence' `
  -EvidenceLabel 'Hero1AxeAOEWaterIdolImpact_UserVideo' `
  -EvidenceSelectedFrames 'start=50,mid=56,impact=64,dissipate=68' `
  -TimeoutSeconds 180
```

- Capture duration is bounded by `72` frames at `12` fps after a `5.0` second capture delay, with a `180` second process timeout.
- The current staged build has already been refreshed after the Water idol structure pass, but this capture uses the editor gameplay capture path required by the combat VFX video process.

## Verification

- Capture command exits 0.
- MP4 exists and is non-empty.
- Evidence bundle/contact sheet exists.
- Inspect the contact sheet or selected frames to confirm the blue Water placeholder sphere appears at the slash impact point.
- Inspect the proof log for `CombatImpactContext Phase=IdolPrimary SourceType=IdolModifier SourceID=Idol_Water`.
- Inspect the proof log for `CombatVFXIdolImpactBindingLookup ... SourceID=Idol_Water ... Result=None`, proving the placeholder branch was the current no-binding fallback.
- Report the absolute MP4 path and evidence path to the user.

## Out Of Scope

- No Niagara authoring.
- No code or data changes.
- No staged standalone rebuild unless the capture command itself reveals a blocking runtime issue.
