# ChatGPT Image Production Workflow

This file describes the current workflow for generating game art through the ChatGPT web UI, saving the finished images locally, and importing them back into Unreal.

This workflow does not use the OpenAI image API. It uses local browser automation against a Chrome session that is already logged into ChatGPT.

## Canonical Files

- Workflow doc: `C:\UE\T66\Docs\Art\ChatGPT_Image_Production_Workflow.md`
- Item prompt pack: `C:\UE\T66\Docs\Art\Item_Icon_Prompt_Pack.md`
- Item prompt index: `C:\UE\T66\Docs\Art\Item_Icon_Prompt_Index.csv`
- Bridge server: `C:\UE\T66\Tools\ChatGPTBridge\server.py`
- Item batch generator: `C:\UE\T66\Tools\ChatGPTBridge\batch_generate.py`
- Idol batch generator: `C:\UE\T66\Tools\ChatGPTBridge\idol_batch_generate.py`
- Bridge setup readme: `C:\UE\T66\Tools\ChatGPTBridge\README.md`
- Chrome launcher: `C:\UE\T66\Tools\ChatGPTBridge\launch_debug_chrome.ps1`
- Item import runner: `C:\UE\T66\Scripts\RunImportItemSpritesAndExit.py`
- Idol import runner: `C:\UE\T66\Scripts\RunImportIdolSpritesAndExit.py`

## Core Rules

- Use one ChatGPT conversation for the whole batch unless there is a specific reason not to.
- Always preview one image first and approve the look before running the batch.
- When ChatGPT shows an image rate limit, wait for the cooldown. Do not resend the same prompt just because rate limiting happened.
- If an item or idol already has `sheet.png` and the four split rarity files on disk, skip it.
- If ChatGPT already produced the image in the conversation but the batch missed it, harvest the existing image from the chat instead of regenerating it.
- Save the final settled render, not the first transient asset that appears in the DOM.

## What the Current Pipeline Produces

The standard production mode is:

- one ChatGPT conversation for the whole batch
- one `2x2` sheet per item or idol
- automatic split into:
  - `black.png`
  - `red.png`
  - `yellow.png`
  - `white.png`

This is the safest mode for both quality review and rate-limit handling.

## Bridge Setup

1. Launch the dedicated Chrome debug profile:

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
powershell -ExecutionPolicy Bypass -File .\launch_debug_chrome.ps1
```

2. In that Chrome window:

- log into ChatGPT
- stay on `https://chatgpt.com`
- keep that browser open

3. Start the local bridge server:

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
$env:CHATGPT_BRIDGE_TOKEN = "replace-with-a-long-random-secret"
.\.venv\Scripts\python.exe .\server.py --host 0.0.0.0 --port 5000
```

4. If the cloud sandbox needs to reach your machine, expose it:

```powershell
ngrok http 5000
```

## Reliability Safeguards In The Current Scripts

These behaviors are part of the current implementation and should be preserved in future conversations:

- `batch_generate.py` in `--same-chat` mode snapshots the current visible images before sending a prompt, then tries to harvest only newly appeared results.
- If ChatGPT returns a rate-limit message, the batcher sleeps for the requested cooldown and keeps waiting for the same prompt instead of resending it.
- If output files for an item already exist, the batcher should treat that item as complete and skip it.
- `idol_batch_generate.py` skips idols whose split files already exist on disk.
- `server.py` waits for render stability before saving an image so the final saved output matches the final image shown in ChatGPT.
- `server.py` supports local reference-image upload by path when the workflow requires a real reference image, not just a text description.

## Item Workflow

Use this for item icons and other object-style art where the prompts are already prepared in the item prompt pack.

### Recommended Preview Command

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
.\.venv\Scripts\python.exe .\batch_generate.py `
  --grid-sheet `
  --same-chat `
  --filter "AOE Damage" `
  --limit 1 `
  --pause-seconds 8
```

### Recommended Full Batch Command

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
.\.venv\Scripts\python.exe .\batch_generate.py `
  --grid-sheet `
  --same-chat `
  --pause-seconds 8 `
  --continue-on-error
```

### Item Art Direction Notes

