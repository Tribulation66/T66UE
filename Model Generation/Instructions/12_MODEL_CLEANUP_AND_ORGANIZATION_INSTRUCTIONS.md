# Model Cleanup And Organization

Use this process after a Pixal3D/FriendSlop import or any model replacement pass. The goal is to keep playable assets current without keeping old processed variants, retry folders, smoke outputs, or one-off generation artifacts around as implicit dependencies.

## Storage Lanes

- Runtime assets live in `Content/` and can only be deleted after Unreal package proof and repo text/binary proof.
- Generated Pixal3D/TRELLIS output lives under `Model Generation/Runs/` and is not a runtime dependency. Keep durable summaries, manifests, and reusable lessons; delete retry/smoke/guideline output once the import is verified or rejected.
- Durable process knowledge belongs in `Model Generation/Instructions/`, `Model Generation/README.md`, or reusable scripts. It should not stay buried in run folders.

## Runtime Content Cleanup

Do not delete runtime model packages from a broad audit alone. Broad audits find candidates; exact audits clear deletion.

1. Refresh broad model audits.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\AuditCharacterModelDataAndExit.py" -NullRHI -unattended -nop4 -nosplash
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\AuditWorldAssetsAndExit.py" -NullRHI -unattended -nop4 -nosplash
```

2. Build the cleanup manifest.

```powershell
python Scripts\BuildModelCleanupCandidateManifest.py --output "Reports\Hygiene\<date>\model_cleanup_candidate_manifest.json"
```

3. Run the exact reference audit against the manifest's combined package list.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\AuditAssetReferencesAndExit.py" -T66AuditPackageFile="C:\UE\T66\Reports\Hygiene\<date>\model_cleanup_candidate_manifest_all_runtime_candidates_packages.txt" -T66AuditTokenMode=package_paths -T66AuditOutput="C:\UE\T66\Reports\Hygiene\<date>\exact_audit_all_runtime_candidates.json" -NullRHI -unattended -nop4 -nosplash
```

4. Convert the exact audit into a deletion gate. Use all runtime groups when the cleanup wave is intended to remove cross-group old assets together.

```powershell
$groups = ((Get-Content "Reports\Hygiene\<date>\model_cleanup_candidate_manifest.json" -Raw | ConvertFrom-Json).runtime_groups.PSObject.Properties | ForEach-Object { $_.Name }) -join ","
python Scripts\EvaluateModelCleanupExactAudit.py --manifest "Reports\Hygiene\<date>\model_cleanup_candidate_manifest.json" --exact-audit "Reports\Hygiene\<date>\exact_audit_all_runtime_candidates.json" --group all_runtime_candidates --groups $groups --output "Reports\Hygiene\<date>\gate_all_runtime_candidates.json"
```

5. Delete only `approved_packages`.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\UE\T66\T66.uproject" -run=pythonscript -script="C:\UE\T66\Scripts\DeleteModelCleanupAssetsAndExit.py" -T66ModelCleanupGate="C:\UE\T66\Reports\Hygiene\<date>\gate_all_runtime_candidates.json" -T66ModelCleanupDeleteReport="C:\UE\T66\Reports\Hygiene\<date>\delete_report_all_runtime_candidates.json" -NullRHI -unattended -nop4 -nosplash
```

6. Verify after deletion.

- Re-run the broad audits and rebuild a post-delete manifest.
- Keep skipped candidates listed with their reasons, usually `external_binary_token_match`.
- If playable content changed, refresh the staged standalone build and verify the taskbar shortcut target.

## Generated Run Cleanup

Generated run folders are cleaned from the same manifest. The cleaner deletes only entries marked `delete_after_summary`, and refuses paths outside the manifest runs root.

```powershell
python Scripts\CleanModelGenerationRuns.py --manifest "Reports\Hygiene\<date>\model_cleanup_candidate_manifest.json" --output "Reports\Hygiene\<date>\generated_run_cleanup_dry_run.json" --dry-run
python Scripts\CleanModelGenerationRuns.py --manifest "Reports\Hygiene\<date>\model_cleanup_candidate_manifest.json" --output "Reports\Hygiene\<date>\generated_run_cleanup_report.json"
```

Keep folders marked `keep_review` until the import provenance is settled. Current FriendSlop production runs, probes, and archived provenance should survive ordinary cleanup.

Current keep-review FriendSlop runs:

- `Model Generation/Runs/Pixal3D/FriendSlopEasyBatch_20260604_1532`
- `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415`

Archived legacy model generations belong under `Model Generation/Runs/Pixal3D/Archive/`. Do not leave old AccuRig / ToonStyle hero demo generations at the root of `Runs/Pixal3D`, because they are easy to mistake for active FriendSlop sources.

## Skip Rules

- Do not delete whole hero folders unless the current audit explicitly marks the whole folder safe.
- Do not delete Backrooms texture sets during model cleanup; `Content/World/pending_issues_World.md` records those as intentionally preserved.
- Do not treat report/script/generated-folder text hits as runtime blockers, but do record them.
- Treat `Source/`, `Config/`, and `Content/Data/` exact package/object path hits as blockers.
- Do not use broad Git/LFS scans as proof for Unreal asset cleanup.
