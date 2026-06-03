# T66 Enemy Roster Review

A plain-language inventory of every enemy in the game, written for design review before the upcoming cleanup pass. Nothing here is a change request — the "attention flags" in Section 8 are things to look at and decide on, not fixes already made.

How to read this:
- The main descriptions avoid code names and file paths on purpose.
- Where a name is unavoidable (so you can point at the right thing), it is the in-game display name.
- All the technical row IDs, class names, and file references live in **Section 9 (Technical Traceability)** so the rest reads cleanly.
- "Reachable in normal play" means a regular player can actually run into it. "Latent" means it exists in the project but nothing spawns it during a normal run.

---

## 1. Quick Summary Counts

| Category | Count | Notes |
|---|---|---|
| Main story/stage bosses | 23 boss entities across 20 stages | One stage (the first Hell stage) is a four-boss fight, which is why 20 stages hold 23 bosses. |
| Hidden / secret bosses | 1 (the Gambler) | Not part of any stage. Appears only if you anger the casino. No formal "hidden boss" list exists in the data — this is the only one. |
| Miniboss types | 1 system, 1 creature | A scaled-up Slime placed as a "gatekeeper" on tower floors. It is the only miniboss type; it is a placeholder. |
| Special enemies | 3 found | Goblin Thief (active), the Gambler boss (casino-triggered), and a "debuff" floating enemy that is currently latent. A fourth, the Backrooms Stalker, is tied to an unfinished mode. |
| Basic mob types | 50 | Ten per theme across five themes. |

**Confidence caveats up front:**
- The 23 bosses, 20 stages, and 50 basic mobs are **defined in data** and I cross-checked them against the spawn code. Confidence: high.
- The Gambler boss, the Goblin Thief, the minibosses, and the latent debuff enemy live in **code**, not the stage data tables, so they don't show up in the boss/enemy spreadsheets. I traced each one to where it is (or isn't) spawned. Confidence: high for what spawns; see flags for the gray areas.
- "Hidden bosses" as a named category does **not** exist in the project. I'm reporting the closest real thing (the Gambler) rather than inventing a list.

---

## 2. Bosses

There are **23 bosses** total, laid out as one boss per stage across 20 stages, except the first Hell stage which throws four bosses at once (the Four Horsemen). Each theme runs four stages: three "normal" bosses then a demonic "Chad" capstone boss, except Hell which is structured differently.

Every boss below is **defined in data and wired to spawn at its stage** (each stage points at a boss encounter, and each encounter points at the boss). Confidence: high.

### Dungeon (Easy)
| Stage | Boss | Concept (plain language) | Status |
|---|---|---|---|
| 1 | The Sewer Slime King | Massive green dungeon slime with a bone crown and sewer bubbles. | Reachable |
| 2 | The Web Matriarch | Giant spider queen with a webbed abdomen and bone-tipped legs. | Reachable |
| 3 | The Bone Jailer | Armored skeleton jailer carrying chains and rusted keys. | Reachable |
| 4 | Bael, Fallen Chad | Demonic Chad king — cracked crown, leathery wings, infernal sword. Capstone. | Reachable |

### Forest (Medium)
| Stage | Boss | Concept | Status |
|---|---|---|---|
| 5 | The Bramble Treant | Walking ancient tree wrapped in thorns and hanging roots. | Reachable |
| 6 | The Myconid Queen | Royal mushroom boss with a spore crown and broad fungal skirt. | Reachable |
| 7 | The Thorn Hive | A wasp nest grown into a thorned wooden hive with swarm openings. | Reachable |
| 8 | Buer, Verdant Chad | Demonic Chad with a root halo, four thorned arms, forest-curse aura. Capstone. | Reachable |

