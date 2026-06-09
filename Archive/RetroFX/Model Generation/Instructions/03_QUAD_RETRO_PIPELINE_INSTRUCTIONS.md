# Quad Retro Pipeline

The reusable Quad Retro scripts now live under `Model Generation/Scripts/Core/QuadRetro`.

## Run One Model

```powershell
powershell -ExecutionPolicy Bypass -File "C:\UE\T66\Model Generation\Scripts\Core\QuadRetro\RunQuadRetroCharacterPipeline.ps1" `
  -InputModel "C:\path\to\source.glb" `
  -OutputDir "C:\path\to\quad_retro_output" `
  -Label "AssetLabel" `
  -TargetQuads 12000 `
  -TextureSize 512 `
  -PaletteMode none `
  -DitherType none `
  -DitherStrength 0 `
  -RenderQA:$true `
  -Background:$false
```

## Current Known-Good Bias

- Preserve colors first.
- Use `PaletteMode none`, `DitherType none`, and `DitherStrength 0` as the baseline.
- The Medium-style target around `12000` quads has been the best balance for readable character tests.
- Low, Medium, and High presets are visual choices, not quality labels.

## Verification

Every run needs:

- generated model
- JSON report
- at least one rendered QA image
- visual review before Unreal import

If PowerShell is shelling through another runner, pass booleans as `-RenderQA:$true -Background:$false`. Loose `$true` or `$false` tokens can be eaten by the wrong shell layer.
