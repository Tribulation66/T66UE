# Operator Packet

- **Task:** Deprecated-code cleanup plan packet (one-pass deletion of dead/deprecated code accumulated across lightweight-mob and roster-restructure work), with CoreRedirect removal conditional on verified asset safety.
- **Operator:** Claude (`claude-opus-4-8`), FullOperator with report-only write scope.
- **Validator:** Codex.
- **Phase:** Planning only. This packet authorizes no edits. Implementation requires a separate Codex-approved FullOperator phase.
- **Write scope honored:** This run wrote exactly one file — this packet. No source/data/config/content/script/staged-build edits, no deletes, no build/stage/capture, no git operations, no broad Git/LFS scans.
- **Anchors re-verified live this pass** (not carried over blindly from the prior read-only draft). One material correction surfaced versus the earlier draft — see Live Anchor Findings #3 and Risks/Decisions R1.

---

# Task Contract

**In scope (deletion targets):**
1. Rich-basic-mob spawn path only, preserving rich miniboss / special / boss spawn paths.
2. Neutralized lightweight/routing/touch CVars (now inert / diagnostic-only).
3. Deprecated enemy/boss projectile actor classes — **subject to live-usage verification** (see R1).
4. GamblerToken legacy enum value, legacy-named save persistence fields, and the `Item_GamblersToken` alias. Per Pablo's explicit decision: no save preservation, no migration; legacy remnants deleted outright.
5. The approved B.13 sandbox worktree `C:\UE\T66_B13_Worktree`.
6. `Config/DefaultEngine.ini` CoreRedirects for the mob-floor property rename and VendorToken function rename — **only if** a source + binary-asset scan proves the old names are unreferenced.

**Out of scope / preserve:**
- Rich miniboss / special / boss code paths.
- Mini / minigame systems.
- Any production behavior change (cleanup is intended to be behavior-neutral).

**Process constraints:** No broad Git/LFS scans. No git stage/commit/revert/clean. No implementation until Codex validates this plan and writes approval.

---

# Live Anchor Findings

All paths relative to `C:\UE\T66`. Line numbers are current as of this pass.

### 1. Rich-basic-mob routing — `Source/T66/Gameplay/T66EnemyDirector.cpp`
- Basic families now route to the lightweight spawn path. When lightweight acquisition fails, the director **already** skips the basic-mob spawn rather than falling back to a rich basic actor: `T66EnemyDirector.cpp:1525-1530` logs `"... skipping basic-mob spawn because rich basic routing is deprecated."` and `continue`s.
- The rich actor spawn branch (`if (bIsMob) { ... }` at `T66EnemyDirector.cpp:1533-1555` and its `else` continuation from `:1556`) is the shared rich spawn machinery. **The miniboss/special/boss spawn branch is part of this shared path and must be preserved.** The cleanup must remove only the basic-mob-specific rich routing/fallback remnants, not the shared rich spawn code that minibosses/specials/bosses depend on.
- Route-attribution diagnostics in `Source/T66/Gameplay/T66MobManagerSubsystem.cpp` (the `RouteAttributionSummary` log at `:2226`) carry many `Routed*Basic` counters. These are diagnostic only; treat as documentation of the deprecated path, not as live routing.

### 2. Inert lightweight / routing / touch CVars (5 total)
Defined/read in `Source/T66/Gameplay/T66MobManagerSubsystem.cpp`:
- `T66.Mob.Diagnostics.UseTouchDamageOverlap` — definition `:60-69`; explicitly self-documented `"DEPRECATED-TOUCH-OVERLAP: inert ... until B.14 cleanup"`. Helper `ShouldUseTouchDamageOverlap()` `:116-120` voids the CVar (`(void)CVarT66MobDiagnosticsUseTouchDamageOverlap;`) and the only caller at `:737` consumes a constant result.
- `T66.Mob.UseLightweight` — read at `:2232` (default 0).
- `T66.Mob.Diagnostics.RouteRushLightweight` — read at `:2233` (default 1). *(Not named in the original task brief but is part of the same inert set — 5 CVars, not 4.)*
- `T66.Mob.Diagnostics.RouteFlyingLightweight` — read at `:2234` (default 1).
- `T66.Mob.Diagnostics.RouteRangedLightweight` — read at `:2235` (default 1).
- As of this pass the four route CVars appear only as fields in the `RouteAttributionSummary` diagnostic log (read via `ReadRouteCVarInt`), not as live routing gates. **Implementation must re-grep each CVar name and confirm every remaining use is diagnostic-logging or voided before removing the definition and its log-field references.**

