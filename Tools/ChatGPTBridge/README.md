# ChatGPT Image Bridge

This is a local bridge that lets a remote caller send a prompt to your PC, drive your already-logged-in ChatGPT browser session via Playwright, wait for an image, and return that image as the HTTP response.

This is UI automation. It is inherently brittle compared to using the API directly. It is also security-sensitive because the tunnel can drive your logged-in ChatGPT account, so use an auth token.

## Files

- `server.py`: Flask app with `/health` and `/generate`
- `request_generate.py`: lightweight client for prompt-file plus reference-image requests
- `requirements.txt`: Python dependencies
- `launch_debug_chrome.ps1`: launches Chrome with remote debugging enabled

## Setup

1. Create and activate a virtual environment.

```powershell
cd C:\UE\T66\Tools\ChatGPTBridge
python -m venv .venv
.venv\Scripts\Activate.ps1
pip install -r requirements.txt
python -m playwright install chromium
```

2. Launch a dedicated Chrome profile with remote debugging.

```powershell
powershell -ExecutionPolicy Bypass -File .\launch_debug_chrome.ps1
```

3. In that Chrome window, log into ChatGPT and verify you can manually generate images.

4. Set a bridge auth token and start the server.

```powershell
$env:CHATGPT_BRIDGE_TOKEN = "replace-this-with-a-long-random-secret"
python .\server.py --host 0.0.0.0 --port 5000
```

5. Expose it with ngrok or Cloudflare Tunnel.

```powershell
ngrok http 5000
```

## Health Check

```powershell
curl http://127.0.0.1:5000/health
```

## Generate Endpoint

`POST /generate`

Headers:

- `Content-Type: application/json`
- `X-Bridge-Token: <your token>`

Request body:

```json
{
  "prompt": "bone explosion vfx white bg"
}
```

The default response is the generated image binary.

Local example:

```powershell
curl.exe -X POST "http://127.0.0.1:5000/generate" `
  -H "Content-Type: application/json" `
  -H "X-Bridge-Token: replace-this-with-a-long-random-secret" `
  --data "{\"prompt\":\"bone explosion vfx white bg\"}" `
  --output output.png
```

Remote example from a cloud sandbox:

```bash
curl -X POST "https://your-subdomain.ngrok.app/generate" \
  -H "Content-Type: application/json" \
  -H "X-Bridge-Token: replace-this-with-a-long-random-secret" \
  --data '{"prompt":"bone explosion vfx white bg"}' \
  --output output.png
```

## Optional JSON Mode

If you call `/generate?mode=json`, the response is JSON metadata instead of raw image bytes. The bridge still saves the image locally under `Tools/ChatGPTBridge/output/`.

## Repo-Friendly Client

Use the helper client when the prompt already lives in a repo file and the request should attach local reference images.

Example:

```powershell
$env:CHATGPT_BRIDGE_TOKEN = "replace-this-with-a-long-random-secret"
python .\request_generate.py `
  --prompt-file C:\UE\T66\Docs\UI\PromptPacks\HeroSelectionSpriteSheets\master_frame_prompt.txt `
  --image C:\UE\T66\SourceAssets\UI\MainMenuReference\reference_main_menu_master.png `
  --image C:\UE\T66\SourceAssets\UI\HeroSelectionReference\current_runtime_anchor.png `
  --image C:\UE\T66\SourceAssets\UI\HeroSelectionReference\current_runtime_anchor_2x.png `
  --json-out C:\UE\T66\Tools\ChatGPTBridge\output\hero_selection_master_request.json `
  --copy-to-dir C:\UE\T66\SourceAssets\UI\HeroSelectionReference
```

The client posts to `/generate?mode=json`, saves the metadata payload, and can optionally copy the generated files into a chosen destination folder.

If you already have a repo-side request manifest, use:

```powershell
python .\request_generate.py --request-file C:\UE\T66\Docs\UI\PromptPacks\HeroSelectionSpriteSheets\master_frame_bridge_request.json
```

## Notes

- The bridge opens a fresh ChatGPT tab per request so image detection does not get confused by old conversation history.
- Only one generation request is processed at a time.
- For mini-game sprite work, prefer 2x2 sheets that pack four related subjects into one image so a single generation covers a complete set.
- Do not regenerate the same requested sprite sheet more than once unless there is a serious issue with the output.
- If ChatGPT changes its DOM, update the selectors in `server.py`.
- If the bridge can see the new image but cannot fetch the original bytes, it falls back to a screenshot of the rendered image.
