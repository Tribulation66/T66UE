#!/usr/bin/env bash
set -euo pipefail

# Run on the RunPod host. This script assumes the repo source plates have already
# been uploaded to /workspace/T66/VideoGeneration/Inputs.

ROOT="/workspace/T66/VideoGeneration"
REPO="${ROOT}/Repos/LTX-Video"
CONFIG="${ROOT}/t66_ltxv-2b-0.9.8-distilled-no-enhance.yaml"
OUT="${ROOT}/OutputsAI"

export HF_HUB_DISABLE_XET=1

mkdir -p "${ROOT}/Repos" "${OUT}"

if [ ! -d "${REPO}/.git" ]; then
  git clone https://github.com/Lightricks/LTX-Video.git "${REPO}"
fi

cd "${REPO}"
python3 -m pip install --upgrade pip setuptools wheel
python3 -m pip install -e '.[inference]'
python3 -m pip install 'diffusers==0.35.2'

sed 's/prompt_enhancement_words_threshold: 120/prompt_enhancement_words_threshold: 0/' \
  "${REPO}/configs/ltxv-2b-0.9.8-distilled.yaml" > "${CONFIG}"

python3 - <<'PY'
from huggingface_hub import hf_hub_download
for filename in [
    "ltxv-2b-0.9.8-distilled.safetensors",
    "ltxv-spatial-upscaler-0.9.8.safetensors",
]:
    print(hf_hub_download(repo_id="Lightricks/LTX-Video", filename=filename, repo_type="model"))
PY

rm -rf "${OUT}/MainMenu_LTX2B_20260513" "${OUT}/Arthur_LTX2B_20260513"
mkdir -p "${OUT}/MainMenu_LTX2B_20260513" "${OUT}/Arthur_LTX2B_20260513"

python3 inference.py \
  --pipeline_config "${CONFIG}" \
  --prompt 'Locked camera cinematic game main menu background. The golden idol statue and altar remain centered and recognizable. Stars twinkle in the black sky. A blue comet shoots across the upper sky once. The fiery eclipse ring behind the idol flickers like solar plasma, glowing and behaving like a living sun. Subtle warm gold reflections shimmer on the statue and water. Atmospheric dark fantasy menu loop, no UI, no text, no cuts.' \
  --negative_prompt 'camera movement, zoom, pan, statue movement, face morphing, altar deformation, changing geometry, melting gold, extra characters, text, UI, logo, smoke covering idol, glitch artifacts, distortion, blur, extra objects' \
  --conditioning_media_paths "${ROOT}/Inputs/mainmenu_clean.png" \
  --conditioning_start_frames 0 \
  --conditioning_strengths 0.82 \
  --image_cond_noise_scale 0.08 \
  --height 544 \
  --width 960 \
  --num_frames 121 \
  --frame_rate 24 \
  --seed 660211 \
  --output_path "${OUT}/MainMenu_LTX2B_20260513"

python3 inference.py \
  --pipeline_config "${CONFIG}" \
  --prompt 'Single continuous locked camera dark fantasy menu video. King Arthur, a handsome battle worn crusader king in heavy plate armor with a red cloak, sits on a stone throne in a torchlit throne room. He slowly rises from the throne, steps forward with calm authority, and unsheathes a shining sword from his side. Cinematic warm torchlight, deep shadows, heroic centered composition, no text, no UI, no cuts.' \
  --negative_prompt 'camera pan, camera zoom, shaky camera, fast movement, face morphing, extra characters, duplicate body, extra arms, extra legs, bad hands, text, UI, logo, modern clothing, helmet covering face, melting armor, glitch artifacts, hard blur, scene cuts' \
  --height 672 \
  --width 704 \
  --num_frames 121 \
  --frame_rate 24 \
  --seed 660331 \
  --output_path "${OUT}/Arthur_LTX2B_20260513"

mkdir -p "${OUT}/RuntimeEncoded/HeroSelection"
MAIN_SOURCE="$(find "${OUT}/MainMenu_LTX2B_20260513" -name '*.mp4' | head -n 1)"
ARTHUR_SOURCE="$(find "${OUT}/Arthur_LTX2B_20260513" -name '*.mp4' | head -n 1)"

ffmpeg -nostdin -y -i "${MAIN_SOURCE}" \
  -vf 'scale=1920:1080:force_original_aspect_ratio=increase,crop=1920:1080,fps=30,format=yuv420p' \
  -an -c:v libx264 -preset slow -crf 20 -movflags +faststart \
  "${OUT}/RuntimeEncoded/MainMenuBackground.mp4"

ffmpeg -nostdin -y -i "${ARTHUR_SOURCE}" \
  -vf 'scale=712:680:force_original_aspect_ratio=increase,crop=712:680,fps=30,format=yuv420p' \
  -an -c:v libx264 -preset slow -crf 20 -movflags +faststart \
  "${OUT}/RuntimeEncoded/HeroSelection/Hero_1_Default_Chad.mp4"

ffprobe -v error -show_entries stream=codec_name,width,height,pix_fmt,r_frame_rate,duration \
  -of default=noprint_wrappers=1 "${OUT}/RuntimeEncoded/MainMenuBackground.mp4"
ffprobe -v error -show_entries stream=codec_name,width,height,pix_fmt,r_frame_rate,duration \
  -of default=noprint_wrappers=1 "${OUT}/RuntimeEncoded/HeroSelection/Hero_1_Default_Chad.mp4"
