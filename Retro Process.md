# Retro Process

This file records the repeatable workflow for producing regular-game item icons in the current trash-treasure style.

## Goal

Create one finished icon per live item rarity:

- 30 live item series from `Content/Data/Items.csv`
- 4 rarity variants per series: `black`, `red`, `yellow`, `white`
- 120 final pixelated PNGs
- 120 imported Unreal texture assets in `/Game/Items/Sprites`

The current item art direction is discarded real-world junk presented like precious fantasy treasure. Avoid medicine-coded objects for regular items because temporary boosts use the medicine visual space.

## Style Rules

Each icon should use:

- One centered trash-object treasure with a clear silhouette
- A flat mono-color rarity background
- No outer frame, box border, bevel, inset panel, grid, label, or text
- A darker same-hue outline or halo around the object only
- Consistent scale and padding across the 4 rarity variants in a series

Rarity background colors:

- `black`: dark charcoal
- `red`: saturated crimson
- `yellow`: golden yellow
- `white`: pale ivory

## Source Sheet Prompt Pattern

Generate one 2x2 source sheet per series. The cell order is fixed:

- Top-left: `black`
- Top-right: `red`
- Bottom-left: `yellow`
- Bottom-right: `white`

Prompt template:

```text
Use case: stylized-concept
Asset type: 2x2 game item icon sheet source art for later pixelation
Primary request: Create one square 2x2 item icon sheet for the <Family> family, <Series> series. Four equal square cells, no visible grid lines, no gutters, absolutely no text, no numbers, no symbols, no logos, no printed markings.
Layout: top-left BLACK rarity <object> named <name>; top-right RED rarity <object> named <name>; bottom-left YELLOW rarity <object> named <name>; bottom-right WHITE rarity <object> named <name>.
Rules: each cell has one centered object, large readable silhouette, orthographic/front three-quarter game icon view, generous padding, consistent scale. Each cell background is flat mono-color rarity color: dark charcoal, saturated crimson, golden yellow, pale ivory. No frame, no box border, no bevel, no inset panel, no decorative corners, no gradients. Object sits directly on flat background. Add only a darker same-hue object outline/halo around each object, not on the canvas edge.
Style: polished stylized game icon, clean cel-shaded discarded trash treasure, high contrast, crisp edges, upper-left lighting, high-resolution source art, not pixel art yet.
Avoid: any readable or pseudo-readable text, digits, printed icons, letters, logos, watermark, medicine, syringe, pill, potion, labels with writing, characters, hands, scene background, outer border.
```

## Working Paths

Main script:

```powershell
C:\UE\T66\Tools\Items\T66ProcessReimaginedItemSheets.py
```

Current pass folder:

```powershell
C:\UE\T66\SourceAssets\ItemSprites\_ReimaginedItemPass_20260427
```

Important subfolders:

```powershell
raw_sheets
source_icons
pixelated_icons
contact_sheets
```

Built-in image generation outputs are saved under:

```powershell
C:\Users\DoPra\.codex\generated_images\019dcef9-8af7-7973-af33-de675a1c8ad3
```

Copy generated sheets into:

```powershell
C:\UE\T66\SourceAssets\ItemSprites\_ReimaginedItemPass_20260427\raw_sheets
```

## Pixelation Parameters

The production pass uses the deterministic Python script, not a hand-tuned editor session.

Current parameters:

- Flatten each cell background using sampled corner color
- Normalize each source cell to `512x512`
- Downsample to a `128x128` working pixel grid
- Quantize to `56` colors
- Upscale back to `512x512` using nearest-neighbor

Pixel Composer is installed at:

```powershell
C:\Tools\PixelComposer\PixelComposer.exe
```

Pixel Composer CLI still needs a prepared `.pxc` graph with export nodes before it can replace the deterministic script.

## Commands

Generate the prompt pack:

```powershell
python .\Tools\Items\T66ProcessReimaginedItemSheets.py --write-prompts
```

Split all raw sheets, flatten backgrounds, pixelate icons, and build the review sheet:

