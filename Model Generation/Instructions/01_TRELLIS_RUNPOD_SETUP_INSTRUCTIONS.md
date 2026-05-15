# TRELLIS RunPod Setup

Use RunPod for TRELLIS generation. Do not rely on local laptop GPU generation for production model batches.

## Durable Helpers

- `Model Generation/Scripts/Core/Trellis/bootstrap_trellis2_pod.sh`
- `Model Generation/Scripts/Core/Trellis/Invoke-RunPodHfLogin.ps1`
- `Model Generation/Tools/Trellis2/trellis_server.py`

## Setup Rules

- Keep live pod IPs, SSH ports, tokens, and secrets out of repo docs.
- Authenticate Hugging Face before starting a fresh TRELLIS server.
- Generate checked-in shell scripts with LF line endings.
- Check pod logs before deciding a long generation is hung.
- Treat long first runs as normal when dependencies or model weights are being downloaded.

## Local Access

Local access files and `.env` material are not durable project files. Put connection details in the active chat or local secret storage, not in `Model Generation`.