### 3. Deprecated projectile actor classes — **CORRECTION vs prior draft**
- **`AT66BossProjectile`** (`Source/T66/Gameplay/T66BossProjectile.{h,cpp}`): boss firing was migrated to the projectile manager (see `Reports/AgentReviews/20260528_BossProjectileManager/`). Reported clean of production `SpawnActor<AT66BossProjectile>` sites. **Deletion candidate, gated on a fresh static + binary scan** confirming no spawn sites, no subclasses, and no Blueprint references remain.
- **`AT66EnemyProjectileBase`** (`Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.{h,cpp}`): **NOT dead code.** It is a live base class:
  - Live subclass `AT66SpitProjectile : public AT66EnemyProjectileBase` at `Source/T66/Gameplay/Enemies/Projectiles/T66SpitProjectile.h:10`.
  - `AT66SpitProjectile` is still spawned at `Source/T66/Gameplay/T66PlayerController_Overlays.cpp:4278-4279` (`SpawnActorDeferred<AT66SpitProjectile>`).
  - Live `UPROPERTY TSubclassOf<AT66EnemyProjectileBase> ProjectileClass` at `Source/T66/Gameplay/T66MobBase.h:154` (forward-declared `:19`), assigned `AT66SpitProjectile::StaticClass()` at `T66MobBase.cpp:329,831` and `T66RangedEnemy.cpp:67`.
  - Note: ranged enemy/mob *firing* now goes through `ProjectileManager->FireProjectile(... EnemySpitProjectileTypeIndex)` (`T66RangedEnemy.cpp:224`, `T66MobBase.cpp:753`), so the `ProjectileClass` assignments are largely vestigial — **but the class hierarchy and the overlay spawn at `T66PlayerController_Overlays.cpp:4278` are live.**
  - **Therefore `AT66EnemyProjectileBase` cannot be deleted in this pass without also retiring `AT66SpitProjectile`, the overlay spawn site, and the vestigial `ProjectileClass` property** — a larger, behavior-touching change. See Risks/Decisions R1.
- Cleanup-filter reference (both classes): `Source/T66/Gameplay/GameMode/T66GameMode_Backrooms.cpp:151-152` (`IsA(AT66BossProjectile::StaticClass())` / `IsA(AT66EnemyProjectileBase::StaticClass())`). If either class is deleted, the corresponding filter line and include must be removed; if a class is retained, its filter line stays.

### 4. GamblerToken legacy enum / fields / item alias
- Enum value `ET66SecondaryStatType::GamblerToken` at `Source/T66/Data/T66DataTypes.h:917` (commented `DEPRECATED legacy enum value; superseded by VendorToken`). Canonical `VendorToken` exists at `:924`. The classifier helper references `GamblerToken` at `:936` alongside `VendorToken` at `:943`.
- **Legacy-named persistence fields holding canonical Vendor Token data:**
  - `Source/T66/Core/T66RunSaveGame.h:378` — `int32 ActiveGamblersTokenLevel` (comment: *"Legacy serialized field name retained for save compatibility; holds the canonical Vendor Token level at runtime."*).
  - `Source/T66/Core/T66ProfileSaveGame.h:132` — `int32 GamblersTokenUnlockedLevel`.
- **Live read/write sites of those fields:**
  - `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp:27` writes runtime `ActiveVendorTokenLevel` → `OutSnapshot.ActiveGamblersTokenLevel`; `:171` reads it back into `ActiveVendorTokenLevel`. This legacy-named field **is the current persistence channel** for the canonical runtime value `UT66RunStateSubsystem::ActiveVendorTokenLevel` (`T66RunStateSubsystem.h:1529`).
  - `Source/T66/Core/T66AchievementsSubsystem.cpp:313,1213,1230,1240` read/write `Profile->GamblersTokenUnlockedLevel` (the profile unlock-level persistence).
- Item alias: `Item_GamblersToken` is resolved to `Item_VendorToken` via `UT66GameInstance::NormalizeLegacyItemID()` and synthetic fallback in `Source/T66/Core/T66GameInstance.cpp` (per `Reports/AgentReviews/20260528_ItemTaxonomyImplementation/review_packet.md`). No `Item_GamblersToken` row exists in `Content/Data/Items.csv`; the alias survives only through the normalizer and token helpers.
- **Per Codex correction (not a Pablo blocker):** Pablo already decided no saves need preserving and no migration is required, so these legacy remnants are deleted outright. **Effect to state plainly:** deleting/renaming `ActiveGamblersTokenLevel` and `GamblersTokenUnlockedLevel` removes old-save compatibility; existing saves will not carry their stored vendor-token level into the new schema (it resets to 0 once). After cleanup, Vendor Token persistence uses canonical current fields/path only.