### Ocean (Hard)
| Stage | Boss | Concept | Status |
|---|---|---|---|
| 9 | The Reef Crab Colossus | Huge armored crab with a coral shell and barnacle claws. | Reachable |
| 10 | The Abyssal Jellyfish | Glowing deep-sea jellyfish with electric tendrils and a translucent bell. | Reachable |
| 11 | The Drowned Captain | Undead sailor captain with an anchor weapon and torn officer's coat. | Reachable |
| 12 | Focalor, Drowned Chad | Demonic Chad with wet raven wings, a storm halo, abyssal armor. Capstone. | Reachable |

### Martian (Very Hard)
| Stage | Boss | Concept | Status |
|---|---|---|---|
| 13 | The Red Sand Behemoth | Bulky alien sand monster with a cracked red hide and shockwave fists. | Reachable |
| 14 | The Crystal Mantis | Tall alien mantis with glassy crystal scythes and faceted armor. | Reachable |
| 15 | The Plasma Saucer Prime | UFO command boss with a glowing plasma cannon and a hovering probe ring. | Reachable |
| 16 | Stolas, Astral Chad | Winged demonic Chad with owl horns, star-map armor, a cosmic spear. Capstone. | Reachable |

### Hell (Impossible)
Hell breaks the four-stage pattern: the first stage is a **four-boss fight**, then three escalating single bosses.

| Stage | Boss(es) | Concept | Status |
|---|---|---|---|
| 17 | The Four Horsemen — Conquest, War, Famine, Death | Four apocalypse riders fought simultaneously (white/bow, red/sword, black/scales, ashen/scythe). | Reachable |
| 18 | The False Prophet | Infernal preacher with a cracked halo, scripture banners, violet fire. | Reachable |
| 19 | The Antichrist | Demonic royal tyrant with a black crown, six eyes, a crimson command aura. | Reachable |
| 20 | The Great Dragon | Colossal seven-headed red dragon — the final boss. | Reachable |

**Boss count detail:** Dungeon 4 + Forest 4 + Ocean 4 + Martian 4 + Hell 7 (four Horsemen + three single) = **23**.

---

## 3. Hidden Bosses

**There is no dedicated "hidden boss" list in the game's data.** I searched the data tables and the design docs; nothing defines a category of secret bosses.

The one boss that behaves like a hidden/secret encounter is:

### The Gambler
- **What it is:** A standalone boss with its own custom multi-part body (head, core, two arms, two legs), a casino/gambling theme, pink-and-cyan attack colors, and a ground-slam area attack.
- **How it's found / triggered:** It is *not* on any stage map. It appears only when you push the casino too far — the casino builds up an "anger" meter as you gamble, and when that meter maxes out during a gameplay level, the Gambler spawns at the casino/vendor's location and attacks.
- **Stage / condition:** Any gameplay level where a casino or vendor is present and its anger reaches the limit. Only one can be active at a time.
- **Reachable status:** **Reachable in normal play**, but only through the casino interaction — a player who never maxes the casino's anger will never see it.

(One older design doc also mentions a separate "Vendor" boss alongside the Gambler, but I found no such enemy class in the current code. See flag in Section 8.)

---

## 4. Minibosses

**Current system, in plain language:** As you descend the tower, each gameplay floor's "drop-hole" to the next floor is guarded. To go down, you must defeat the guardian standing on that floor's exit. That guardian is the miniboss.

- **What the miniboss actually is:** A **scaled-up Slime** — the same basic Slime mob, made roughly 1.75× bigger, with about 3× the health and 2× the touch damage of a normal Slime, and it drops no loot. It's explicitly a **placeholder**: every floor's gatekeeper is the same enlarged Slime regardless of theme.
- **Where it appears:** On the tower's gameplay floors, one per descent gate. (The design framing you've used is "floors 2, 3, and 4." In the code, the gate-guardian rule applies to *all* gameplay floors of a normal tower stage, not a hard-coded 2/3/4 — see flag in Section 8.)
- **Any other miniboss types?** **No.** The Slime gatekeeper is the only miniboss creature and the only miniboss system. An older "random chance to promote a wave enemy into a miniboss" feature still has leftover tuning values, but that random promotion is **switched off** — minibosses are now only these deliberate placed gate guardians.
- **Note on naming confusion:** Several *basic* mobs are tagged with a "miniboss feel" design label (the bigger, rarer enemies like the Tomb Spider, Treant Ancient, Anglerfish Stalker, Plasma Sentinel, Bone Knight, Demon Sentinel). That tag is about how beefy they *feel* — it does **not** make them actual minibosses. The only real miniboss is the placed Slime.

