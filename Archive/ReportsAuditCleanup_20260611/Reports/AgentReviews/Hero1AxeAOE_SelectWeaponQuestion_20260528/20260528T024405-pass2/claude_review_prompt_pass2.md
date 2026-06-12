You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these three lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Only use Verdict: APPROVE when the reviewed plan/output is safe for Codex to present as greenlit. If implementation still requires user go-ahead under AGENTS.md, APPROVE means safe to present at that go-ahead gate, not permission to skip the gate.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\Hero1AxeAOE_SelectWeaponQuestion_20260528\answer_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Answer Review Packet - Hero 1 AOE Weapon Altar VFX

## Working Goal

Confirm whether selecting the Hero 1 AOE weapon at the weapon altar currently applies the production AOE VFX, and state any caveats.

## Proposed User Answer

Yes, for the current working tree/editor asset state: if the weapon altar offer is `Ashfall Cleaver` / `Hero_1_black_aoe` and you choose it, the weapon manager equips that weapon ID. When the hero then fires an AOE auto-attack at an enemy, `UT66CombatComponent` uses the equipped weapon data, resolves the runtime `DT_CombatVFXBindings` row `Hero1Axe_AOE_Base`, and spawns `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`.

Important caveats:

- The VFX does not play at the moment of selecting the weapon card; it plays when the attack fires.
- The runtime uses the DataTable asset `Content/Data/DT_CombatVFXBindings.uasset`; `Content/Data/CombatVFXBindings.csv` is the setup source used to generate/reload it.
- This is true for the current working tree / current editor asset state. The binding CSV/uasset, Niagara asset, and setup/validator scripts are still untracked, so this is not guaranteed in a clean checkout until those files are staged/committed.
- I did not do a fresh manual click-through altar-pick capture in this answer pass. I did rerun the production binding validator with the UE 5.7 absolute path, and the prior EdgeFinal binding proof is auditable at `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/`.

## Evidence From Live Repo Inspection

- `Content/Data/Weapons.csv` contains `Hero_1_black_aoe` with branch `AOE`, display name `Ashfall Cleaver`, and `AoeInnerRadiusRatio=0.54`.
- `Content/Data/CombatVFXBindings.csv` contains `Hero1Axe_AOE_Base` with `SourceType=WeaponBase`, `SourceID=Hero_1_black_aoe`, `AttackCategory=AOE`, and Niagara system `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash`.
- The bound Niagara `.uasset` exists on disk at `Content/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.uasset`; `git status` currently reports it untracked.
- `Scripts/ValidateCombatVFXProductionBindings.py` was rerun via `"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"` and succeeded with `Success - 0 error(s), 3 warning(s)`. That validator confirms the CSV row, the required production assets including the Niagara system, and `BP_T66GameInstance` pointing at `/Game/Data/DT_CombatVFXBindings.DT_CombatVFXBindings`.
- `UT66WeaponManagerSubsystem::BuildWeaponOffers` builds `Pierce`, `Bounce`, `AOE`, and `DOT` offers for the selected hero/rarity, using IDs like `Hero_1_black_aoe`.
- `UT66WeaponManagerSubsystem::MakeWeaponID` formats Hero 1 + Black + AOE as `Hero_1_black_aoe`; the weapon altar has black as its default rarity and the world-interactable spawn path sets `WeaponOfferRarity` from difficulty tuning before building offers.
- `UT66WeaponAltarOverlayWidget::OnChooseSlot` calls `WeaponManager->SelectWeapon(WeaponID)`.
- `UT66WeaponManagerSubsystem::SelectWeapon` sets `EquippedWeaponID = WeaponID`.
- `UT66CombatComponent::RefreshCachedStats` gets `CachedWeaponData` from `WeaponManager->GetEquippedWeaponData`.
- `UT66CombatComponent::PerformSlash` calls `TrySpawnBoundWeaponBaseSlashVFX(...)`.
- `TrySpawnBoundWeaponBaseSlashVFX` resolves a `WeaponBase` binding by `CachedWeaponData.WeaponID` and `AttackCategory`, spawns the bound Niagara system, and logs `CombatVFXProductionSpawned`.
- Prior proof path: `Saved/VideoCaptures/Hero1AxeAOE_HitboxCleanup_20260528_EdgeFinal_20260528_014810/T66.log` contains the production spawn proof and 8 target hitbox PASS rows from the binding proof harness.

## Review Request

Review the proposed answer as read-only. First non-empty line must be exactly one of:

`Verdict: APPROVE`

`Verdict: REVISE`

`Verdict: BLOCK`

</review_packet>