- same object across all four rarities
- solid single-color rarity backgrounds
- bright artificial lighting
- minimal glow and effects
- no cast shadow or grounding shadow
- explicit Dota 2 item-icon look when that art direction is desired

## Idol Workflow

Use this for idol skulls and any future asset set where ChatGPT needs to work from an approved reference image inside the conversation.

### Idol Setup Pattern

1. Start a fresh or dedicated ChatGPT conversation for idols.
2. Send setup instructions that explain the art direction and explicitly tell ChatGPT to wait for the reference image before generating.
3. Attach the actual reference image in that conversation.
4. Generate one preview idol first.
5. Approve the look.
6. Run the remaining idols in the same conversation.

### Idol Prompting Rules

- Use the attached reference image already in the chat.
- Keep the skull silhouette and overall crystal-skull design locked to the reference.
- Change only the internal content, color, and element-specific interior details.
- Hard-lock the interior dominant color in every idol prompt to avoid drift.
- Hard-lock the interior motif and shape language in every idol prompt to avoid drift.
- Request all four rarities as a single `2x2` output so the aspect ratio stays square and consistent.
- Keep the background as a solid rarity color with no gradient.

### Recommended Idol Batch Mode

Use `idol_batch_generate.py` against the existing ChatGPT conversation URL that already contains:

- the attached reference image
- the approved art-direction setup
- the approved preview result

That script is intended to continue in the same conversation instead of creating a new one per idol.

## Output Layout

Each run writes to:

- `C:\UE\T66\Tools\ChatGPTBridge\output\<run-name>.json`
- `C:\UE\T66\Tools\ChatGPTBridge\output\<run-name>\`

Each item or idol folder looks like:

- `<slug>\sheet\sheet.png`
- `<slug>\sheet\sheet-alt-02.png` if ChatGPT returned multiple candidates
- `<slug>\icons\black.png`
- `<slug>\icons\red.png`
- `<slug>\icons\yellow.png`
- `<slug>\icons\white.png`

## Resume And Recovery Rules

If a run stops partway through:

1. Check the run manifest in `C:\UE\T66\Tools\ChatGPTBridge\output\`.
2. Check whether the missing asset already has:
   - `sheet\sheet.png`
   - `icons\black.png`
   - `icons\red.png`
   - `icons\yellow.png`
   - `icons\white.png`
3. If those files exist, do not regenerate it.
4. If those files do not exist but the ChatGPT conversation still shows the generated result, harvest that result from the current chat before asking ChatGPT to generate again.
5. Only regenerate if there is no saved output and no recoverable result in the current chat.

This rule exists because duplicate regeneration wastes the ChatGPT image allowance and was the failure mode that caused repeated `Gambler's Token` generations.

## Importing Generated Files Into Unreal

### Items

1. Stage the final PNGs into:

- `C:\UE\T66\SourceAssets\ItemSprites`

2. Run:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\UE\T66\T66.uproject" `
  -ExecutePythonScript="C:\UE\T66\Scripts\RunImportItemSpritesAndExit.py"
```

3. Imported assets land under:

- `/Game/Items/Sprites`

4. The data table reload target is:

- `/Game/Data/DT_Items`

### Idols

1. Stage the final PNGs into:

- `C:\UE\T66\SourceAssets\IdolSprites`

2. Run:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\UE\T66\T66.uproject" `
  -ExecutePythonScript="C:\UE\T66\Scripts\RunImportIdolSpritesAndExit.py"
```

3. Imported assets land under:

- `/Game/Idols/Sprites`

4. The data table reload target is:

- `/Game/Data/DT_Idols`

## What To Tell A Future Chat

If a future Codex or ChatGPT conversation needs to continue this pipeline, tell it to:

- reference `C:\UE\T66\Docs\Art\ChatGPT_Image_Production_Workflow.md`
- use the existing bridge scripts in `C:\UE\T66\Tools\ChatGPTBridge`
- preview one image first
- use one conversation per batch
- wait through rate limits instead of resending prompts
- skip anything already present on disk
- harvest existing chat outputs before regenerating
- save the final settled render

## Current Best Practice

- Preview one image.
- Approve the look.
- Run the batch in one conversation.
- Let rate limits pause the run instead of causing duplicates.
- Split sheets automatically.
- Import into Unreal only after the set is complete.
