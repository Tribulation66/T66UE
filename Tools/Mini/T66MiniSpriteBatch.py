from __future__ import annotations

import argparse
import json
import shutil
import sys
import urllib.error
import urllib.request
from datetime import UTC, datetime
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = PROJECT_ROOT / "Docs" / "Mini" / "T66Mini_SpriteBatchManifest.json"
DEFAULT_BRIDGE_URL = "http://127.0.0.1:5000/generate?mode=json"
DEFAULT_TOKEN = "t66-mini-local"


def utc_now() -> str:
	return datetime.now(UTC).isoformat()


def load_manifest(path: Path) -> dict:
	return json.loads(path.read_text(encoding="utf-8"))


def post_generation(bridge_url: str, token: str, payload: dict) -> dict:
	request = urllib.request.Request(
		bridge_url,
		data=json.dumps(payload).encode("utf-8"),
		headers={
			"Content-Type": "application/json",
			"X-Bridge-Token": token,
		},
		method="POST",
	)

	try:
		with urllib.request.urlopen(request, timeout=int(payload.get("timeout_seconds", 180)) + 90) as response:
			return json.loads(response.read().decode("utf-8"))
	except urllib.error.HTTPError as exc:
		body = exc.read().decode("utf-8", errors="replace")
		try:
			payload = json.loads(body)
			message = payload.get("error", body)
		except json.JSONDecodeError:
			message = body
		raise RuntimeError(f"Bridge request failed ({exc.code}): {message}") from exc
	except urllib.error.URLError as exc:
		raise RuntimeError(f"Bridge request failed: {exc.reason}") from exc


def ensure_dir(path: Path) -> None:
	path.mkdir(parents=True, exist_ok=True)


def resolve_project_path(raw_path: str) -> Path:
	return (PROJECT_ROOT / raw_path).resolve()


def copy_generated_files(batch: dict, response: dict, batch_output_dir: Path) -> list[dict]:
	results: list[dict] = []
	for index, file_info in enumerate(response.get("files", []), start=1):
		source_path = Path(file_info["saved_file"]).resolve()
		if not source_path.is_file():
			raise RuntimeError(f"Bridge reported generated file that does not exist: {source_path}")

		target_name = f"{batch['id']}-{index:02d}{source_path.suffix.lower()}"
		target_path = batch_output_dir / target_name
		shutil.copy2(source_path, target_path)
		results.append(
			{
				"source_file": str(source_path),
				"copied_file": str(target_path),
				"mime_type": file_info.get("mime_type", "image/png"),
				"size_bytes": file_info.get("size_bytes", 0),
			}
		)
	return results


def run_batch(batch: dict, bridge_url: str, token: str, dry_run: bool) -> dict:
	output_root = resolve_project_path(batch["output_dir"])
	batch_output_dir = output_root / batch["id"]
	ensure_dir(batch_output_dir)

	manifest_path = batch_output_dir / "batch_manifest.json"
	if batch.get("skip_if_done", True) and manifest_path.is_file():
		return {
			"id": batch["id"],
			"status": "skipped",
			"reason": "manifest_exists",
			"output_dir": str(batch_output_dir),
		}

	reference_paths = [str(resolve_project_path(path)) for path in batch.get("image_paths", [])]

	request_payload = {
		"prompt": batch["prompt"],
		"image_paths": reference_paths,
		"timeout_seconds": int(batch.get("timeout_seconds", 240)),
	}

	if dry_run:
		return {
			"id": batch["id"],
			"status": "dry_run",
			"output_dir": str(batch_output_dir),
			"request": request_payload,
		}

	response = post_generation(bridge_url, token, request_payload)
	copied_files = copy_generated_files(batch, response, batch_output_dir)

	batch_manifest = {
		"id": batch["id"],
		"title": batch.get("title", batch["id"]),
		"category": batch.get("category", "unknown"),
		"created_at": utc_now(),
		"output_dir": str(batch_output_dir),
		"prompt": batch["prompt"],
		"reference_images": reference_paths,
		"bridge_response": response,
		"copied_files": copied_files,
	}
	manifest_path.write_text(json.dumps(batch_manifest, indent=2), encoding="utf-8")

	return {
		"id": batch["id"],
		"status": "generated",
		"output_dir": str(batch_output_dir),
		"count": len(copied_files),
	}


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="Run mini-specific sprite batches through the local ChatGPT bridge.")
	parser.add_argument("--manifest", default=str(DEFAULT_MANIFEST), help="Path to the mini sprite batch manifest.")
	parser.add_argument("--bridge-url", default=DEFAULT_BRIDGE_URL, help="Bridge JSON endpoint.")
	parser.add_argument("--token", default=DEFAULT_TOKEN, help="Bridge auth token.")
	parser.add_argument("--only", nargs="*", default=[], help="Optional batch IDs to generate.")
	parser.add_argument("--dry-run", action="store_true", help="Print batch requests without calling the bridge.")
	return parser.parse_args()


def main() -> int:
	args = parse_args()
	manifest = load_manifest(Path(args.manifest).resolve())
	selected_ids = set(args.only)

	results = []
	for batch in manifest.get("batches", []):
		batch_id = batch["id"]
		if selected_ids and batch_id not in selected_ids:
			continue

		print(f"[T66MiniSpriteBatch] {batch_id}: starting")
		try:
			result = run_batch(batch, args.bridge_url, args.token, args.dry_run)
			results.append(result)
			print(f"[T66MiniSpriteBatch] {batch_id}: {result['status']}")
		except Exception as exc:
			print(f"[T66MiniSpriteBatch] {batch_id}: failed: {exc}", file=sys.stderr)
			return 1

	run_summary = {
		"completed_at": utc_now(),
		"results": results,
	}
	summary_path = PROJECT_ROOT / "SourceAssets" / "Mini" / "last_batch_run.json"
	ensure_dir(summary_path.parent)
	summary_path.write_text(json.dumps(run_summary, indent=2), encoding="utf-8")
	return 0


if __name__ == "__main__":
	raise SystemExit(main())