---

## 5. Specials

"Special" enemies are ones that behave differently from a normal wave mob. I found these:

### Goblin Thief — ACTIVE
- **What it is:** A goblin that chases you.
- **What makes it different:** On contact it **steals gold instead of dealing heart damage**. It carries a "rarity" tier that scales how much gold it takes (default ~50 per hit).
- **Trigger / spawn:** It mixes into normal enemy waves at random, with the chance influenced by your Luck. When it rolls in, a small number spawn within that wave.
- **Status:** **Active and reachable in normal play.**

### The Gambler boss — ACTIVE (conditional)
- Covered in Section 3. Listed here too because it's a special, non-stage encounter. Triggered by maxing casino anger. **Reachable, but only via the casino.**

### Unique "Debuff" enemy — LATENT
- **What it is:** A floating enemy that hovers and fires fast straight projectiles which apply a status effect to the hero (e.g. Burn) on hit, and it expires after a short lifetime.
- **What makes it different:** It's the one enemy built specifically to *debuff* the hero with a status effect rather than just chip health, and it floats and shoots rather than chasing.
- **Trigger / spawn:** I could only find it being spawned in the developer **Lab** test mode. Nothing in the normal tower wave logic spawns it. So in a normal run, **a player would not encounter it.**
- **Status:** **Latent / unspawned in normal play.** Built and functional, but not wired into regular gameplay.

