# Video Generation Scripts

Reusable helpers for RunPod and local movie preparation belong here. Task-specific scripts should be deleted after their run, with durable improvements folded into a reusable helper or instruction doc.

The first runtime slice generated placeholder MP4s directly on the RunPod host with `ffmpeg`.

`build_frontend_video_catalog.py` is the roster-wide catalog builder:

- reads the active hero and companion CSVs;
- writes the runtime catalog and source job manifest;
- creates one prompt file per hero skin/body and companion skin;
- creates poster plates under `RuntimeDependencies/T66/Video/Posters`;
- optionally encodes placeholder H.264 loops for every catalog entry.

`validate_frontend_video_catalog.py` checks that the runtime catalog has complete active-roster coverage and that every registered MP4/poster exists.

`run_ltx2b_frontend_videos.sh` documents the first real AI generation pass for the main menu plus Arthur:

- installs `Lightricks/LTX-Video`;
- pins `diffusers==0.35.2` for the pod's Torch 2.4 CUDA environment;
- disables Hugging Face Xet downloads with `HF_HUB_DISABLE_XET=1`;
- runs `ltxv-2b-0.9.8-distilled`;
- encodes the outputs into the game-ready MP4 slots.

`run_ltx2b_frontend_roster_batch.py` is the roster-wide RunPod batch runner:

- reads `Video Generation/Manifests/frontend_video_jobs.json`;
- uses each generated poster plate as LTX image conditioning;
- targets jobs still marked as `placeholder_generated`;
- writes encoded H.264 clips under `OutputsAI/.../RuntimeEncoded` using the same relative movie paths as `Content/Movies`;
- writes a JSONL status file so local manifests can be updated after download.

`kling_client.py` and `run_kling_reference_pilot.py` are the native Kling API
smoke/capability helpers:

- read local credentials from `Model Generation/LOCAL_ACCESS.env`;
- write run evidence under `Video Generation/Runs/KlingReferencePilot_*`;
- check image-to-video, text-to-video, and image-generation endpoints before
  spending credits;
- submit a single explicit smoke task only with `--submit-smoke-task`;
- accept a project-local source image override with `--source-image` and a
  run-specific prompt with `--prompt-file` for imagegen-first tests;
- keep downloaded and encoded candidates in the run folder until a clip is
  hand-approved for copying into `Content/Movies`.

`run_kling_demo_idle_roster.py` is the demo-roster idle-loop installer:

- reads `Video Generation/Manifests/kling_demo_idle_roster.json`;
- uses run-local first-frame images as Kling image-to-video anchors;
- writes one evidence folder per target under `Video Generation/Runs`;
- encodes review/runtime candidates to 712x680 H.264/yuv420p/30fps/no-audio;
- writes contact sheets and first/last-frame loop metrics;
- only copies successful outputs into `Content/Movies` and flips
  `posterOnly=false` when explicitly run with `--install --update-manifests`.

Current durable scripts are intentionally kept even after a run completes because they encode reusable setup, catalog, validation, and roster-batch behavior. One-off commands and pod-local throwaway scripts should instead be folded into these helpers or into a run README.
