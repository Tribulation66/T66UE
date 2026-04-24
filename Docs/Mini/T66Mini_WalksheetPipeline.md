# T66 Mini Walksheet Pipeline

This is the local process for generating minigame walk animation sheets with Codex-native image generation.

## Goal

Generate small, coherent 2D walk sheets that feel alive in motion without chopping the body into separate generated parts.

The workflow is asset-first:

1. Generate a full walk sheet for one facing.
2. Normalize the frames so the feet line and body occupancy stay consistent.
3. Compare 2-3 candidates side by side.
4. Choose a winner before touching runtime code.
5. Only derive separate upper/lower layers later if the winning full sheet still needs it.

## Why This Approach

Generating upper and lower body separately as the first step causes seam drift, scale drift, and the "sawed in half" look.

Generating a full sheet first keeps:

- pose continuity
- silhouette continuity
- stable armor and face details
- consistent foot placement

## Inputs

- reference portrait or sprite for the character
- a manifest entry in [T66Mini_WalkCandidateManifest.json](/C:/UE/T66/Docs/Mini/T66Mini_WalkCandidateManifest.json)
- Codex-native image generation

Arthur references currently used:

- [Arthur_male_full.png](/C:/UE/T66/SourceAssets/FinalPortraits/Arthur_male_full.png)
- [Arthur.png](/C:/UE/T66/SourceAssets/Mini/Heroes/Singles/Arthur.png)

## Candidate Generation Rules

- Generate at most 3 candidates per pass.
- Use one 2x2 image per candidate.
- Each quadrant is one sequential walk frame.
- Keep the same camera angle, scale, feet line, and framing in all four cells.
- Prefer transparent or flat white background.
- Do not regenerate the same request unless there is a serious issue.

## Commands

Generate the walk candidates with Codex-native image generation from the manifest prompts and reference images. Save the raw candidates to `SourceAssets/Mini/Heroes/WalkCandidates/`.

Do not use browser-automation generation tooling or legacy request manifests.

Normalize the frames:

```powershell
python .\Tools\Mini\T66MiniNormalizeWalkSheet.py --manifest .\Docs\Mini\T66Mini_WalkCandidateManifest.json --only arthur_walk_candidate_01 arthur_walk_candidate_02 arthur_walk_candidate_03
```

Build the side-by-side comparison sheet:

```powershell
python .\Tools\Mini\T66MiniBuildWalkComparison.py --manifest .\Docs\Mini\T66Mini_WalkCandidateManifest.json --group arthur_walk_candidates
```

## Output Locations

Raw candidate sheets:

- `SourceAssets/Mini/Heroes/WalkCandidates/`

Normalized frames:

- `SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Candidate01/Normalized/`
- `SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Candidate02/Normalized/`
- `SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Candidate03/Normalized/`

Comparison sheet:

- [arthur_walk_candidates.png](/C:/UE/T66/SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Comparison/arthur_walk_candidates.png)

## Selection Criteria

Choose the candidate with:

- the clearest leg motion
- the smallest silhouette jitter
- the most stable face and torso
- no visible frame-to-frame resizing
- no chopped or floating-body look

Reject candidates that:

- bounce the entire body up and down
- change framing between cells
- drift off the foot baseline
- lose Arthur's identity

## Current Arthur Pass

The first Arthur pass produced three usable full-sheet candidates:

- [Candidate 01](/C:/UE/T66/SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Candidate01/Normalized/Arthur_Idle_R.png)
- [Candidate 02](/C:/UE/T66/SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Candidate02/Normalized/Arthur_Idle_R.png)
- [Candidate 03](/C:/UE/T66/SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Candidate03/Normalized/Arthur_Idle_R.png)

Comparison image:

![Arthur Walk Comparison](/C:/UE/T66/SourceAssets/Mini/Heroes/WalkCandidates/Arthur/Comparison/arthur_walk_candidates.png)

Selected winner:

- `Candidate 01`
- reason: strongest compact read and acceptable mirrored left/right runtime presentation

## Next Step

Do not modify the movement runtime again until one candidate is selected and integrated cleanly.

Once a winner is chosen:

1. Export the final runtime frame set into the hero animation folder.
2. Replace the temporary Arthur presentation assets.
3. Tune sprite size and frame rate in the standalone build.
4. Repeat the same pipeline for enemies.