### 5. CoreRedirects — `Config/DefaultEngine.ini:16-21`
- Property redirects (mob-floor terminology rename), lines `:16-18`:
  - `T66SpawnBudget.GameplayFloorsPerStage` → `MobFloorsPerStage`
  - `T66SpawnBudget.InitialEnemiesPerGameplayFloor` → `InitialEnemiesPerMobFloor`
  - `T66EnemyDirector.InitialTowerEnemiesPerGameplayFloor` → `InitialTowerEnemiesPerMobFloor`
- Function redirects (VendorToken runtime rename), lines `:20-21`:
  - `T66RunStateSubsystem.ApplyGamblersTokenPickup` → `ApplyVendorTokenPickup`
  - `T66RunStateSubsystem.GetActiveGamblersTokenLevel` → `GetActiveVendorTokenLevel`
- The header comments mark `:16-18` as bridging `DT_PlayerExperience` until a Phase 2 uasset rebuild, and `:20-21` as bridging Blueprint references. **Removal is conditional — see CoreRedirect Verification Plan.**

### 6. B.13 sandbox worktree
- `C:\UE\T66_B13_Worktree` **EXISTS** (confirmed this pass). Disposition before deletion: confirm preserved evidence already lives under the live repo — `Reports/AgentReviews/20260529_B13_NoLand_Closeout/preserved_worktree_evidence/` contains the B13 HISM/ISM proof logs, indicating closeout evidence is already captured in-repo.

---

# CoreRedirect Verification Plan

Redirects must not be removed on assumption. For each old name, run a source/config/data text scan **and** a binary `.uasset` reference scan; remove a redirect only if its old name is unreferenced everywhere, otherwise leave it in place and report the references.

**Old names to verify** (the five redirect sources):
- `GameplayFloorsPerStage`
- `InitialEnemiesPerGameplayFloor`
- `InitialTowerEnemiesPerGameplayFloor`
- `ApplyGamblersTokenPickup`
- `GetActiveGamblersTokenLevel`

**Method (FullOperator implementation phase):**
1. Text scan across `Source/`, `Config/`, and `Content/Data/*.csv` / `*.json` for each old name (targeted ripgrep, not a broad Git/LFS scan).
2. Binary asset scan for old-name references in the relevant `.uasset` set — primarily `Content/Data/DT_PlayerExperience.uasset` and any Blueprint that referenced the renamed `T66RunStateSubsystem` UFUNCTIONs. Use an in-editor reference/audit (e.g., editor commandlet or Reference Viewer) scoped to the candidate assets — **not** a repo-wide binary sweep. This is the gating step the read-only/planning profile cannot perform.
3. Per-redirect rule:
   - **Clean (zero references):** remove that redirect line.
   - **Any hit, OR scan inconclusive:** retain the redirect, and record the referencing asset/config/source in the completion report.
4. The property redirects (`:16-18`) and function redirects (`:20-21`) are evaluated independently — it is valid to drop some and keep others.
5. Note: the mob-floor property-redirect header explicitly says it bridges `DT_PlayerExperience` "until the Phase 2 uasset rebuild." If that uasset rebuild has not happened, expect `:16-18` to still be needed — verify `DT_PlayerExperience.uasset` first.

---

# Implementation Plan

Phased; each proof-bearing phase runs under a separate Codex-approved FullOperator mandate. No phase here is authorized by this packet.

**Phase 0 — Confirm + lock anchors (read-only).**
Re-grep every identifier in Live Anchor Findings, lock exact line ranges, and resolve the R1 projectile-scope decision (descope `AT66EnemyProjectileBase` vs expand to retire `AT66SpitProjectile`). Produce no edits.

**Phase 1 — Rich-basic routing removal.**
In `T66EnemyDirector.cpp`, delete only the basic-mob-specific deprecated rich routing/fallback remnants while preserving the shared rich spawn branch used by miniboss/special/boss. Keep the lightweight path and its acquire-fail skip behavior intact (behavior-neutral).

**Phase 2 — Inert CVar removal.**
Remove the 5 inert CVar definitions and their diagnostic-log field references in `T66MobManagerSubsystem.cpp` (and the `ShouldUseTouchDamageOverlap` voided helper + its constant-result caller at `:737`). Confirm no remaining gate uses first.

