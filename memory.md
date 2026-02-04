# memory.md — T66 agent context

**Purpose:** This file is **context for Cursor agents** — key rules, where to look, and current state. Use it to onboard quickly and avoid breaking conventions. Full change history lives in **git log**.

---

## For Cursor agents

- **Read this file first** before making changes.
- **Acknowledge the localization rule** in your next message (see HARD RULE below).
- **Build command (PowerShell):**  
  `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development "C:\UE\T66\T66.uproject" -waitmutex`
- **Repo path:** `c:\UE\T66` (Windows).

---

## HARD RULE (Non‑Negotiable) — Localize every new player-facing string

**Any time you add or change player-facing text, complete the full localization pipeline in the same change-set.**

1. **Author:** Use **`LOCTEXT` / `NSLOCTEXT`** or String Tables. Never `FText::FromString(TEXT("..."))` for shipping UI. No per-language `switch(CurrentLanguage)`.
2. **Gather:** Run GatherText so keys land in `Content/Localization/T66/`.
3. **Translate:** Populate all 22 cultures (e.g. `.archive` or `Scripts/AutoTranslateLocalizationArchives.py`).
4. **Compile:** Regenerate `Content/Localization/T66/<culture>/T66.locres`.
5. **Verify:** Confirm text changes when switching culture.

**Acknowledge in your next message:** e.g. “Acknowledged: new player-facing text will be localized (author → gather → translate → compile → verify).”

---

## Where to look (quick reference)

| Need to… | Look here |
|----------|-----------|
| **Skin ownership / purchase / equip** | `UT66SkinSubsystem` (hero + companion). `GetSkinsForEntity()`, `PurchaseSkin()`, `SetEquippedSkin()`, `OnSkinStateChanged`. |
| **AC balance / profile save** | `UT66AchievementsSubsystem`: `GetAchievementCoinsBalance()`, profile load/save. Skin *data* is in profile; skin *API* is in SkinSubsystem. |
| **Hero/companion definitions** | `UT66GameInstance` (GetHeroData, GetCompanionData, GetAllHeroIDs, etc.) + DataTables `DT_Heroes`, `DT_Companions` + CSVs under `Content/Data/`. |
| **Character visuals / meshes / animations** | `UT66CharacterVisualSubsystem` (ApplyCharacterVisual, alert vs walk anim). Data: `DT_CharacterVisuals` / `CharacterVisuals.csv`. |
| **Hero selection UI** | `T66HeroSelectionScreen`: `RefreshSkinsList()`, `AddSkinRowsToBox()`, `SkinsListBoxWidget`, `ACBalanceTextBlock`, `PreviewSkinIDOverride`. Uses `UT66SkinSubsystem`. |
| **Companion selection UI** | `T66CompanionSelectionScreen`: same pattern (SkinsListBoxWidget, RefreshSkinsList, AddSkinRowsToBox); uses `UT66SkinSubsystem`. |
| **3D hero preview** | `AT66HeroPreviewStage` (Tick → CaptureScene after pawn anim). `T66HeroBase::InitializeHero(bPreviewMode)` → alert anim in preview. |
| **Localization** | `UT66LocalizationSubsystem::GetText_*()`. All UI strings via this or NSLOCTEXT; no hardcoded player-facing text. |
| **Run state (hearts, gold, inventory)** | `UT66RunStateSubsystem`. |
| **Save/load run** | `UT66SaveSubsystem`, `UT66RunSaveGame`. Profile (AC, skins, achievements): `UT66ProfileSaveGame`, saved by AchievementsSubsystem. |

---

## Current state

- **Project:** T66 (Tribulation 66) · **Engine:** UE 5.7
- **Active branch:** `version-0.5`
- **Build:** ✅ C++ compiles (ValidateFast as above).
- **Skins:** Centralized in `UT66SkinSubsystem`. Hero and companion selection use it; AC and skin inventory persist (profile, no one-time reset).
- **Pipelines:** Sprites (`Scripts/ImportSpriteTextures.py`), world models (`Scripts/ImportModels.py`), ground atlas (`Scripts/BuildGroundAtlas.py`, `ImportGroundAtlas.py`). Full setup: `Scripts/RunFullSetup.bat` or editor `T66Setup`.

---

## Guardrails

- Atomic change-sets; ValidateFast after each.
- Small, descriptive commits.
- UI: **event-driven** only (no per-frame UI logic).
- **All UI text localized** via `UT66LocalizationSubsystem::GetText_*()` or NSLOCTEXT.

---

## Open questions / blockers

- **WBP_LanguageSelect / WBP_Achievements:** Optional; C++ works without. Re-run `T66Setup` if you add Blueprint overrides.
- **Leaderboard:** Placeholder until Steam.
- **Nav:** Enemies use Tick toward player; no nav required. For future pathfinding, add Nav Mesh Bounds in GameplayLevel.

---

## Known issues / tech debt

- Coliseum: `ColiseumLevel.umap` may be missing; code falls back to GameplayLevel.
- Optional: move ID-keyed copy (achievements, hero/companion names) to String Tables for designers.

---

## Architecture (summary)

- **Core:** `T66GameInstance` (hero/companion/data tables), `T66AchievementsSubsystem` (profile, AC, save), `T66SkinSubsystem` (all skin API), `T66CharacterVisualSubsystem` (meshes/anim), `T66LocalizationSubsystem`, `T66RunStateSubsystem`, `T66SaveSubsystem`.
- **UI:** `T66UIManager`, `T66ScreenBase`; screens under `UI/Screens/` (Slate in `BuildSlateUI()`). Hero/Companion selection: skin list + AC from SkinSubsystem; refresh via `RefreshSkinsList()`.
- **Gameplay:** `T66GameMode`, `T66HeroBase`, `T66CompanionBase`, `T66EnemyBase`, preview stages in `Gameplay/`.
- **Data:** `Source/T66/Data/T66DataTypes.h` (FHeroData, FCompanionData, FSkinData, etc.). DataTables and CSVs in `Content/Data/`.
- **Flow:** Frontend (MainMenu → PartySizePicker → HeroSelection → CompanionSelection → Enter) → GameplayLevel. Run state in RunStateSubsystem; death → RunSummary.

---

## Keystone audit (Bible vs repo)

- Checklist: `T66_Keystone_Audit_Report.md`
- Reconciled sections listed in that report; keep Bible and repo in sync.

---

## Recent context (for continuity)

- **Central skin subsystem:** `UT66SkinSubsystem` holds all skin logic (hero + companion). AchievementsSubsystem keeps profile and delegates skin calls to SkinSubsystem. Hero and companion selection use `GetSkinsForEntity()`, `RefreshSkinsList()`, dynamic AC display.
- **Persistence:** Profile (AC, owned/equipped skins) persists; one-time reset that cleared purchases was removed.
- **Hero selection:** When switching heroes, skin is validated for the *new* hero (if not owned, falls back to Default); `PreviewSkinIDOverride` cleared on switch.
- **Companion selection:** Same skin/AC wiring as hero selection (SkinsListBoxWidget, RefreshSkinsList, AddSkinRowsToBox, SkinSubsystem).

**Full history:** `git log` (this file is context, not a full changelog).
