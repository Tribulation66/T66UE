I have enough evidence to give the independent answer.

Result: OK

## Independent Answer

The request is authorized and internally executable by Codex — the user already said "good go," and whether aliases are safe to delete is an evidence question, not a user decision. But the audit must distinguish two different things that share the same words, because conflating them will break content.

**1. What must be audited before deleting legacy idol aliases**
- The `NormalizeLegacyIdolID()` map in `Source/T66/Core/T66IdolManagerSubsystem.cpp:131-164` (legacy element IDs → new category IDs like `Idol_Ice_AOE`).
- Every raw `IdolID == FName(TEXT("Idol_<element>"))` comparison still compiled in runtime (VFX, localization, combat).
- Save round-trip: what idol IDs are actually persisted in save games and whether old saves still carry legacy IDs that only the normalize map rescues.
- Proof harness expectations: the `.ps1` proof runners and the diagnostic log vocabulary.
- **Critical:** sprite *asset* names in `Content/Data/Idols.csv` (`Idol_Water_black`, `Idol_Storm_red`, `Idol_Light_yellow`, `Idol_Poison_*`, `Idol_Lava_*`, etc.) are texture asset paths in the Icon columns — NOT idol IDs. They are live and referenced by current rows keyed on new IDs. Deleting/renaming those sprites is a separate, content-breaking action and must not be swept into "alias deletion."

**2. Files/systems most likely to hold dependencies**
- `Source/T66/Core/T66IdolManagerSubsystem.cpp:131-164` — the legacy normalize map itself (the alias source of truth).
- `Source/T66/Core/T66LocalizationSubsystem.cpp:1705-1775` — idol **names and tooltips are keyed entirely on legacy element IDs** (`Idol_Water`, `Idol_Storm`, `Idol_Light`, `Idol_Poison`, `Idol_Lava`, etc.). This is the largest live dependency; deleting aliases without re-keying loses all idol display text.
- `Source/T66/Gameplay/T66CombatVFX.cpp:444-467` — a legacy-keyed VFX fallback branch (`IdolID == Idol_Lava/Water/Light/...` and the `Idol_Storm` BP_Storm path).
- `Source/T66/Gameplay/T66CombatComponent.cpp:3586-3752` — legacy-element status branches, **but gated behind `bLegacyIdolSpecificStatusEnabled = false` with an early return at line 3565-3568**, so these are currently dead code (not a live runtime dependency, but still a deletion target to clean).
- `Content/Data/Idols.csv` Icon columns — sprite assets named after the aliases (live; see caveat above).
- `Scripts/RunHero1Axe*IdolImpactProof.ps1` and the `SourceID=Idol_Water` diagnostic strings (`T66CombatComponent.cpp:4035,4765`) — proof harness parses these; preserved intentionally.
- Save/backend: `T66BackendSubsystem.cpp:175-179` and overlay `EquippedIdols` use **new** category IDs, which is good evidence authored/current content has already migrated.

**3. If audit finds no required dependencies — safe scoped implementation**
The evidence already shows this is unlikely to be the case for localization/VFX. But where a key truly has zero live readers, the safe scoped change is: remove that authored alias key and its single runtime lookup branch, leaving the new category ID as the only path — without touching sprite asset names or proof diagnostic strings.

**4. If dependencies exist (they do) — what to change first**
Re-key before deleting. Order:
1. Migrate localization name/tooltip lookups to new category IDs (or route them through normalize) so no display text is lost.
2. Fold the legacy VFX fallback branch into the normalized lookup already present above it.
3. Keep `NormalizeLegacyIdolID` until a save audit proves no shipped/old save carries legacy IDs; the map is the back-compat safety net for persisted data.
4. Only then delete the now-unreferenced alias authoring keys. Leave proof diagnostic vocabulary and sprite asset names alone unless separately scoped.

**5. Verification Codex should run**
- C++ compile/build of the T66 module after any source edit (runtime-facing).
- The save round-trip proof the user explicitly asked for: load a save written with legacy IDs, confirm idols resolve, equip/run, re-save — capture the log.
- The existing `RunHero1Axe*IdolImpactProof.ps1` proof runners; confirm the `Idol_Water`/category diagnostic lines still PASS.
- If `Idols.csv` is touched, run the owning DataTable import commandlet (repo rule).
- Staged standalone validation if anything in the playable runtime path changes.

## Evidence Checked
- `Content/Data/Idols.csv` — rows keyed on new IDs (`Idol_Fire_DOT`…); alias words appear only as sprite asset paths in Icon columns.
- `T66IdolManagerSubsystem.cpp:131-164` — legacy→new normalize map.
- `T66LocalizationSubsystem.cpp:1705-1775` — names/tooltips keyed on legacy IDs (live).
- `T66CombatVFX.cpp:438-467` — mixed normalized + legacy-keyed VFX lookups.
- `T66CombatComponent.cpp:3564-3752, 4035, 4765` — legacy status branches dead-gated; diagnostic strings preserved.
- `T66BackendSubsystem.cpp`, `T66PlayerController_Overlays.cpp`, `T66IdolAltar.h`, `T66TutorialManager.cpp` — use new category IDs.
- Proof scripts `RunHero1AxeIdolCategoryNativeImpactProof.ps1`, `RunHero1AxeAOECategoryIdolImpactProof.ps1`.

## Questions Or Blockers
None requiring the user. The user has authorized the audit-and-scoped-change work. Codex can run it internally.

## Caveats
- No save/proof run has been executed in this validation; do not report deletion as proven safe until the save round-trip and proof runners actually run and pass.
- The single biggest risk is conflating the legacy **idol-ID aliases** (deletable after re-keying) with the **sprite asset names** of the same spelling in `Idols.csv` (live content — must not be deleted under this task).
- Localization is fully keyed on legacy IDs; a blanket alias deletion without re-keying will silently drop idol names/tooltips.
- Keep `NormalizeLegacyIdolID` until the save audit proves no persisted legacy IDs remain — it is the back-compat net, not dead weight.
- Mini/minigames excluded per scope; do not extend the sweep there.