**Phase 3 — Projectile class removal (scoped by R1 decision).**
- Always-in-scope: delete `AT66BossProjectile` (`T66BossProjectile.{h,cpp}`) after a fresh static + binary scan confirms no spawn/subclass/Blueprint refs; remove its filter line + include at `T66GameMode_Backrooms.cpp:151`.
- Conditional: `AT66EnemyProjectileBase` deletion **only** if Codex/Pablo approve expanding scope to also retire `AT66SpitProjectile` + the overlay spawn (`T66PlayerController_Overlays.cpp:4278`) + the vestigial `ProjectileClass` property. Otherwise descope and leave the base + its filter line at `:152`.

**Phase 4 — GamblerToken legacy removal (no migration).**
- Remove the `GamblerToken` enum value (`T66DataTypes.h:917`) and its branch in the classifier helper (`:936`).
- Replace the legacy-named persistence fields with canonical Vendor Token field names: `ActiveGamblersTokenLevel` → canonical (`T66RunSaveGame.h:378`) and `GamblersTokenUnlockedLevel` → canonical (`T66ProfileSaveGame.h:132`); update read/write sites (`T66RunStateSubsystem_Snapshot.cpp:27,171`; `T66AchievementsSubsystem.cpp:313,1213,1230,1240`). No save migration — old saves drop their stored level (resets to 0 once).
- Remove the `Item_GamblersToken` alias handling in `T66GameInstance.cpp` (normalizer + synthetic fallback), confirming no live drop/lookup still relies on the alias.

**Phase 5 — CoreRedirect removal (conditional).**
Execute the CoreRedirect Verification Plan; remove only the redirects proven clean.

**Phase 6 — B.13 worktree deletion (Windows-safe).**
1. Resolve the absolute path and assert it equals exactly `C:\UE\T66_B13_Worktree`.
2. Confirm preserved evidence already exists in-repo under `Reports/AgentReviews/20260529_B13_NoLand_Closeout/preserved_worktree_evidence/`.
3. Delete via PowerShell native: `Remove-Item -LiteralPath C:\UE\T66_B13_Worktree -Recurse -Force`. If it is a registered git worktree, prefer `git worktree remove` semantics over a raw delete and stop for Codex approval if state is unexpected.

---

# Files / Paths To Touch

**Source (edit/delete) — implementation phases only:**
- `Source/T66/Gameplay/T66EnemyDirector.cpp` (Phase 1 — basic-mob rich-routing remnants).
- `Source/T66/Gameplay/T66MobManagerSubsystem.cpp` (Phase 2 — 5 inert CVars + log fields + touch-overlap helper/caller).
- `Source/T66/Gameplay/T66BossProjectile.{h,cpp}` (Phase 3 — delete, gated).
- `Source/T66/Gameplay/GameMode/T66GameMode_Backrooms.cpp` (Phase 3 — filter lines `:151-152` + includes, per which classes are deleted).
- *(R1-conditional, Phase 3):* `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.{h,cpp}`, `T66SpitProjectile.{h,cpp}`, `Source/T66/Gameplay/T66PlayerController_Overlays.cpp:4278`, `Source/T66/Gameplay/T66MobBase.{h,cpp}`, `Source/T66/Gameplay/Enemies/T66RangedEnemy.{h,cpp}`.
- `Source/T66/Data/T66DataTypes.h` (Phase 4 — enum value `:917`, helper `:936`).
- `Source/T66/Core/T66RunSaveGame.h` (Phase 4 — `:378`).
- `Source/T66/Core/T66ProfileSaveGame.h` (Phase 4 — `:132`).
- `Source/T66/Core/RunState/T66RunStateSubsystem_Snapshot.cpp` (Phase 4 — `:27,171`).
- `Source/T66/Core/T66AchievementsSubsystem.cpp` (Phase 4 — `:313,1213,1230,1240`).
- `Source/T66/Core/T66GameInstance.cpp` (Phase 4 — `Item_GamblersToken` alias).

**Config (conditional edit) — implementation phase only:**
- `Config/DefaultEngine.ini` (Phase 5 — redirects `:16-21`, only those proven clean).

**Filesystem (delete) — implementation phase only:**
- `C:\UE\T66_B13_Worktree` (Phase 6, path-asserted).

**Verify-only (no edits expected):**
- `Content/Data/DT_PlayerExperience.uasset` and Blueprints referencing renamed UFUNCTIONs (CoreRedirect binary scan).
- `Content/Data/Items.csv` (confirm no `Item_GamblersToken` row).

---

# Verification Plan

