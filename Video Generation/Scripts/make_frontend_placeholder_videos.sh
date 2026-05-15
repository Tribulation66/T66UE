#!/usr/bin/env bash
set -euo pipefail

INPUT_ROOT="${1:-/workspace/T66/VideoGeneration/Inputs}"
OUTPUT_ROOT="${2:-/workspace/T66/VideoGeneration/Outputs}"

mkdir -p "${OUTPUT_ROOT}/HeroSelection"

ffmpeg -nostdin -hide_banner -y \
  -loop 1 \
  -framerate 30 \
  -t 5 \
  -i "${INPUT_ROOT}/mainmenu_clean.png" \
  -vf "scale=1920:1080,noise=alls=2:allf=t+u,eq=contrast=1.03:saturation=1.04,format=yuv420p" \
  -c:v libx264 \
  -profile:v high \
  -level 4.1 \
  -pix_fmt yuv420p \
  -r 30 \
  -movflags +faststart \
  "${OUTPUT_ROOT}/MainMenuBackground.mp4"

ffmpeg -nostdin -hide_banner -y \
  -loop 1 \
  -framerate 30 \
  -i "${INPUT_ROOT}/arthur_male_full.png" \
  -vf "scale=760:760,crop=712:680:24:40,zoompan=z='1+0.006*sin(on/18)':x='iw/2-(iw/zoom/2)':y='ih/2-(ih/zoom/2)':d=150:s=712x680:fps=30,noise=alls=2:allf=t+u,eq=contrast=1.04:saturation=1.03,format=yuv420p" \
  -frames:v 150 \
  -c:v libx264 \
  -profile:v high \
  -level 4.1 \
  -pix_fmt yuv420p \
  -r 30 \
  -movflags +faststart \
  "${OUTPUT_ROOT}/HeroSelection/Hero_1_Default_Chad.mp4"

ffprobe -v error \
  -select_streams v:0 \
  -show_entries stream=codec_name,width,height,r_frame_rate,pix_fmt,duration \
  -of default=noprint_wrappers=1 \
  "${OUTPUT_ROOT}/MainMenuBackground.mp4"

ffprobe -v error \
  -select_streams v:0 \
  -show_entries stream=codec_name,width,height,r_frame_rate,pix_fmt,duration \
  -of default=noprint_wrappers=1 \
  "${OUTPUT_ROOT}/HeroSelection/Hero_1_Default_Chad.mp4"
