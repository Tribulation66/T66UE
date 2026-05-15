# Video Generation Agents

## Owns

Frontend video-generation workflow, RunPod video setup, source prompts, source plates, generated MP4 review copies, runtime movie manifest coordination, and handoff notes for replacing placeholder clips.

## Trigger Words

video generation, RunPod video, WAN, LTX, ComfyUI, main menu video, hero selection video, cinematic panel, MP4, poster frame, frontend video.

## Read First

- `Video Generation/README.md`
- `Video Generation/Instructions/00_VIDEO_GENERATION_ROUTING_INSTRUCTIONS.md`
- `Video Generation/Manifests/frontend_video_jobs.json`
- `RuntimeDependencies/T66/Video/frontend_videos.json`

## Hard Rules

- Do not store API keys, pod private keys, Hugging Face tokens, or RunPod tokens in this folder.
- Generated videos are not runtime content until copied to `Content/Movies`, encoded as packaged-safe MP4, and registered in `RuntimeDependencies/T66/Video/frontend_videos.json`.
- Keep source prompts, manifests, and review notes here; keep shipped movie files under `Content/Movies`.
- Preserve static fallback images for every runtime video entry.

## Verification

Report pod reachability, generation command/workflow, output file size, codec/probe evidence, Unreal compile result, and staged standalone evidence when the video affects the playable build.
