# ChatGPT Image Production Workflow

This file describes the exact workflow used to generate the item icon set through the ChatGPT web UI, split the results into per-rarity files, and import them back into the game.

This workflow does not use the OpenAI image API. It uses local browser automation against a Chrome session that is already logged into ChatGPT.

## Source Files

- Prompt pack: `C:\UE\T66\Item_Icon_Prompt_Pack.md`
- Prompt index: `C:\UE\T66\Item_Icon_Prompt_Index.csv`
- Bridge server: `C:\UE\T66\Tools\ChatGPTBridge\server.py`
- Batch generator: `C:\UE\T66\Tools\ChatGPTBridge\batch_generate.py`
- Bridge readme: `C:\UE\T66\Tools\ChatGPTBridge\README.md`
- Chrome launcher: `C:\UE\T66\Tools\ChatGPTBridge\launch_debug_chrome.ps1`
- Unreal import script: `C:\UE\T66\Scripts\RunImportItemSpritesAndExit.py`

## What This Produces

The standard production mode is:

- one ChatGPT conversation for the whole batch
- one `2x2` rarity sheet per item
- automatic split into:
  - `black.png`
  - `red.png`
  - `yellow.png`
  - `white.png`

The current art direction is:

- explicit Dota 2 item icon look
- same object across all four rarities
- flat solid rarity-color backgrounds
- bright artificial lighting
- minimal glow and effects
- no cast shadow or dark grounding shadow

## Setup

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

4. If cloud access is needed, expose it:

```powershell
ngrok http 5000
```

## Recommended Generation Mode

Always use:

- `--same-chat`
- `--grid-sheet`

That is the production-safe mode for this project.

## Preview Command

Use this first for one item before a full run:

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
.\.venv\Scripts\python.exe .\batch_generate.py `
  --grid-sheet `
  --same-chat `
  --filter "AOE Damage" `
  --limit 1 `
  --pause-seconds 8
```

## Full Batch Command

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
.\.venv\Scripts\python.exe .\batch_generate.py `
  --grid-sheet `
  --same-chat `
  --pause-seconds 8 `
  --continue-on-error
```

## Output Layout

Each run writes to:

- `C:\UE\T66\Tools\ChatGPTBridge\output\<run-name>.json`
- `C:\UE\T66\Tools\ChatGPTBridge\output\<run-name>\`

Each item folder looks like:

- `<item-slug>\sheet\sheet.png`
- `<item-slug>\sheet\sheet-alt-02.png` if ChatGPT returned multiple candidates
- `<item-slug>\icons\black.png`
- `<item-slug>\icons\red.png`
- `<item-slug>\icons\yellow.png`
- `<item-slug>\icons\white.png`

## Duplicate-Avoidance Rule

This was the failure mode that caused repeated `Gambler's Token` generations.

The fix is now built into `batch_generate.py` for `--same-chat` mode:

1. Before sending a title, the script snapshots the current image keys already visible in the chat.
2. It sends the prompt once.
3. If ChatGPT returns an image rate limit, the script waits for the cooldown and continues waiting for that same prompt.
4. It does not re-send the prompt just because a rate limit happened.
5. If image extraction fails after ChatGPT already produced something, the script first tries to harvest any images that appeared after the original snapshot before treating the item as failed.

Operational rule:

- never resend the same title manually just because ChatGPT said to wait 2 minutes
- let the batcher wait and harvest
- only retry a title after checking whether its `sheet.png` and split icons are already present on disk

## Recovery / Resume Rule

If a run stops partway through:

1. Check the run manifest in `Tools/ChatGPTBridge\output\`.
2. Check whether the missing item already has:
   - `sheet\sheet.png`
   - `icons\black.png`
   - `icons\red.png`
   - `icons\yellow.png`
   - `icons\white.png`
3. If those files exist, do not regenerate that item.
4. If they do not exist and the ChatGPT tab still shows the item result, harvest that result from the current chat before asking ChatGPT to generate the item again.
5. Only regenerate if there is no saved output and no recoverable result in the current chat.

## Importing the Generated Icons Into Unreal

The import flow used for this project was:

1. Copy or stage the final PNGs into:

- `C:\UE\T66\SourceAssets\ItemSprites`

2. Run the Unreal import script:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\UE\T66\T66.uproject" `
  -ExecutePythonScript="C:\UE\T66\Scripts\RunImportItemSpritesAndExit.py"
```

3. The imported assets land under:

- `/Game/Items/Sprites`

4. The item data table is then reloaded:

- `/Game/Data/DT_Items`

## Notes For Future Conversations

If you want a future Codex/chat session to repeat this workflow, tell it to:

- reference `C:\UE\T66\ChatGPT_Image_Production_Workflow.md`
- reference `C:\UE\T66\Item_Icon_Prompt_Pack.md`
- use `Tools/ChatGPTBridge\batch_generate.py` in `--grid-sheet --same-chat` mode
- preview one item first
- keep flat solid rarity backgrounds
- avoid resending prompts on rate limit

## Current Best Practice

- Preview one item.
- Approve the look.
- Run full batch in one chat.
- Let rate limits pause, not duplicate.
- Split sheets automatically.
- Import into Unreal only after the final set is complete.
