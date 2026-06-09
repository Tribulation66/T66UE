# Codex CLI Image Generation Workflow

This workflow keeps generated images out of the Codex chat body. The local Codex CLI does not expose a standalone `codex imagegen` subcommand, but `codex features list` confirms `image_generation` is enabled and stable. A smoke test generated a PNG through `codex exec` and saved it to `C:\UE\T66\tmp\codex_cli_imagegen_probe\probe.png`.

Use `ToonStyle/Tools/RunCodexImageGen.ps1` for project-bound generations. It asks a noninteractive Codex CLI run to use the built-in image-generation capability, copy the selected PNG into the workspace, and return only the saved path.

## Default Command

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\ToonStyle\Tools\RunCodexImageGen.ps1 `
  -Prompt "flat-color cel-shaded front-view concept art of Lu Bu for T66, clean anime shapes, no grain, no dithering" `
  -Name "lubu_front_v01" `
  -OutDir "SourceAssets\ToonStyle\ImageGen\Phase1A\LuBu" `
  -Size 1024x1024 `
  -Quality high
```

## Prompt File Command

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\ToonStyle\Tools\RunCodexImageGen.ps1 `
  -PromptFile "ToonStyle\Docs\Prompts\lubu_front_v01.txt" `
  -Name "lubu_front_v01" `
  -OutDir "SourceAssets\ToonStyle\ImageGen\Phase1A\LuBu"
```

## Output Convention

- Store ToonStyle references under `SourceAssets\ToonStyle\ImageGen\...` unless a phase prompt gives a more specific path.
- Use stable asset names such as `lubu_front_v01.png`, `gambler_prop_sheet_v02.png`, or `test_room_wall_north_v01.png`.
- In chat handoffs, return Markdown links or absolute paths only. Do not use `![](...)` image embeds unless Pablo explicitly asks to display the image in the thread.

## What This Uses

- Uses local Codex CLI auth and the built-in image-generation feature.
- Does not require `OPENAI_API_KEY`.
- Does not call the OpenAI API fallback script by default.
- Uses `codex exec --dangerously-bypass-approvals-and-sandbox` so the noninteractive CLI run can copy the generated PNG into the workspace. Use this wrapper only with trusted local prompts.

## Troubleshooting

- If the wrapper reports `IMAGE_TOOL_UNAVAILABLE`, run `codex features list` and confirm `image_generation` is `true`.
- If the image is generated but the final copy is missing, inspect `tmp\codex_imagegen_cli\<run-name>\last_message.txt` and `codex_stdout.txt`.
- Plugin marketplace `403` warnings during `codex exec` are non-blocking if the output PNG is created.
- The system imagegen skill also ships an API fallback at `%USERPROFILE%\.codex\skills\.system\imagegen\scripts\image_gen.py`, but that path requires `OPENAI_API_KEY` and should not be the default T66 workflow.
