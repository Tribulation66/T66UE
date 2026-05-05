# Batch01 Source Audit Summary

Audit command:

```powershell
python "C:\UE\T66\Model Generation\Scripts\audit_chad_source_images.py" --input-dir "C:\UE\T66\Model Generation\Runs\Heroes\TypeA_Masculine_Batch01\Inputs\approved_seed_images" --output-json "C:\UE\T66\Model Generation\Runs\Heroes\TypeA_Masculine_Batch01\Review\ProcessBuild_20260504\batch01_source_audit_green.json" --expected-bg green
```

Result:

- audited: `57`
- failed: `28`
- audit JSON:
  [batch01_source_audit_green.json](C:/UE/T66/Model%20Generation/Runs/Heroes/TypeA_Masculine_Batch01/Review/ProcessBuild_20260504/batch01_source_audit_green.json)

## What Failed

The recurring failures are source-image failures:

- body images that are square fallback canvases instead of portrait production references
- internal green contamination inside the character silhouette
- non-flat or non-green borders on supposed green sources
- head sources that are portraits/panels instead of isolated head-plus-neck inputs

The strongest reroll candidates are:

- Shroud standard and Beachgoer bodies
- xQc standard and Beachgoer bodies, plus standard head
- Moist standard and Beachgoer bodies, plus standard head
- North standard and Beachgoer bodies, plus standard head
- Asmon standard and Beachgoer bodies, plus standard head
- Billy standard and Beachgoer bodies, plus standard head
- Rabbit standard and Beachgoer bodies, plus standard head
- Robo Beachgoer body

Weapons are intentionally out of scope for this pass even when the legacy audit
flags some weapon images.

## Decision

For Pass02, do not reuse failing Batch01 sources as production inputs. Generate
new body/head sources from the Pass02 prompt templates, audit the source PNGs
first, then run TRELLIS only for sources that pass.

