# Answer Review Packet - Hero 1 AOE Weapon Altar VFX

## Working Goal

Confirm whether selecting the Hero 1 AOE weapon at the weapon altar currently applies the production AOE VFX, and state any caveats.

## Proposed User Answer

Not end-to-end playthrough-confirmed in this answer pass. What is confirmed is narrower: in this exact uncommitted working tree, the black-rarity Hero 1 AOE weapon route is wired and validator-clean so that selecting `Ashfall Cleaver` / `Hero_1_black_aoe` and then firing an AOE attack will attempt to spawn the production Niagara binding.

Three important limits:

- This is for the black-rarity AOE offer only. If the altar's resolved rarity is not black, it will build a different weapon ID and this `Hero_1_black_aoe` binding will not fire unless that other rarity gets its own binding row.
- This is true only in the current working tree/editor asset state. `DT_CombatVFXBindings.uasset`, `CombatVFXBindings.csv`, `NS_Hero1AxeAOE_MeshSlash.uasset`, and the setup/validator scripts are untracked right now. A clean checkout of current `HEAD` will not reproduce this VFX path until those files are staged/committed or restored.
- The previous visual proof was the EdgeFinal binding-proof harness, not a manual altar-pick capture. The validator confirms the binding row, required asset presence, and GameInstance DataTable pointer; it does not by itself observe the altar-click, rarity resolution, equip path, and Niagara spawn in one gameplay run.

So the practical answer is: if you are in this same working tree, the altar is showing `Ashfall Cleaver`, and you select it, the next AOE attack is wired to use the production AOE VFX. I would still run one manual altar-pick capture before treating that as fully playthrough-confirmed.

The exact code route is: choosing `Hero_1_black_aoe` makes the weapon manager equip that ID. When the hero then fires an AOE auto-attack at an enemy, `UT66CombatComponent` uses the equipped weapon data, resolves the runtime `DT_CombatVFXBindings` row `Hero1Axe_AOE_Base`, and spawns the bound Niagara system `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`.

Important caveats:

- The answer is code-path and validator verified, not fresh manual-playthrough verified.
- This is specifically for the black-rarity AOE offer. I did not inspect the active difficulty-tuning row in this answer pass, so I am not claiming that every current gameplay run resolves the altar to black.
- The VFX does not play at the moment of selecting the weapon card; it plays when the attack fires.
- The runtime uses the DataTable asset `Content/Data/DT_CombatVFXBindings.uasset`; `Content/Data/CombatVFXBindings.csv` is the setup source used to generate/reload it. A CSV edit without rerunning the setup script, a missing untracked uasset, or a missing/uncompiled Niagara asset would invalidate this working-tree answer.
- I did not do a fresh manual click-through altar-pick capture in this answer pass. I did rerun the production binding validator with the UE 5.7 absolute path, and the prior EdgeFinal binding proof is auditable at `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/`; that proof was produced through the binding proof harness, not through manual altar interaction.

## Evidence From Live Repo Inspection

- `Content/Data/Weapons.csv` contains `Hero_1_black_aoe` with branch `AOE`, display name `Ashfall Cleaver`, and `AoeInnerRadiusRatio=0.54`.
- `Content/Data/CombatVFXBindings.csv` contains `Hero1Axe_AOE_Base` with `SourceType=WeaponBase`, `SourceID=Hero_1_black_aoe`, `AttackCategory=AOE`, and Niagara system `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash`.
- The bound Niagara `.uasset` exists on disk at `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`; `git status` currently reports it untracked.
- `Scripts/ValidateCombatVFXProductionBindings.py` was rerun via `"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"` and succeeded with `Success - 0 error(s), 3 warning(s)`. That validator confirms the CSV row, the required production assets including the Niagara system, and `BP_T66GameInstance` pointing at `/Game/Data/DT_CombatVFXBindings.DT_CombatVFXBindings`.
- `UT66WeaponManagerSubsystem::BuildWeaponOffers` builds `Pierce`, `Bounce`, `AOE`, and `DOT` offers for the selected hero/rarity.
- `UT66WeaponManagerSubsystem::MakeWeaponID` uses `FString::Printf(TEXT("%s_%s_%s"), *HeroID.ToString(), *WeaponRarityToString(Rarity), *AttackBranchToString(Branch))`; `WeaponRarityToString(Black)` returns `black`, and `AttackBranchToString(AOE)` returns `aoe`, so Hero 1 + Black + AOE formats to `Hero_1_black_aoe`. The weapon altar has black as its default rarity in `T66WeaponAltar.h`, and the world-interactable spawn path sets `WeaponOfferRarity` from difficulty tuning before building offers. If difficulty tuning resolves the altar to black, the AOE slot is `Hero_1_black_aoe`.
- `UT66WeaponAltarOverlayWidget::OnChooseSlot` calls `WeaponManager->SelectWeapon(WeaponID)`.
- `UT66WeaponManagerSubsystem::SelectWeapon` sets `EquippedWeaponID = WeaponID`.
- `UT66CombatComponent::RefreshCachedStats` gets `CachedWeaponData` from `WeaponManager->GetEquippedWeaponData`.
- `UT66CombatComponent::PerformSlash` calls `TrySpawnBoundWeaponBaseSlashVFX(...)`.
- `TrySpawnBoundWeaponBaseSlashVFX` resolves a `WeaponBase` binding by `CachedWeaponData.WeaponID` and `AttackCategory`, spawns the bound Niagara system, and logs `CombatVFXProductionSpawned`.
- Prior proof path: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/T66.log` contains the production spawn proof and 8 target hitbox PASS rows from the binding proof harness, not from manual altar interaction.
- The validator was rerun after the previous setup/reload pass and succeeded after checking the CSV row, required production assets, and `BP_T66GameInstance`'s bound DataTable. Validator warnings were unrelated to this row/path: repeated `r.Upscale.Quality` scalability priority warnings plus the existing ToonStyle material include warning. The commandlet ended with `Success - 0 error(s), 3 warning(s)`.

## Review Request

Review the proposed answer as read-only. First non-empty line must be exactly one of:

`Verdict: APPROVE`

`Verdict: REVISE`

`Verdict: BLOCK`