### Backrooms Stalker — LATENT (unfinished mode)
- **What it is:** An effectively unkillable chaser enemy (defined in data, currently using the Slime's visual as a stand-in) that instantly kills the hero on contact, bypassing revives and saves.
- **Trigger / spawn:** Belongs to a "Backrooms" mode that is **unimplemented / disabled** in the current build.
- **Status:** **Latent.** Data exists; the mode that would use it does not run.

---

## 6. Basic Mobs

There are **50 basic mob types**, ten per theme. They are organized into four movement/attack families:

- **Melee** — walk up and hit you in close range.
- **Rush** — fast chargers that close distance aggressively.
- **Flying** — airborne movers that come at you over obstacles.
- **Ranged** — keep distance and fire projectiles.

On top of family, some mobs carry a "special archetype" label — **Exploder, Stutterer, Burrower** — describing intended behavior (blow up on contact, jitter/teleport-step, dig and resurface). **Important:** those three behaviors are **not actually built yet**; those mobs currently just behave like their underlying family (see flag in Section 8). All 50 have finished art/meshes.

Below, each mob is grouped by theme with its family and a plain-language behavior note. The bracketed label notes special archetype intent where present.

### Dungeon (10)
| Mob | Family | Behavior (plain language) |
|---|---|---|
| Slime | Melee | Bouncy green blob; waddles up and bumps you. |
| Bone Walker | Melee | Stripped skeleton that shambles in to swing. |
| Rat Pack | Rush | A fused cluster of rats that swarms toward you fast. |
| Cave Bat | Flying | Bat that flies in to bite. |
| Hex Slinger | Ranged | Hooded caster that stays back and throws hexes. |
| Tomb Spider | Melee | Big hairy spider with a heavier "miniboss-feel" presence. |
| Stone Sentinel | Ranged | Stone statue that fires from range. |
| Mimic Lure | Rush [Exploder] | Treasure-chest creature that scuttles in; *meant* to detonate. |
| Bone Conjurer | Ranged | Robed skeleton caster, support/specialist feel. |
| Crypt Wraith | Melee [Stutterer] | Translucent shroud; *meant* to jitter/disrupt. |

### Forest (10)
| Mob | Family | Behavior |
|---|---|---|
| Mushroom Brute | Melee | Legless mushroom that lurches in to chomp. |
| Treant Sapling | Melee | Small walking tree that swipes with branches. |
| Thorn Imp | Ranged | Small spiked imp that pelts you from range. |
| Tusker Boar | Rush | Boar that charges with tusks. |
| Hive Wasp | Flying | Wasp that darts in to sting. |
| Treant Ancient | Melee | Large gnarled tree, heavier "miniboss-feel" presence. |
| Forest Wraith | Ranged | Drifting fae spirit that attacks from distance. |
| Spore Bomb | Rush [Exploder] | Walking mushroom-sphere; *meant* to burst. |
| Vine Strangler | Melee [Burrower] | Vine-mass; *meant* to dig and resurface. |
| Myconid Druid | Ranged | Robed mushroom caster, specialist feel. |

### Ocean (10)
| Mob | Family | Behavior |
|---|---|---|
| Crab Guard | Melee | Armored crab that pinches in close. |
| Drowned Sailor | Melee | Waterlogged corpse that shambles in. |
| Jelly Hover | Ranged | Floating jellyfish that attacks from range. |
| Reef Shark | Rush | Land-walking shark that rushes in. |
| Ghost Ray | Flying | Manta-ray glider that swoops. |
| Anglerfish Stalker | Melee [Stutterer] | Deep-sea angler with "miniboss-feel"; *meant* to jitter/disrupt. |
| Coral Mortar | Ranged | Stationary coral lobber. |
| Sea Mine | Rush [Exploder] | Spiked floating sphere; *meant* to detonate. |
| Brine Strafer | Ranged | Watery spirit that harries from range. |
| Drowned Priestess | Ranged | Robed drowned caster, specialist feel. |

### Martian (10)
| Mob | Family | Behavior |
|---|---|---|
| Drone Grunt | Ranged | Bipedal robot that shoots from range. |
| Crystal Crawler | Melee | Faceted crystal beast that closes in. |
| Plasma Spitter | Ranged | Squat alien that spits plasma. |
| Rocket Leaper | Rush | Rocket-boosted insectoid that lunges in. |
| Saucer Drone | Flying | Small UFO disc that hovers and attacks. |
| Plasma Sentinel | Ranged | Stationary obelisk, "miniboss-feel" presence. |
| Mind Slug | Melee [Stutterer] | Floating brain; *meant* to jitter/disrupt. |
| Crystal Bomber | Rush [Exploder] | Crystalline arachnid; *meant* to burst. |
| Sand Tunneler | Melee [Burrower] | Worm; *meant* to dig and resurface. |
| Cyber Lich | Ranged | Cybernetic alien skeleton caster, specialist feel. |

### Hell (10)
| Mob | Family | Behavior |
|---|---|---|
| Pit Imp | Rush | Small demon that rushes in. |
| Bone Knight | Melee | Armored undead warrior, "miniboss-feel" presence. |
| Fire Skull | Flying | Floating flaming skull that flies in. |
| Hellhound | Rush | Demon dog that charges. |
| Gargoyle | Flying | Winged stone demon that swoops. |
| Demon Sentinel | Melee [Stutterer] | Towering demon with "miniboss-feel"; *meant* to jitter/disrupt. |
| Brimstone Mortar | Ranged | Stationary demonic cannon. |
| Sin Eater | Rush [Exploder] | Bloated demon; *meant* to burst. |
| Plague Cultist | Ranged | Robed cultist caster, specialist feel. |
| Hell Wyrm | Melee [Burrower] | Demon worm; *meant* to dig and resurface. |

---

## 7. Theme / Stage Coverage

Every theme is consistently built out: **10 basic mobs, 4 stages, and a full boss set each.** Coverage is even; the gaps are about *depth* (placeholder minibosses, unbuilt special behaviors) rather than missing content.

### Dungeon (Easy, stages 1–4)
- **Bosses:** 4 (Slime King, Web Matriarch, Bone Jailer, Bael capstone).
- **Minibosses:** Placeholder Slime gate guardian (shared system, not themed).
- **Specials:** Goblin Thief can appear; Gambler possible via casino.
- **Basic mobs:** 10, full family spread (melee/rush/flying/ranged present).
- **Gaps:** Three mobs carry unbuilt special behaviors (Mimic Lure explode, Crypt Wraith stutter).

### Forest (Medium, stages 5–8)
- **Bosses:** 4 (Bramble Treant, Myconid Queen, Thorn Hive, Buer capstone).
- **Minibosses:** Same placeholder Slime system.
- **Specials:** Goblin Thief; Gambler via casino.
- **Basic mobs:** 10, full family spread.
- **Gaps:** Spore Bomb (explode) and Vine Strangler (burrow) behaviors unbuilt.

### Ocean (Hard, stages 9–12)
- **Bosses:** 4 (Reef Crab Colossus, Abyssal Jellyfish, Drowned Captain, Focalor capstone).
- **Minibosses:** Same placeholder Slime system.
- **Specials:** Goblin Thief; Gambler via casino.
- **Basic mobs:** 10, full family spread.
- **Gaps:** Anglerfish Stalker (stutter) and Sea Mine (explode) behaviors unbuilt.

### Martian (Very Hard, stages 13–16)
- **Bosses:** 4 (Red Sand Behemoth, Crystal Mantis, Plasma Saucer Prime, Stolas capstone).
- **Minibosses:** Same placeholder Slime system.
- **Specials:** Goblin Thief; Gambler via casino.
- **Basic mobs:** 10, full family spread.
- **Gaps:** Mind Slug (stutter), Crystal Bomber (explode), Sand Tunneler (burrow) behaviors unbuilt.

### Hell (Impossible, stages 17–20)
- **Bosses:** 7 — the Four Horsemen as a single four-boss fight (stage 17), then False Prophet, Antichrist, and the Great Dragon finale.
- **Minibosses:** Same placeholder Slime system.
- **Specials:** Goblin Thief; Gambler via casino.
- **Basic mobs:** 10. **Coverage note:** the always-present "core" Hell mobs are melee/rush/flying only — the ranged Hell mobs (Brimstone Mortar, Plague Cultist) are rarer/later additions, so a Hell floor can run without a ranged threat. This is flagged in the project notes as **intentional**, not a bug.
- **Gaps:** Sin Eater (explode) and Hell Wyrm (burrow) behaviors unbuilt; ranged scarcity (intentional).

---

## 8. Pablo Attention Flags

These are things worth a look and a decision. None of them are changes I made or am recommending you make right now — they're surfaced so nothing gets quietly deleted in the cleanup pass before you've seen it.

### Flag 1 — Three special mob behaviors are named but not built
- **Why it caught attention:** Twelve basic mobs are labeled Exploder, Stutterer, or Burrower, implying distinct behavior (blow up, jitter, dig). In reality those mobs just act like their base family — exploders don't explode, burrowers don't burrow. The art is done; the behavior isn't.
- **Decision you may want to make:** Keep these as plain reskins for now, or schedule a behavior pass to make the labels real, or drop the labels so they don't promise something the game doesn't deliver.

### Flag 2 — Minibosses are a single placeholder Slime
- **Why it caught attention:** Every tower gate guardian is the same enlarged Slime, in every theme, dropping no loot. It's functional but obviously placeholder, and it's the entire miniboss system.
- **Decision you may want to make:** Whether to author themed minibosses (e.g. a real Dungeon vs. Hell gatekeeper) before this ships, or accept the Slime as an intentional stand-in for now.

### Flag 3 — "Floors 2/3/4" framing vs. the code's actual rule
- **Why it caught attention:** The working description is "minibosses on floors 2, 3, and 4." The code actually places a gate guardian on **every gameplay floor** of a normal tower stage (bounded by the first and last gameplay floor), not a fixed 2/3/4. In practice these may line up, but the rule is "all gameplay floors," not three specific ones.
- **Decision you may want to make:** Confirm whether the general "all gameplay floors" rule is what you want, or whether minibosses should be limited to specific floors.

### Flag 4 — A "Vendor" boss is referenced in docs but has no enemy in the code
- **Why it caught attention:** A combat design doc lists a "Vendor" boss next to the Gambler boss as a bespoke special encounter. I could not find any Vendor-boss enemy in the current code — only the Gambler exists. This looks like a stale doc reference or a removed/never-built boss.
- **Decision you may want to make:** Decide whether the Vendor boss was intended (and is missing) or was dropped, and update the doc so cleanup doesn't chase a ghost.

### Flag 5 — The "Debuff" floating enemy is built but never spawns in a real run
- **Why it caught attention:** There's a complete floating debuff-projectile enemy, but the only place it spawns is the developer Lab. Normal waves never use it. It's the only enemy designed to apply a status effect to the hero — and players never see it.
- **Decision you may want to make:** Wire it into normal waves (e.g. as a rare special), keep it as a lab/test asset, or retire it. Worth deciding before cleanup, since "unused" code is a deletion target.

### Flag 6 — Status-effect-on-hit is wired off for all 50 basic mobs
- **Why it caught attention:** Status effects exist in the game, but every one of the 50 production mobs has its on-hit status set to "none," and the normal-mob hit path doesn't apply status effects yet. So no basic mob inflicts a status on you.
- **Decision you may want to make:** Decide whether basic mobs should ever apply statuses; if yes, that's a combat + data pass, if no, the status content may be boss/special-only by design.

### Flag 7 — Hell's core mobs have no ranged threat
- **Why it caught attention:** The always-present Hell mobs are melee/rush/flying; the ranged ones are rarer. A Hell floor can therefore play with no ranged pressure. Project notes mark this as **intentional**, so this is a confirm-only flag.
- **Decision you may want to make:** Confirm you're happy with Hell leaning non-ranged, or add a guaranteed ranged Hell mob.

### Flag 8 — Backrooms Stalker exists for a mode that doesn't run
- **Why it caught attention:** A unique unkillable chaser is defined in data (using the Slime as a visual stand-in), but the Backrooms mode that uses it is unimplemented/disabled.
- **Decision you may want to make:** Keep it parked for a future Backrooms pass, or clear it out. Flagged so it isn't deleted if Backrooms is still on the roadmap.

### Flag 9 — "Miniboss-feel" tag vs. real minibosses
- **Why it caught attention:** Six basic mobs carry a "miniboss feel" design tag (Tomb Spider, Treant Ancient, Anglerfish Stalker, Plasma Sentinel, Bone Knight, Demon Sentinel). They are *not* minibosses — just chunkier basic mobs. Easy to misread the tag during cleanup.
- **Decision you may want to make:** None required; noted so the tag isn't confused with the actual placed-Slime miniboss system.

---

## 9. Technical Traceability

Data row IDs, class names, and file references for everything above. Main prose intentionally avoids these.

### Data sources
- Bosses: `Content/Data/Bosses.csv` — 23 rows.
- Boss encounters: `Content/Data/BossEncounters.csv` (20 encounters) and `Content/Data/BossEncounterMembers.csv` (23 member rows; Stage 17 holds the 4 Horsemen).
- Stage rosters: `Content/Data/Stages.csv` — 20 stages, columns `EnemyA..EnemyJ` listing that stage's mob pool.
- Basic mobs: `Content/Data/Enemies.csv` — 50 rows.
- Unique enemies: `Content/Data/UniqueEnemies.csv` — 1 row (`BackroomsChaser`, visual `Slime`, `bUnkillable=true`, `bBypassLethalSaves=true`).

### Boss IDs by theme (from `Bosses.csv`)
- Dungeon: `Dungeon_SewerSlimeKing`, `Dungeon_WebMatriarch`, `Dungeon_BoneJailer`, `Dungeon_BaelFallenChad` (FinaleChad).
- Forest: `Forest_BrambleTreant`, `Forest_MyconidQueen`, `Forest_ThornHive`, `Forest_BuerVerdantChad` (FinaleChad).
- Ocean: `Ocean_ReefCrabColossus`, `Ocean_AbyssalJellyfish`, `Ocean_DrownedCaptain`, `Ocean_FocalorDrownedChad` (FinaleChad).
- Martian: `Martian_RedSandBehemoth`, `Martian_CrystalMantis`, `Martian_PlasmaSaucerPrime`, `Martian_StolasAstralChad` (FinaleChad).
- Hell: `Hell_Horseman_Conquest/War/Famine/Death` (Encounter_Stage_17, `EncounterType=MultiBoss`), `Hell_FalseProphet`, `Hell_Antichrist`, `Hell_GreatDragon` (ApocalypseFinale).
- Boss part layouts come from the `BossPartProfile` column (Juggernaut / Sharpshooter / HumanoidBalanced / Duelist).

### Gambler boss (code-only, not in CSV)
- Class: `AT66GamblerBoss` (`Source/T66/Gameplay/T66GamblerBoss.cpp/.h`), `BossID="GamblerBoss"`, `MaxHP=1000`, bespoke 6-part hit zones, `AttackProfile=Gambler`.
- Spawn: `AT66PlayerController::TriggerCasinoBossIfAngry()` in `Source/T66/Gameplay/T66PlayerController_Overlays.cpp:5979`; spawns at `:6048` when `GetCasinoAnger01() >= 1.0`, no boss already active, gameplay level, at the vendor/casino interactable location.
- Triggered from casino UI: `UT66CasinoGamblerTabWidget::TriggerGamblerBossIfAngry()` and `UT66CasinoVendorTabWidget::TriggerCasinoBossIfAngry()`.
- Doc reference to a `AT66VendorBoss` appears in `Gameplay/Combat/MASTER_COMBAT.md` (lines ~164, ~378) but **no such class exists** in `Source/T66` — see Flag 4.

### Minibosses (placed tower gate guardians)
- Spawn: `T66SpawnTowerGateGuardian()` and `AT66GameMode::EnsurePlacedTowerMinibossForFloor()` in `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` (~146, ~574).
- Identity/scaling constants (`T66GameMode_Tower.cpp:15-18`): `T66PlacedTowerMinibossMobID="Slime"`, `HPScalar=3.0`, `DamageScalar=2.0`, `Scale=1.75`; guardian sets `bDropsLoot=false`, tag `T66_Tower_DescentGuardian`.
- Floor rule: `AT66GameMode::IsPlacedTowerMinibossFloor()` (`:554`) = tower layout, not boss-rush finale, floor within `[FirstGameplayFloorNumber, LastGameplayFloorNumber]` — i.e. **all gameplay floors**, not hard-coded 2/3/4 (Flag 3).
- Random per-wave promotion disabled: `AT66EnemyDirector::SpawnRuntimeTrickleWave()` sets `MiniBossIndex = INDEX_NONE` (`T66EnemyDirector.cpp:1162-1164`). Dormant tuning still present in `T66EnemyDirector.h` (`MiniBossChancePerWave`, `MiniBossScale=1.75`, `MiniBossHPScalar=3.0`, `MiniBossDamageScalar=2.0`). See `Source/T66/Gameplay/pending_issues_Gameplay.md` "Placed miniboss update."

### Specials
- Goblin Thief: `AT66GoblinThiefEnemy` (`Source/T66/Gameplay/T66GoblinThiefEnemy.cpp/.h`); steals gold on touch (`GoldStolenPerHit=50`, rarity-scaled). Spawned per wave via Luck-biased chance in `AT66EnemyDirector::SpawnRuntimeTrickleWave()` (`T66EnemyDirector.cpp:1107-1160`, using `GoblinWaveChanceBase` / `GoblinCountPerWave` from `T66RngTuningConfig`).
- Unique Debuff enemy: `AT66UniqueDebuffEnemy` (`Source/T66/Gameplay/T66UniqueDebuffEnemy.h`), fires `AT66UniqueDebuffProjectile` (applies `ET66HeroStatusEffectType`, default Burn). Only spawn path found: `T66GameMode_Lab.cpp:163-165` (id `UniqueEnemy`). No normal-wave spawn — latent (Flag 5).
- Backrooms Stalker: `AT66BackroomsChaser` (`Source/T66/Gameplay/T66BackroomsChaser.cpp/.h`), reads `FUniqueEnemyData` row `BackroomsChaser`. Tied to `T66GameMode_Backrooms`, which is unimplemented/disabled (see `pending_issues_Gameplay.md` "Backrooms GameMode" notes).

### Basic mob families / archetypes (from `Enemies.csv`)
- `FamilyID` ∈ {Melee, Rush, Flying, Ranged} drives spawn class resolution (`T66ResolveEnemyClassFromFamilyID` in `T66EnemyDirector.cpp:206`): `AT66MeleeEnemy / AT66RushEnemy / AT66FlyingEnemy / AT66RangedEnemy`.
- `Archetype` adds {Exploder, Stutterer, Burrower} on top of the family for 12 mobs, but no `AT66ExploderEnemy/AT66StuttererEnemy/AT66BurrowerEnemy` classes exist — they fall back to family behavior. See `Source/T66/Gameplay/Enemies/pending_issues_Enemies.md` "Missing Production Archetype Classes" and `Source/T66/Data/pending_issues_Data.md` "Enemy Family, Role, And Archetype Redundancy."
  - Exploder (5): MimicLure, SporeBomb, SeaMine, CrystalBomber, SinEater.
  - Stutterer (4): CryptWraith, AnglerfishStalker, MindSlug, DemonSentinel.
  - Burrower (3): VineStrangler, SandTunneler, HellWyrm.
- `Feeling=MiniBossFeel` tag (6, Rare rarity): TombSpider, TreantAncient, AnglerfishStalker, PlasmaSentinel, BoneKnight, DemonSentinel — cosmetic/design feel only (Flag 9).
- `StatusEffectOnHit=None` for all 50 rows; see `Content/Data/pending_issues_Data.md` "Status Effects Not Assigned To Production Mobs" (Flag 6).
- All 50 rows are `ModelStatus=MeshReady`.
- Hell ranged-coverage note: `Source/T66/Gameplay/pending_issues_Gameplay.md` "Hell Core Has No Ranged Mob" [Minor, intentional] (Flag 7).
- Basic mobs route to the lightweight `AT66MobBase` path; minibosses/specials/bosses stay "rich." See `T66EnemyDirector.cpp:713 ShouldRouteSpawnToLightweightMob` and `pending_issues_Gameplay.md` "B.11/B.12 lightweight-only closure."

### Spawn vs. data cross-check (defined vs. reachable)
- **Defined in data + reachable:** all 23 bosses (Stages.csv → BossEncounters.csv → BossEncounterMembers.csv all resolve), all 50 basic mobs (referenced by Stages.csv `EnemyA..J` and resolved in `T66ResolveStageMobIDs`).
- **Code-defined + reachable:** Goblin Thief (wave spawn), Gambler boss (casino trigger), placed Slime miniboss (descent gates).
- **Defined but latent:** Unique Debuff enemy (Lab-only), Backrooms Stalker (disabled mode).
- **Doc-referenced but missing:** Vendor boss (no class found).

---

*Scope note: This review covers the enemy roster only (bosses, hidden/special bosses, minibosses, specials, basic mobs). NPCs, items, weapons, idols, heroes, companions, projectile systems, and any Mini/minigame content are out of scope. No code, data, content, or config was changed in producing this report.*