```powershell
python .\Tools\Items\T66ProcessReimaginedItemSheets.py --process --contact-sheet
```

Review sheet:

```powershell
C:\UE\T66\SourceAssets\ItemSprites\_ReimaginedItemPass_20260427\contact_sheets\ReimaginedItems_pixelated_contact_sheet.png
```

Promote pixelated icons into the live source sprite names consumed by Unreal:

```powershell
python .\Tools\Items\T66ProcessReimaginedItemSheets.py --promote-live
```

Import the live source sprites into Unreal and reload `DT_Items`:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe' 'C:\UE\T66\T66.uproject' -ExecutePythonScript='C:\UE\T66\Scripts\RunImportItemSpritesAndExit.py'
```

Build after C++ localization changes:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -FromMsBuild
```

## Validation

Expected counts after a complete pass:

```powershell
(Get-ChildItem 'C:\UE\T66\SourceAssets\ItemSprites\_ReimaginedItemPass_20260427\raw_sheets' -File -Filter *.png).Count
(Get-ChildItem 'C:\UE\T66\SourceAssets\ItemSprites\_ReimaginedItemPass_20260427\source_icons' -File -Filter *.png).Count
(Get-ChildItem 'C:\UE\T66\SourceAssets\ItemSprites\_ReimaginedItemPass_20260427\pixelated_icons' -File -Filter *.png).Count
(Get-ChildItem 'C:\UE\T66\Content\Items\Sprites' -File -Filter 'Item_*.uasset').Count
```

Expected result:

```text
30
120
120
120
```

Check the Unreal import log:

```powershell
Select-String -Path 'C:\UE\T66\Saved\Logs\T66.log' -Pattern '\[ImportItemSprites\] Imported|Reloaded /Game/Data/DT_Items|Imported DataTable' | Select-Object -Last 5
```

Successful import should include:

```text
Imported DataTable 'DT_Items' - 0 Problems
[ImportItemSprites] Reloaded /Game/Data/DT_Items
[ImportItemSprites] Imported 120 textures into /Game/Items/Sprites
```

Run a whitespace check after script/data/code edits:

```powershell
git diff --check -- Content/Data/Items.csv Scripts/ImportItemSprites.py Source/T66/Core/T66LocalizationSubsystem.cpp Tools/Items/T66ProcessReimaginedItemSheets.py
```

## Live Item Rules

`Content/Data/Items.csv` is the source of truth for the live item set. `Item_Alchemy` is intentionally removed for now.

The live source sprite names must be:

```text
SourceAssets/ItemSprites/Item_<SeriesItemID>_<rarity>.png
```

Example:

```text
SourceAssets/ItemSprites/Item_AoeDamage_black.png
```

`Scripts/ImportItemSprites.py` imports only live rows from `Items.csv`. It should not skip `HpRegen` or `LifeSteal`; only `Alchemy` is deprecated right now.

## Name Mapping

The 4 unique treasure names per series are defined in:

```powershell
C:\UE\T66\Source\T66\Core\T66LocalizationSubsystem.cpp
```

Look for:

```text
T66GetReimaginedItemVariantName
```

`GetText_ItemDisplayNameForRarity` should return these unique treasure names instead of building `Worn/Fine/Grand/Divine + base name`.

## Cleanup

To detect unused top-level source PNGs after item list changes:

```powershell
$csv = Import-Csv -LiteralPath 'C:\UE\T66\Content\Data\Items.csv'
$live = @{}
foreach ($row in $csv) {
  foreach ($rarity in 'black','red','yellow','white') {
    $live["$($row.ItemID)_$rarity.png"] = $true
  }
}
Get-ChildItem -LiteralPath 'C:\UE\T66\SourceAssets\ItemSprites' -File -Filter 'Item_*.png' |
  Where-Object { -not $live.ContainsKey($_.Name) } |
  Select-Object -ExpandProperty Name
```

Remove only those unused top-level `Item_*.png` files. Do not delete `_ReimaginedItemPass_*` folders unless the pass archive is no longer needed.