Run in the FullOperator implementation phase, per the user's stated proof requirements:
1. **Grep-clean:** confirm every deleted identifier is gone from live source/config/data — `GamblerToken` (enum value), `ActiveGamblersTokenLevel`, `GamblersTokenUnlockedLevel`, `Item_GamblersToken` alias handling, the 5 inert CVar names, deleted projectile class symbols, and any removed redirect old-names. Allow only intended canonical replacements.
2. **Build:** `T66Editor Win64 Development` compiles clean; inspect for enum-switch / item-ID warnings (`C4061`, `C4062`, `LogDataTable`).
3. **Stage:** stage standalone build and record the staged build SHA256.
4. **Staged smoke — full-resolution `enemywaveperf` capture:** validate basic mobs spawn and behave correctly, ranged enemies fire and hit through the projectile manager, boss/special/miniboss paths intact, FPS healthy, no missing-asset / redirect-fallback warnings.
5. **CoreRedirect evidence:** paste the per-old-name scan results (text + binary) into the completion report; justify each redirect kept or removed.
6. **Completion report** written to `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/` (completion packet path), including the SHA256, capture summary, and B.13 deletion confirmation.

---

# Risks / Decisions

- **R1 — `AT66EnemyProjectileBase` is live, not dead (scope decision needed).** It is the base of `AT66SpitProjectile`, which is still spawned at `T66PlayerController_Overlays.cpp:4278` and referenced by a live `TSubclassOf` property in `T66MobBase`. Deleting the base is **not** a behavior-neutral dead-code removal; it requires also retiring `AT66SpitProjectile`, the overlay spawn, and the vestigial `ProjectileClass` property. **Decision required:** (a) descope the enemy projectile base from this cleanup and delete only `AT66BossProjectile`; or (b) expand scope to retire the SpitProjectile hierarchy + overlay spawn under explicit approval. Recommendation: **(a) descope** for a clean behavior-neutral pass; handle the SpitProjectile retirement as its own validated change.
- **R2 — Old-save compatibility intentionally dropped (no longer a blocker).** Per Pablo's explicit decision and the Codex correction, deleting/renaming `ActiveGamblersTokenLevel` and `GamblersTokenUnlockedLevel` is approved with no migration. Stated effect: existing saves lose their stored Vendor Token level once (resets to 0); future persistence is canonical-only. Not a human-decision blocker.
- **R3 — Mid-enum value removal serialization.** `ET66SecondaryStatType` is name-serialized (UENUM), so removing the `GamblerToken` value does not reindex siblings. Legacy serialized `GamblerToken` values will resolve to the enum default without an EnumRedirect — acceptable under the no-migration decision, but should be confirmed not to surface as a runtime warning in the staged smoke.
- **R4 — CoreRedirect removal gated on binary scan.** The mob-floor property redirects (`:16-18`) explicitly bridge `DT_PlayerExperience.uasset` until a Phase 2 uasset rebuild; if that rebuild has not occurred, they must stay. Removal of any redirect is conditional on a clean text + binary scan; inconclusive ⇒ retain.
- **R5 — `AT66BossProjectile` deletion gate.** Reported clean by the 2026-05-28 boss projectile-manager migration, but deletion still requires a fresh static (`SpawnActor<AT66BossProjectile>` / `::StaticClass(`) + Blueprint/subclass scan at implementation time before removing the class and its `T66GameMode_Backrooms.cpp:151` filter line.
- **R6 — B.13 deletion is irreversible.** Path-assert `== C:\UE\T66_B13_Worktree`, confirm in-repo preserved evidence, and prefer `git worktree remove` semantics if it is a registered worktree. Stop for Codex approval on any unexpected state.

---

# Codex Approval Request

Requesting Codex validation of this plan and approval to proceed to a FullOperator implementation phase. Specific decisions/approvals needed before any edits:

1. **R1 projectile scope:** approve **(a) descope `AT66EnemyProjectileBase`** (delete only `AT66BossProjectile` this pass) — recommended — or **(b) expand scope** to also retire `AT66SpitProjectile` + overlay spawn + `ProjectileClass` property.
2. **Confirm** the no-migration GamblerToken removal (R2/R3) proceeds as planned: canonical-rename the two persistence fields, drop the enum value, remove the item alias, accept the one-time save reset.
3. **Approve** the conditional CoreRedirect verification gate (R4) and conditional `AT66BossProjectile` deletion gate (R5) as the binding acceptance rules.
4. **Approve** the B.13 worktree deletion procedure (R6).
5. **Authorize** the proof-bearing FullOperator capabilities the planning profile lacks: editor build, standalone stage + SHA256, full-resolution `enemywaveperf` capture, and the in-editor binary `.uasset` reference scan for the CoreRedirect gate.

This packet is an Operator work artifact, not a greenlight; Codex validates actual changes and writes the final user-facing report.
