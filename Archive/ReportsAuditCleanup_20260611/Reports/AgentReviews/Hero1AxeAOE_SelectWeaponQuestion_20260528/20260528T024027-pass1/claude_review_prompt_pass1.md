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

Yes, in the current working tree, if the weapon altar offer is `Ashfall Cleaver` / `Hero_1_black_aoe` and you choose it, the weapon manager equips that weapon ID. When the hero then fires an AOE auto-attack at an enemy, `UT66CombatComponent` uses the equipped weapon data, resolves the `Hero1Axe_AOE_Base` binding, and spawns `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash`.

Important caveats:

- The VFX does not play at the moment of selecting the weapon card; it plays when the attack fires.
- This is true for the current working tree / current editor asset state. The binding CSV/uasset and setup/validator scripts are still untracked, so this is not guaranteed in a clean checkout until those files are staged/committed.
- I have not just done a fresh manual altar-pick capture in this exact answer pass, but the live code/data path and the previous binding proof support the answer.

## Evidence From Live Repo Inspection

- `Content/Data/Weapons.csv` contains `Hero_1_black_aoe` with branch `AOE`, display name `Ashfall Cleaver`, and `AoeInnerRadiusRatio=0.54`.
- `Content/Data/CombatVFXBindings.csv` contains `Hero1Axe_AOE_Base` with `SourceType=WeaponBase`, `SourceID=Hero_1_black_aoe`, `AttackCategory=AOE`, and Niagara system `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash`.
- `UT66WeaponManagerSubsystem::BuildWeaponOffers` builds `Pierce`, `Bounce`, `AOE`, and `DOT` offers for the selected hero/rarity, using IDs like `Hero_1_black_aoe`.
- `UT66WeaponAltarOverlayWidget::OnChooseSlot` calls `WeaponManager->SelectWeapon(WeaponID)`.
- `UT66WeaponManagerSubsystem::SelectWeapon` sets `EquippedWeaponID = WeaponID`.
- `UT66CombatComponent::RefreshCachedStats` gets `CachedWeaponData` from `WeaponManager->GetEquippedWeaponData`.
- `UT66CombatComponent::PerformSlash` calls `TrySpawnBoundWeaponBaseSlashVFX(...)`.
- `TrySpawnBoundWeaponBaseSlashVFX` resolves a `WeaponBase` binding by `CachedWeaponData.WeaponID` and `AttackCategory`, spawns the bound Niagara system, and logs `CombatVFXProductionSpawned`.

## Review Request

Review the proposed answer as read-only. First non-empty line must be exactly one of:

`Verdict: APPROVE`

`Verdict: REVISE`

`Verdict: BLOCK`

</review_packet>
