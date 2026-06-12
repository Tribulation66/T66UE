Cleanup pass complete. Summary:

**Code (behavior-preserving):**
- Centralized proof-idol metadata in `T66CombatShared` ΓÇö `GetImpactPresentationProofIdols()` (Water/Light/Electric/Poison) and `GetSupportedProofIdols()` (+ Earth) as a base/derived pair so they can't drift. Wired both call sites (`T66CombatComponent.cpp` `UsesImpactPresentationForIdol`, `T66PlayerController_Overlays.cpp` axe proof harness) to it.
- De-Watered internal diagnostic locals/lambda params and one suppression `Reason=` token. Kept every proof-parsed log string byte-identical ΓÇö including the preserved `CombatIdolImpactDiagnostic SourceID=Idol_Water Water*ΓÇª` compatibility diagnostic. No proof-runner changes needed.

**Docs:** Corrected `VFX_PROCESS_INDEX.md` and `CombatVFXInfrastructureInventory.md` to match live `CombatVFXBindings.csv` ΓÇö AOE/Pierce/Bounce all have active Hero 1 rows, DOT has none, idol category proofs are placeholder/proof paths.

**Verification:**
- Compile `T66Editor Win64 Development` ΓåÆ Succeeded (exit 0; only a pre-existing unrelated Niagara deprecation warning).
- `RunHero1AxeIdolCategoryNativeImpactProof.ps1` ΓåÆ all 5 cases PASS (Light/Electric/Poison/Water pass, Earth confirmed neutral; Water regression's `WaterIdolContextParity=PASS` fired, proving log contracts intact).

**Deliverable:** `Reports/AgentReviews/WeaponIdolVFXCleanup/operator_packet.md` written, first line `Operator Packet: COMPLETE`, with changed files, before/after naming map, docs correction, proof artifact paths, and caveats.

One note worth flagging: the impact-context infrastructure this touched is uncommitted working-tree work (absent from `HEAD`), so the cleanup layered on in-session state rather than a committed baseline ΓÇö documented in the packet. No commit was made (would need Codex approval).
