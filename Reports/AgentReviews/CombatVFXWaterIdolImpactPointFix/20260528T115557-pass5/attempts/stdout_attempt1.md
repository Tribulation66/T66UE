Verdict: APPROVE

Blockers
- None.

Major Issues
- Root-transform ambiguity is a plausible cause, but it is one of several. If the placeholder mesh has a non-centered pivot, or if the later "scale it" step writes a non-identity RelativeTransform on the mesh component, the actor location can be correct yet the visible mesh can still read off-position. The packet does not rule these out before committing to SetActorLocation as the fix. If the upcoming capture still shows the marker on the hero, Codex should immediately pivot to (a) inspecting the static mesh asset pivot, and (b) auditing the scale/relative-transform calls on the new root component, rather than re-trying transform tweaks.

Minor Issues
- `+ FVector(0.f, 0.f, 76.f)` is a magic number. It should either be derived (e.g., from the mesh bounds half-height of the placeholder primitive) or named as a local const with a one-line justification so the verification log (`VisualLocation=Z=140`) is self-explaining.
- Consider also asserting the component-side transform: after `SetRootComponent` and `RegisterComponent`, an explicit `MeshComp->SetRelativeLocation(FVector::ZeroVector)` (or equivalent) would close the secondary failure mode of a non-zero relative offset on the new root, with no extra risk.
- The verification step 5 ("inspect contact sheet... no longer on top of the hero") is subjective. Define the pass criterion in coordinate terms — e.g., the marker's screen-space centroid must lie forward of the hero footprint at the logged X=696.89 — so the capture either passes or fails objectively.

Clarifying Questions
- None blocking; all questions above can be resolved by Codex during implementation/verification.

Required Verification
- Confirm focused compile succeeds for `T66Editor Win64 Development` with the single edit to `T66CombatVFX.cpp`.
- Unreal-owned `hero1axeaoewateridolimpact` capture must show all four log lines listed in the packet, including the new `VisualLocation=V(X=696.89, Z=140.00)` and unchanged `ImpactPoint=V(X=696.89, Z=64.00)`, `VisualScale=0.850`, `SourceID=Idol_Water`, `Binding=Hero1Axe_AOE_Base`.
- Contact-sheet/video evidence must show the blue marker at the forward impact point and not overlapping the hero silhouette, judged against an explicit pass criterion (see Minor Issues).
- If `VisualLocation` matches the expected value but the marker still visually reads on the hero, do not iterate on transform — escalate to mesh pivot / relative transform inspection.

Rationale
- Scope is tight: one file, one function, additive `SetActorLocation` + one extra log field; no gameplay-behavior change (damage radius, query, source untouched).
- The hypothesis is consistent with UE behavior: when a runtime-created `USceneComponent` is promoted to root after `SpawnActor`, the actor's effective world transform follows the new root, and an explicit `SetActorLocation` after `SetRootComponent` + `RegisterComponent` is the defensive correct sequence.
- PPF and Artifact Parity gates are satisfied: same placeholder artifact, same method class, same impact-context source of truth, no new asset introduced.
- Verification is Unreal-owned with concrete log fields and a contact sheet, matching project discipline.
- Residual risk (camera projection or pivot/relative-transform cause) is acknowledged and is safe to discover via the same capture, so it does not block approval — only shapes the follow-up branch.

