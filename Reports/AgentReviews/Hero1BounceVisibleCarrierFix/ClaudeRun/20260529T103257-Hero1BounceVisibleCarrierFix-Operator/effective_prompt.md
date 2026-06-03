You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\Hero1BounceVisibleCarrierFix\codex_operator_approval.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Verification freshness: if the user explicitly asks for current compile, run, capture, test, or editor verification, you must attempt that exact current verification now unless it is physically impossible. Recent or prior evidence does not satisfy an explicit current-verification request; if you cannot run it, say so explicitly instead of substituting older evidence.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
You are Claude acting as Operator for `C:\UE\T66`. Codex is Validator/Finisher.

Read and follow:
- `AGENTS.md`
- `.t66/operator-state.json`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/Combat/CombatVFXAuthoringProcedure.md`
- `Gameplay/Combat/Hero1AxeBounceMechanismPacket.md`
- `Gameplay/Combat/CombatVFXDefinitionOfDone.md`
- `Gameplay/Combat/CombatVFXVisualDamageAlignmentContract.md`
- `Gameplay/Combat/CombatVFXImpactContextContract.md`
- Any local `pending_issues_*.md` in touched source/data folders.

Task contract:

```text
Working task: fix the Bounce weapon visual proof so the authored horizontal slash carrier visibly travels from hero to primary, then primary to second, in the standard original-camera proof view.
Operator: Claude
Validator: Codex
Scope: Bounce visual-carrier presentation and proof only; preserve combat damage, target selection, per-link impact contexts, and current standard proof-camera target isolation.
Stop condition: fresh compile and Unreal-owned video/log evidence prove a visible moving Bounce slash carrier on both links, or return a decision gate if the current Niagara carrier cannot support that without substituting the primary method class.
```

User's latest correction:

> The view is now correct, but the projectile is not visible at all. There seems to be a projectile from outside coming and the timing is the same as the mobs take damage, but this should be a projectile that looks just like the first iteration of the bounce one. The only issue the original had was that it appeared on the enemies rather than originating at the hero and moving toward enemies. We may have overcomplicated the solution.

Known current evidence:
- The prior fixed proof video path was:
  `C:\UE\T66\Saved\VideoCaptures\Hero1BounceOriginalCameraFixed_20260529\hero1axebouncevfxbinding.mp4`
- That run fixed the camera and target isolation, but visual review found the carrier itself is not readable. Damage numbers and link logs occur, while the authored slash does not visibly travel from the hero to the enemies.
- The current packet already declares the right behavior: a moving two-link projectile/slash carrier, one link in flight at a time, with the authored Bounce Niagara slash as the primary silhouette.

Important constraints:
- Do not use native goal tools.
- Do not use Anthropic API billing. You are running through the local Claude Code subscription helper.
- Do not inspect/change Mini/minigame systems.
- Do not make Git commits, pushes, tags, resets, cleans, or broad Git/LFS scans.
- Do not revert user or peer changes.
- Preserve the authored Bounce Niagara slash (`NS_Hero1AxeBounce_MeshSlash`) as the primary silhouette. Actor-side transforms can place/orient/scale it, but the accepted carrier cannot become procedural C++ debug geometry, a cube, or an actor-arranged lookalike.
- If replacing the Niagara primary carrier or doing a broad asset-regeneration/binary asset pass is the only viable route, stop with `NEEDS_HUMAN_DECISION` and explain the choices.

Implementation target:
- The visual should look like the first Bounce iteration's readable horizontal slash, but it must move along the segment:
  1. starts from the hero attack origin and travels to the primary enemy;
  2. after arrival, starts from the primary enemy and travels to the second enemy;
  3. only one visible projectile/slash link is in flight at a time.
- Fix the current issue where the projectile is invisible or appears to come from outside the camera/side instead of being readable from the hero-to-enemy path.
- Prefer the smallest repo-appropriate correction: orientation, local/world-space behavior, component transform, carrier scale, lifetime timing, spawn start, proof timing, or use of the existing first-iteration visual carrier inside the moving Niagara/projectile path.

Likely anchors:
- `Source/T66/Gameplay/T66CombatComponent.cpp`
  - `PerformBounce`
  - `StageBounceProjectileChain`
  - `SpawnBounceChainLinkSequential`
  - `SpawnBounceLinkProjectile`
- `Source/T66/Gameplay/T66HeroProjectile.cpp`
  - constructor
  - `Tick`
  - `SetPrimaryCarrierNiagara`
  - `SetTargetLocation`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`
  - Hero 1 Bounce proof/capture harness and target isolation only if needed
- `Source/T66/Gameplay/T66Hero1AxeBounceVFXCommandlet.cpp`
  - inspect only if needed to understand the Niagara/mesh local orientation or generated asset setup.

Required proof:
- Focused compile:
  `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
- Use an Unreal-owned gameplay video capture, not a desktop screenshot. Use the same standard camera family as the prior accepted Bounce original-view proof. Enemies must be visible. Do not put the yellow proof block in front of the hero.
- Produce a contact sheet or selected frames that prove the slash is visible in motion on both segments.
- Provide log proof for:
  - `CombatVFXBounceLinkProjectile LinkIndex=0 LinkCount=2`
  - later `CombatVFXBounceLinkProjectile LinkIndex=1 LinkCount=2`
  - no third link
  - Bounce damage on the intended targets

Deliver an Operator packet under:
`Reports/AgentReviews/Hero1BounceVisibleCarrierFix/claude_operator_report.md`

Packet fields required:
- Verdict line: `Operator Result: COMPLETE`, `Operator Result: NEEDS_HUMAN_DECISION`, or `Operator Result: BLOCKED`
- User goal and constraints
- Files inspected
- Files changed
- Root cause
- Fix summary
- Process fidelity / PPF close
- Mechanism close for the required Bounce mechanisms
- Verification commands and results
- Video/contact-sheet/frame paths
- Runtime log anchors
- Remaining caveats
- Token usage from the helper manifest if available

