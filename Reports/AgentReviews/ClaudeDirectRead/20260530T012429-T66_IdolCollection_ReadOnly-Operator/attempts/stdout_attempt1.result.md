I have enough live evidence to answer. Here's the operator packet.

---

Operator Packet: T66 Idol Collection Answer

**Idol roster (how many exist)**
- **12 idols total.** Fixed list in `T66IdolManagerSubsystem.cpp:136-153` (`GetAllIdolIDs`) and matching 12 data rows in `Content/Data/Idols.csv`: Curse, Lava, Poison (DOT); Electric, Ice, Shadow (Bounce); Earth, Water, Storm (AOE); Light, Steel, Wood (Pierce).

**Equip cap (how many at once)**
- **3 equipped slots max** — `MaxEquippedIdolSlots = 3` (`T66IdolManagerSubsystem.h:22`). Selecting into a full set requires selling one first (`SellEquippedIdolInSlot`). Idols you don't equip aren't "collected" — there is no separate inventory; the only ownership is the 3 equipped slots.
- Altar "stock" always shows all 12 idols (`IdolStockSlotCount = 12`), each offered at the difficulty's base rarity tier; already-owned idols are pre-marked as selected and can't be re-picked (`RerollIdolStock`, `ApplyStockOfferToEquipped`).

**Selection opportunities / events (current normal tower play)**
Each idol altar grants `RemainingSelections = 1` (one pick per altar) and self-destroys when consumed. Altars appear from:
1. **Tower gate guardian (miniboss) defeated** → spawns one idol altar at the drop, 1 selection (`T66GameMode_Tower.cpp:887`, via `SpawnIdolAltarAtLocation`).
2. **Stage boss defeated** → spawns one idol altar at the boss death location, 1 selection (`T66GameMode_BossFlow.cpp:492`). (On the final difficulty clear it opens Run Summary instead — no altar.)
- Floor 1 of a tower stage is **reserved for the Weapon Altar**, not an idol altar.

**Non-current / separate paths (called out distinctly)**
- **Tutorial:** one scripted altar forcing a single **Electric** idol (`bUseTutorialSingleOffer`, `TutorialOfferedIdolID = Electric`, 1 selection) — `T66TutorialManager.cpp:241-247`.
- **Gallery/world-showcase:** an idol altar with `RemainingSelections = 999` (free browsing) — `T66GameMode_WorldInteractables.cpp:1766`. Lab runs skip idol altars entirely (`IsLabRun()` early-returns).
- **Dead/unused path:** `SpawnIdolAltarForPlayer` (a "stage-entry" idol altar, budget hardcoded `SelectionBudget = 1`) is **defined but has no caller** in the source — it does not fire in current normal play.

**How many total a player gets**
- **At any moment: at most 3 equipped idols** (hard cap). Selecting is one-per-altar; you can sell and re-pick, so total *distinct* idols touched over a run can exceed 3, but the standing collection never exceeds 3.
- **Number of pick opportunities in a run = (tower guardians defeated) + (stage bosses defeated).** That count scales with how many descent-floor guardians and stage bosses the chosen difficulty band contains (each difficulty band spans 4 stages per `DifficultyTuning.json`), but practically you can only ever hold 3.
- Rarity/tier of offered idols is set by the difficulty band's `IdolBaseRarity` (Black→Easy, Red→Medium, Yellow→Hard, White→VeryHard/Impossible), capped at tier 4 (`MaxIdolLevel = 4`).

**Evidence list**
- `Content/Data/Idols.csv` — 12 idol rows.
- `Source/T66/Core/T66IdolManagerSubsystem.h:22-24` — slots=3, level cap=4, stock=12.
- `Source/T66/Core/T66IdolManagerSubsystem.cpp:136-153, 277-318` — roster + stock build.
- `Source/T66/Gameplay/GameMode/T66GameMode_WorldInteractables.cpp:1863-1971, 2348-2386` — altar spawn, `RemainingSelections=1`; dead stage-entry path.
- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp:887` — guardian-defeat altar.
- `Source/T66/Gameplay/GameMode/T66GameMode_BossFlow.cpp:492` — boss-death altar.
- `Source/T66/Gameplay/T66IdolAltar.h:57-62` — `RemainingSelections`, tutorial single-offer.
- `Source/T66/Gameplay/T66TutorialManager.cpp:241-247` — forced Electric idol.
- `Content/Data/DifficultyTuning.json` — 5 bands, idol base rarity per band.

Validator note (Codex): confirm whether any Blueprint/level-script path invokes the otherwise-uncalled `SpawnIdolAltarForPlayer`, and the per-run guardian/boss counts per difficulty band — those determine the exact number of pick opportunities, which I could only bound structurally from C++.

Claude Tokens Spent: Unavailable
