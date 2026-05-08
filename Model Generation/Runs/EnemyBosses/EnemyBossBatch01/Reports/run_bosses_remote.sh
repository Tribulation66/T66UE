#!/usr/bin/env bash
set -u

ROOT=/workspace/t66_enemyboss_bosses_stage01
INPUT="$ROOT/inputs"
OUTPUT="$ROOT/outputs"
LOGDIR="$ROOT/logs"
mkdir -p "$OUTPUT" "$LOGDIR"

SEED=1337
TEXTURE=2048
DECIMATION=200000
TOTAL=$(find "$INPUT" -maxdepth 1 -type f -name '*.png' | wc -l | tr -d ' ')

echo "[boss-stage01] start $(date -Is) total=$TOTAL seed=$SEED texture=$TEXTURE decimation=$DECIMATION"

idx=0
for img in $(find "$INPUT" -maxdepth 1 -type f -name '*.png' | sort); do
  idx=$((idx+1))
  base=$(basename "$img" .png)
  out="$OUTPUT/${base}_Trellis.glb"
  tmp="$out.tmp"

  if [ -s "$out" ]; then
    echo "[boss-stage01] skip existing $idx/$TOTAL $base"
    continue
  fi

  echo "[boss-stage01] generate $idx/$TOTAL $base $(date -Is)"
  rm -f "$tmp"
  if curl --fail --silent --show-error --max-time 3600 \
    -X POST \
    -H "X-Seed: $SEED" \
    -H "X-Texture-Size: $TEXTURE" \
    -H "X-Decimation: $DECIMATION" \
    --data-binary "@$img" \
    http://127.0.0.1:8000/generate \
    -o "$tmp"; then
    mv "$tmp" "$out"
    bytes=$(stat -c %s "$out")
    echo "[boss-stage01] done $idx/$TOTAL $base bytes=$bytes $(date -Is)"
  else
    code=$?
    rm -f "$tmp"
    echo "[boss-stage01] FAIL $idx/$TOTAL $base code=$code $(date -Is)"
  fi
done

outputs=$(find "$OUTPUT" -maxdepth 1 -type f -name '*_Trellis.glb' | wc -l | tr -d ' ')
echo "[boss-stage01] finish $(date -Is) outputs=$outputs"
